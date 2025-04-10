#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <dbDefs.h>
#include <registryFunction.h>
#include <aSubRecord.h>
#include <mbboRecord.h>
#include <epicsExport.h>

#include <okFrontPanelDLL.h>

#define	InF 0
#define	UtF 1
#define SvF 2
#define	NFl 3
#define	FNamLen 256
#define	okSerLen 15

int okSNoASubDebug;

static long okSNoRdInit(aSubRecord *precord)
{
    if (okSNoASubDebug)
        printf("Record %s called okSNoRdInit(%p)\n",
               precord->name, (void*) precord);
    return 0;
}

static long okSNoWrInit(aSubRecord *precord)
{
    if (okSNoASubDebug)
        printf("Record %s called okSNoWrInit(%p)\n",
               precord->name, (void*) precord);
    return 0;
}

char	*okSNoRetSelected(aSubRecord *precord)
{
    char *p="";

    switch( *(unsigned int *)precord->p )
      {
      case  0: return (char *)precord->a;
      case  1: return (char *)precord->b;
      case  2: return (char *)precord->c;
      case  3: return (char *)precord->d;
      case  4: return (char *)precord->e;
      case  5: return (char *)precord->f;
      case  6: return (char *)precord->g;
      case  7: return (char *)precord->h;
      case  8: return (char *)precord->i;
      case  9: return (char *)precord->j;
      case 10: return (char *)precord->k;
      case 11: return (char *)precord->l;
      case 12: return (char *)precord->m;
      case 13: return (char *)precord->n;
      case 14: return (char *)precord->o;
      }
  return p;
}

