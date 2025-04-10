#include <ctype.h>
#include <stdio.h>
#include <recGbl.h>
#include <dbAccess.h>
#include <devSup.h>
#include <waveformRecord.h>
#include <epicsVersion.h>
#ifndef BASE_VERSION
/* 3.14 */
#include <epicsExport.h>
#endif

long devFileReadWaveformRead0(waveformRecord *record);

struct {
    long      number;
    DEVSUPFUN report;
    DEVSUPFUN init;
    DEVSUPFUN init_record;
    DEVSUPFUN get_ioint_info;
    DEVSUPFUN write;
} devFileReadWaveform0 = {
    5,
    NULL,
    NULL,
    devFileReadWaveformRead0,
    NULL,
    devFileReadWaveformRead0
};

#ifndef BASE_VERSION
/* 3.14 */
epicsExportAddress(dset, devFileReadWaveform0);
#endif

int devFileReadWaveformReadValue0(FILE* file, char* format, void* variable)
{
    int c;
    
    /* read until we get something matching or EOF */
    while (1)
    {
        /* skip blanks */
        while (isspace(c = getc(file)));
        /* ignore comments */
        if (c == '#')
        {
            while ((c = getc(file)) != EOF && c != '\n');
            continue;
        }
        if (c == EOF) return -1;
        ungetc(c, file);
        if (fscanf(file, format, variable) == 1) return 0;
        /* ignore non-value words */
        while (!isspace(c = getc(file)) && c != EOF);
    };
}

long devFileReadWaveformRead0(waveformRecord *record)
{
    int i;
    FILE* file;
    
    file = fopen(record->inp.value.instio.string, "r");
    if (!file) 
    {
        printf ("%s: Can't open file %s for reading\n",
            record->name, record->inp.value.instio.string);
        record->nord = 0;
        record->udf = 1;
        return -1;
    }
    
    switch (record->ftvl)
    {
        case DBF_DOUBLE:
            for (i=0; i<record->nelm; i++)
                if (devFileReadWaveformReadValue0(file, "%lf", &((double*)(record->bptr))[i]) != 0) break;
            break;
        case DBF_FLOAT:
            for (i=0; i<record->nelm; i++)
                if (devFileReadWaveformReadValue0(file, "%f", &((float*)(record->bptr))[i]) != 0) break;
            break;
        case DBF_LONG:
        case DBF_ULONG:
            for (i=0; i<record->nelm; i++)
                if (devFileReadWaveformReadValue0(file, "%li", &((long*)(record->bptr))[i]) != 0) break;
            break;
        case DBF_SHORT:
        case DBF_USHORT:
        case DBF_ENUM:
            for (i=0; i<record->nelm; i++)
                if (devFileReadWaveformReadValue0(file, "%hi", &((short*)(record->bptr))[i]) != 0) break;
            break;
        case DBF_CHAR:
        case DBF_UCHAR:
            for (i=0; i<record->nelm; i++)
            {
                short x;
                if (devFileReadWaveformReadValue0(file, "%hi", &x) != 0) break;
                ((char*)(record->bptr))[i] = x;
            }
            break;
        case DBF_STRING:
             for (i=0; i<record->nelm; i++)
                if (devFileReadWaveformReadValue0(file, "%39[^\n]", &((char*)(record->bptr))[i*MAX_STRING_SIZE]) != 0) break;
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
