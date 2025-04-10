#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <recGbl.h>
#include <dbAccess.h>
#include <devSup.h>
#include <waveformRecord.h>
#include <epicsVersion.h>
#ifndef BASE_VERSION
/* 3.14 */
#include <epicsExport.h>
#endif

long devFileReadWaveformRead(waveformRecord *record);

struct {
    long      number;
    DEVSUPFUN report;
    DEVSUPFUN init;
    DEVSUPFUN init_record;
    DEVSUPFUN get_ioint_info;
    DEVSUPFUN write;
} devFileReadWaveform = {
    5,
    NULL,
    NULL,
    devFileReadWaveformRead,
    NULL,
    devFileReadWaveformRead
};

#ifndef BASE_VERSION
/* 3.14 */
epicsExportAddress(dset, devFileReadWaveform);
#endif

int devFileReadWaveformReadValue(FILE* file, int rcol, char* format, void* variable)
{
    int		colcnt = 0;
    int		i = 0;
    int		j = 0;
    int		k = 0;
    int		l = 0;
    int		found = 0;
    char	s0[1024];
    char	s1[128];
    double	d;
    short	s = 0;
    char	*p[2] = {NULL, NULL};

    do
	{
	memset(s0, 0, sizeof(s0));
	p[0] = fgets(s0, sizeof(s0), file);
	if( !p[0] ) return -1;
	l = strlen( s0 );
	i = k = 0;
	if( s0[0] != '#' && l )
		{
		colcnt = 0;
		p[1] = s0;
		do
			{
			while( isspace( *p[1] ) && i < l ) { p[1]++; i++; }
			memset(s1, 0, sizeof(s1));
			j = sscanf(p[1], " %s", s1);
			/*printf("rcol = %d, colcnt = %d, s = %s\n", rcol, colcnt, s1);*/
			if( !strcmp(format, "%hi") ) {
				k = sscanf(s1, " %d", &s);
				if( k == 1) {
					*(short *)variable = s;
					found = 1;
					/*printf("short FOUND = %d, s = %d\n", found, s);*/
					}
				}
			else
			if( (j > 0) && (colcnt == rcol) )
				{
				k = sscanf(s1, " %lg", &d);
				if( k == 1)
					{
					*(double *)variable = d;
					found = 1;
					/*printf("FOUND = %d, d = %lf\n", found, d);*/
					}
				}
			if( j > 0 ) { p[1] += strlen(s1); i++; }
			colcnt++;
			}
		while( j > 0 && !found && i < l );
		}
	}
    while( p[0] && !found );

    return found ? 0 : -1;
}

long devFileReadWaveformRead(waveformRecord *record)
{
    int		i, n, rcol;	// rcol = read column No
    FILE	*file;

    char	ss0[128];
    char	ss1[128];

    /* Check for an  integer following the filename on instio.string -?		*/
    /* Interpret it as the column number of requesed data			*/

    n = rcol = 0;

    memset(ss0, 0, sizeof(ss0));
    memset(ss1, 0, sizeof(ss1));

    n = sscanf(record->inp.value.instio.string, " %s %s", ss0, ss1 );

    /* printf("Sscanf of instio.string n=%d, substr %s %s\n", n, ss0, ss1 );	*/

    if( n > 1 )
        {
        i = 0;
        n = sscanf(ss1, " %d", &i);
        if( n == 1 )
		{
		/* printf("Requested reading of column: %d\n", i); */
		}
	rcol = i;
	}

    file = fopen(ss0, "r");

    if (!file) 
    {
        printf ("%s: Can't open file %s for reading\n",
            record->name, record->inp.value.instio.string);
        record->nord = 0;
        record->udf = 1;
        return -1;
    }
    else
    {
       /*printf("%s: File %s opened for reading, reading column No %d\n", record->name, ss0, rcol);*/
    }
    
    switch (record->ftvl)
    {
        case DBF_DOUBLE:
            for (i=0; i<record->nelm; i++)
                if (devFileReadWaveformReadValue(file, rcol, "%lf", &((double*)(record->bptr))[i]) != 0) break;
            break;
        case DBF_FLOAT:
            for (i=0; i<record->nelm; i++)
                if (devFileReadWaveformReadValue(file, rcol, "%f", &((float*)(record->bptr))[i]) != 0) break;
            break;
        case DBF_LONG:
        case DBF_ULONG:
            for (i=0; i<record->nelm; i++)
                if (devFileReadWaveformReadValue(file, rcol, "%li", &((long*)(record->bptr))[i]) != 0) break;
            break;
        case DBF_SHORT:
        case DBF_USHORT:
        case DBF_ENUM:
            for (i=0; i<record->nelm; i++)
                if (devFileReadWaveformReadValue(file, rcol, "%hi", &((short*)(record->bptr))[i]) != 0) break;
            break;
        case DBF_CHAR:
        case DBF_UCHAR:
            for (i=0; i<record->nelm; i++)
            {
                short x;
                if (devFileReadWaveformReadValue(file, rcol, "%hi", &x) != 0) break;
                ((char*)(record->bptr))[i] = x;
            }
            break;
        case DBF_STRING:
             for (i=0; i<record->nelm; i++)
                if (devFileReadWaveformReadValue(file, rcol, "%39[^\n]", &((char*)(record->bptr))[i*MAX_STRING_SIZE]) != 0) break;
            break;
        default:
            fclose(file);
            return -1;
    }
    fclose(file);
    record->nord = i;
    record->udf = 0;
    return 0;
}