int okSNoStCmdProc(aSubRecord *precord, int ReplaceSNo)
{
/*  File[0] is Input, File[1] is Output */

    char *fnam[2];
    char ofnam[NFl][FNamLen];
    FILE *stcmd[NFl] = { (FILE *)NULL, (FILE *)NULL, (FILE *)NULL };
    FILE *pipIn;

    char  a[1024];
    char  b[16][64];
    char  c;
    int   ic = 0;
    char  *frmt;

register
    int   i, j;
    int   l = 0;
    int   n = 0;
    int   nplace = 0;
    int   r = 0;
    int   insideOk = 0;

    char  serialno[16];
    char  *p[2];

struct
    timespec    ts[2];
    char        tims[256];

    fnam[InF] = "st.cmd";
    stcmd[InF] = fopen( fnam[InF], "r");

    if( ReplaceSNo ) {
        memset((char *)&ofnam[UtF][0], 0, FNamLen );
        memset((char *)&ofnam[SvF][0], 0, FNamLen );
        strcpy((char *)&ofnam[SvF][0], "st.cmd_");
        strcpy((char *)&ofnam[UtF][0], (char *)&ofnam[SvF][0]);
        }

    if (okSNoASubDebug && ReplaceSNo) {
        printf("Record %s called okSNoStCmdProc(%p) - Entering --- >\n",
               precord->name, (void*) precord);
               printf("precord->a = %s\n", (char *)precord->a);
               printf("precord->b = %s\n", (char *)precord->b);
               printf("precord->c = %s\n", (char *)precord->c);
               printf("precord->d = %s\n", (char *)precord->d);
               printf("precord->e = %s\n", (char *)precord->e);
               printf("precord->f = %s\n", (char *)precord->f);
               printf("precord->g = %s\n", (char *)precord->g);
               printf("precord->h = %s\n", (char *)precord->h);
               printf("precord->i = %s\n", (char *)precord->i);
               printf("precord->j = %s\n", (char *)precord->j);
               printf("precord->k = %s\n", (char *)precord->k);
               printf("precord->l = %s\n", (char *)precord->l);
               printf("precord->m = %s\n", (char *)precord->m);
               printf("precord->n = %s\n", (char *)precord->n);
               printf("precord->o = %s\n", (char *)precord->o);
               printf("precord->p = %d\n", *(unsigned int*)precord->p);
        }

    if( okSNoASubDebug && !stcmd[0] )
        fprintf(stderr, "File %s open error: %s\n", fnam[InF], strerror(errno) );
    else {
        clock_gettime(CLOCK_REALTIME, &ts[0]);
        sprintf(tims, "%d", (unsigned int)ts[0].tv_sec);
        if( okSNoASubDebug ) printf("Time=%s\n", tims);

        if( ReplaceSNo ) {
           stcmd[UtF] = fopen( (char *)&ofnam[UtF][0], "w");
           strcat((char *)&ofnam[SvF][0], tims);
           strcat((char *)&ofnam[SvF][0], ".txt");
           stcmd[SvF] = fopen( (char *)&ofnam[SvF][0], "w");
           }

        do { /* Line by line untile EOF */
           memset(a, 0, sizeof(a));
           for(i = 0; i < 16; i++ )
               memset( (void *)&b[i][0], 0, 64 );
           memset(serialno, 0, sizeof(serialno));

           p[0] = fgets(a, sizeof(a), stcmd[0]);

           if( ReplaceSNo ) {
             /* fprintf( stcmd[UtF], "%s", a); */
             fprintf( stcmd[SvF], "%s", a);
             }

           if( p[0] ) {
             /* If !comment line - parse, else just copy to output ... */
             if( a[0] != '#' ) {
               n++;
               if( okSNoASubDebug ) printf("Line %3d:%s", n, a);
               j = l = r = nplace = 0; p[1] = a;

               if( strstr(a, "okLoad_bitfile") ) { frmt = " %s"; nplace = 1; }
               else
               if( strstr(a, "createOKPort") && strstr(a, "rfport") ) {
                /*frmt = " \"%16s\" "; nplace = 2;*/
                frmt = " \"%s\" "; nplace = 2;
		p[1] = strstr( p[1], ","); p[1]++;
                }

               if( nplace == 1 || nplace == 2 ) {
                 /* Write UtF substring by substring, else write whole input string */   
                 do {
                    r = sscanf(p[1], frmt, (char *)&b[j][0] );

                    if( ReplaceSNo && (nplace == 1) && (j == 0) )
                      fprintf( stcmd[UtF], "%s", (char *)&b[j][0]);
                    else
                    if( ReplaceSNo && (nplace == 1) && (j == 2) )
                      fprintf( stcmd[UtF], "%s", (char *)&b[j][0]);
                    else
                    if( ReplaceSNo && (nplace == 2) && (j == 0) ) {
                      i = 0;
                      do { fprintf( stcmd[UtF], "%c", a[i]); i++; }
                      while( &a[i] != p[1] );
                      }

                    if( r > 0 ) {
                      l = strlen( (char *)&b[j][0] );

                      i = 0;
                      while( i < l && b[j][i] ) {
                        if( !isdigit(b[j][i]) && !isalpha(b[j][i]) ) { ic = i; c = b[j][i]; b[j][i] = 0; }
                        else i++;
			}
 
	              if( okSNoASubDebug ) printf("Scanned substring %s, %d\n", &b[j][0], nplace);

                      if( r > 0 && l > 1) {
                        if( nplace == 1 && j == 1) {
                            if( ReplaceSNo ) fprintf( stcmd[UtF], " %s ", okSNoRetSelected(precord) );
		            strncpy((char *)precord->valb, &b[j][0], okSerLen); strcat((char *)precord->valb, "");
                            /* nplace = 0; */
                            }
                        else
                        if( nplace == 2 ) {
                            if( ReplaceSNo ) fprintf( stcmd[UtF], "\"%s\")", okSNoRetSelected(precord) );
                            strncpy((char *)precord->valc, &b[j][0], okSerLen); strcat((char *)precord->valc, "");
                            /* nplace = 0; */
                            }
                        }
                      p[1] += l; p[1]++; j++;
                      } /* ENDOF if (r > 0) */

                   } 
                   while( r > 0 && l > 1 ); /* ENDOF do */
                   
                   if( ReplaceSNo ) fprintf( stcmd[UtF], "\n");
                 } /* ENDOF if nplace = 1 or 2 */
                 else  if( ReplaceSNo ) fprintf( stcmd[UtF], "%s", a); /* ... else !nplace =1 || 2, write whole a */ 
               } /* ENDOF if a[0] != '#' */
             else  if( ReplaceSNo ) fprintf( stcmd[UtF], "%s", a);
             } /* ENDOF if p[0] */
          } /* ENDOF do */
        while( p[0] );
    }

    fclose( stcmd[InF] );
    if( ReplaceSNo ) {
      fclose(stcmd[UtF]);
      fclose( stcmd[SvF] );
      system("mv -f st.cmd st.cmd,1");
      system("mv -f st.cmd_ st.cmd");
      system("chmod a+x st.cmd");
    }

    if( okSNoASubDebug ) printf("st.cmd contained %d lines.\n", n);

    pipIn = popen("lsusb -v -d 151f:0022", "r");
    nplace = 0;
    do {
       memset(a, 0, sizeof(a) );
       p[0] = fgets(a, sizeof(a), pipIn);
       if( strstr(a, "Opal") ) {
           insideOk = 1; if (okSNoASubDebug) printf("Detected Opal, %s, Inside =%d\n", a, insideOk);
           }
       if( insideOk && strstr(a, "iSerial") ) {
           memset( &b[0][0], 0, 64);  memset( &b[1][0], 0, 64); memset( &b[2][0], 0, 64);
           sscanf(a, " %s %s %s", &b[0][0], &b[1][0], &b[2][0]);
           insideOk = 0; if (okSNoASubDebug) printf("Detected iSerial, %s, Serial = %s, Inside =%d\n",
                                                                        a, (char *)&b[2][0], insideOk);
           switch( nplace )
               {
            case 0: strncpy((char *)precord->vald, &b[2][0], okSerLen); strcat((char *)precord->vald, ""); break;
            case 1: strncpy((char *)precord->vale, &b[2][0], okSerLen); strcat((char *)precord->vale, ""); break;
               }
           nplace++;
           }
       }
    while( p[0] );
    pclose(pipIn);

    if( nplace < 2 ) strcpy((char *)precord->vale, "- - -");
    if( nplace < 1 ) strcpy((char *)precord->vald, "- - -");

    return 0;
}

static long okSNoRdProcess(aSubRecord *precord)
{
    if (okSNoASubDebug) {
        printf("Record %s called okSNoRdProcess(%p) - Entering --- >\n",
               precord->name, (void*) precord);
        }
 
    okSNoStCmdProc(precord, 0);

    if (okSNoASubDebug) {
        printf("Record %s called okSNoRdProcess(%p) - Exiting!\n",
               precord->name, (void*) precord);
        }

    return 0;
}

static long okSNoWrProcess(aSubRecord *precord)
{
    if (okSNoASubDebug) {
        printf("Record %s called okSNoWrProcess(%p) - Entering --- >\n",
               precord->name, (void*) precord);
        }

    okSNoStCmdProc(precord, 1);

    if (okSNoASubDebug) {
        printf("Record %s called okSNoWrProcess(%p) - Exiting!\n",
               precord->name, (void*) precord);
        }

    return 0;
}

/* Register these symbols for use by IOC code: */

epicsExportAddress(int, okSNoASubDebug);
epicsRegisterFunction(okSNoRdInit);
epicsRegisterFunction(okSNoWrInit);
epicsRegisterFunction(okSNoRdProcess);
epicsRegisterFunction(okSNoWrProcess);
