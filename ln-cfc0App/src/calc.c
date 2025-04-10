#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <complex.h>
#include <math.h>
#include <errlog.h>
#include <values.h>
#include <time.h>

#include <menuFtype.h>
#include <aSubRecord.h>
#include <dbAccess.h>
#include <waveformRecord.h>
#include <recGbl.h>
#include <alarm.h>

#include <registryFunction.h>
#include <epicsExport.h>
#include <postfix.h>

#include <epicsMath.h>
#include <epicsTypes.h>

#define MIN(A,B) ((A)<(B) ? (A) : (B))
#define MAX(A,B) ((A)>(B) ? (A) : (B))

#define	LOGFILNAM	"Interlog.TXT"
#define	LOGFILNAMWF	"Interwav.TXT"
#define	LogLevel	0

/* Element wise waveform calculator
 *
 * record(stringout, "$(N):expr") {
 *  field(VAL, "C*sin(2*A+B)")
 *  field(FLNK, "$(N)")
 * }
 * record(aSub, "$(N)") {
 *  field(INAM, "WG Init")
 *  field(SNAM, "WG Gen")
 *  field(FTA , "STRING")
 *  field(FTB , "DOUBLE")
 *  field(FTC , "DOUBLE")
 *  field(FTVB, "DOUBLE")
 *  field(NOC , "128")
 *  field(NOVB, "128")
 *  field(INPA, "$(N):expr NPP")
 *  field(INPB, "someacalar")
 *  field(INPC, "somearray")
 *  field(OUTB, "outarray PP")
 * }
 *
 * Each element of the output is computed from the corresponding
 * element of each input.  For arrays of a given length the last
 * value read (or 0.0 for empty arrays) is used.
 *
 * For example, B*C
 * Gives different results depending in the length of its inputs.
 *
 * B = [1,2,3]
 * C = [1,2,3]
 * OUTB = [1,4,9]
 *
 * B = [1,2,3]
 * C = [2]
 * OUTB = [2,4,6]
 *
 * B = [1,2,3]
 * C = [2,3]
 * OUTB = [2,6,9]
 *
 * Note: To support longer expressions set FTA=CHAR and NOA<=100
 */

struct calcPriv {
    char prev[MAX_INFIX_SIZE];
    char postfix[MAX_POSTFIX_SIZE];
    double stack[CALCPERFORM_NARGS];
    epicsUInt32 usein, useout; // can be used
    unsigned long ins, outs;   // will be used
    unsigned long val; // output used for the expression result

    double *curin[CALCPERFORM_NARGS],
           *lastin[CALCPERFORM_NARGS],
           *curout[CALCPERFORM_NARGS],
           *lastout[CALCPERFORM_NARGS];
};
typedef struct calcPriv calcPriv;

// must be enough bits to represent all arguments in 'usein' and 'useout'
STATIC_ASSERT(sizeof(epicsUInt32)*8 >= CALCPERFORM_NARGS);

static
long update_expr(aSubRecord* prec, calcPriv *priv)
{
    short err;
    unsigned long val;
    size_t inlen;
    if(prec->fta==menuFtypeCHAR && prec->nea<MAX_INFIX_SIZE)
        inlen=prec->nea;
    else if(prec->fta==menuFtypeCHAR)
        inlen=MAX_INFIX_SIZE-1;
    else
        inlen=strlen((char*)prec->a);

    if(strlen(priv->prev)==inlen && strncmp(priv->prev, (char*)prec->a, inlen)==0)
        return 0; /* no change */

    memcpy(priv->prev, (char*)prec->a, inlen);
    priv->prev[inlen]='\0';

    if(strlen(priv->prev)==0)
        strcpy(priv->prev, "0");

    if(postfix(priv->prev, priv->postfix, &err)) {
        int junk;

        junk=recGblSetSevr(prec, CALC_ALARM, INVALID_ALARM);
        junk=4; /* quiet warning */
        errlogPrintf("%s: Expression error: %s from '%s' - SEVR=%d\n",
                     prec->name, calcErrorStr(err), priv->prev, junk);
        return -1;
    }
    calcArgUsage(priv->postfix, &priv->ins, &priv->outs);

    priv->val = ~priv->outs;   // consider unassigned outputs
    priv->val &= priv->useout; // exclude unuseable

    val = priv->val & (priv->val-1); // clear lowest bit which is set
    priv->val = val ^ priv->val; // pick only lowest bit which is set

    return 0;
}

static
long init_waveform(aSubRecord* prec)
{
    size_t i;
    calcPriv* priv;

    if(prec->fta != menuFtypeSTRING && prec->fta!=menuFtypeCHAR) {
        errlogPrintf("%s: FTA must be STRING or CHAR\n", prec->name);
        return -1;
    }

    priv = calloc(1, sizeof(calcPriv));
    if(!priv)
        return -1;
    strcpy(priv->prev, "Something highly unlikely");

    for(i=0; i<CALCPERFORM_NARGS; i++) {
        if((&prec->fta)[i]==menuFtypeDOUBLE)
            priv->usein |= (1<<i);

        if((&prec->ftva)[i]==menuFtypeDOUBLE)
            priv->useout |= (1<<i);
    }

    if(priv->usein==0) {
        errlogPrintf("%s: No input arguments???", prec->name);
    }
    if(priv->useout==0) {
        errlogPrintf("%s: No output arguments???", prec->name);
    }

    prec->dpvt = priv;
    return 0;
}

static
long gen_waveform(aSubRecord* prec)
{
    size_t i, s;
    epicsUInt32 nelem=0, nin=0;
    calcPriv* priv=prec->dpvt;
    if(!priv) {
        errlogPrintf("%s: Not initialized\n",prec->name);
        return -1;
    }

    if(update_expr(prec, priv)) {
        return -1;
    }

    // reset stack
    memset(&priv->stack, 0, sizeof(priv->stack));

    // initialize input pointers
    for(i=0; i<CALCPERFORM_NARGS; ++i) {
        priv->curin[i]  = ((double**)&prec->a)[i];
        priv->curout[i] = ((double**)&prec->vala)[i];
        priv->lastin[i] = priv->curin[i] + (&prec->noa)[i];
        priv->lastout[i]= priv->curout[i]+ (&prec->nova)[i];

        nelem= MAX(nelem,(&prec->nova)[i]);
        nin  = MAX(nin  ,(&prec->noa)[i]);
    }

    nelem = MIN(nelem, nin);

    // A is taken by the calc expression so use it to store M_PI
    priv->stack[0] = M_PI;
    priv->curin[0] = priv->lastin[0] = 0;

    for(s=0; s<nelem; s++) {
        double val;
        // for each output sample

        // update stack from inputs
        for(i=1; i<CALCPERFORM_NARGS; ++i) {
            if(!(priv->usein & (1<<i)))
                continue;
            if(priv->curin[i] >= priv->lastin[i])
                continue;

            priv->stack[i] = *(priv->curin[i])++;
        }

        // calculate
	if (calcPerform(priv->stack, &val, priv->postfix)) {
	    recGblSetSevr(prec, CALC_ALARM, INVALID_ALARM);
	} else prec->udf = isnan((double)prec->val);

        // update outputs from stack
        for(i=0; i<CALCPERFORM_NARGS; ++i) {
            if(!(priv->useout & (1<<i)))
                continue;
            if(priv->curout[i] >= priv->lastout[i])
                continue;

            // place the expression result or the stack value
            if(priv->val == (1<<i))
                *(priv->curout[i])++ = val;

            else
                *(priv->curout[i])++ = priv->stack[i];
        }

    }

    for(i=0; i<CALCPERFORM_NARGS; ++i) {
        if(!(priv->useout & (1<<i)))
            continue;
        (&prec->neva)[i] = MIN( nelem, (&prec->nova)[i]);
    }

    return 0;
}

static	long	align_wf(aSubRecord* prec)
{
/* Align two waveform along x axis		*/
/*						*/
/* Input arguments:				*/
/* 						*/
/* A wf[0] = xIn = time in			*/
/* B wf[1] = yIn = val  in			*/
/* VALA wf[2] = yOut = VALUE OUT, nelm = 512	*/
/*						*/
/* C = Time [s] of one setpoint, DAC step	*/
/* for output waveform				*/
/*						*/
/* i - setpoint table index			*/
/* set yOut[0] = yIn[0]				*/
/* i++						*/
/* Next xIn ... calc output pos			*/
/* xOut[1] = xIn[i] / C				*/
/* if( (xOut[1] - xOut[0]) >=1 ) ->interpolate	*/
/* if( (xOut[1] - xOut[0]) < 1 ) ->discard	*/
/* else (same point or next)			*/
/* yOut[xOut[1]] = yIn[xIn[1]]			*/
/* .............................................*/

register
    int    i, j, ii;
    int    n[3] = { prec->noa,
                    prec->nob,
                    prec->nova };

    double *pDbl[3] = {NULL, NULL, NULL};
    double dt = 0;
    double xIn[2]   = {0, 0};
    double xOut[2]  = {0, 0};
    int    xiOut[2] = {0, 0};
    double yOut[2]  = {0, 0};
    double tanf     = 0;
    double dx	    = 0;
    double dy	    = 0;
    double dlastvv  = 0; /* Last valid value */
    int    ix	    = 0;

    FILE   *fp;

    if( LogLevel ) fp = fopen(LOGFILNAM, "a+");

    i = ii = j = 0;

    pDbl[0] = (double *)prec->a;    /* x (time) In	*/
    pDbl[1] = (double *)prec->b;    /* y (value)In	*/
    dt      = *(double*)prec->c;    /* x step Out	*/
    pDbl[2] = (double *)prec->vala; /* y value Out	*/

    if( n[0] > 0 && n[1] > 0 ) {   /* If any values read from the file */
        xIn[1]   = *(pDbl[0] + j);
        yOut[1]	 = *(pDbl[1] + j);
        xOut[1]  = xIn[1] / dt;
        xiOut[1] = (int)floor(xOut[1] + 0.5);
	
        dy = yOut[1] - yOut[0];
        ix = xiOut[1]- xiOut[0];
        dx = (double)ix;

        if( dx > 0) tanf = dy / dx;

        if( LogLevel > 0 ) {
            fprintf(fp, "xIn[1] = %12.5lf, dt=%12.5lf xOut[1] = %12.3lf, xiOut[1] = %7d, dx = %12.3lf, dix=%3d, yOut[1]=%12.5lf, yOut[0]=%12.5lf dy=%12.5lf\n",
            xIn[1], dt, xOut[1], xiOut[1], dx, ix, yOut[1], yOut[0], dy); }

        yOut[1]	= *(pDbl[1] + j);

        if( dx > 1 ) {
            ii = 0;
            do {
                *(pDbl[2] + i) = dlastvv = yOut[0] + tanf*(ii+1);
                i++; ii++; }
            while( ii < ix );
            }
        else {
            *(pDbl[2] + i) = dlastvv = yOut[1];
            i++;
            }

        xIn[0]   = xIn[1];
        xOut[0]  = xOut[1];
        xiOut[0] = xiOut[1];
        yOut[0]  = yOut[1];

        j++;

        do {
            xIn[1]   = *(pDbl[0] + j);
            yOut[1]  = *(pDbl[1] + j);
            xOut[1]  = xIn[1] / dt;
            xiOut[1] = (int)floor(xOut[1] + 0.5);
            dy = yOut[1] - yOut[0];
            ix = xiOut[1]- xiOut[0];
            dx = (double)ix;

            if( dx > 0) tanf = dy / dx;

            if( LogLevel > 0 ) {
                fprintf(fp, "xIn[1] = %12.5lf, dt=%12.5lf xOut[1] = %12.3lf, xiOut[1] = %7d, dx = %12.3lf, dix=%3d\n",
                xIn[1], dt, xOut[1], xiOut[1], dx, ix); }

            if( dx > 1 ) {
                ii = 0;
                do {
                    *(pDbl[2] + i) = dlastvv = yOut[0] + tanf*(ii+1);
                    if( LogLevel > 0 ) {
                        fprintf(fp, "+i =%3d ii=%3d j=%3d tanf=%12.5lf yOut[1] = %12.5lf, yOut[0]=%12.5lf\n",
                            i, ii, j, tanf, *(pDbl[2] + i), yOut[0]); }
                    i++; ii++;
                    }
                while( ii < ix && i < n[2]);
                }
                else {
                    if( dx > 0 ) /* If 1 */
                        *(pDbl[2] + i) = dlastvv = yOut[1];
                    else
                        *(pDbl[2] + i) = dlastvv = yOut[0]; /* If short of values, use previous	*/

	            if( LogLevel > 0 ) {
                        fprintf(fp, "-i =%3d ii=%3d j=%3d tanf=%12.5lf yOut[1] = %12.5lf, yOut[0]=%12.5lf\n",
                            i, ii, j, tanf, *(pDbl[2] + i), yOut[0]); }
                    i++;
                }

            /* Fill out the waveform with the last value to the end ...	*/
            ii = i;
            while( ii < n[2] ) {
                dlastvv = *(pDbl[1] + n[1] - 1);
                *(pDbl[2] + ii) = dlastvv;
                ii++; if( dx < 1 && i < n[2]) i++;
                }

            xIn[0]   = xIn[1];
            xOut[0]  = xOut[1];
            xiOut[0] = xiOut[1];
            yOut[0]  = yOut[1];

        j++;
        }
    while( i < n[2]); // end do
    } // end if( n[0]>0 && n[1] > 0)

    /* ...if any values were read from the file - else do nothing!)	*/
    if( LogLevel > 0 ) fclose(fp);
    if( LogLevel > 0 ) {
        fp = fopen(LOGFILNAMWF, "a+");
        for( i = 0; i < n[2]; i++ ) {
            fprintf(fp, "%3d %12.5lf\n", i, *(pDbl[2] + i));
            }
        fclose(fp);
        }
    return 0;
}

static	long	unwrap_wf(aSubRecord* prec)
{
/* Unwrap circular buffer			*/
/*						*/
/* Input arguments:				*/
/* 						*/
/* A wf[0] = input wf of 16777216 short vals	*/
/*						*/
/* VALA wf[2] = output wf of the same length	*/
/*						*/
/* C = pageloc					*/
/* .............................................*/
register
	int		i, j;
	int		n[2] = {0, 0};
	int		nlm[2] = {0, 0};
	int		pageloc = 0;

	short		*pSh[2];

	pageloc = (int)floor(*(double *)prec->b + 0.5);
	printf("pageloc=%d\n", pageloc);

        n[0] = nlm[0] = prec->noa;
        nlm[1] = prec->nova;

	if( n[0] ) {
            i = j = 0;

            pSh[0] = (short *)prec->a;
            pSh[1] = (short *)prec->vala;

            i = pageloc;
            while( i < nlm[0] ) {
                *(pSh[1] + j) = *(pSh[0] + i); i++; j++;
                }
            i = 0;
            while( i < pageloc ) {
                *(pSh[1] + j) = *(pSh[0] + i); i++; j++;
                }
            }

   	// Output wf nord = input wf nelm ==> we unwrap always, whether nord or not
        printf("UNWRAP DONE!\n");
        prec->neva = prec->nova;	
    return 0;
}

static	long	invpair_wf(aSubRecord* prec)
{
/* ++-- sequence remover of the	circular buffer	*/
/*						*/
/* Input arguments:				*/
/* 						*/
/* A wf[0] = input wf of 16777216 short vals	*/
/*						*/
/* VALA wf[2] = output wf of the same length	*/
/*						*/
/* .............................................*/
register
	int		i, j;
	int		n[2] = {0, 0};
	int		nlm[2] = {0, 0};

	int		mask[4] = {+1, +1, -1, -1};

	short		*pSh[2];

        n[0] = nlm[0] = prec->noa;
        nlm[1] = prec->nova;

	if( n[0] ) {
            i = j = 0;

            pSh[0] = (short *)prec->a;
            pSh[1] = (short *)prec->vala;

            while( i < nlm[0] ) {
                *(pSh[1] + i) = *(pSh[0] + i) * mask[j];
                i++; j++;
                if( j > 3 ) j = 0;
                }
        }
   	// Output wf nord = input wf nelm ==> we invert the pairs always, whether nord or not
        prec->neva = prec->nova;
    return 0;
}

static	long	circpart_wf(aSubRecord* prec)
{
/* Separate 5 channels with signal and quadrant	*/
/* alignment from the circular buffer		*/
/*						*/
/* MATLAB file llrf100909_record.m, lines 41-53	*/
/* Input arguments:				*/
/* 						*/
/* A wf[00] = input wf of 16777216 short vals	*/
/* B elapsedsamples = (wrapctr-1)*memsize/2 +	*/
/* + pageloc					*/
/*						*/
/* VALA wf[01] = output Ch[0] wf I 		*/
/* VALB wf[02] = output Ch[0] wf Q		*/
/* VALC wf[03] = output Ch[1] wf I 		*/
/* VALD wf[04] = output Ch[1] wf Q		*/
/* VALE wf[05] = output Ch[2] wf I 		*/
/* VALF wf[06] = output Ch[2] wf Q		*/
/* VALE wf[07] = output Ch[3] wf I 		*/
/* VALF wf[08] = output Ch[3] wf Q		*/
/* VALE wf[09] = output Ch[4] wf I 		*/
/* VALF wf[10] = output Ch[4] wf Q		*/
/*						*/
/* .............................................*/
register
	int    i, j;

	int    n[11]	= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        int    nlm[11]  = { prec->noa,
                            prec->nova,
                            prec->novb,
                            prec->novc,
                            prec->novd,
                            prec->nove,
                            prec->novf,
                            prec->novg,
                            prec->novh,
                            prec->novi,
                            prec->novj};

	short		*pSh[1] =  { (short  *)prec->a };
	double		*pDbl[11]= { (double *)prec->a,		/* 00	*/
					(double *)prec->vala,	/* 01	*/
					(double *)prec->valb,	/* 02	*/
					(double *)prec->valc,	/* 03	*/
					(double *)prec->vald,	/* 04	*/
					(double *)prec->vale,	/* 05	*/
					(double *)prec->valf,	/* 06	*/
					(double *)prec->valg,	/* 07	*/
					(double *)prec->valh,	/* 08	*/
					(double *)prec->vali,	/* 09	*/
					(double *)prec->valj };	/* 10	*/

	int		elapsedsamples = (int)floor(*(double *)prec->b + 0.5);
	int		length, d, v;	/* From the MATLAB script llrfllrf100909_record.m	*/
	int		o[2];		/* From the MATLAB script llrfllrf100909_record.m	*/

	/* printf("circpart_wf: elapsedsamples =%d\n", elapsedsamples);	*/

	length =  nlm[0];			/* 16 777 216 */
	d = (int)floor((double)length/10.0 - 2);/*  1 677 719 */

	/* Cycle through 0 ... (d-1): 0 ... 1 677 719 (No of samples)						*/
	/* MATLAB file llrf100909_record.m,									*/
	/* where d = int32(floor(length(buf)/10-2) = int32(floor(16 777 216/10-2)) = 1 677 721 - 2 = 1 677 719	*/
	/* MATLAB creates 1 677 719 5x2 matrices, one for each sample: 5 channels, first column I, second Q	*/
	/*													*/

	j = (int)floor(nlm[0]/10 - 2);	/* Should give j = 1 677 719						*/
					/* For 32Mb long OpalKelly memory					*/
					/*									*/

	for( v = 0; v < 5; v++)	/* Cycle through the 5 channels ...	*/
		{
		o[0] = -4 * v - elapsedsamples + 6;
		/* ( ((page=0) + 3)*(pagesize=512) = 3*512 = 1536 	*/
		/* printf("--1. v=%2d o = %6d ", v, o[0]);		*/
		/* printf("---2. o = %6d ", (o[0] % 20) );		*/
		o[0] -= (int)floor(o[0]/20.0)*20;
		/* MATLAB mod( -4*v - elapsedsamples + 6, 20), where elapsedsamples = 1536	*/
		/*printf("----3. o = %6d\n", o[0]);						*/
		/*										*/
		/* The long cycle should run inside the short one				*/

		i = 0;
		while( i < j )
			{
			o[1] = o[0] + 10 * i;

			/********************************************************************************************************/
			/*													*/
			/* pDbl[0] points to input array!									*/
			/*													*/
			/*													*/
			*(pDbl[2*v + 0 + 1] + i) = (double) *(pSh[0] + o[1] + 0);	/* / 32767.0; * Scale to -1 ... + 1	*/
			*(pDbl[2*v + 1 + 1] + i) = (double) *(pSh[0] + o[1] + 5);	/* / 32767.0; * 			*/
			i++;
			}
		}

    prec->neva = nlm[ 1];
    prec->nevb = nlm[ 2];
    prec->nevc = nlm[ 3];
    prec->nevd = nlm[ 4];
    prec->neve = nlm[ 5];
    prec->nevf = nlm[ 6];
    prec->nevg = nlm[ 7];
    prec->nevh = nlm[ 8];
    prec->nevi = nlm[ 9];
    prec->nevj = nlm[10];

    printf(" - circpart_wf: DONE!\n");
    return 0;
}

/********************************************************************************************************************************/
/*------------------------------------------------------------------------------------------------------------------------------*/
static	long	decimate_wf(aSubRecord* prec)
{
/*						*/
/* Input arguments:				*/
/* 						*/
/* wf[00] A = input wf of 0x1A0000 = 1703936 	*/
/* B = method (default = 0)			*/
/* C = (double)decimation factor (2048)		*/
/* 						*/
/* OUTA = output wf				*/
/*						*/
/* .............................................*/
register
    int    i, j;
    double factor = 1.0;
    int    method = 0;
    double *pDbl[2]= { (double *)prec->a,    /* 00	*/
                       (double *)prec->vala, /* 01	*/
                     };

    method = *(int *)prec->b;
    // printf("decimate_wf:..."); fflush(stdout);
    // printf("decimate_wf: noa=%d nova=%d\n", prec->noa, prec->nova);

    if( !method )
        //factor = (double)nelm[0]/(double)nelm[1];
        factor = prec->noa/prec->nova;
    else
        factor = *(double *)prec->c;

    // printf("decimate_wf: method = %d, factor = %lf\n", method, factor);

    j = 0;
    while( j < prec->nova ) {
        i = (int)floor(j*factor + 0.5);

        if (i >= prec->noa) { i = prec->noa-1; }

        *(pDbl[1] + j) = *(pDbl[0] + i);

        // printf("decimate: Dbl[i=%d]=%12.5lf Dbl[j=%d] = %12.5lf\n",
        //     i, *(pDbl[0] + i), j, *(pDbl[1] + j) );

        j++;
    }

    *(pDbl[1] + prec->nova - 1) = *(pDbl[0] + prec->noa - 1);
    prec->neva = prec->nova;
    // printf(" - Decimation DONE!\n");
    return 0;
}
/*------------------------------------------------------------------------------------------------------------------------------*/
/********************************************************************************************************************************/


static	long	circdump_wf(aSubRecord* prec)
{
/* Dump 5 circular bufferwaveforms into ASCII	*/
/* file. The first column is time [ms], followed*/
/* by I[0], Q[0], I[1]. Q[1], I[2]. Q[2]. I[3],	*/
/* Q[3]. I[4]. Q[4]. I[5]. Q[5]			*/
/*						*/
/* Input arguments:				*/
/* 						*/
/* wf[00] A = input wf of 0x1A0000 = 1703936 	*/
/* double vals	of time	[ms]			*/
/* wf[01] B = Ch[0] wf I 			*/
/* wf[02] C = Ch[0] wf Q			*/
/* wf[03] D = Ch[1] wf I 			*/
/* wf[04] E = Ch[1] wf Q			*/
/* wf[05] F = Ch[2] wf I 			*/
/* wf[06] G = Ch[2] wf Q			*/
/* wf[07] H = Ch[3] wf I 			*/
/* wf[08] I = Ch[3] wf Q			*/
/* wf[09] J = Ch[4] wf I 			*/
/* wf[10] K = Ch[4] wf Q			*/
/* wf[11] L = Filename				*/
/* wf[12] M = sdramPage				*/
/* wf[13] N = Timestamp                         */
/*						*/
/* .............................................*/
register
    int    i, j;
    int    nlm[14] = { prec->noa,
                       prec->nob,
                       prec->noc,
                       prec->nod,
                       prec->noe,
                       prec->nof,
                       prec->nog,
                       prec->noh,
                       prec->noi,
                       prec->noj,
                       prec->nok,
                       prec->nol,
                       prec->nom,
                       prec->non};


    double *pDbl[14]= { (double *)prec->a, /* 00 */
                        (double *)prec->b, /* 01 */
                        (double *)prec->c, /* 02 */
                        (double *)prec->d, /* 03 */
                        (double *)prec->e, /* 04 */
                        (double *)prec->f, /* 05 */
                        (double *)prec->g, /* 06 */
                        (double *)prec->h, /* 07 */
                        (double *)prec->i, /* 08 */
                        (double *)prec->j, /* 09 */
                        (double *)prec->k, /* 10 */
                        (double *)prec->l, /* 11 */
                        (double *)prec->m, /* 12 */
                        (double *)prec->n, /* 13 */
                      };

    FILE   *fp;

    // printf("circdump_wf: Filename: %s\n", (char *)prec->r);
    // printf("circdump_wf: File %s opened, start dump ...\n", (char *)prec->r);
    // TrgA = *(unsigned int *)prec->s;
    // printf("circdump_wf: TrigAddr = %d\n", TrgA);

    fp = fopen((char *)prec->r, "w");
    if( !fp ) {
        fprintf(stderr, "Error opening file %s: %s\n",
            (char *)prec->r, strerror(errno) ); return 0;
    } else {
        /* printf("circdump_wf: File %s opened, start dump ...\n", (char *)prec->l);
        printf("circdump_wf: sdramPage = %d\n", *(unsigned int *)prec->m); */

        i = 0;
        while( i < nlm[0] ) {
            j = 0;
            /* if( i < nelm[0]/2 + 10 ) */
            { 
            while( j < 11 ) {
                /* if( j == 0 ) fprintf(fp, "%12.5lf ", *(pDbl[j] + i) );*/ /* A is expected to be X scale of circ buf [ms]	*/
                if( j == 0 ) fprintf(fp, "%8d %5d ", i, *(unsigned int *)prec->m );
                else	     fprintf(fp, "%8.0lf ",  *(pDbl[j] + i) );
                j++;
                }
            fprintf(fp, "\n");
            }
            i++;
            }
        fclose(fp);
        /* printf("circdump_wf: File %s closed, dump done.\n", (char *)prec->l);				*/
        }
    return 0;
}

/* I/Q to amplitude/phase converter
 *
 * record(aSub, "$(N)") {
 *  field(SNAM, "IQ2AP")
 *  field(FTA , "DOUBLE")
 *  field(FTB , "DOUBLE")
 *  field(FTVA ,"DOUBLE")
 *  field(FTVB ,"DOUBLE")
 *  field(NOA , "128")
 *  field(NOB , "128")
 *  field(NOVA, "128")
 *  field(NOVB, "128")
 *  field(INPA, "I")
 *  field(INPB, "Q")
 *  field(OUTA, "AMP PP")
 *  field(OUTB, "PHA PP")
 * }
 */
#define MAGIC ((void*)&convert_iq2ap)
#define BADMAGIC ((void*)&gen_waveform)

static
long convert_iq2ap(aSubRecord* prec)
{
    size_t i;
    epicsEnum16 *ft = &prec->fta,
                *ftv= &prec->ftva;
    // actual length of inputs, and max length of outputs
    epicsUInt32 lens[4] = { prec->noa, prec->nob, prec->nova, prec->novb };
    epicsUInt32 len = lens[0];

    if(prec->dpvt==BADMAGIC)
        return 1;
    if(prec->dpvt!=MAGIC) {
        // Only do type checks in not already passed
        for(i=0; i<2; i++) {
            if(ft[i]!=menuFtypeDOUBLE) {
                prec->dpvt=BADMAGIC;
                errlogPrintf("%s: FT%c must be DOUBLE\n",
                             prec->name, (char)('A'+i) );
                return 1;

            } else if(ftv[i]!=menuFtypeDOUBLE) {
                prec->dpvt=BADMAGIC;
                errlogPrintf("%s: FTV%c must be DOUBLE\n",
                             prec->name, (char)('A'+i) );
                return 1;

            }
        }
        prec->dpvt = MAGIC;
    }

    for(i=0; i<4; i++)
        len = MIN(len, lens[i]);

    double *dI = (double*)prec->a,
           *dQ = (double*)prec->b,
           *A  = (double*)prec->vala,
           *P  = (double*)prec->valb;

    for(i=0; i<len; i++) {
        A[i] = sqrt(dI[i]*dI[i] + dQ[i]*dQ[i]);
        P[i] = atan2(dQ[i], dI[i]) * 180 / M_PI;
    }

    prec->neva = prec->nevb = len;

    return 0;
}

/* Amplitude/phase to I/Q converter
 *
 * record(aSub, "$(N)") {
 *  field(SNAM, "AP2IQ")
 *  field(FTA , "DOUBLE")
 *  field(FTB , "DOUBLE")
 *  field(FTVA ,"DOUBLE")
 *  field(FTVB ,"DOUBLE")
 *  field(NOA , "128")
 *  field(NOB , "128")
 *  field(NOVA, "128")
 *  field(NOVB, "128")
 *  field(INPA, "AMP")
 *  field(INPB, "PHA")
 *  field(OUTA, "I PP")
 *  field(OUTB, "Q PP")
 * }
 */

static
long convert_ap2iq(aSubRecord* prec)
{
    size_t i;
    epicsEnum16 *ft = &prec->fta,
                *ftv= &prec->ftva;
    // actual length of inputs, and max length of outputs
    epicsUInt32 lens[4] = { prec->noa, prec->nob, prec->nova, prec->novb };
    epicsUInt32 len = lens[0];


    if(prec->dpvt==BADMAGIC)
        return 1;
    if(prec->dpvt!=MAGIC) {
        // Only do type checks in not already passed
        for(i=0; i<2; i++) {
            if(ft[i]!=menuFtypeDOUBLE) {
                prec->dpvt=BADMAGIC;
                errlogPrintf("%s: FT%c must be DOUBLE\n",
                             prec->name, (char)('A'+i) );
                return 1;

            } else if(ftv[i]!=menuFtypeDOUBLE) {
                prec->dpvt=BADMAGIC;
                errlogPrintf("%s: FTV%c must be DOUBLE\n",
                             prec->name, (char)('A'+i) );
                return 1;

            }
        }
        prec->dpvt = MAGIC;
    }

    for(i=0; i<4; i++)
        len = MIN(len, lens[i]);

    double *dI = (double*)prec->vala,
           *dQ = (double*)prec->valb,
           *A  = (double*)prec->a,
           *P  = (double*)prec->b;

    for(i=0; i<len; i++) {
        dI[i] = A[i] * cos(P[i] * M_PI / 180.0);
        dQ[i] = A[i] * sin(P[i] * M_PI / 180.0);
    }

    prec->neva = prec->nevb = len;

    return 0;
}

/* Waveform statistics
 *
 * Computes mean and std of the (subset of)
 * waveform Y.  The values of waveform X
 * (aka time) are used to compute the windows
 *
 * record(aSub, "$(N)") {
 *  field(SNAM, "Wf Stats")
 *  field(FTA , "DOUBLE")
 *  field(FTB , "DOUBLE")
 *  field(FTC , "DOUBLE")
 *  field(FTD , "DOUBLE")
 *  field(FTVA ,"DOUBLE")
 *  field(FTVB ,"DOUBLE")
 *  field(NOA , "128")
 *  field(NOB , "128")
 *  field(INPA, "Waveform Y")
 *  field(INPB, "Waveform X")
 *  field(INPC, "Start X") # window start
 *  field(INPD, "Width X") # window width
 *  field(OUTA, "Mean PP")
 *  field(OUTB, "Std PP")
 */

static
long wf_stats(aSubRecord* prec)
{
    size_t i, N=0;
    epicsEnum16 *ft = &prec->fta,
                *ftv= &prec->ftva;
    // actual length of inputs
    epicsUInt32 len = MIN(prec->noa, prec->nob);

    double *data = prec->a,
           *time = prec->b,
           sum   = 0.0,
           sum2  = 0.0,
           start = *(double*)prec->c,
           width = *(double*)prec->d;


    if(prec->dpvt==BADMAGIC)
        return 1;
    if(prec->dpvt!=MAGIC) {
        // Only do type checks in not already passed
        for(i=0; i<4; i++) {
            if(ft[i]!=menuFtypeDOUBLE) {
                prec->dpvt=BADMAGIC;
                errlogPrintf("%s: FT%c must be DOUBLE\n",
                             prec->name, (char)('A'+i) );
                return 1;

            }
        }
        for(i=0; i<2; i++) {
            if(ftv[i]!=menuFtypeDOUBLE) {
                prec->dpvt=BADMAGIC;
                errlogPrintf("%s: FTV%c must be DOUBLE\n",
                             prec->name, (char)('A'+i) );
                return 1;

            }
        }
        prec->dpvt = MAGIC;
    }

    if(len==0) {
        printf("wf_stats: prec->noa=%d, prec->nob=%d, len=%d\n", prec->noa, prec->nob, len);
        recGblSetSevr(prec, CALC_ALARM, INVALID_ALARM);
        return 0;
    }

    for(i=0; i<len; i++) {
        if(time[i]<start)
            continue;
        if(time[i]>=start+width)
            break;

        sum  += data[i];
        sum2 += data[i] * data[i];
        N++;
    }

    
    if(N==0) {
        printf("wf_stats: N=%d\n", N);
        recGblSetSevr(prec, CALC_ALARM, INVALID_ALARM);
        return 0;
    }

    sum  /= N; // <x>
    sum2 /= N; // <x^2>

    *(double*)prec->vala = sum;
    prec->neva=1;
    *(double*)prec->valb = sqrt(sum2 - sum*sum);
    prec->nevb=1;
    return 0;
}

/* Unwrap phase
 *
 * Takes a phase signal which may be wrapped around [-180, 180].
 * Unwrap for jumps where the unwrapped phase would not change
 * by more then Max Difference degrees between samples.
 *
 * record(aSub, "$(N)") {
 *  field(SNAM, "Phase Unwrap")
 *  field(FTA , "DOUBLE")
 *  field(NOA , "128")
 *  field(NOVA, "128")
 *  field(INPA, "Wrapped phase")
 *  field(INPB, "Max Difference")
 *  field(OUTA, "Unwrapped phase PP")
 */

static
long unwrap(aSubRecord* prec)
{
    size_t i;
    double *in = (double*)prec->a,
           *out= (double*)prec->vala,
           delta, thres;
    epicsUInt32 len=MIN(prec->noa, prec->nova);

    if(prec->dpvt==BADMAGIC)
        return 1;
    if(prec->dpvt!=MAGIC) {
        // Only do type checks in not already passed
        if(prec->fta!=menuFtypeDOUBLE &&
           prec->ftb!=menuFtypeDOUBLE) {
            prec->dpvt=BADMAGIC;
            errlogPrintf("%s: FTA and FTB must be DOUBLE\n",
                         prec->name);
            return 1;

        }
        if(prec->ftva!=menuFtypeDOUBLE) {
            prec->dpvt=BADMAGIC;
            errlogPrintf("%s: FTVA must be DOUBLE\n",
                         prec->name);
            return 1;

        }
        prec->dpvt = MAGIC;
    }

    // printf("unwrap: prec->name:%s,prec->noa=%d, prec->nob=%d, prec->nova=%d, len=%d\n",
    //                 prec->name, prec->noa, prec->nob, prec->nova, len);

    if(len==0 || prec->nob==0) {
        recGblSetSevr(prec, CALC_ALARM, INVALID_ALARM);
        return 0;
    }
    thres = *(double*)prec->b;
    thres /= 2.0; // half on either side of the fold

    out[0]=in[0]; // start at same point

    for(i=1; i<len; i++) {
        delta = in[i] - in[i-1];

        // the following will only work for wrapping
        // at shallow angles

        // wrapped from positive to negative
        if(in[i-1]>(180.0-thres) && in[i]<(-180.0+thres))
            delta += 360.0;

        // wrapped from negative to positive
        else if(in[i-1]<(-180.0+thres) && in[i]>(180.0-thres))
            delta += -360.0;

        out[i] = out[i-1] + delta;
    }

    prec->neva = len;
    return 0;
}

//==================================================================================================

static	long wfSel(aSubRecord* prec)
{
register
    int    i, j;
    int    l = 0;
    short  *shp;
    int    sel = 0;

    int    nlm[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    char   *p[8]  = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

    shp = (short *)prec->a;
    sel = (int)*(shp + 0);

    i = 0;
    while( i < 8 ) {
        switch( i ) {
            case  0: p[i] = (char *)prec->vala; nlm[i] = prec->nova; break;
            case  1: p[i] = (char *)prec->b; nlm[i] = prec->nob; break;
            case  2: p[i] = (char *)prec->c; nlm[i] = prec->noc; break;
            case  3: p[i] = (char *)prec->d; nlm[i] = prec->nod; break;
            case  4: p[i] = (char *)prec->e; nlm[i] = prec->noe; break;
            case  5: p[i] = (char *)prec->f; nlm[i] = prec->nof; break;
            case  6: p[i] = (char *)prec->g; nlm[i] = prec->nog; break;
            case  7: p[i] = (char *)prec->h; nlm[i] = prec->noh; break;
            default: p[i] = (char *)NULL; nlm[i] = 0;
            }
        if( i == 0 ) {
            p[i] = (char *)prec->vala;
            for( j = 0; j < nlm[i]; j++ ) *(p[i] + j) = 0;
        }
        i++;
    }

    j = sel+1;
    if( j < 8 ) {
        l = (nlm[0] < nlm[j])? nlm[0] : nlm[j];
        for( i = 0; i < l; i++ ) *(p[0] + i) = *(p[j] + i);
    }
    prec->neva = l;
    return 0;
}

static	long fbRampSegment(aSubRecord* prec)
{
register
    int    i, j;
    int    reshape = 0;
    int    n[2] = { prec->noa, prec->nova };

    /*FILE	*fp;*/

    double *pI = (double *)prec->a;
    double *pQ = (double *)prec->b;
    double *pIQ= (double *)prec->vala;

    double dI[2] = { *pI, *(pI + prec->noa - 1) };
    double dQ[2] = { *pQ, *(pQ + prec->nob - 1) };


    double dAmp[2] = { *(double *)prec->d, *(double *)prec->e }; /* SP:Amp-SP.VAL, SP:Amp-SP.PVAL */
    double dPha[2] = { *(double *)prec->f, *(double *)prec->g }; /* SP:Pha-SP.VAL, SP:Pha-SP.PVAL */

    /* fp = fopen("FBRampSegm.TXT", "w+"); */

    reshape = (int)(*(char *)prec->c);

    if( reshape ) {
        dI[0] = dAmp[1] * cos(dPha[1] * M_PI / 180.0);
        dI[1] = dAmp[0] * cos(dPha[0] * M_PI / 180.0);
        dQ[0] = dAmp[1] * sin(dPha[1] * M_PI / 180.0);
        dQ[1] = dAmp[0] * sin(dPha[0] * M_PI / 180.0);
    }
    /*
    printf("fbRampSegment: Reshape=%d, A.VAL=%7.3lf, A.PVAL=%7.3lf, P.VAL=%7.3lf, P.PVAL=%7.3lf\n",
        reshape, dAmp[0], dAmp[1], dPha[0], dPha[1]);
    */

    i = j = 0;
    while( i < n[0] ) {
        if( reshape )
            *(pIQ + j) = dI[0] + 0.5 * (dI[1] - dI[0]) * (1 - cos( M_PI * (double)i / (n[0]-1) ) );
	else
            *(pIQ + j) = *(pI + i);
        /*
        fprintf(fp, "j = %4d I = %12.5lf Q = %12.5lf IQ = %12.5lf\n",
            j, *(pI + i), *(pQ + i), *(pIQ + j) ); */
        j++;

        if( reshape )
            *(pIQ + j) = dQ[0] + 0.5 * (dQ[1] - dQ[0]) * (1 - cos( M_PI * (double)i / (n[0]-1) ) );
        else
            *(pIQ + j) = *(pQ + i);
        /*
        fprintf(fp, "j = %4d I = %12.5lf Q = %12.5lf IQ = %12.5lf\n",
            j, *(pI + i), *(pQ + i), *(pIQ + j) ); */
        j++; i++;
    }

    /* fclose(fp); */
    prec->neva = prec->nova;
    return 0;
}

static	long	unPhase_wf(aSubRecord* prec)
{
/* Unwrap phase in waveform record		*/
/*						*/
/* .............................................*/

register
    int    i, j, min_n;
    int    n[2] = {0, 0};
    int    nelm[2] = {0, 0};
    int    pageloc = 0;
    double d = 0;
    double *pDbl[2];

    // printf("unPhase_wf: called by record:%s\n", prec->name);
    n[0] = prec->noa;
    n[1] = prec->nova;

    min_n = (n[0] < n[1]) ? n[0] : n[1];

    if( min_n ) {
        i = j = 0;
        pDbl[0] = (double *)prec->a;
        pDbl[1] = (double *)prec->vala;
        while( i < min_n ) {
            d = *(pDbl[0] + i);
            while( d > +180.0 ) d -= 360.0;
            while( d < -180.0 ) d += 360.0;
            *(pDbl[1] + j) = d; i++; j++;
        }
    }
    prec->neva = min_n;
    return 0;
}

//==================================================================================================

static	long	dBm2kW(aSubRecord* prec)
{
/* Unwrap phase in waveform record		*/
/*						*/
/* .............................................*/

register
    int    i, j, min_n;
    int    n[2]  = { prec->noa,
                     prec->nova
                   };
    int    nlm[2]= { prec->noa,
                     prec->nova
                   };

    double d = 0;
    double *pDbl[2];

    min_n = (n[0] < n[1]) ? n[0] : n[1];

    if( min_n ) {
        i = j = 0;

        pDbl[0] = (double *)prec->a;
        pDbl[1] = (double *)prec->vala;

        while( i < min_n ) {
            d  = pow(10.0, (*(pDbl[0] + i)/10.0) );
            d /= 1000.0; /* In order to convert to [W]*/
            d /= 1000.0; /* Conversion to [kW]	  */
            /* printf("P[%3d] = %12.5lf kW\n", i, d); */
            *(pDbl[1] + j) = d; i++; j++;
        }
    }
    // Output wf nord = input wf nelm ==> we unwrap always, whether nord or not	
    prec->neva = min_n;
    return 0;
}


//==================================================================================================
//==================================================================================================

static	long	TrigA(aSubRecord* prec)
{
/* Find the Trigger Address in Carlos CB	*/
/*						*/
/* .............................................*/

register
    int    i, ii, j, k, nn;
    int    n[1] = { prec->noa };
unsigned
    int    nlm[1] = { prec->noa };
unsigned
    int    trigaddr = 0;
    int    found = 0;

    //int  REFI = 14; // C. Marques 4/15/20: Transition is the LSB
    int    REFQ = 15; // of the Reference channel I=[14]; Q=15;

    short  *pSh[1];
    short  sh = 0;

    // This CB contains 8 channels x I/Q = 16 values. Scan the buffer by blocks of
    // of 16 to find the TriggerAddr = whare value[0] LSB == 0

    pSh[0] = (short *)prec->a;
    /*
    for(i = 0; i < 2; i++ ) {
        for(j = 0; j < 16; j++) 
            printf("[%2d]=%08x=DEC%12d\n",
                j, (short)*(pSh[0]+i*16+j), (short)*(pSh[0]+i*16+j));
        printf("\n");
    }
    printf("SEARCHING FOR TRIGADDR ...\n");

    sh = *pSh[0] & 0x01;
    k = 0;
    while( ((*(pSh[0]+k)&0x01)==sh) && k < nelm[0] ) {
        //i = *(pSh[0]+k);
        //printf("***[%09d]=0x%08x=%12d DEC sh=%d\n", k, i, i, sh);
        k+=16;
    }

    //i = *(pSh[0]+k);
    //printf("*-*[%09d]=0x%08x=%12d DEC sh=%d\n", k, i, i, sh);

    switch( sh ) {
        case 0: found = 1;
                break;
        case 1: k+= nelm[0]/2;
                k = (k>=nelm[0])?k-nelm[0]:k;
                found = 2;
                break;
    }
    trigaddr = k;
    printf("1. TrigAddr=%d, found=%d, sh=0x%02x\n", trigaddr, found, sh);      
    */

    k = found = 0;
    if( !(*(pSh[0]+REFQ) & 0x01) ) {
        printf("Even: I[%d]=0x%02x ", REFQ, *(pSh[0]+REFQ) );
        while( !(*(pSh[0]+k+REFQ) & 0x01) && k < nlm[0] ) k+=16;
        // printf("k=%d, nelm[0]=%d, found=%d\n", k, nelm[0], found);
        if( k < nlm[0] ) { found = 1; trigaddr = k; /*printf("NFOUND1\n");*/ }
    }
    else {
        // printf("2: ");
        printf("Odd: I[%d]=0x%02x ", REFQ, *(pSh[0]+REFQ) );
        while(  (*(pSh[0]+k+REFQ) & 0x01) && (k < nlm[0]) ) k+=16;
        // printf("k=%d, nelm[0]=%d, found=%d\n", k, nelm[0], found);
	/*
        //===============================================================
        nn = (unsigned int)floor(k/16);
        ii = nn-1;
        if( ii < 0 ) ii =0;
        for(i = ii; i <= nn; i++ ) {
            for(j = 0; j < 16; j++)
                printf("-[%09d][%2d][%09d]=%08x=DEC%12d\n",
                i*16, j, i*16+j, (short)*(pSh[0]+i*16+j), (short)*(pSh[0]+i*16+j));
            printf("\n");
            printf("***************************************************\n");
            }
        //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        //===============================================================
	*/
        if( k < nlm[0] ) {
            found = 2;
            k += nlm[0]/2; k = (k>=nlm[0])? k-nlm[0] : k;
            trigaddr = k;
	    //printf("NFOUND2\n");
         }
    }
    printf("Last: I[%d]=0x%02x ", nlm[0]-1, *pSh[nlm[0]-1] );
    if( !found ) printf("TRIGADDR NOT FOUND\n");
    /*
    i = *(pSh[0]+k-1);
    printf("*[%09d]=0x%08x=%12d DEC\n", k-1, i, i);
    i = *(pSh[0]+k-0);
    printf("*[%09d]=0x%08x=%12d DEC\n", k  , i, i);

    nn = (unsigned int)floor(trigaddr/16);
    ii = nn-1;
    if( ii < 0 ) ii = 0;
    for(i = ii; i <= nn; i++ ) {
        for(j = 0; j < 16; j++)
            printf("-[%09d][%2d][%09d]=%08x=DEC%12d\n",
                i*16, j, i*16+j, (short)*(pSh[0]+i*16+j), (short)*(pSh[0]+i*16+j));
        printf("\n");
    }
    */
    // 268435456 = 256M = 0x10000000 short values
    //  16777216 =  16M = values long samples
    /*
    for(i = 16777216 - 3; i < 16777216; i++ ) {
        for(j = 0; j < 16; j++) 
            printf("[%09d][%2d][%09d]=%08x=DEC%12d\n",
                i*16, j, i*16+j, (short)*(pSh[0]+i*16+j), (short)*(pSh[0]+i*16+j));
        printf("\n");
    }
    */
    *(unsigned int *)prec->vala = trigaddr;
    prec->neva=1;
    return 0;
}

//==================================================================================================

static	long	PhaseShift(aSubRecord* prec)
{
/* Phase shift the input I/Q (INPA, INPB)	*/
/* by angle phi[deg] specified in INPC		*/
/* output values to OUTA, OUTB                  */
/* .............................................*/
    double iIn = *(double *)prec->a;
    double qIn = *(double *)prec->b;
    double phi = *(double *)prec->c;
    double complex a, b, u;

    a = iIn + qIn*I;;
    u =   0 + (phi*M_PI/180.0)*I;
    u = cexp(u);
    b = a*u;
  
    // printf("Record %s called PhaseShift(%p)\n", prec->name, (void*)prec);

    *(double *)prec->vala = creal(b);
    prec->neva = 1;
    *(double *)prec->valb = cimag(b);
    prec->nevb = 1;
    // printf("iIn=%8.3f qIn=%8.3f phi=%8.1f\n", creal(a), cimag(a), 180.0*carg(u)/M_PI);
    // printf("iUt=%8.3f qUt=%8.3f\n", creal(b), cimag(b));
  return 0;
}

//==================================================================================================

static registryFunctionRef asub_seq[] = {
    {"IQ2AP", (REGISTRYFUNCTION) &convert_iq2ap},
    {"AP2IQ", (REGISTRYFUNCTION) &convert_ap2iq},
    {"WG Gen", (REGISTRYFUNCTION) &gen_waveform},
    {"WG Init", (REGISTRYFUNCTION) &init_waveform},
    {"Wf Stats", (REGISTRYFUNCTION) &wf_stats},
    {"Phase Unwrap", (REGISTRYFUNCTION) &unwrap},
    {"Wf Pha Unwrap", (REGISTRYFUNCTION) &unPhase_wf},
    {"fbRampSegment", (REGISTRYFUNCTION) &fbRampSegment},
    {"WfSel", (REGISTRYFUNCTION) &wfSel},
    {"WfAlign", (REGISTRYFUNCTION) &align_wf},
    {"CircUnwr", (REGISTRYFUNCTION) &unwrap_wf},
    {"CircInvPairs", (REGISTRYFUNCTION) &invpair_wf},
    {"CircPart", (REGISTRYFUNCTION) &circpart_wf},
    {"CircDump", (REGISTRYFUNCTION) &circdump_wf},
    {"Decimate", (REGISTRYFUNCTION) &decimate_wf},
    {"dBm2kW", (REGISTRYFUNCTION) &dBm2kW},
    {"TrigA", (REGISTRYFUNCTION) &TrigA},
    {"PhsShft", (REGISTRYFUNCTION) &PhaseShift},
};


static
void calcRegister(void) {
    registryFunctionRefAdd(asub_seq, NELEMENTS(asub_seq));
}

#include <epicsExport.h>

epicsExportRegistrar(calcRegister);
