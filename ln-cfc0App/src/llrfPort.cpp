#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <complex>

#include <byteswap.h>
#include <epicsMath.h>
#include <string.h>
#include <time.h>
#include <dbDefs.h>
#include <epicsExit.h>
#include <epicsTime.h>
#include <epicsEndian.h>

#include <unistd.h>

using namespace std;

#include "llrfPort.h"
#include "iqmath.h"

#define PAIR std::make_pair

static
const char * ModelNames_[] =
{
    "Unknown",
    "XEM3001v1",
    "XEM3001v2",
    "XEM3010",
    "XEM3005",
    "XEM3001CL",
    "XEM3020",
    "XEM3050",
};

static
const char* ModelName(ok_BoardModel m)
{
  if      (m == 41 ) return "XEM7010 A50";
  else if (m == 42 ) return "XEM7010 A200";
  else if (m < 0 ||
          (size_t)m >= NELEMENTS(ModelNames_)) return "Unknown model";
  else                                         return ModelNames_[m];
}

char *okStrErr(int e) {
  int i = abs(e);
  char *okStrErr[21] = { (char *)"ok_NoError",
                       (char *)"ok_Failed",
                       (char *)"ok_Timeout",
                       (char *)"ok_DoneNotHigh",
                       (char *)"ok_TransferError",
                       (char *)"ok_CommunicationError",
                       (char *)"ok_InvalidBitstream",
                       (char *)"ok_FileError",
                       (char *)"ok_DeviceNotOpen",
                       (char *)"ok_InvalidEndpoint",
                       (char *)"ok_InvalidBlockSize",
                       (char *)"ok_I2CRestrictedAddress",
                       (char *)"ok_I2CBitError",
                       (char *)"ok_I2CNack",
                       (char *)"ok_I2CUnknownStatus",
                       (char *)"ok_UnsupportedFeature",
                       (char *)"ok_FIFOUnderflow",
                       (char *)"ok_FIFOOverflow",
                       (char *)"ok_DataAlignmentError",
                       (char *)"ok_InvalidResetProfile",
                       (char *)"ok_InvalidParameter" };

if( i >= 0 && i < 21 ) return okStrErr[i];
else return (char *)"Unknown error";
}

static
const char* ErrorName(ok_ErrorCode m)
{
  return (const char *)okStrErr(m);
}

int	WireInGet( okFrontPanel_HANDLE H, int chan, int verbose)
	{
	ok_ErrorCode		e;

	int			r = 0;
	int			ep, val, mask;	// FrontPanel/FrontPanel-FC7-3.0.10/API/okFrontPanelDLL.h
	
	ep	= 0x02;		// See Matlab function WireInGet
	val	= chan << 5;	// Selector in ep02 bits [9:5]
	mask	= 0x07E0;	// 31*32 - Changed in revision 77: bits are now [10:5], mask should be 0x07E0
				// July 19, 2012

	e = okFrontPanel_SetWireInValue(H, ep, val, mask);

	if(e != ok_NoError) {
		std::cerr << "OpalKelly: Error at SetWireInValue:" << e << " ";
		std::cerr << "EP = " << ep << " Val = 0x" << std::hex << val << " Mask = 0x" << mask << std::endl;
		}
	else	{
		if( verbose )
			{
			std::cout << "OpalKelly: SetWireInValue: ";
			std::cout << "EP = 0x" << setfill('0') << hex << setw(2) << ep;
			std::cout << " Val = 0x" << std::hex << val << " Mask = 0x" << mask;
			std::cout << " successfully!" << std::dec << std::endl;
			}
		}

	okFrontPanel_UpdateWireIns(H);
	okFrontPanel_UpdateWireOuts(H);
	r = okFrontPanel_GetWireOutValue(H, EP_WireIn_Mux_RB);
	if( verbose )
		{
		cout << "Read Wire Out EP = 0x" << setfill('0') << hex << setw(2) << EP_WireIn_Mux_RB << " ";
		cout << "Val = 0x" << setw(8) << hex << r << dec << endl;
		}
	return r;
	}

#define HWERR(ERR) hwError(ERR, __FILE__, __LINE__)

#define HWERR2(USR, ERR) hwError(ERR, __FILE__, __LINE__, USR)

#define NUM_PARAMS (&LAST_CMD - &FIRST_CMD + 1)

void	llrfPort::SDRAM_Reset(asynUser *pasynUser)
{
	int	ep  = EP_SDRAM;
	int	val = 0;
	int	mask= SDRAM_RESET;
	
        cout << "SDRAM_Reset1: Val=" << val << " Mask=" << mask << " EP=" << ep << endl;

	HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, ep, val, mask));
        okFrontPanel_UpdateWireIns(H);

        val = SDRAM_RESET;
        cout << "SDRAM_Reset2: Val=" << val << " Mask=" << mask << " EP=" << ep << endl;

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, ep, val, mask));
        okFrontPanel_UpdateWireIns(H);

	val = 0;
        cout << "SDRAM_Reset3: Val=" << val << " Mask=" << mask << " EP=" << ep << endl;

	HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, ep, val, mask));
        okFrontPanel_UpdateWireIns(H);
}

void llrfPort::SDRAM_RstEn(asynUser *pasynUser, int enable)
{
	int	ep  = EP_SDRAM;
	int	val = 0;
	int	mask= SDRAM_RESET;
        
	val = enable ? SDRAM_RESET : 0;

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, ep, val, mask));
	okFrontPanel_UpdateWireIns(H);
}

void llrfPort::SDRAM_RdEn(asynUser *pasynUser, int enable)
{
	int	ep  = EP_SDRAM;
	int	val = 0;
	int	mask= SDRAM_READ;
        
	val = enable ? SDRAM_READ : 0;

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, ep, val, mask));
	okFrontPanel_UpdateWireIns(H);

}

void llrfPort::SDRAM_WrEn(asynUser *pasynUser, int enable)
{
	int	ep  = EP_SDRAM;
	int	val = 0;
	int	mask= SDRAM_WRITE;

	val = enable ? SDRAM_WRITE : 0;

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, ep, val, mask));
	okFrontPanel_UpdateWireIns(H);
}

int llrfPort::SDRAM_Rd(asynUser *pasynUser)
{
register
	int	i;

	int	ep = EP_RAM;
	int	bl_size = CIRC_BYTES / 4;
	int	len = CIRC_BYTES;
	int	l = 0;
	int	nz= 0;

unsigned
	char	*p   = (unsigned char *)pCirc0;

    // printf("SDRAM_Rd: ep=0x%02x, CIRC_BYTES=%d, bl_size=%d\n", ep, len, bl_size);

    do {
        l = len;
        if( l > bl_size ) l = bl_size;

        l = okFrontPanel_ReadFromPipeOut(H, ep, bl_size, p);

        i = 0;
        while( i < l ) {
            if( *(p+i) != 0 ) nz = 1;
            i++;
            }

        if( l <= 0 ) {
            // printf("okFrontPanel_ReadFromBlockPipeOut returned %d\n", l);
            }
        else {
            // printf("okFrontPanel_ReadFromBlockPipeOut: read l =%d\n", l);
            len -= l;
            p += l;
            }
    } while( l > 0 && len > 0 );

/*******************************************************************************************************
The test pattern generates value 128 at buf[8387073 <-> MATLAB:buf(memsize/4 + 2)] which will be wrapped to Q[838705] value of channel 3(0...4)
I[838705] of channel 3(0...4) <-> MATLAB: buf(memsize/4 - 3)

jtagger@rfctrl01:/epics/iocs/OKLLRF1$ less iocBoot/iocbr-llrf/CIRCBUF_DATA.TXT
/838701

  838701     1024     -512     1024     -512     1024     -512     1024     -512     1024     -512 
  838702    -1024      512    -1024      512    -1024      512    -1024      512    -1024      512 
  838703     1024     -512     1024     -512     1024     -512     1024     -512     1024     -512 
  838704    -1024      512    -1024      512    -1024      512    -1024      512    -1024      512 
  838705     1024     -512     1024     -512     1024     -512     1024      128     1024     -512 
  838706    -1024        0    -1024      512    -1024      512        0        0        0        0 
  838707        0        0        0        0        0        0        0        0        0        0 

:q

    printf("SDRAM_Rd: CREATING A TEST PATTERN ---\n");

    i = 0;
    len = CIRC_SHORTS; **** 0x0100 0000 = 16 777 216 ****
short
    *pSh[1] = { (short *)pCirc0 };

    while( i < len)
	{
	if( i < len/2) **** 8 388 608 = 0x0080 0000 ****
		{
		if( i < len/8) **** 2 097 152 = 0x0020 0000 ****
			{
   			*(pSh[0] + i) = -2048; i++;
			*(pSh[0] + i) = +1024; i++;			
			}
		else
			{
			*(pSh[0] + i) = +1024; i++;
			*(pSh[0] + i) =  -512; i++;
			}
		}
	else
		{
		*(pSh[0] + i) = 0; i++;
		*(pSh[0] + i) = 0; i++;
		}
	}

    *(pSh[0] + 8388609) = 128;	**** 8388609 = memsize/4 + 2*(sizeof(short)) ****

    printf("SDRAM_Rd: END OF CREATING THE TEST PATTERN.\n");
*******************************************************************************************************/

    return l;
}

llrfPort::llrfPort(const char *portName, const char *serial, int debug)
    : asynPortDriver(portName, NUM_CHANNELS, NUM_PARAMS,
                     asynInt16ArrayMask | asynInt32Mask | asynFloat64Mask | asynFloat64ArrayMask | asynOctetMask | asynDrvUserMask,
                     asynInt16ArrayMask | asynInt32Mask | asynFloat64Mask | asynFloat64ArrayMask,
                     ASYN_MULTIDEVICE | ASYN_CANBLOCK, 
                     1, // autoconnect
                     0, // default priority
                     0)
    , serial(serial)
    , H(NULL)
    , nextdac(false)
    , zombie(false)
{
try {
    if(debug) {
        // early debugging
        pasynTrace->setTraceMask(pasynUserSelf, 0x09);
        pasynTrace->setTraceIOMask(pasynUserSelf, 0x04);
    }

	Log = LOGGING_ENABLED;
	FB_TrigReq = 0;

	for( int i = 0; i < NUM_CHANNELS; i++ )
		// zoom_v_scale[i] = (i == 4) ? 0.5 : 1.0;
		zoom_v_scale[i] = 1.0;

    std::fill(connected, connected+NUM_CHANNELS, false);

    std::cout << "Created OpalKelly port: Name=" << portName << " Serial:" << serial << " Debug=" << debug << endl;

    AERR(createParam("Model", asynParamOctet, &P_model));
    AERR(createParam("ID", asynParamOctet, &P_id));
    AERR(createParam("Major", asynParamInt32, &P_major));
    AERR(createParam("Minor", asynParamInt32, &P_minor));
    AERR(createParam("Serial", asynParamOctet, &P_serial));

//----------------------------------------------------------------------------
//	Nathan, March 14, 2012

    AERR(createParam("WinRB EP00", asynParamInt32, &P_WinRB_EP00));
    AERR(createParam("WinRB EP01", asynParamInt32, &P_WinRB_EP01));
    AERR(createParam("WinRB EP02", asynParamInt32, &P_WinRB_EP02));
    AERR(createParam("WinRB EP03", asynParamInt32, &P_WinRB_EP03));
    AERR(createParam("WinRB EP04", asynParamInt32, &P_WinRB_EP04));
    AERR(createParam("WinRB EP05", asynParamInt32, &P_WinRB_EP05));
    AERR(createParam("WinRB EP06", asynParamInt32, &P_WinRB_EP06));
    AERR(createParam("WinRB EP07", asynParamInt32, &P_WinRB_EP07));
    AERR(createParam("WinRB EP08", asynParamInt32, &P_WinRB_EP08));
    AERR(createParam("WinRB EP09", asynParamInt32, &P_WinRB_EP09));
    AERR(createParam("WinRB EP10", asynParamInt32, &P_WinRB_EP10));
    AERR(createParam("WinRB EP11", asynParamInt32, &P_WinRB_EP11));
    AERR(createParam("WinRB EP12", asynParamInt32, &P_WinRB_EP12));
    AERR(createParam("WinRB EP13", asynParamInt32, &P_WinRB_EP13));
    AERR(createParam("WinRB EP14", asynParamInt32, &P_WinRB_EP14));
    AERR(createParam("WinRB EP15", asynParamInt32, &P_WinRB_EP15));
    AERR(createParam("WinRB EP16", asynParamInt32, &P_WinRB_EP16));
    AERR(createParam("WinRB EP17", asynParamInt32, &P_WinRB_EP17));

//----------------------------------------------------------------------------
//	Nathan, June 1, 2012

    AERR(createParam("WinRB EP18", asynParamInt32, &P_WinRB_EP18));
    AERR(createParam("WinRB EP19", asynParamInt32, &P_WinRB_EP19));
    AERR(createParam("WinRB EP20", asynParamInt32, &P_WinRB_EP20));
    AERR(createParam("WinRB EP21", asynParamInt32, &P_WinRB_EP21));
    AERR(createParam("WinRB EP22", asynParamInt32, &P_WinRB_EP22));
    AERR(createParam("WinRB EP23", asynParamInt32, &P_WinRB_EP23));
    AERR(createParam("WinRB EP24", asynParamInt32, &P_WinRB_EP24));
    AERR(createParam("WinRB EP25", asynParamInt32, &P_WinRB_EP25));
    AERR(createParam("WinRB EP26", asynParamInt32, &P_WinRB_EP26));
    AERR(createParam("WinRB EP27", asynParamInt32, &P_WinRB_EP27));
    AERR(createParam("WinRB EP28", asynParamInt32, &P_WinRB_EP28));
    AERR(createParam("WinRB EP29", asynParamInt32, &P_WinRB_EP29));
    AERR(createParam("WinRB EP30", asynParamInt32, &P_WinRB_EP30));
    AERR(createParam("WinRB EP31", asynParamInt32, &P_WinRB_EP31));
    AERR(createParam("WinRB EP32", asynParamInt32, &P_WinRB_EP32));
    AERR(createParam("WinRB EP33", asynParamInt32, &P_WinRB_EP33));
    AERR(createParam("WinRB EP34", asynParamInt32, &P_WinRB_EP34));
    AERR(createParam("WinRB EP35", asynParamInt32, &P_WinRB_EP35));
    AERR(createParam("WinRB EP36", asynParamInt32, &P_WinRB_EP36));

//----------------------------------------------------------------------------
//	September 17, 2012
    AERR(createParam("WinRB EP37", asynParamInt32, &P_WinRB_EP37));
    AERR(createParam("WinRB EP38", asynParamInt32, &P_WinRB_EP38));
    AERR(createParam("WinRB EP39", asynParamInt32, &P_WinRB_EP39));
    AERR(createParam("WinRB EP40", asynParamInt32, &P_WinRB_EP40));
    AERR(createParam("WinRB EP41", asynParamInt32, &P_WinRB_EP41));
    AERR(createParam("WinRB EP42", asynParamInt32, &P_WinRB_EP42));
    AERR(createParam("WinRB EP43", asynParamInt32, &P_WinRB_EP43));
    AERR(createParam("WinRB EP44", asynParamInt32, &P_WinRB_EP44));
    AERR(createParam("WinRB EP45", asynParamInt32, &P_WinRB_EP45));
    AERR(createParam("WinRB EP46", asynParamInt32, &P_WinRB_EP46));
    AERR(createParam("WinRB EP47", asynParamInt32, &P_WinRB_EP47));
    AERR(createParam("WinRB EP48", asynParamInt32, &P_WinRB_EP48));
    AERR(createParam("WinRB EP49", asynParamInt32, &P_WinRB_EP49));
    AERR(createParam("WinRB EP50", asynParamInt32, &P_WinRB_EP50));
    AERR(createParam("WinRB EP51", asynParamInt32, &P_WinRB_EP51));
    AERR(createParam("WinRB EP52", asynParamInt32, &P_WinRB_EP52));
    AERR(createParam("WinRB EP53", asynParamInt32, &P_WinRB_EP53));
    AERR(createParam("WinRB EP54", asynParamInt32, &P_WinRB_EP54));
    AERR(createParam("WinRB EP55", asynParamInt32, &P_WinRB_EP55));
    AERR(createParam("WinRB EP56", asynParamInt32, &P_WinRB_EP56));
    AERR(createParam("WinRB EP57", asynParamInt32, &P_WinRB_EP57));
    AERR(createParam("WinRB EP58", asynParamInt32, &P_WinRB_EP58));
    AERR(createParam("WinRB EP59", asynParamInt32, &P_WinRB_EP59));
    AERR(createParam("WinRB EP60", asynParamInt32, &P_WinRB_EP60));
    AERR(createParam("WinRB EP61", asynParamInt32, &P_WinRB_EP61));
    AERR(createParam("WinRB EP62", asynParamInt32, &P_WinRB_EP62));
    AERR(createParam("WinRB EP63", asynParamInt32, &P_WinRB_EP63));
//----------------------------------------------------------------------------
//	Apr 5, 2012
	AERR(createParam("WireInADCMUX", asynParamInt32, &P_WinADCMUX));

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//	Apr 23, 2012
	AERR(createParam("Tuner Ph Offs", asynParamInt32, &P_Win_Tuner_PhOffset));

//----------------------------------------------------------------------------
//	March 20, 2012

	AERR(createParam("WireOut EP20", asynParamInt32, &P_WO_EP20));
	AERR(createParam("WireOut EP21", asynParamInt32, &P_WO_EP21));
	AERR(createParam("WireOut EP22", asynParamInt32, &P_WO_EP22));
	AERR(createParam("WireOut EP23", asynParamInt32, &P_WO_EP23));
	AERR(createParam("WireOut EP24", asynParamInt32, &P_WO_EP24));
	AERR(createParam("WireOut EP25", asynParamInt32, &P_WO_EP25));
	AERR(createParam("WireOut EP26", asynParamInt32, &P_WO_EP26));
	AERR(createParam("WireOut EP27", asynParamInt32, &P_WO_EP27));
	AERR(createParam("WireOut EP28", asynParamInt32, &P_WO_EP28));
	AERR(createParam("WireOut EP29", asynParamInt32, &P_WO_EP29));
	AERR(createParam("WireOut EP2a", asynParamInt32, &P_WO_EP2a));
	AERR(createParam("WireOut EP2b", asynParamInt32, &P_WO_EP2b));
	AERR(createParam("WireOut EP2c", asynParamInt32, &P_WO_EP2c));
	AERR(createParam("WireOut EP2d", asynParamInt32, &P_WO_EP2d));
	AERR(createParam("WireOut EP2e", asynParamInt32, &P_WO_EP2e));
	AERR(createParam("WireOut EP2f", asynParamInt32, &P_WO_EP2f));
	AERR(createParam("WireOut EP30", asynParamInt32, &P_WO_EP30));
	AERR(createParam("WireOut EP31", asynParamInt32, &P_WO_EP31));
	AERR(createParam("WireOut EP32", asynParamInt32, &P_WO_EP32));
	AERR(createParam("WireOut EP33", asynParamInt32, &P_WO_EP33));
	AERR(createParam("WireOut EP34", asynParamInt32, &P_WO_EP34));
	AERR(createParam("WireOut EP35", asynParamInt32, &P_WO_EP35));
	AERR(createParam("WireOut EP36", asynParamInt32, &P_WO_EP36));
	AERR(createParam("WireOut EP37", asynParamInt32, &P_WO_EP37));
	AERR(createParam("WireOut EP38", asynParamInt32, &P_WO_EP38));
	AERR(createParam("WireOut EP39", asynParamInt32, &P_WO_EP39));
	AERR(createParam("WireOut EP3a", asynParamInt32, &P_WO_EP3a));
	AERR(createParam("WireOut EP3b", asynParamInt32, &P_WO_EP3b));
	AERR(createParam("WireOut EP3c", asynParamInt32, &P_WO_EP3c));
	AERR(createParam("WireOut EP3d", asynParamInt32, &P_WO_EP3d));
	AERR(createParam("WireOut EP3e", asynParamInt32, &P_WO_EP3e));
	AERR(createParam("WireOut EP3f", asynParamInt32, &P_WO_EP3f));
        AERR(createParam("TrigOut EP60", asynParamInt32, &P_TO_EP60));

//----------------------------------------------------------------------------
//	Nathan, June 1, 2012
	AERR(createParam("RefPr Ena", asynParamInt32, &P_RefPr_En));

//----------------------------------------------------------------------------
//	Nathan, March 15, 2012

    AERR(createParam("FF Enable", asynParamInt32, &P_ff_enable));
    AERR(createParam("Scope Trigger", asynParamInt32, &P_scope_trigger));
    AERR(createParam("Scope Trig Run", asynParamInt32, &P_scope_trigger_run));

//----------------------------------------------------------------------------
    AERR(createParam("Load", asynParamInt32, &P_load));
    AERR(createParam("Reset", asynParamInt32, &P_reset));
    AERR(createParam("SDRAM Reset", asynParamInt32, &P_sdramReset));
    AERR(createParam("Update", asynParamInt32, &P_update));

    AERR(createParam("CNT T", asynParamFloat64, &P_cnt_t));
    AERR(createParam("ADC count", asynParamInt32, &P_adc_count));
    AERR(createParam("ADC time", asynParamFloat64, &P_adc_time));
    AERR(createParam("Trig time", asynParamFloat64, &P_trig_time));

    AERR(createParam("RF Freq", asynParamFloat64, &P_rf_clk));

    AERR(createParam("Bit Directory", asynParamOctet, &P_bit_dir));
    AERR(createParam("Bit File", asynParamOctet, &P_bit_file));

    AERR(createParam("Cavity Type", asynParamInt32, &P_cav_type));
/*********************************************************/
/* March 13, 2012					 */
/*							 */
/* EP_CONTROL, 0x03, bit[14], now controls the limiter	 */
/* 0 = disable, 1 = enable				 */
/* CW/Single is controlled by bit[09], ff_duty_cycle	 */
/*
    AERR(createParam("CW", asynParamInt32, &P_cw));
							*/
    AERR(createParam("Limiter", asynParamInt32, &P_limiter));
/********************************************************/
/*							*/
/* March 1, 2012					*/
/*	#define	CONTROL_FF_DUTY_CYCLE	0x0200		*/
    AERR(createParam("FF DutyCycle", asynParamInt32, &P_ff_dutycycle));
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* April 9, 2012					*/
    AERR(createParam("ZoomSelector", asynParamInt32, &P_zoom_selector));
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* April 13, 2012					*/
    AERR(createParam("Rampinterval", asynParamInt32, &P_rampinterval_SP));
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* September 25, 2020					*/
    AERR(createParam("naFreq", asynParamInt32, &P_naFreq_SP));
    AERR(createParam("naPII",  asynParamInt32, &P_naPII_SP));
    AERR(createParam("naPIC0", asynParamInt32, &P_naPIC0_SP));
    AERR(createParam("naPIC1", asynParamInt32, &P_naPIC1_SP));
    AERR(createParam("naInts", asynParamInt32, &P_naIntensity_SP));
    AERR(createParam("NA I",   asynParamInt32, &P_na_I));
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* October 14, 2020					*/
    AERR(createParam("pgEnbl", asynParamInt32, &P_PG_Enbl_SP));

    AERR(createParam("pgKp",   asynParamFloat64, &P_PG_AltKp_SP));
    AERR(createParam("pgKi",   asynParamFloat64, &P_PG_AltKi_SP));

    AERR(createParam("pgTime", asynParamInt32, &P_PG_SwTime_SP));
    AERR(createParam("pgGO",   asynParamInt32, &P_PG_GO));
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* April 16, 2012					*/
    AERR(createParam("FB RampTrigger", asynParamInt32, &P_fbramptrig));
/*							*/
/********************************************************/

    AERR(createParam("FF Mode", asynParamInt32, &P_ff_mode));

    AERR(createParam("Scope Zoom", asynParamInt32, &P_scope_zoom));
    AERR(createParam("Scope Delay", asynParamFloat64, &P_scope_dly));

    AERR(createParam("Feedback Enable", asynParamInt32, &P_fb));

    AERR(createParam("I Gain", asynParamFloat64, &P_gain_I));
    AERR(createParam("P gain", asynParamFloat64, &P_gain_P));

    AERR(createParam("Loop Phase", asynParamFloat64, &P_loop_phase));

    AERR(createParam("DAC Time", asynParamFloat64, &P_ff_T));

    AERR(createParam("SP I", asynParamFloat64, &P_sp_I));
    AERR(createParam("SP Q", asynParamFloat64, &P_sp_Q));

/*------------------------------------------------------*/
/********************************************************/
/* JUNE 4, 2015: Added second set of I/Q setpoints      */
/* for phase toggling by 100 deg (G.Wang experiment     */
/* EP 0, bit 0x08 enables toggling (the second set)     */
/* EP 0x15, 0x16 are the second set of values           */

    AERR(createParam("SP I2", asynParamFloat64, &P_spI2));
    AERR(createParam("SP Q2", asynParamFloat64, &P_spQ2));

    AERR(createParam("Enbl I2Q2", asynParamInt32, &P_EnblI2Q2));

/* SEP. 25. 2017: Added External Circular Buffer Trigger*/
/* Enable (bit 9 to EP SDRAM                            */
    AERR(createParam("Enbl EBT", asynParamInt32, &P_EnblExtCbufTr));

/********************************************************/
/*------------------------------------------------------*/
/*------------------------------------------------------*/
/********************************************************/
/* MAR 15, 2016: Added two signed integers for          */
/* debugging the Limiter: Ep 0x17 = Limiter Level       */
/* Ep 0x1A = Number of clocks                           */

    AERR(createParam("SP LL", asynParamInt32, &P_LimLev));
    AERR(createParam("SP NL", asynParamInt32, &P_NLimClks));

/********************************************************/
/*------------------------------------------------------*/

    AERR(createParam("SP_I_PVAL", asynParamFloat64, &P_sp_IPVAL));
    AERR(createParam("SP_Q_PVAL", asynParamFloat64, &P_sp_QPVAL));

    AERR(createParam("FF IQ", asynParamFloat64, &P_ff_IQ));
    AERR(createParam("FB IQ", asynParamFloat64, &P_fb_IQ));

    AERR(createParam("Ref Phase", asynParamFloat64, &P_ref_phase));
    AERR(createParam("Scope T", asynParamFloat64, &P_adc_T));

    AERR(createParam("ADC I", asynParamFloat64, &P_adc_I));
    AERR(createParam("ADC Q", asynParamFloat64, &P_adc_Q));

    AERR(createParam("TS RdAll",   asynParamInt32, &P_TS_RdAll));
    AERR(createParam("TS Word0 1", asynParamInt32, &P_TS_RdWord0_1));
    AERR(createParam("TS Word1 0", asynParamInt32, &P_TS_RdWord1_0));
    AERR(createParam("TS Word2 0", asynParamInt32, &P_TS_RdWord2_0));

    AERR(createParam("RAM RstEn", asynParamInt32, &P_ramRstEn));
    AERR(createParam("RAM RdEn", asynParamInt32, &P_ramRdEn));
    AERR(createParam("RAM WrEn", asynParamInt32, &P_ramWrEn));
    AERR(createParam("RAM Arm",  asynParamInt32, &P_ramArm));
    AERR(createParam("RAM ClrExT",  asynParamInt32, &P_ramClrExTrig));
    AERR(createParam("RAM TS",  asynParamInt32, &P_ram_TS));

    AERR(createParam("CircTripA",  asynParamInt32, &P_CircBufSWTripA));
    AERR(createParam("CircTripB",  asynParamInt32, &P_CircBufSWTripB));

    AERR(createParam("CIRC IQ",asynParamInt32, &P_circ_IQ));
    AERR(createParam("RAM IQ", asynParamInt32, &P_ram_IQ));
    AERR(createParam("RAM WrapCnt", asynParamInt32, &P_ram_wrap));
    AERR(createParam("RAM Lastpage", asynParamInt32, &P_ram_Lastpage));
    AERR(createParam("RAM TrigSrc", asynParamInt32, &P_ram_TrSrc));

    AERR(createParam("RAM I", asynParamFloat64, &P_ram_I));
    AERR(createParam("RAM Q", asynParamFloat64, &P_ram_Q));

/********************************************************/
/*							*/
/* April 23, 2012					*/
	AERR(createParam("TrigIn:EP40_01", asynParamInt32, &P_Force_SoftTrip));
	AERR(createParam("TrigIn:EP40_02", asynParamInt32, &P_Reset_SoftTrip));

	AERR(createParam("TrigIn:EP40_09", asynParamInt32, &P_Force_RFInhTrip));
	AERR(createParam("TrigIn:EP40_0a", asynParamInt32, &P_Reset_RFInh));

	AERR(createParam("TrigIn:EP40_0d", asynParamInt32, &P_Force_QUENCHTrip));
	AERR(createParam("TrigIn:EP40_0e", asynParamInt32, &P_Reset_QUENCH));

	AERR(createParam("TrigIn:EP41_00", asynParamInt32, &P_Force_VacTrip));
	AERR(createParam("TrigIn:EP41_01", asynParamInt32, &P_Reset_VacTrip));

	AERR(createParam("TrigIn:EP41_02", asynParamInt32, &P_Force_PPSTrip));
	AERR(createParam("TrigIn:EP41_03", asynParamInt32, &P_Reset_PPS));

	AERR(createParam("TrigIn:EP41_04", asynParamInt32, &P_Force_AVAGO2Trip));
	AERR(createParam("TrigIn:EP41_05", asynParamInt32, &P_Reset_AVAGO2));
/********************************************************/
/*							*/
/* September 25, 2020					*/
        AERR(createParam("TrigIn:EP41_0a", asynParamInt32, &P_na_Start));
	AERR(createParam("TrigIn:EP41_0b", asynParamInt32, &P_na_Stop));
/********************************************************/
/* Dec. 3, 2012						*/
	AERR(createParam("TrigIn:EP41_08", asynParamInt32, &P_TSNext));
	AERR(createParam("TrigIn:EP41_09", asynParamInt32, &P_TSReset));
/*							*/
/********************************************************/
/********************************************************/
/* Aug. 24, 2016						*/
	AERR(createParam("TrIn:EP41_8000", asynParamInt32, &P_Force_Limit));
/*							*/
/********************************************************/
/********************************************************/
/* Sep. 24, 2018					*/
	AERR(createParam("TopTrDly:EP0C", asynParamInt32, &P_TopTrDly));
/*							*/
/********************************************************/
/********************************************************/
/* Feb. 27, 2020					*/
        AERR(createParam("okSNo", asynParamOctet, &P_ok_SN));
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* March 22, 2017: Create 8 SP params and 8 RB params	*/
/* Plus one for Wr and one for Rd command.              */
	AERR(createParam("EQ_EP1_SP", asynParamInt32, &P_EQ_EP1_SP));
	AERR(createParam("EQ_EP2_SP", asynParamInt32, &P_EQ_EP2_SP));

	AERR(createParam("EQ_EP3_SP", asynParamInt32, &P_EQ_EP3_SP));
	AERR(createParam("EQ_EP4_SP", asynParamInt32, &P_EQ_EP4_SP));

	AERR(createParam("EQ_EP5_SP", asynParamInt32, &P_EQ_EP5_SP));
	AERR(createParam("EQ_EP6_SP", asynParamInt32, &P_EQ_EP6_SP));

	AERR(createParam("EQ_EP7_SP", asynParamInt32, &P_EQ_EP7_SP));
	AERR(createParam("EQ_EP8_SP", asynParamInt32, &P_EQ_EP8_SP));

	AERR(createParam("EQ_EP9_SP", asynParamInt32, &P_EQ_EP9_SP));
	AERR(createParam("EQ_EPA_SP", asynParamInt32, &P_EQ_EPA_SP));

	AERR(createParam("EQ_EPB_SP", asynParamInt32, &P_EQ_EPB_SP));
	AERR(createParam("EQ_EPC_SP", asynParamInt32, &P_EQ_EPC_SP));

	AERR(createParam("EQ_EPD_SP", asynParamInt32, &P_EQ_EPD_SP));
	AERR(createParam("EQ_EPE_SP", asynParamInt32, &P_EQ_EPE_SP));

	AERR(createParam("EQ_EPF_SP", asynParamInt32, &P_EQ_EPF_SP));

	AERR(createParam("EQ_EP1_RB", asynParamInt32, &P_EQ_EP1_RB));
	AERR(createParam("EQ_EP2_RB", asynParamInt32, &P_EQ_EP2_RB));

	AERR(createParam("EQ_EP3_RB", asynParamInt32, &P_EQ_EP3_RB));
	AERR(createParam("EQ_EP4_RB", asynParamInt32, &P_EQ_EP4_RB));

	AERR(createParam("EQ_EP5_RB", asynParamInt32, &P_EQ_EP5_RB));
	AERR(createParam("EQ_EP6_RB", asynParamInt32, &P_EQ_EP6_RB));

	AERR(createParam("EQ_EP7_RB", asynParamInt32, &P_EQ_EP7_RB));
	AERR(createParam("EQ_EP8_RB", asynParamInt32, &P_EQ_EP8_RB));

	AERR(createParam("EQ_EP9_RB", asynParamInt32, &P_EQ_EP9_RB));
	AERR(createParam("EQ_EPA_RB", asynParamInt32, &P_EQ_EPA_RB));

	AERR(createParam("EQ_EPB_RB", asynParamInt32, &P_EQ_EPB_RB));
	AERR(createParam("EQ_EPC_RB", asynParamInt32, &P_EQ_EPC_RB));

	AERR(createParam("EQ_EPD_RB", asynParamInt32, &P_EQ_EPD_RB));
	AERR(createParam("EQ_EPE_RB", asynParamInt32, &P_EQ_EPE_RB));

	AERR(createParam("EQ_EPF_RB", asynParamInt32, &P_EQ_EPF_RB));

	AERR(createParam("EQ_Rd",   asynParamInt32, &P_EQ_Rd));
	AERR(createParam("EQ_Wr",   asynParamInt32, &P_EQ_Wr));
        AERR(createParam("EQ_Enbl", asynParamInt32, &P_EQ_Enbl));

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

    AERR(setIntegerParam(P_TO_EP60, 0));

    AERR(setDoubleParam(P_rf_clk, 499.68));
    AERR(setIntegerParam(P_limiter, 1));
    AERR(setIntegerParam(P_fb, 0));
    AERR(setDoubleParam(P_gain_I, 0.05));
    AERR(setDoubleParam(P_gain_P, 0.05));
    AERR(setIntegerParam(P_scope_zoom, 0));
    AERR(setIntegerParam(P_scope_trigger_run, 0));
    AERR(setIntegerParam(P_zoom_selector, 14));
    AERR(setDoubleParam(P_loop_phase, 0));

/*------------------------------------------------------*/
/********************************************************/
/* JUNE 4, 2015: Added second set of I/Q setpoints      */
/* for phase toggling by 100 deg (G.Wang experiment     */
/* EP 0, bit 0x08 enables toggling (the second set)     */
/* EP 0x15, 0x16 are the second set of values           */
    AERR(setDoubleParam (P_spI2,     0));
    AERR(setDoubleParam (P_spQ2,     0));
    AERR(setIntegerParam(P_EnblI2Q2, 0));
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/********************************************************/

    AERR(setStringParam(P_serial, serial));

    // size scratch buffer for largest
    scratch.resize(ADC_LENGTH/2, 0);

    wfs[PAIR(P_ff_T,0)] = new std::vector<double>(DAC_COUNT, 0.0);

    wfs[PAIR(P_sp_I,0)] = new std::vector<double>(DAC_COUNT, 0.0);
    wfs[PAIR(P_sp_Q,0)] = new std::vector<double>(DAC_COUNT, 0.0);

    for(int i = 0; i < NUM_CHANNELS; i++) {
        wfs[PAIR(P_adc_I,i)] = new std::vector<double>(DAC_COUNT, 0.0);
        wfs[PAIR(P_adc_Q,i)] = new std::vector<double>(DAC_COUNT, 0.0);
    }

    wfs[PAIR(P_cnt_t,0)] = new std::vector<double>(DAC_COUNT, 0.0);

    wfs[PAIR(P_adc_T,0)] = new std::vector<double>(DAC_COUNT, 0.0);

    double rf_freq = 499.68;
    double period  = 1.0 / rf_freq;
unsigned
    int    zoomsel = 14;

    AERR(getDoubleParam(P_rf_clk, &rf_freq));
    AERR(getIntegerParam(P_zoom_selector, (int *)&zoomsel));

    period = 1.0/rf_freq;

    // period *= 100.0; // DAC table period is 100x DAC sample clock

   std::vector<double>::iterator fill=wfs[PAIR(P_cnt_t, 0)]->begin();
   for(int i = 0; i < DAC_COUNT; ++i) *fill++ = i;

   fill=wfs[PAIR(P_ff_T,0)]->begin();

   for(int i=0; i<DAC_COUNT; ++i) {
   /* SEP 27, 2012 - Change the x-axis of FF & FB tables to counts: 0 ... 512
      to match the scopes and readback channels ... all the equations in tables have to follow ...*/
   // period = 1.63845; // ms == 323769*25.6 = 838886.4 us / 512 at zoom = 14 <== 2^14+1 = 32769
   /* *fill++ = i*period; */
      *fill++ = i;
    }

    fill=wfs[PAIR(P_adc_T,0)]->begin();

    period = (50.0 / rf_freq)*512.0/2.0; // 25.6 [us] at 500 MHz

    if(P_zoom_selector > 0) period *= (double)(1 << (zoomsel+1))+1;

    period/= 512.0;

    period*= 0.001; 			 // convert to ms

    for(int i=0; i<DAC_COUNT; ++i) {
        /* *fill++ = i;	*/
	*fill++ = i*period;
    }

   /*********************/
   /* Circular buffer	*/
   cout << "Trying to allocate pCirc0, size=0x" << setfill('0') << hex << setw(8) << CIRC_DBL << endl;
   double *pDbl0 = (double *)calloc( CIRC_DBL, sizeof(double) );
   pCirc0 = (short *)pDbl0;
   cout << "Allocated pCirc0 = 0x" << hex << setfill('0') << setw(8) << (short *)pCirc0 << endl;
   /*********************/

    epicsAtExit(&shutdown, (void*)this);
} catch(std::exception& e) {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "%s: Failed to initialize port\nError: %s\n",
              portName, e.what());
    stop();
    }
}

llrfPort::~llrfPort()
{
    stop();
}

void llrfPort::shutdown(void *raw)
{
    llrfPort *port=(llrfPort*)raw;
    try{
        port->stop();
    }catch(std::exception& e) {
        std::cerr<<port->portName<<": Shutdown error: "<<e.what()<<"\n";
    }
}

void llrfPort::stop()
{
    lock();
    if(zombie) {
        unlock();
        return;
    }
    zombie=true;

    for(wfs_t::const_iterator it=wfs.begin(); it!=wfs.end(); ++it) {
        delete it->second;
    }
    wfs.clear();

    okFrontPanel_Destruct(H);

    unlock();
}

asynStatus llrfPort::connect(asynUser *pasynUser)
{
try {
    FLOW(pasynUser);
    int addr;

    AERR(getAddress(pasynUser, &addr));

    if(zombie) {
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s: Can't connect to zombie...\n", portName);
        return asynError;
    }
    if(connected[addr]) {
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s: Already open\n", portName);
        return asynError;
    }

    // if device already connected then done
    if(connected[0]) {
        connected[addr] = true;
        return asynPortDriver::connect(pasynUser);
    }

    H = okFrontPanel_Construct();
    if(!H)
        throw std::bad_alloc();

    error_t err = okFrontPanel_OpenBySerial(H, serial.c_str());
    if(err!=ok_NoError) {
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s: connect error: %s\n",
                  portName, ErrorName(err));
        return asynError;
    }

    AERR(setStringParam(P_model, ModelName(okFrontPanel_GetBoardModel(H))));
    char tmp[40]; // Hope this is enough...
    okFrontPanel_GetDeviceID(H, tmp);
    tmp[39]='\0';
    setStringParam(P_id, tmp);

    AERR(setIntegerParam(P_major, okFrontPanel_GetDeviceMajorVersion(H)));
    AERR(setIntegerParam(P_minor, okFrontPanel_GetDeviceMinorVersion(H)));

    AERR(setIntegerParam(P_adc_count, 0));

    init(pasynUser);

    connected[0] = true;
    connected[addr] = true;
    if(addr!=0)
        asynPortDriver::connect(pasynUserSelf); // self is connected to addr 0

    asynPrint(pasynUser, ASYN_TRACE_FLOW, "%s: connected\n", portName);
    return asynPortDriver::connect(pasynUser);
}CATCHALL(pasynUser)
}

asynStatus llrfPort::disconnect(asynUser *pasynUser)
{
try {
    int addr;
    FLOW(pasynUser);

    AERR(getAddress(pasynUser, &addr));

    if(!connected[addr]) {
        asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s: Already closed\n", portName);
        return asynError;
    }
    connected[addr] = false;

    if(addr==0) {
        // zero disconnects everyone
        for(int i=1; i<NUM_CHANNELS; i++) {
            AsynUser usr(portName, i);
            connected[i]=false;
            asynPortDriver::disconnect(usr);
        }
    }

    okFrontPanel_Destruct(H);
    H=NULL;

    return asynPortDriver::disconnect(pasynUser);
}CATCHALL(pasynUser)
}

void llrfPort::hwError(error_t err, const char* file, int line,
                       asynUser *usr)
{
    if(!usr)
        usr=pasynUserSelf;
    if(err==ok_NoError)
        return;
    switch(err) {
    case ok_FileError:
        // occurs when loading a new bit file
        break;
    default:
        disconnect(usr);
    }

    std::ostringstream msg;
    msg<<portName<<": "<<ErrorName(err)<<": on line "<<line;
    throw std::runtime_error(msg.str());
}

void llrfPort::init(asynUser *pasynUser)
{
    // DAC length
    HWERR2(pasynUser,
           okFrontPanel_SetWireInValue(H, EP_LENGTH,
                                          2048,
                                          0xffff));

    // ??? matlab script does this
    HWERR2(pasynUser,
           okFrontPanel_SetWireInValue(H, EP_CONTROL,
                                          4<<CONTROL_DEBUG_shft,
                                          CONTROL_DEBUG_mask));

    // Setup 1-wire
    HWERR2(pasynUser,
           okFrontPanel_SetWireInValue(H, EP_1WIRE_CMD,
                                          51,
                                          0xffff));

    okFrontPanel_UpdateWireIns(H);

    HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS, ACTIONS_UPDATE_DS1825) );

    okFrontPanel_UpdateWireOuts(H);

    epicsInt16 wire1[4];
    for(int i=0; i<4; i++) {
        wire1[i] = okFrontPanel_GetWireOutValue(H, EP_1WIRE_RES_0 + i);
    }
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
              "%s: 1wire %02x %02x %02x %02x\n",
              portName, wire1[0], wire1[1], wire1[2], wire1[3]);

    nextdac=false;
    // initially run from DAC buffer B

    // move read pointer to B
    HWERR2(pasynUser,
           okFrontPanel_SetWireInValue(H,EP_CONTROL,
                                          CONTROL_DAC_READ,
                                          CONTROL_DAC_READ));

    // move write pointer to A
    HWERR2(pasynUser,
           okFrontPanel_SetWireInValue(H,EP_CONTROL,
                                          0 << CONTROL_DAC_WRITE_shft,
                                          CONTROL_DAC_WRITE_mask));

/* Quick fix by Michael, March 14, 2012

    HWERR2(pasynUser,
           okFrontPanel_SetWireInValue(H,EP_CONTROL,
                                          CONTROL_FF_ENABLE,
                                          CONTROL_FF_ENABLE));

	Let's also trig the ADCs!
*/

    okFrontPanel_UpdateWireIns(H);
}

asynStatus llrfPort::writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual)
{
try {
    FLOW(pasynUser);
    int addr;
    int function = pasynUser->reason;

    AERR(getAddress(pasynUser, &addr));

    if( function == P_bit_dir )
	{
	strncpy( bitdirname, value, sizeof(bitdirname) );
	// cout << "writeOctet: bitdir:" << value << endl;
	}
    else
    if( function == P_bit_file )
	{
	strncpy( bitfilename, value, sizeof(bitfilename) );
        // cout << "writeOctet: bitfile:" << value << endl;
	}

    AERR(setStringParam(addr, function, value));

    *nActual = strlen(value);

//  AERR(callParamCallbacks());

    return asynSuccess;

   }CATCHALL(pasynUser)
}

asynStatus llrfPort::readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int *eomReason)
{
try {
    FLOW(pasynUser);
    int addr;
    int function = pasynUser->reason;
    ok_ErrorCode  okErr;
    okTDeviceInfo okDevInfo;

    AERR(getAddress(pasynUser, &addr));

    if( function == P_bit_dir )
	{
	// strncpy( bitdirname, value, sizeof(bitdirname) );
	// cout << "writeOctet: bitdir:" << value << endl;
	}
    else
    if( function == P_bit_file )
	{
	//strncpy( bitfilename, value, sizeof(bitfilename) );
        // cout << "writeOctet: bitfile:" << value << endl;
	}
    else
    if( function == P_ok_SN )
	{
        okErr = okFrontPanel_GetDeviceInfo(H, &okDevInfo);
        if( !okErr ) strncpy( value, okDevInfo.serialNumber, 40);
        else         strcpy( value, "0000000000");
	}

    AERR(setStringParam(addr, function, value));

    *nActual = strlen(value);

    //AERR(callParamCallbacks());

    return asynSuccess;

   }CATCHALL(pasynUser)
}

asynStatus llrfPort::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    double	dt[3] = {0, 0, 0};
    int         ep   = 0;
    int         val  = value;

unsigned
    int        eQ_Param[16];
    int        eQi;
unsigned
    short      *p_eQsh;

try {
    FLOW(pasynUser);
    int addr, function = pasynUser->reason;

    AERR(getAddress(pasynUser, &addr));

    if(function==P_load) {
        char bitfile[80];

        AERR(getStringParam(P_bit_dir, NELEMENTS(bitfile), bitfile));
        strcat(bitfile,"/");
        size_t blen=strlen(bitfile);
        AERR(getStringParam(P_bit_file, NELEMENTS(bitfile)-blen, bitfile+blen));

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: Load bitfile: '%s'\n",
                  portName, bitfile);

        lock();

	cout << "P_load: 1. Loaded bitfile:" << bitfile << endl;

        // HWERR2(pasynUser, okFrontPanel_LoadDefaultPLLConfiguration(H) );
        HWERR2(pasynUser, okFrontPanel_ConfigureFPGA(H, bitfile) );

	cout << "P_load: 2. Loaded bitfile:" << bitfile << endl;

        ts[0].tv_sec = 		0;
        ts[0].tv_nsec=	500000000; // Undocumented delay: pause(0.5)
        ts[1].tv_sec = 		0;
        ts[1].tv_nsec=		0;
        nanosleep( &ts[0], &ts[1]);

        cout << "P_load: 3. Loaded bitfile:" << bitfile << endl;

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, 0x10, 0x43, 0xffff) );	// Maxim DS1825
        okFrontPanel_UpdateWireIns(H);

        cout << "P_load: 4. Loaded bitfile:" << bitfile << endl;

        HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS, ACTIONS_RESET) );

        okFrontPanel_UpdateTriggerOuts(H);
        ts[0].tv_sec = 		0;
        ts[0].tv_nsec=	500000000; // Undocumented delay: pause(0.5)
        ts[1].tv_sec = 		0;
        ts[1].tv_nsec=		0;
        nanosleep( &ts[0], &ts[1]);
        unlock();

        cout << "P_load: 5. Loaded bitfile:" << bitfile << endl;

        init(pasynUser);

    } else if(function==P_reset) {

        lock();
        HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS, ACTIONS_RESET) );
        okFrontPanel_UpdateTriggerOuts(H);
        ts[0].tv_sec = 		0;
        ts[0].tv_nsec=	500000000; // Undocumented delay: pause(0.5)
        ts[1].tv_sec = 		0;
        ts[1].tv_nsec=		0;
        nanosleep( &ts[0], &ts[1]);

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: Global RESET:\n",
                  portName);
        unlock();

    } else if(function==P_sdramReset) {

        lock();

	SDRAM_Reset(pasynUser);

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: SDRAM Reset:\n",
                  portName);
        unlock();
    } else if(function==P_ramRstEn) {
        lock();

	SDRAM_RstEn(pasynUser, value);

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: SDRAM RstEn:\n",
                  portName);
        unlock();
    } else if(function==P_ramRdEn) {

        lock();

	SDRAM_RdEn(pasynUser, value);

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: SDRAM RdEn:\n",
                  portName);
        unlock();

    } else if(function==P_ramWrEn) {

        lock();

	SDRAM_WrEn(pasynUser, value);

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: SDRAM WrEn:\n",
                  portName);
        unlock();

    } else if(function==P_ramArm) {

	ep = EP_ACTIONS;	val = value ? ACTIONS_RAM_START : 0;

	if( val ) HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	// printf("Reset SDRAM Arm: ep=0x%04x, value=0x%04x\n", ep, val);

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: SDRAM Arm:\n",
	                  portName);

    } else if(function==P_CircBufSWTripA) {

	ep = EP_ACTIONS;	val = value ? ACTIONS_CIRCA_TRIP : 0;

	if( val ) HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	// printf("CircBufSWTripA: ep=0x%04x, value=0x%04x\n", ep, val);

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: CircBufSWTripA:\n",
	                  portName);

    } else if(function==P_CircBufSWTripB) {

	ep = EP_ACTIONS;	val = value ? ACTIONS_CIRCB_TRIP : 0;

	if( val ) HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	// printf("CircBufSWTripB: ep=0x%04x, value=0x%04x\n", ep, val);

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: CircBufSWTripB:\n",
	                  portName);

    } else if(function==P_update) {
        okFrontPanel_UpdateWireIns(H);
        okFrontPanel_UpdateWireOuts(H);

//**************************************************************************************************************
    int TrigOutVal = okFrontPanel_IsTriggered(H, EP_STATUS, 0x02);

    if( TrigOutVal )
	{
        int	SDRAM_lastpage = 0;
        int	SDRAM_wrap = 0;
        int     TrigSource = 0;
unsigned
        int     T0 = 0;

        clock_gettime(CLOCK_REALTIME, &ts[2]);
        T0 = (unsigned int)ts[2].tv_sec;
        AERR(setIntegerParam(P_ram_TS, T0));
        AERR(callParamCallbacks());

        TrigSource |= okFrontPanel_IsTriggered(H, EP_STATUS, 0x10)?1:0;
        printf("SDRAM DONE Is Triggered, Phase jump bit = %d\n", TrigSource);
	printf("Going to read out the SDRAM ---\n");
	lock();
	// 1. Stop FF retrig (FF:Dutycycle
        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_CONTROL, 0, CONTROL_FF_DUTY_CYCLE));
        okFrontPanel_UpdateWireIns(H);
	AERR(setIntegerParam(P_ff_dutycycle, 0));

	// 2. Wait for ff_busy to clear!
	int ff_busy = 0;
	int nn = 0;
	do
		{
		okFrontPanel_UpdateWireOuts(H);
		ff_busy = okFrontPanel_GetWireOutValue(H, EP_WireOut_StatusBits) & STATUS_FF_Busy;
		if( ff_busy )
			{
        		ts[0].tv_sec = 		 0;
        		ts[0].tv_nsec=	 500000000; // Delay llrf100909_gui2b.m line No 365: pause(0.5)
        		ts[1].tv_sec = 		 0;
        		ts[1].tv_nsec=		 0;
        		nanosleep( &ts[0], &ts[1]);
			}
		nn++;
		}
	while( ff_busy && nn < 10);

	// 3. Update wire outs
        okFrontPanel_UpdateWireOuts(H);

	// 4. Read SDRAM lastpage, WireOut ep 0x2d
	SDRAM_lastpage = okFrontPanel_GetWireOutValue(H, EP_RAM_LASTPAGE);
	AERR( setIntegerParam( P_WO_EP2d, SDRAM_lastpage ) );
        AERR( setIntegerParam( P_ram_Lastpage, SDRAM_lastpage ) ); // Should not be overwritten by update()!

	// 5. Don't update, just obtain sdramWrap
	SDRAM_wrap = ( okFrontPanel_GetWireOutValue(H, EP_2fWIRE) >> 12 ) & 0x07;
	AERR( setIntegerParam( P_ram_wrap, SDRAM_wrap ) );
        AERR( setIntegerParam( P_ram_TrSrc, TrigSource ) );
	AERR(callParamCallbacks());
	// 6. Do llrf100909_record.m ...
	SDRAM_Reset(pasynUser);
	SDRAM_WrEn(pasynUser, 0);
	SDRAM_RdEn(pasynUser, 1);
	SDRAM_Rd(pasynUser);
	AERR( doCallbacksInt16Array( (short *)pCirc0, CIRC_SHORTS, P_circ_IQ, CIRC_ADDR) );
	printf("SDRAM READ DONE!\n");
	unlock();
	}

//**************************************************************************************************************

        okFrontPanel_UpdateTriggerOuts(H);
        update(pasynUser);

    } else if(function==P_cav_type) {
        if(value&~1)
            throw std::invalid_argument("Value out of range");

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_CONTROL,
                                                         (value ? CONTROL_CAVITY_TYPE : 0),
                                             CONTROL_CAVITY_TYPE));
        okFrontPanel_UpdateWireIns(H);

    } else if(function==P_limiter) {
        if(value&~1)
            throw std::invalid_argument("Value out of range");

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_CONTROL,
                                             (value ? CONTROL_LIMITER : 0),
                                             CONTROL_LIMITER));
        okFrontPanel_UpdateWireIns(H);

	} else if(function==P_ff_dutycycle) {
		if(value&~1)
	            throw std::invalid_argument("Value out of range");

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_CONTROL,
                                             	(value ? CONTROL_FF_DUTY_CYCLE : 0),
                                             	CONTROL_FF_DUTY_CYCLE));
        okFrontPanel_UpdateWireIns(H);

    } else if(function==P_ff_mode) {
        if(value&~1)
            throw std::invalid_argument("Value out of range");

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_CONTROL,
                                             (value ? CONTROL_FF_DUTY_CYCLE : 0),
                                             CONTROL_FF_DUTY_CYCLE));
        okFrontPanel_UpdateWireIns(H);
/*==========================================================================================================================*/
/* JUNE 4, 2015: Added second set of I/Q setpoints for phase toggling by 100 deg (G.Wang experiment)                        */
/* EP 0, bit 0x08 enables toggling (the second set)EP 0x15, 0x16 are the second set of values                               */
/* This bit enables/disables toggling (of the phase, i.e. second set of setpoints                                           */

    } else if(function==P_EnblI2Q2) {
        if(value&~1)
            throw std::invalid_argument("Value out of range");

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_SDRAM,
                                             (value ? PHS_TOGGLE_ENBL : 0),
                                             PHS_TOGGLE_ENBL));

        // cout << "Wrote val=" << (value ? PHS_TOGGLE_ENBL : 0) << " to EP " << EP_SDRAM << endl;
        okFrontPanel_UpdateWireIns(H);
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*==========================================================================================================================*/
/* SEP. 25, 2015: Added External Circular Buffer Trigger Enable                                                             */
/*                                                                                                                          */
    } else if(function==P_EnblExtCbufTr) {
        if(value&~1)
            throw std::invalid_argument("Value out of range");

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_SDRAM,
                                             (value ? EXT_CBUFTR_ENBL : 0),
                                             EXT_CBUFTR_ENBL));

        // cout << "Wrote val=" << (value ? EXT_CBUFTR_ENBL : 0) << " to EP " << EP_SDRAM << endl;
        okFrontPanel_UpdateWireIns(H);
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/****************************************************************************************************************************/
/*==========================================================================================================================*/
/* MAR 15, 2016: Added two signed integers for                                                                              */
/* debugging the Limiter: Ep 0x17 = Limiter Level                                                                           */
/* Ep 0x1A = Number of clocks                                                                                               */

    } else if ( function == P_LimLev ) {
        short  shyLL = (short)value;
//        cout << "Lim Level: Ep=0x" << hex << setfill('0') << setw(2) << EP_LIMLEVEL << " ";
//        cout << "value=" << dec << setfill(' ') << shyLL << endl;
        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_LIMLEVEL,
                                             shyLL, 0xffff));
        okFrontPanel_UpdateWireIns(H);
    } else if ( function == P_NLimClks ) {
        short  shyNL = (short)value;
//        cout << "Lim N clk: Ep=0x" << hex << setfill('0') << setw(2) << EP_NLIMCLKS << " ";
//        cout << "value=" << dec << setfill(' ') << shyNL << endl;
        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_NLIMCLKS,
                                             shyNL, 0xffff));
        okFrontPanel_UpdateWireIns(H);
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/****************************************************************************************************************************/
    } else if(function==P_RefPr_En) {
        if(value&~1)
            throw std::invalid_argument("Value out of range");

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_CONTROL,
                                             (value ? CONTROL_REFPR_EN : 0),
                                             CONTROL_REFPR_EN));
        okFrontPanel_UpdateWireIns(H);

    } else if(function==P_scope_zoom) {

	for( int i = 0; i < NUM_CHANNELS; i++ )
		// zoom_v_scale[i] = (i == 4) ? 0.5 : 1.0;
		zoom_v_scale[i] = (i == 4) ? 1.0 : 1.0;

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_CONTROL_0x02,
                                             value,
                                             CONTROL_ZOOM_mask));
        okFrontPanel_UpdateWireIns(H);

//-----------------------------------------------------------------------------------

    } else if(function==P_TopTrDly) {

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_Tr_DLY,
                                             value,
                                             0x7FFF));
        okFrontPanel_UpdateWireIns(H);

//-----------------------------------------------------------------------------------

    } else if(function==P_fb) {
        if(value&~1)
            throw std::invalid_argument("Value out of range");

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_CONTROL,
                                             (value ? CONTROL_FB : 0),
                                             CONTROL_FB));
        okFrontPanel_UpdateWireIns(H);
//-----------------------------------------------------------------------------------
    } else if( function == P_ff_enable ) {
        if(value&~1)
            throw std::invalid_argument("Value out of range");

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_CONTROL,
                                             (value ? CONTROL_FF_ENABLE : 0),
                                             CONTROL_FF_ENABLE));
        okFrontPanel_UpdateWireIns(H);

    } else if( function == P_scope_trigger ) {

	if( value ) {
		AERR(setIntegerParam(P_scope_trigger_run, 1));
		HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS, ACTIONS_ADC_START) );
                // printf("ADC_START!\n");
	}
	else {
		AERR(setIntegerParam(P_scope_trigger_run, 0));
	}		
//-----------------------------------------------------------------------------------

    } else if( function == P_WinADCMUX ) {

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_CONTROL,
                                             ( value << 4 ),
                                             CONTROL_DEBUG_mask));

        okFrontPanel_UpdateWireIns(H);
//-----------------------------------------------------------------------------------
// AERR(createParam("ZoomSelector", asynParamInt32, &P_zoom_selector));
//-----------------------------------------------------------------------------------
    } else if( function == P_Win_Tuner_PhOffset ) {

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_TUNER_PHASE_OFFSET,
                                             value,
                                             0xffff));

        okFrontPanel_UpdateWireIns(H);
//-----------------------------------------------------------------------------------
    } else if( function == P_zoom_selector ) {

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_CONTROL_0x02,
                                             value,
                                             CONTROL_ZOOM_mask));

        okFrontPanel_UpdateWireIns(H);
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
    } else if( function == P_rampinterval_SP ) {

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_RAMPINTERVAL_SP,
                                             value,
                                             0xFFFF));

        okFrontPanel_UpdateWireIns(H);
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
    } else if( function == P_naFreq_SP ) {

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_NA_FREQ,
                                             value,
                                             0xFFFF));
        okFrontPanel_UpdateWireIns(H);
        // cout << "naFreq: Ep=0x" << hex << setfill('0') << setw(2) << EP_NA_FREQ << " ";
        // cout << "value=" << dec << setfill(' ') << value << endl;

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
    } else if( function == P_naPII_SP ) {

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_NA_PII,
                                             value,
                                             0xFFFF));
        okFrontPanel_UpdateWireIns(H);
        // cout << "naPII: Ep=0x" << hex << setfill('0') << setw(2) << EP_NA_PII << " ";
        // cout << "value=" << dec << setfill(' ') << value << endl;
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
    } else if( function == P_naPIC0_SP ) {

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_NA_PIC0,
                                             value,
                                             0xFFFF));
        okFrontPanel_UpdateWireIns(H);
        // cout << "naPIC0: Ep=0x" << hex << setfill('0') << setw(2) << EP_NA_PIC0 << " ";
        // cout << "value=" << dec << setfill(' ') << value << endl;
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
    } else if( function == P_naPIC1_SP ) {

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_NA_PIC1,
                                             value,
                                             0xFFFF));
        okFrontPanel_UpdateWireIns(H);
        // cout << "naPIC1: Ep=0x" << hex << setfill('0') << setw(2) << EP_NA_PIC1 << " ";
        // cout << "value=" << dec << setfill(' ') << value << endl;
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
    } else if( function == P_naIntensity_SP ) {

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_NA_INTENSITY,
                                             value,
                                             0x0F));
        okFrontPanel_UpdateWireIns(H);
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
    } else if( function == P_PG_Enbl_SP ) {

        ep = EP_PG_ENBL;
        val= value;
        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, ep, val, 0x03));
        okFrontPanel_UpdateWireIns(H);
        //cout << "P_PG_Enbl_SP: okFrontPanel_ActivateTriggerIn: Ep = 0x";
	//cout << hex << setfill('0') << setw(4) << ep << " ";
	//cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl; 
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
    } else if( function == P_PG_SwTime_SP ) {

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_PG_SWTIME,
                                             value,
                                             0xffffffff));
        okFrontPanel_UpdateWireIns(H);
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
    } else if( function == P_PG_GO ) {

	ep  = EP_ACTIONS_EP0x41;

        if( value ) {
	     val = ACTIONS_PG_GO;
	     HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );
             //cout << "P_PG_GO: okFrontPanel_ActivateTriggerIn: Ep = 0x";
	     //cout << hex << setfill('0') << setw(4) << ep << " ";
	     //cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl; 
          } else {
	     //cout << "P_PG_GO: okFrontPanel_ActivateTriggerIn: No value, do nothing" << endl;
          }
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//===================================================================================
//===================================================================================
    } else if( function == P_fbramptrig ) {

	int	ii = 0;
	int	jj = 0;

	clock_gettime(CLOCK_REALTIME, &ts[0]);
	dt[0] = ts[0].tv_sec + ts[0].tv_nsec*1.0E-09;

	do
		{
		okFrontPanel_UpdateWireOuts(H);
		jj = okFrontPanel_GetWireOutValue(H, EP_WireOut_StatusBits); // EP_WireOut_StatusBits = 0x20, mask RampBusy = 0x02
		clock_gettime(CLOCK_REALTIME, &ts[0]);
		dt[1] = ts[0].tv_sec + ts[0].tv_nsec*1.0E-09;
		dt[2] = dt[1] - dt[0];
		if( jj && STATUS_FB_Busy ) {
		        ts[0].tv_sec = 		0;
        		ts[0].tv_nsec=	100000000;
        		ts[1].tv_sec = 		0;
        		ts[1].tv_nsec=		0;
        		nanosleep( &ts[0], &ts[1]);
			}
		ii++;
		}
	while( (dt[2] < 2.0) && (jj && STATUS_FB_Busy));
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS, ACTIONS_FB_SETPOINT_RAMP_TRIG) );

    } else if( function == P_Force_SoftTrip ) {

	ep  = EP_ACTIONS;
	val = ACTIONS_FORCE_SOFT_TRIP;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	/*
	cout << "P_Force_SoftTrip: okFrontPanel_ActivateTriggerIn: Ep = 0x";
	cout << hex << setfill('0') << setw(4) << ep << " ";
	cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl; 
	*/

    } else if( function == P_Reset_SoftTrip ) {

	ep  = EP_ACTIONS;
	val = ACTIONS_RESET_SOFT_TRIP;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	/*
	cout << "P_Reset_SoftTrip: okFrontPanel_ActivateTriggerIn: Ep = 0x";
	cout << hex << setfill('0') << setw(4) << ep << " ";
	cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl;
	*/

    } else if( function == P_Force_RFInhTrip ) {

	ep  = EP_ACTIONS;
	val = ACTIONS_FORCE_RF_INHIBIT;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	/*
	cout << "P_Force_RFInhTrip: okFrontPanel_ActivateTriggerIn: Ep = 0x";
	cout << hex << setfill('0') << setw(4) << ep << " ";
	cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl;
	*/

    } else if( function == P_Reset_RFInh ) {

	ep  = EP_ACTIONS;
        val = ACTIONS_RESET_RF_INHIBIT;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	/*
	cout << "P_Reset_RFInh: okFrontPanel_ActivateTriggerIn: Ep = 0x";
	cout << hex << setfill('0') << setw(4) << ep << " ";
	cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl;
	*/

    } else if( function == P_Force_QUENCHTrip ) {

	ep  = EP_ACTIONS;
        val = ACTIONS_FORCE_QUENCH;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	/*
	cout << "P_Force_QUENCHTrip: okFrontPanel_ActivateTriggerIn: Ep = 0x";
	cout << hex << setfill('0') << setw(4) << ep << " ";
	cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl;
	*/

    } else if( function == P_Reset_QUENCH ) {

	ep  = EP_ACTIONS;
        val = ACTIONS_RESET_QUENCH;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	/*
	cout << "P_Reset_QUENCH: okFrontPanel_ActivateTriggerIn: Ep = 0x";
	cout << hex << setfill('0') << setw(4) << ep << " ";
	cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl;
	*/

    } else if( function == P_Force_VacTrip ) {

	ep  = EP_ACTIONS_EP0x41;
	val = ACTIONS_FORCE_VACTRIP;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	/*
	cout << "P_Force_VacTrip: okFrontPanel_ActivateTriggerIn: Ep = 0x";
	cout << hex << setfill('0') << setw(4) << ep << " ";
	cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl;
	*/

    } else if( function == P_Reset_VacTrip) {

	ep  = EP_ACTIONS_EP0x41;
	val = ACTIONS_RESET_VACTRIP;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	/*
	cout << "P_Reset_VacTrip: okFrontPanel_ActivateTriggerIn: Ep = 0x";
	cout << hex << setfill('0') << setw(4) << ep << " ";
	cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl;
	*/

    } else if( function == P_Force_PPSTrip ) {

	ep  = EP_ACTIONS_EP0x41;
	val = ACTIONS_FORCE_PPSTRIP;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	/*
	cout << "P_Force_PPSTrip: okFrontPanel_ActivateTriggerIn: Ep = 0x";
	cout << hex << setfill('0') << setw(4) << ep << " ";
	cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl;
	*/

    } else if( function == P_Reset_PPS) {

	ep  = EP_ACTIONS_EP0x41;
	val = ACTIONS_RESET_PPSTRIP;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	/*
	cout << "P_Reset_PPSTrip: okFrontPanel_ActivateTriggerIn: Ep = 0x";
	cout << hex << setfill('0') << setw(4) << ep << " ";
	cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl;
	*/

    } else if( function == P_Force_AVAGO2Trip ) {

	ep  = EP_ACTIONS_EP0x41;
	val = ACTIONS_FORCE_AVAGO2;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	/*
	cout << "P_Force_AVAGO2Trip: okFrontPanel_ActivateTriggerIn: Ep = 0x";
	cout << hex << setfill('0') << setw(4) << ep << " ";
	cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl;
	*/

    } else if( function == P_Reset_AVAGO2 ) {

	ep  = EP_ACTIONS_EP0x41;
	val = ACTIONS_RESET_AVAGO2;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	/*
	cout << "P_Reset_AVAGO2Trip: okFrontPanel_ActivateTriggerIn: Ep = 0x";
	cout << hex << setfill('0') << setw(4) << ep << " ";
	cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl;
	*/

/*************************************************************************************************************/
    } else if( function == P_na_Start ) {

	ep  = EP_ACTIONS_EP0x41;
	val = 10;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

    } else if( function == P_na_Stop ) {

	ep  = EP_ACTIONS_EP0x41;
	val = 11;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

    } else if( function == P_TSNext ) {

	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS_EP0x41, ACTIONS_TSNEXT) );

    } else if( function == P_TSReset ) {

	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS_EP0x41, ACTIONS_TSRESET) );

    } else if( function == P_Force_Limit ) {

	ep  = EP_ACTIONS_EP0x41;
	val = ACTIONS_LIMIT;
	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, ep, val) );

	/*
	cout << "P_Force_Limit: okUsbFrontPanel_ActivateTriggerIn: Ep = 0x";
	cout << hex << setfill('0') << setw(4) << ep << " ";
	cout << "Val = 0x" << setw(4) << val << dec << setfill(' ') << endl;
	*/

    } else if( function == P_TS_RdAll ) {

    	int TripStackWord = 0;

    	okFrontPanel_UpdateWireOuts(H);
    	TripStackWord = okFrontPanel_GetWireOutValue(H, EP_TS_WORD0);
    	AERR(setIntegerParam(addr, P_TS_RdWord0_1, TripStackWord));
 	/* if( verbose ) cout << "TS: 0x" << hex << setfill('0') << setw(8) << TripStackWord << " ";*/
    	TripStackWord = okFrontPanel_GetWireOutValue(H, EP_TS_WORD1);
    	AERR(setIntegerParam(addr, P_TS_RdWord1_0, TripStackWord));
        /* if( verbose ) cout << "0x" << hex << setfill('0') << setw(8) << TripStackWord << " ";   */
    	TripStackWord = okFrontPanel_GetWireOutValue(H, EP_TS_WORD2);
    	AERR(setIntegerParam(addr, P_TS_RdWord2_0, TripStackWord));
        /* if( verbose ) cout << "0x" << hex << setfill('0') << setw(8) << TripStackWord << endl;  */

    } else if( function == P_EQ_EP1_SP || function == P_EQ_EP2_SP ||
               function == P_EQ_EP3_SP || function == P_EQ_EP4_SP ||
               function == P_EQ_EP5_SP || function == P_EQ_EP6_SP ||
               function == P_EQ_EP7_SP || function == P_EQ_EP8_SP ) {

      /* Don't do anything with Equench parameters here, only at the function == P_EQ_Wr */

    } else if( function == P_EQ_Wr ) {

      if( val ) { /* Write only at transition from 0->1, ignore transition 1->0 */

        /* Write out all eQuench parameters using multiplexing selector on EP_EQ_MUX bits 4,3,2 */
        /* and TriggerIn ep41, bit 13 as strobe                                                 */

        AERR(getIntegerParam(P_EQ_EP1_SP, (int *)&eQ_Param[0]));
        // cout << "EP1=0x" << hex << setfill('0') << setw(8) << eQ_Param[0] << endl;

        AERR(getIntegerParam(P_EQ_EP2_SP, (int *)&eQ_Param[1]));
        // cout << "EP2=0x" << hex << setfill('0') << setw(8) << eQ_Param[1] << endl;

        AERR(getIntegerParam(P_EQ_EP3_SP, (int *)&eQ_Param[2]));
        // cout << "EP3=0x" << hex << setfill('0') << setw(8) << eQ_Param[2] << endl;

        AERR(getIntegerParam(P_EQ_EP4_SP, (int *)&eQ_Param[3]));
        // cout << "EP4=0x" << hex << setfill('0') << setw(8) << eQ_Param[3] << endl;

        AERR(getIntegerParam(P_EQ_EP5_SP, (int *)&eQ_Param[4]));
        // cout << "EP5=0x" << hex << setfill('0') << setw(8) << eQ_Param[4] << endl;

        AERR(getIntegerParam(P_EQ_EP6_SP, (int *)&eQ_Param[5]));
        // cout << "EP6=0x" << hex << setfill('0') << setw(8) << eQ_Param[5] << endl;

        AERR(getIntegerParam(P_EQ_EP7_SP, (int *)&eQ_Param[6]));
        // cout << "EP7=0x" << hex << setfill('0') << setw(8) << eQ_Param[6] << endl;

        AERR(getIntegerParam(P_EQ_EP8_SP, (int *)&eQ_Param[7]));
        // cout << "EP8=0x" << hex << setfill('0') << setw(8) << eQ_Param[7] << endl;

        lock();
        for( eQi = 0; eQi < 8; eQi++ ) {
          p_eQsh = (unsigned short *)&eQ_Param[eQi];

          HWERR2(pasynUser, okFrontPanel_SetWireInValue (H, EP_EQ_MUX, (eQi<<2),0x3C) );
          // cout << "i=" << dec << eQi << " SetWireIn 0x" << hex << setfill('0') << setw(4) << EP_EQ_MUX << " Value=0x" << setfill('0') << setw(4) << (eQi<<2) << " Mask=" << setfill('0') << setw(4) << 0x1C << " ";
          okFrontPanel_UpdateWireIns(H);

          // cout << " WireIn 0x" << hex << setfill('0') << setw(4) << EP_EQ_Win_LSW << " Value=0x" << setfill('0') << setw(4) << *p_eQsh << " ";
          HWERR2(pasynUser, okFrontPanel_SetWireInValue (H, EP_EQ_Win_LSW, *p_eQsh, 0xffff) );

          p_eQsh++;
          HWERR2(pasynUser, okFrontPanel_SetWireInValue (H, EP_EQ_Win_MSW, *p_eQsh, 0xffff) );
          // cout << " WireIn 0x" << hex << setfill('0') << setw(4) << EP_EQ_Win_MSW << " Value=0x" << setfill('0') << setw(4) << *p_eQsh << " ";
          okFrontPanel_UpdateWireIns(H);

          HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS_EP0x41, ACTIONS_EQ_Strobe_WireIn) );
          // cout << " ActivateTriggerIn 0x" << hex << setfill('0') << setw(4) << EP_ACTIONS_EP0x41 << " Value=" << dec << ACTIONS_EQ_Strobe_WireIn << endl;
          }
        unlock();
      } /* End of if( val ) */
    } else if( function == P_EQ_Rd ) {

      cout << "Function P_EQ_Rd called, val=" << val << endl;

      lock();
      for( eQi = 0; eQi < 10; eQi++ ) {
        p_eQsh = (unsigned short *)&eQ_Param[eQi];
        HWERR2(pasynUser, okFrontPanel_SetWireInValue (H, EP_EQ_MUX, (eQi<<2),0x3C) );
        cout << "i=" << dec << eQi << " SetWireIn 0x" << hex << setfill('0') << setw(4) << EP_EQ_MUX << " Value=0x" << setfill('0') << setw(4) << (eQi<<2) << " Mask=" << setfill('0') << setw(4) << 0x1C << " ";
        okFrontPanel_UpdateWireIns(H);

        HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS_EP0x41, ACTIONS_EQ_Strobe_WireOut) );
        cout << " ActivateTriggerIn 0x" << hex << setfill('0') << setw(4) << EP_ACTIONS_EP0x41 << " Value=" << dec << ACTIONS_EQ_Strobe_WireOut << " ";

        okFrontPanel_UpdateWireOuts(H);
	*p_eQsh = okFrontPanel_GetWireOutValue(H, EP_EQ_WOut_LSW);
        cout << " WireOut=0x" << hex << setfill('0') << setw(4) << EP_EQ_WOut_LSW << "=" << hex << setfill('0') << setw(4) << *p_eQsh << " ";

	p_eQsh++;
        *p_eQsh = okFrontPanel_GetWireOutValue(H, EP_EQ_WOut_MSW);
        cout << " WireOut=0x" << hex << setfill('0') << setw(4) << EP_EQ_WOut_MSW << "=" << hex << setfill('0') << setw(4) << *p_eQsh << " ";
        cout << " EP[" << (eQi+1) << "]=" << eQ_Param[eQi] << endl;
        }

      AERR( setIntegerParam( P_EQ_EP1_RB , eQ_Param[0] ) );
      AERR( setIntegerParam( P_EQ_EP2_RB , eQ_Param[1] ) );
      AERR( setIntegerParam( P_EQ_EP3_RB , eQ_Param[2] ) );
      AERR( setIntegerParam( P_EQ_EP4_RB , eQ_Param[3] ) );
      AERR( setIntegerParam( P_EQ_EP5_RB , eQ_Param[4] ) );
      AERR( setIntegerParam( P_EQ_EP6_RB , eQ_Param[5] ) );
      AERR( setIntegerParam( P_EQ_EP7_RB , eQ_Param[6] ) );
      AERR( setIntegerParam( P_EQ_EP8_RB , eQ_Param[7] ) );
      AERR( setIntegerParam( P_EQ_EP9_RB , eQ_Param[8] ) );
      AERR( setIntegerParam( P_EQ_EPA_RB , eQ_Param[9] ) ); 
      unlock();

    } else if( function == P_EQ_Enbl ) {

      // cout << "P_EQ_Enbl called, val=" << val << endl;

      lock();
      if( val ) HWERR2(pasynUser, okFrontPanel_SetWireInValue (H, EP_EQ_Enbl, EP_EQ_EnbleBIT, EP_EQ_EnbleBIT ) );
      else      HWERR2(pasynUser, okFrontPanel_SetWireInValue (H, EP_EQ_Enbl,              0, EP_EQ_EnbleBIT ) );
      okFrontPanel_UpdateWireIns(H);  
      unlock();

      //    } else throw std::runtime_error("Parameter is readonly");
    } else { printf("function=%d - no match\n", function); }

    AERR(setIntegerParam(addr, function, value));
    AERR(callParamCallbacks());

    return asynSuccess;
}CATCHALL(pasynUser)
}

asynStatus llrfPort::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
//	long	ret = 0;
try {
    FLOW(pasynUser);

    int    addr, function = pasynUser->reason;
    double dval = 0;

    AERR(getAddress(pasynUser, &addr));

    if(function == P_gain_P || function==P_loop_phase ) {

        double amp, pha;
        if(function==P_gain_P) {
            amp=value;
            AERR(getDoubleParam(P_loop_phase, &pha));
        } else {
            pha=value;
            AERR(getDoubleParam(P_gain_P, &amp));
        }

        IQ rot = AP2IQ(amp, pha * M_PI / 180.0);
        epicsInt16 I=squash16(rot.I * 32767),
                   Q=squash16(rot.Q * 32767);

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_ROT_I,
                                             I,
                                             0xffff));
        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_ROT_Q,
                                             Q,
                                             0xffff));

        okFrontPanel_UpdateWireIns(H);

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: Rot %04x %04x\n",
                  portName, I, Q);

    }
    else if(function == P_PG_AltKp_SP || function==P_loop_phase ) {

        double amp, pha;
        if( function==P_PG_AltKp_SP ) {
            amp=value;
            AERR(getDoubleParam(P_loop_phase, &pha));
          } else {
            pha=value;
            AERR(getDoubleParam(P_PG_AltKp_SP, &amp));
          }

        IQ rot = AP2IQ(amp, pha * M_PI / 180.0);
        epicsInt16 I=squash16(rot.I * 32767),
                   Q=squash16(rot.Q * 32767);

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_PG_ALTROT_I,
                                             I,
                                             0xffffffff));
        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_PG_ALTROT_Q,
                                             Q,
                                             0xffffffff));

        okFrontPanel_UpdateWireIns(H);

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: altRot %04x %04x\n",
                  portName, I, Q);
    }
/****************************************************************************************************************************/
/*==========================================================================================================================*/
/* JUNE 4, 2015: Added second set of I/Q setpoints for phase toggling by 100 deg (G.Wang experiment)                        */
/* EP 0, bit 0x08 enables toggling (the second set)EP 0x15, 0x16 are the second set of values                               */

    else if ( function == P_spI2 || function == P_spQ2 ) {
        double dI2, dQ2;
        short  shyI2, shyQ2;
 
        if( function == P_spI2 ) {
          dI2 = value;
          AERR( getDoubleParam( P_spQ2, &dQ2) );
          // cout << "P_spI2: value = " << fixed << dec << setprecision(6) << value <<  " dI2=" << dI2 << " dQ2=" << dQ2 << endl;
          } else {
          dQ2 = value;
          AERR( getDoubleParam( P_spI2, &dI2) );
          // cout << "P_spQ2: value = " << fixed << dec << setprecision(6) << value << " dI2=" << dI2 << " dQ2=" << dQ2 << endl;

	  shyI2 = (short)(int)floor( 32767 * dI2 + 0.5);
          shyQ2 = (short)(int)floor( 32767 * dQ2 + 0.5);

	  // cout << "(double) I2=" << fixed << dec << setprecision(6) << setw(10) << dI2 << " ";
	  // cout << fixed << dec << setprecision(3) << "Q2=" << setw(10) << dQ2 << endl;

	  // cout << "(short) shyI2=" << fixed << setw(6) << shyI2 << " ";
	  // cout << fixed << "shyQ2=" << setw(6) << shyQ2 << endl;

	  // cout << "Writing to EP(I)=" << fixed << hex << "0x" << EP_SP_I2 << " ";
	  // cout << fixed << hex << "EP(Q)=0x" << EP_SP_Q2 << endl;

          HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_SP_I2,
                                               shyI2,
                                               0xffff));
          HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_SP_Q2,
                                               shyQ2,
                                               0xffff));

          okFrontPanel_UpdateWireIns(H);
          }
        }
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/****************************************************************************************************************************/
    else if ( function == P_sp_I || function == P_sp_Q ) {
        IQ SP;

	short	shyI[2], shyQ[2];
	short	shOut_I, shOut_Q;
	double	dyeI[2], dyeQ[2];

        try {
          if( function == P_sp_I ) {
              SP.I = value;
              AERR( getDoubleParam( P_sp_Q, &SP.Q ) );

	      // cout << "At SP.I=" << fixed << dec << setprecision(6) << setw(12) << SP.I << " ";
	      // cout << fixed << dec << setprecision(6) << "SP.Q=" << setw(12) << SP.Q << endl;
	      // Output only at setpoint Q!

              } else  {

              SP.Q = value;

              AERR( getDoubleParam( P_sp_I, &SP.I ) );

	     // cout << "At SP.Q=" << fixed << dec << setprecision(6) << setw(12) << SP.Q << " ";
	     // cout << fixed << dec << setprecision(6) << "SP.I=" << setw(12) << SP.I << endl;

	      AERR( getDoubleParam( P_sp_IPVAL, &dyeI[1] ) );
	      AERR( getDoubleParam( P_sp_QPVAL, &dyeQ[1] ) );

	      shyI[1] = (short)(int)floor( 32767 * dyeI[1] + 0.5);
	      shyQ[1] = (short)(int)floor( 32767 * dyeQ[1] + 0.5);
	      shyI[0] = (short)(int)floor( 32767 * SP.I + 0.5);
	      shyQ[0] = (short)(int)floor( 32767 * SP.Q + 0.5); 

 	      int iCnt = 0;
	      while( iCnt < 512 )
		{
		// dyeI[0]= (double)shyI[1] + ((double)shyI[0] - (double)shyI[1])*(double)iCnt/512.0;
		// dyeQ[0]= (double)shyQ[1] + ((double)shyQ[0] - (double)shyQ[1])*(double)iCnt/512.0;

		dyeI[0]= (double)shyI[1] + 0.5*((double)shyI[0] - (double)shyI[1])*(1-cos(M_PI*iCnt/512.0));
		dyeQ[0]= (double)shyQ[1] + 0.5*((double)shyQ[0] - (double)shyQ[1])*(1-cos(M_PI*iCnt/512.0));

		shOut_I = (short)(int)floor( dyeI[0] + 0.5);
		shOut_Q = (short)(int)floor( dyeQ[0] + 0.5);

		// cout << "Out " << setw(3) << iCnt << " I=" << setw(5) << shOut_I << " Q=" << setw(5) << shOut_Q << endl;

        	HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_SP_I,
                                             shOut_I,
                                             0xffff));
        	HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_SP_Q,
                                             shOut_Q,
                                             0xffff));
		okFrontPanel_UpdateWireIns(H);
		iCnt++;
		}
              }

       } catch (std::runtime_error&) {
         AERR(setDoubleParam(function, value));
         AERR(callParamCallbacks());
         return asynSuccess; // asynError; // throw; P_sp_I fails because P_sp_Q is not set at PINI
			     // P_sp_Q succeeds because previous call set P_sp_I (to smth.)
        }

    } else if (function==P_scope_dly) {

	/* Only 11 bits, 0x07ff, are used: Nathan March 13, 2012	*/
	/* The units are [samples: 0 ... 2047]				*/
	/* The value from the record: ao record, 0 ... 1.0		*/

        epicsInt16 dly = (unsigned short)(int)floor(value*0x07ff + 0.5);

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_T_SCOPE_START,
                                             dly,
                                             0xffff));

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: SP %04x\n", portName, dly);
	okFrontPanel_UpdateWireIns(H);
 
    } else if ( function==P_gain_I ) {

	dval = value * DAC_MAXVALUE; // Val = 0 ... 1

        epicsInt16 gain = (unsigned short)((int)floor(dval + 0.5) & 0x00007fff);

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_GAIN_INT,
                                             gain,
                                             0xffff));

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: P_gain_I %04x\n",
                  portName, gain);
	
        okFrontPanel_UpdateWireIns(H);

    } else if ( function==P_PG_AltKi_SP ) {

	dval = value * DAC_MAXVALUE; // Val = 0 ... 1

        epicsInt16 altKi = (unsigned short)((int)floor(dval + 0.5) & 0x00007fff);

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_PG_ALTKi,
                                             altKi,
                                             0xffffffff));

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: P_PG_AltKi_SP %04x\n",
                  portName, altKi);
	
        okFrontPanel_UpdateWireIns(H);

    } else if ( function == P_sp_IPVAL ) {

 	// cout << "Previous I val=" << dec << fixed << setprecision(6) << setw(12) << value << endl;

    } else if ( function == P_sp_QPVAL ) {
 
	// cout << "Previous Q val=" << dec << fixed << setprecision(6) << setw(12) << value << endl;

    } else throw std::runtime_error("Parameter is readonly");
    
    AERR(setDoubleParam(function, value));
    AERR(callParamCallbacks());

    return asynSuccess;

 } CATCHALL(pasynUser)
}

asynStatus llrfPort::writeFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements)
{
register	int		jj = 0;
		int		ii = 0;
		char		*p;

//		double		FBISF[2];
//		double		FBQSF[2];

try {
    FLOW(pasynUser);
    int addr, function = pasynUser->reason;

    ofstream	*pOf[2] = {NULL, NULL};

    AERR(getAddress(pasynUser, &addr));

    if ( function == P_ff_IQ ) {

	if( Log )
		{
		pOf[0] = new ofstream(FILENAME_FF_TABLE_IQ);
		pOf[1] = new ofstream(FILENAME_FF_TABLERAW);
		cout << "Dumping FF table to: " << FILENAME_FF_TABLE_IQ << " and " << FILENAME_FF_TABLERAW << flush;
		}

	ii = 0;
	for(jj = 0; jj < DAC_COUNT; jj++ )
		{
		FFI[jj] = ((unsigned)jj < nElements)? *(value + ii) : 0; ii++;
		FFQ[jj] = ((unsigned)jj < nElements)? *(value + ii) : 0; ii++;
		}


        for(size_t i = 0; i < DAC_COUNT; i++ ) {

		int	n[2];
		n[0] = 2*i + 0;
		n[1] = 2*i + 1;
 
        scratch[2*i+0] = htole16( squash16( 32767 * FFI[i] ));
        scratch[2*i+1] = htole16( squash16( 32767 * FFQ[i] ));

	if( Log )
		{
		if( i < 2 || (i > DAC_COUNT - 4) )
			{
			cout << dec << "P_ff_IQ:  Q[" << setw(1) << i << "]=" << fixed << setprecision(3) << setw(8) << FFQ[i] << " ";
			cout << dec << "I[" << setw(1) << i << "]=" << fixed << setprecision(3) << setw(8) << FFI[i] << " ";
			cout << dec << "scratch[" << setw(1) << n[0] << "]=" << scratch[n[0]] << " ";
			cout << dec << "scratch[" << setw(1) << n[1] << "]=" << scratch[n[1]] << endl;
			char *p = (char *)&scratch[n[0]];
			for( int k = 0; k < 4; k++)
				cout << hex << (int)(0xff & *(p+k)) << endl;
			}

		*pOf[0] << dec << setw(3) << i << " I = " << fixed << setprecision(3) << setw(8) << FFI[i];
		*pOf[0] << " Q = " << fixed << setprecision(3) << setw(8) << FFQ[i] << " nElements = " << nElements << endl;
		}
        }

	if( Log ) { pOf[0]->close(); delete pOf[0]; }

        asynPrintIO(pasynUser, ASYN_TRACEIO_DRIVER, (char*)&scratch[0],
                    DAC_LENGTH, "%s: Update FF\n", portName);

// ================================================================================================================
// Wait for ramp to finish: MATLAB function fbTableWrite -> fbWait(xptr, mask = 4, interval = 0.1, timeout = 10)
// Setup timer, wait 1.1 seconds ...
// ================================================================================================================

	ii = 0;
	do
		{
		okFrontPanel_UpdateWireOuts(H);
		jj = okFrontPanel_GetWireOutValue(H, EP_WireOut_StatusBits); // EP_WireOut_StatusBits = 0x20, mask RampBusy = 0x02
		ii++;
		}
	while( ii < 100 && (jj && STATUS_FB_Busy));

        HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS, ACTIONS_RESET_DAC_POINTER) );

	if( Log )
		{
		cout << "P_ff_IQ: Activated Trigger In: EP = " << hex << EP_ACTIONS << " Value = " << ACTIONS_RESET_DAC_POINTER << endl;

		jj = 0;
		p  = (char *)&scratch[0];
		while( jj < DAC_LENGTH )
			{
			ii = 0;
			ii = 0xff & (unsigned char)*(p + jj);
			*pOf[1] << dec << setw(6) << jj << " 0x" << hex << setfill('0') << setw(2) << ii << " = ";
			*pOf[1] << dec << setfill(' ') << setw(3) << ii << " DEC" << endl;
			jj++;
			}
		// EP_DAC = 0x80
		cout << "P_ff_IQ: Trying WriteToBlockPipeIn FF-IQ EP: 0x" << hex << EP_DAC;
		cout << " Block size = " << dec << BLOCK_SIZE << " Length = " << DAC_LENGTH << " ... " << flush;
		}

        long ret = okFrontPanel_WriteToBlockPipeIn(H, EP_DAC, BLOCK_SIZE,
                                          DAC_LENGTH,
                                          (unsigned char*)&scratch[0]);

        if( ret < 0 || (size_t)ret != DAC_LENGTH) {
            asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s: Incomplete send %ld of %lu\n",
                      portName, ret, (unsigned long)(DAC_LENGTH));
            //TODO: how to handle this???
        }

	if( Log ) {
		cout << " - done!" << endl;
		// cout << "Wrote To Block Pipe In: EP = " << hex << EP_DAC << " DAC_LENGTH = " << dec << DAC_LENGTH << " ret = " << ret << endl;
		pOf[1]->close(); delete pOf[1]; 
		cout << " - done." << endl;
	}

        // move read pointer
        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_CONTROL,
                                                         (nextdac ? CONTROL_DAC_READ : 0),
                                             CONTROL_DAC_READ));

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: Set read ptr %04x\n",
                  portName, (nextdac ? CONTROL_DAC_READ : 0));

        nextdac = !nextdac; // toggle

        // move write pointer to A
        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_CONTROL,
                                             (nextdac ? 1 : 0) << CONTROL_DAC_WRITE_shft,
                                             CONTROL_DAC_WRITE_mask));

        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: Set write ptr %04x\n",
                  portName, (nextdac ? 1 : 0) << CONTROL_DAC_WRITE_shft);

        okFrontPanel_UpdateWireIns(H);

	/* Nathan, March 14, 2012	*/

	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS, ACTIONS_FFTRIG) );
	
	if( Log )
		cout << "Activated Trigger In: EP = " << hex << EP_ACTIONS << " Value = " << ACTIONS_FFTRIG << endl;

// **************************************************************************************************************************

    } else

//===========================================================================================================================
	if ( function == P_fb_IQ ) {

	if( Log )
		{
		pOf[0] = new ofstream(FILENAME_FB_TABLE_IQ);
		pOf[1] = new ofstream(FILENAME_FB_TABLERAW);
		cout << "Dumping FB table to: " << FILENAME_FB_TABLE_IQ << " and " << FILENAME_FB_TABLERAW << flush;
		}

	ii = 0;
	for(jj = 0; jj < DAC_COUNT; jj++ )
		{
		FBI[jj] = ((unsigned)jj < nElements)? *(value + ii) : 0; ii++;
		FBQ[jj] = ((unsigned)jj < nElements)? *(value + ii) : 0; ii++;
		}


        for(size_t i = 0; i < DAC_COUNT; i++ ) {

		int	n[2];
		n[0] = 2*i + 0;
		n[1] = 2*i + 1;
 
        scratch[2*i+0] = htole16( squash16( 32767 * FBI[i] ));
        scratch[2*i+1] = htole16( squash16( 32767 * FBQ[i] ));

	/*
          scratch[2*i+0] = htole16( squash16( 32767 * *(value+2*i+0) ));
          scratch[2*i+1] = htole16( squash16( 32767 * *(value+2*i+1) ));
	*/

		if( Log )
			{
			if( i < 2 || (i > DAC_COUNT - 4) )
				{
				cout << dec << "P_fb_IQ:  Q[" << setw(1) << i << "]=" << fixed << setprecision(3) << setw(8) << FBQ[i] << " ";
				cout << dec << "I[" << setw(1) << i << "]=" << fixed << setprecision(3) << setw(8) << FBI[i] << " ";
				cout << dec << "scratch[" << setw(1) << n[0] << "]=" << scratch[n[0]] << " ";
				cout << dec << "scratch[" << setw(1) << n[1] << "]=" << scratch[n[1]] << endl;
				char *p = (char *)&scratch[n[0]];
				for( int k = 0; k < 4; k++)
					cout << hex << (int)(0xff & *(p+k)) << endl;
				}

			*pOf[0] << dec << setw(3) << i << " I = " << fixed << setprecision(3) << setw(8) << FBI[i];
			*pOf[0] << " Q = " << fixed << setprecision(3) << setw(8) << FBQ[i] << " nElements = " << nElements << endl;
			}
        }

	if( Log ) { pOf[0]->close(); delete pOf[0]; }

        asynPrintIO(pasynUser, ASYN_TRACEIO_DRIVER, (char*)&scratch[0],
                    DAC_LENGTH, "%s: Update FF\n", portName);

// ================================================================================================================
// Wait for ramp to finish: MATLAB function fbTableWrite -> fbWait(xptr, mask = 4, interval = 0.1, timeout = 10)
// Setup timer, wait 1.1 seconds ...
// ================================================================================================================
	ii = 0;
	do
		{
		okFrontPanel_UpdateWireOuts(H);
		jj = okFrontPanel_GetWireOutValue(H, EP_WireOut_StatusBits); // EP_WireOut_StatusBits = 0x20, mask RampBusy = 0x02
		ii++;
		}
	while( ii < 100 && (jj && STATUS_FB_Busy));

// ================================================================================================================
/* Don't reset FB waypoint cnt for booster ramp!

        HWERR2(pasynUser, okFrontPanel_SetWireInValue(H, EP_RAMPINTERVAL_SP,
                                             30,		// Nathan, Apr. 15, 2012
                                             0xFFFF));

        okFrontPanel_UpdateWireIns(H);
*/

        HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS, ACTIONS_RESET_DAC_POINTER) );

	if( Log )
		{
		cout << "P_fb_IQ: Activated Trigger In: EP = " << hex << EP_ACTIONS << " Value = " << ACTIONS_RESET_DAC_POINTER << endl;

		jj = 0;
		p  = (char *)&scratch[0];
		while( jj < DAC_LENGTH )
			{
			ii = 0;
			ii = 0xff & (unsigned char)*(p + jj);
			*pOf[1] << dec << setw(6) << jj << " 0x" << hex << setfill('0') << setw(2) << ii << " = ";
			*pOf[1] << dec << setfill(' ') << setw(3) << ii << " DEC" << endl;
			jj++;
			}
		// EP_DAC = 0x83
		cout << "P_fb_IQ: Trying WriteToBlockPipeIn FB-IQ EP: 0x" << hex << EP_SP;
		cout << " Block size = " << dec << BLOCK_SIZE << " Length = " << DAC_LENGTH << " ... " << flush;
		}


        long ret = okFrontPanel_WriteToBlockPipeIn(H, EP_SP, BLOCK_SIZE,
                                          DAC_LENGTH,
                                          (unsigned char*)&scratch[0]);



        if( ret < 0 || (size_t)ret != DAC_LENGTH) {
            asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s: Incomplete send %ld of %lu\n",
                      portName, ret, (unsigned long)(DAC_LENGTH));
            //TODO: how to handle this???
        }

	if( Log ) {
		cout << " - done!" << endl;
		// cout << "Wrote To Block Pipe In: EP = " << hex << EP_SP << " DAC_LENGTH = " << dec << DAC_LENGTH << " ret = " << ret << endl;
		pOf[1]->close(); delete pOf[1]; 
		cout << " - done." << endl;
	}

	HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS, ACTIONS_FB_SETPOINT_RAMP_TRIG) );
	
	if( Log )
		cout << "Activated Trigger In: EP = " << hex << EP_ACTIONS << " Value = " << ACTIONS_FB_SETPOINT_RAMP_TRIG << endl;

	// FB_TrigReq = 1;
	/* Nathan, March 14, 2012	*/
	// EP_ACTIONS = 0x40, ACTIONS_FFTRIG = 0x03, ACTIONS_FB_SETPOINT_RAMP_TRIG = 8

//*****************************************************************************************************************
//=================================================================================================================
    // } else throw std::runtime_error("Parameter is readonly");
    } else { printf("function=%d - no match\n", function); }
    return asynSuccess;
}CATCHALL(pasynUser)
}

asynStatus llrfPort::readInt16Array(asynUser *pasynUser, epicsInt16 *value, size_t nElements, size_t *nIn)
{
register
	int	i, j;
	int	l = 0;
	int	n = 0;
        int     nz= 0;
        int     ep;
	short	*pSh = (short *)pCirc0;
unsigned
        char    *p;
        char    nac[128];
        int     chans = 16;
        int     bl;

	FILE	*fp;

try {
    FLOW(pasynUser);
    int addr, function = pasynUser->reason;

    AERR(getAddress(pasynUser, &addr));

    if( function == P_circ_IQ )
	{
        *nIn = nElements;

	// l = SDRAM_Rd(pasynUser); - Read should be done at the only right moment! In the beginning of update!

        for( i = 0; i < (int)nElements; i++ )
		{
		*(value+i) = *(pSh + i);
		if( *(value+i) != 0 ) nz = 1;
		}

	if( Log )
		{
		if( Log ) fp = fopen("SDRAM.TXT", "w");
		i = j = l = 0;
		while( l < (int)nElements )
			{
			if( j == 0 ) fprintf(fp, "%6d -", n);
			while(j < 16) { fprintf(fp, " %6d", *(value+l) ); l++; j++; }
			fprintf(fp, "\n"); { j = 0; n++; }
			}	
		fclose(fp);
		}
	l = nz;
	} else if (function == P_na_I) {
          // cout << "P_na_I" << endl;
          memset(naBuf,0, sizeof(naBuf));
          memset(nac,  0, sizeof(nac)  );
          ep = EP_NA_WF;
          p  = (unsigned char *)&nac[0];
          bl = 8*chans;
          l  = okFrontPanel_ReadFromBlockPipeOut(H, ep, bl, bl, p);

          //printf("NA RD: okFrontPanel_ReadFromBlockPipeOut(0x%0X,0x%02X,%d,%d,0x%0X)= %d: ",
	  //	 (int *)H, ep, bl, bl, p,  l );
          if( l == bl ) {
            // virtual asynStatus doCallbacksInt16Array(epicsInt16 *value,
            //                                        size_t nElements, int reason, int addr);
            // AERR( doCallbacksInt16Array( (short *)pCirc0, CIRC_SHORTS, P_circ_IQ, CIRC_ADDR) );
            // ADDR  Must be set to the same addr in the record INP field!
            *nIn = l;
            for(i = 0; i < l; i++) {
              naBuf[i] = (short)nac[i];
              *(value+i) = naBuf[i];
              // cout << "NA[" << setw(3) << i << "]=" << (int)naBuf[i] << "=" << (int)nac[i] << endl;
              }      
            //printf("SUCCESS!\n");
            }
          else printf("%s\n", ErrorName( (ok_ErrorCode)l ) ) ;
        }
    AERR( doCallbacksInt16Array( (short *)&naBuf[0], bl, P_na_I, 0) );
    return (l > 0)? asynSuccess : asynError;
    }CATCHALL(pasynUser)
}

asynStatus llrfPort::readFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements, size_t *nIn)
{
try {
    FLOW(pasynUser);
    int addr, function = pasynUser->reason;

    AERR(getAddress(pasynUser, &addr));

    wfs_t::const_iterator it=wfs.find(PAIR(function,addr));

    if(it==wfs.end())
        throw std::runtime_error("Unknown waveform");

    *nIn = std::min(nElements, it->second->size());

    std::copy(it->second->begin(),
              it->second->begin() + *nIn,
              value);

    return asynSuccess;
    }CATCHALL(pasynUser)
}

void llrfPort::update(asynUser* pasynUser)
{
register
	int		i, ii, jj, ix;
	int		nRBVals = N_RBVALS;
	int		nWOVals = N_WOVALS;
	int		ival[N_RBVALS];
	int		oval[N_WOVALS];
	int		verb = 0;

	int		TrigOutVal = 0;

	char		filename[NUM_CHANNELS][64];
        char		cfilenam[NUM_CHANNELS][64];
	char		*p;
        short           *pSh = (short *)&scratch[0];
        short           shI;
        short           shQ;
	ofstream	*pOf[NUM_CHANNELS];

// B. Holub: Apr. 26, 2017: required for eQuench read back sequence
unsigned
        int            eQ_Param[16];
        int            eQi;
unsigned
        short          *p_eQsh;

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	if( Log >= SCOPE_LOG_LEVEL  ) {
	    i = 0;
            while( i < NUM_CHANNELS ) {
                memset(&filename[i][0], 0, sizeof(64));
                memset(&cfilenam[i][0], 0, sizeof(64));
                sprintf(&filename[i][0],"%s%02d",
                                              FILENAME_SCOPE_IQDATA, i);
                sprintf(&cfilenam[i][0],"C%s%02d",
                                              FILENAME_SCOPE_IQDATA, i);
                strcat(&filename[i][0], ".TXT");
                strcat(&cfilenam[i][0], ".TXT");
                i++;
                }
            }

    epicsInt32 ref[4];

    ok_ErrorCode  okErr;
    okTDeviceInfo okDevInfo;

    okErr = okFrontPanel_GetDeviceInfo(H, &okDevInfo);
    // cout << "update: SN:" << okDevInfo.serialNumber << endl;
    if( !okErr ) AERR(setStringParam(P_ok_SN, okDevInfo.serialNumber));

	i = 0;
	do
		{
		// verb = (i >= 32 && i <= 36)? 1:0;
		
		ival[i] = WireInGet( H, i, verb );
		if( verb )
			{
			cout << "WireInGet[" << i << "] = 0x" << setw(8) << hex << ival[i] << " ";
			cout << " = dec = " << dec << ival[i] << endl;
			}
		i++;
		}
	while( i < nRBVals );

	AERR( setIntegerParam( P_WinRB_EP00, ival[ 0] ) );
	AERR( setIntegerParam( P_WinRB_EP01, ival[ 1] ) );
	AERR( setIntegerParam( P_WinRB_EP02, ival[ 2] ) );
	AERR( setIntegerParam( P_WinRB_EP03, ival[ 3] ) );
	AERR( setIntegerParam( P_WinRB_EP04, ival[ 4] ) );
	AERR( setIntegerParam( P_WinRB_EP05, ival[ 5] ) );
	AERR( setIntegerParam( P_WinRB_EP06, ival[ 6] ) );
	AERR( setIntegerParam( P_WinRB_EP07, ival[ 7] ) );
	AERR( setIntegerParam( P_WinRB_EP08, ival[ 8] ) );
	AERR( setIntegerParam( P_WinRB_EP09, ival[ 9] ) );
	AERR( setIntegerParam( P_WinRB_EP10, ival[10] ) );
	AERR( setIntegerParam( P_WinRB_EP11, ival[11] ) );
	AERR( setIntegerParam( P_WinRB_EP12, ival[12] ) );
	AERR( setIntegerParam( P_WinRB_EP13, ival[13] ) );
	AERR( setIntegerParam( P_WinRB_EP14, ival[14] ) );
	AERR( setIntegerParam( P_WinRB_EP15, ival[15] ) );
	AERR( setIntegerParam( P_WinRB_EP16, ival[16] ) );
	AERR( setIntegerParam( P_WinRB_EP17, ival[17] ) );
	AERR( setIntegerParam( P_WinRB_EP18, ival[18] ) );
	AERR( setIntegerParam( P_WinRB_EP19, ival[19] ) );
	AERR( setIntegerParam( P_WinRB_EP20, ival[20] ) );
	AERR( setIntegerParam( P_WinRB_EP21, ival[21] ) );
	AERR( setIntegerParam( P_WinRB_EP22, ival[22] ) );
	AERR( setIntegerParam( P_WinRB_EP23, ival[23] ) );
	AERR( setIntegerParam( P_WinRB_EP24, ival[24] ) );
	AERR( setIntegerParam( P_WinRB_EP25, ival[25] ) );
	AERR( setIntegerParam( P_WinRB_EP26, ival[26] ) );
	AERR( setIntegerParam( P_WinRB_EP27, ival[27] ) );
	AERR( setIntegerParam( P_WinRB_EP28, ival[28] ) );
	AERR( setIntegerParam( P_WinRB_EP29, ival[29] ) );
	AERR( setIntegerParam( P_WinRB_EP30, ival[30] ) );
	AERR( setIntegerParam( P_WinRB_EP31, ival[31] ) );
	AERR( setIntegerParam( P_WinRB_EP32, ival[32] ) );
	AERR( setIntegerParam( P_WinRB_EP33, ival[33] ) );
	AERR( setIntegerParam( P_WinRB_EP34, ival[34] ) );
	AERR( setIntegerParam( P_WinRB_EP35, ival[35] ) );
	AERR( setIntegerParam( P_WinRB_EP36, ival[36] ) );

// Nathan, March 20, 2012
	okFrontPanel_UpdateWireOuts(H);
	i = 0;
	do
		{
		oval[i] = okFrontPanel_GetWireOutValue(H, (0x20 + i) );
		i++;
		}
	while( i < nWOVals );

	AERR( setIntegerParam( P_WO_EP20, oval[ 0] ) );
	AERR( setIntegerParam( P_WO_EP21, oval[ 1] ) );
	AERR( setIntegerParam( P_WO_EP22, (int)(short)oval[ 2] ) );
	AERR( setIntegerParam( P_WO_EP23, (int)(short)oval[ 3] ) );
	AERR( setIntegerParam( P_WO_EP24, (int)(short)oval[ 4] ) );
	AERR( setIntegerParam( P_WO_EP25, (int)(short)oval[ 5] ) );
	AERR( setIntegerParam( P_WO_EP26, (int)(short)oval[ 6] ) );
	AERR( setIntegerParam( P_WO_EP27, (int)(short)oval[ 7] ) );
	AERR( setIntegerParam( P_WO_EP28, oval[ 8] ) );
        AERR( setIntegerParam( P_WO_EP29, (int)(short)oval[ 9] ) );
	AERR( setIntegerParam( P_WO_EP2a, oval[10] ) );
	AERR( setIntegerParam( P_WO_EP2b, oval[11] ) );
	AERR( setIntegerParam( P_WO_EP2c, oval[12] ) );
	AERR( setIntegerParam( P_WO_EP2d, oval[13] ) );
        AERR( setIntegerParam( P_WO_EP2e, (int)(short)oval[14] ) );
	AERR( setIntegerParam( P_WO_EP2f, oval[15] ) );
	AERR( setIntegerParam( P_WO_EP30, (int)(short)oval[16] ) );
	AERR( setIntegerParam( P_WO_EP31, (int)(short)oval[17] ) );
	AERR( setIntegerParam( P_WO_EP32, (int)(short)oval[18] ) );
	AERR( setIntegerParam( P_WO_EP33, (int)(short)oval[19] ) );
	AERR( setIntegerParam( P_WO_EP34, (int)(short)oval[20] ) );
	AERR( setIntegerParam( P_WO_EP35, (int)(short)oval[21] ) );
	AERR( setIntegerParam( P_WO_EP36, (int)(short)oval[22] ) );
	AERR( setIntegerParam( P_WO_EP37, (int)(short)oval[23] ) );
	AERR( setIntegerParam( P_WO_EP38, (int)(short)oval[24] ) );
	AERR( setIntegerParam( P_WO_EP39, (int)(short)oval[25] ) );
	AERR( setIntegerParam( P_WO_EP3a, oval[26] ) );
	AERR( setIntegerParam( P_WO_EP3b, oval[27] ) );
	AERR( setIntegerParam( P_WO_EP3c, oval[28] ) );
	AERR( setIntegerParam( P_WO_EP3d, oval[29] ) );
	AERR( setIntegerParam( P_WO_EP3e, oval[30] ) );
	AERR( setIntegerParam( P_WO_EP3f, oval[31] ) );

    //**********************************************************************************************************************************
    // B. Holub: Apr. 26, 2017: eQuench read back sequence
    // cout << "Function P_EQ_Rd called, val=" << val << endl;

    lock();
    for( eQi = 0; eQi < 10; eQi++ ) {
      p_eQsh = (unsigned short *)&eQ_Param[eQi];
      HWERR2(pasynUser, okFrontPanel_SetWireInValue (H, EP_EQ_MUX, (eQi<<2),0x3C) );
      // cout << "i=" << dec << eQi << " SetWireIn 0x" << hex << setfill('0') << setw(4) << EP_EQ_MUX << " Value=0x" << setfill('0') << setw(4) << (eQi<<2) << " Mask=" << setfill('0') << setw(4) << 0x1C << " ";
      okFrontPanel_UpdateWireIns(H);

      HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS_EP0x41, ACTIONS_EQ_Strobe_WireOut) );
      // cout << " ActivateTriggerIn 0x" << hex << setfill('0') << setw(4) << EP_ACTIONS_EP0x41 << " Value=" << dec << ACTIONS_EQ_Strobe_WireOut << " ";

      okFrontPanel_UpdateWireOuts(H);
      *p_eQsh = okFrontPanel_GetWireOutValue(H, EP_EQ_WOut_LSW);
      // cout << " WireOut=0x" << hex << setfill('0') << setw(4) << EP_EQ_WOut_LSW << "=" << hex << setfill('0') << setw(4) << *p_eQsh << " ";

      p_eQsh++;
      *p_eQsh = okFrontPanel_GetWireOutValue(H, EP_EQ_WOut_MSW);
      // cout << " WireOut=0x" << hex << setfill('0') << setw(4) << EP_EQ_WOut_MSW << "=" << hex << setfill('0') << setw(4) << *p_eQsh << " ";
      // cout << " EP[" << (eQi+1) << "]=" << eQ_Param[eQi] << endl;
      }
    unlock();

    AERR( setIntegerParam( P_EQ_EP1_RB , eQ_Param[0] ) );
    AERR( setIntegerParam( P_EQ_EP2_RB , eQ_Param[1] ) );
    AERR( setIntegerParam( P_EQ_EP3_RB , eQ_Param[2] ) );
    AERR( setIntegerParam( P_EQ_EP4_RB , eQ_Param[3] ) );
    AERR( setIntegerParam( P_EQ_EP5_RB , eQ_Param[4] ) );
    AERR( setIntegerParam( P_EQ_EP6_RB , eQ_Param[5] ) );
    AERR( setIntegerParam( P_EQ_EP7_RB , eQ_Param[6] ) );
    AERR( setIntegerParam( P_EQ_EP8_RB , eQ_Param[7] ) );
    AERR( setIntegerParam( P_EQ_EP9_RB , eQ_Param[8] ) );
    AERR( setIntegerParam( P_EQ_EPA_RB , eQ_Param[9] ) );

    //**********************************************************************************************************************************
    // Nathan: March 14, 2012
    /* Don't activate trigger here, do it manually from a push button. Retrigger at the end
    HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS, ACTIONS_ADC_START) );
    */

    ref[0]=okFrontPanel_GetWireOutValue(H, EP_REF_A); // 0x36
    ref[1]=okFrontPanel_GetWireOutValue(H, EP_REF_B); // 0x37
    ref[2]=okFrontPanel_GetWireOutValue(H, EP_REF_C); // 0x38
    ref[3]=okFrontPanel_GetWireOutValue(H, EP_REF_D); // 0x39

    double ref_pha = angle(ref[0]-ref[2], ref[1]-ref[3]);

    AERR(setDoubleParam(P_ref_phase, ref_pha * 180.0 / M_PI));

    // okFrontPanel_UpdateTriggerOuts(H);

    TrigOutVal = 0;

    for( i = 0; i < 16; i++) {
      ii = 1<<i;
      if( okFrontPanel_IsTriggered(H, EP_STATUS, ii ) )
         TrigOutVal |= ii; else TrigOutVal &= ~ii;
      }

    AERR( setIntegerParam( P_TO_EP60, TrigOutVal ) );

    int done[2] = { 0, 0 };


    // STATUS_ADC2_DONE =  2 - not functional any more: Nathan, March 14, 2012
    // done[1] = okFrontPanel_IsTriggered(H, EP_STATUS, STATUS_ADC2_DONE);

    done[0] = TrigOutVal & STATUS_ADC_DONE;

    // OCTOBER 22, 2013 - 
    // TriggerOut, EP = 0x60 does not seem to function any more!?

    // done[0] = (oval[0] & 0x01)? 0:1; // oval[0] = EP 0x20, bit[0] = 1 = ADC busy

    // if( (oval[0] & 0x01) == 0 ) done[0] = done[1]?1:0;
    /*
    done[0] = 1;
    cout << "done[0] = " << hex << setw(8) << done[0] << " ";
    cout << "done[1] = " << hex << setw(8) << done[1] << " ";
    cout << "EP_STATUS=" << EP_STATUS << " STAT_ADC_DONE=" << STATUS_ADC_DONE << endl;
    */

    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s: Status %u\n",
              portName, done[0] );

    if(done[0]) {

        epicsTime start;

        start = epicsTime::getCurrent();

        HWERR2(pasynUser,
               okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS,
                                                 ACTIONS_RESET_ADC_POINTER) );

	// EP_ADC = 0xa0, BLOCK_SIZE = 1024, ADC_LENGTH = 0x4000 = 16384

        long ret = okFrontPanel_ReadFromBlockPipeOut(H, EP_ADC, BLOCK_SIZE, ADC_LENGTH,
                                          (unsigned char*)&scratch[0]);

        for(size_t c = 0; c < NUM_CCHN; c++) {
            for(size_t i = 0; i < DAC_COUNT; i++) {
                ix = c*2 + i*NUM_CHANNELS;
                shI= *(pSh+ix);
                ix++;
                shQ= *(pSh+ix);
                cscratch[c][i][0] = shI;
                cscratch[c][i][1] = shQ;
                cchn[c][i] = complex<double>( zoom_v_scale[c]*((double)shI / 32767.0),
                                              zoom_v_scale[c]*((double)shQ / 32767.0) );
                }
            if( Log >= SCOPE_LOG_LEVEL ) {
                pOf[c] = new ofstream( &cfilenam[c][0] );
                    for(size_t i = 0; i < DAC_COUNT; i++) {
                        *pOf[c] << setw(3) << i << " ret=" << ret << " II=";
                        *pOf[c] << setw(6) << cscratch[c][i][0];
                        *pOf[c] << " QQ=";
                        *pOf[c] << setw(6) << cscratch[c][i][1] << " I=";
                        *pOf[c] << fixed << setprecision(3) << setw(6) << real(cchn[c][i]);
                        *pOf[c] << " Q=";
                        *pOf[c] << fixed << setprecision(3) << setw(6) << imag(cchn[c][i]);
                        *pOf[c] << " A=";
                        *pOf[c] << fixed << setprecision(3) << setw(6) << abs(cchn[c][i]);
                        *pOf[c] << endl;
                    }
		pOf[c]->close();
		delete pOf[c];
                }
            }

	if( Log >= SCOPE_LOG_LEVEL )
		{
		ofstream	LogFileRaw(FILENAME_SCOPE_RAWDATA);
		cout << "Dumping ADC raw data to file: " << FILENAME_SCOPE_RAWDATA << flush;

		i = 0;	p = (char *)&scratch[0];

		short	sht	= 0;
	unsigned
		short	usht	= 0;

		while( i < ADC_LENGTH )
			{
			int	ii = 0;
			char	*pChar[2];

			pChar[0] = (char *)&sht;
			pChar[1] = (char *)&usht;

			ii = 0xff & (unsigned char)*(p + i);

			LogFileRaw << setw(6) << i << " 0x" << hex << setfill('0') << setw(8) << ii << " = ";
			LogFileRaw << dec << setfill(' ') << setw(8) << ii << " DEC -> ";

			if( !( i % 2 ) )
				{
				*(pChar[0] + 0) = 0xff & (unsigned char)*(p + i);
				*(pChar[1] + 0) = 0xff & (unsigned char)*(p + i);
				}
			else
				{
				*(pChar[0] + 1) = 0xff & (unsigned char)*(p + i);
				*(pChar[1] + 1) = 0xff & (unsigned char)*(p + i);

				LogFileRaw << "SHORT = 0x" << hex << setfill('0') << setw(8) << sht << " = ";
				LogFileRaw << dec << setfill(' ') << setw(8) << sht << " DEC ";
				LogFileRaw << "USHORT = 0x" << hex << setfill('0') << setw(8) << usht << " = ";
				LogFileRaw << dec << setfill(' ') << setw(8) << usht << " DEC ";
				sht = usht = 0;
				}

			LogFileRaw << endl;

			i++;
			}

		LogFileRaw.close();
		cout << " - done." << flush;
		}

        if( ret < 0 )
            HWERR2(pasynUser, (ok_ErrorCode)ret);
        else if ( (size_t)ret != ADC_LENGTH) {
            asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s: Incomplete recv %ld of %lu\n",
                      portName, ret, (unsigned long)(DAC_LENGTH));

            //TODO: how to handle this???

            HWERR2(pasynUser, ok_TransferError);

        } else {

            asynPrintIO(pasynUser, ASYN_TRACEIO_DRIVER,
                        (char*)&scratch[0], ret,
                        "%s: ADC Done\n", portName);

            scratch_t::const_iterator S=scratch.begin();

            /* ADC buffer contains channels sequencially
             * [ C0, C1, C2, ...]
             * For each channel I/Q pairs are interlaced
             * [ I0, Q0, -I1, -Q1, ... ]
             * Odd #'d pairs are inverted (for some reason)
             */

            for(size_t c = 0; c < NUM_CHANNELS; c++) {

		if( Log >= SCOPE_LOG_LEVEL )
			{
			pOf[c] = new ofstream( &filename[c][0] );
			cout << "Dumping I/Q data to file: " << &filename[c][0] << flush;
			}

                wfs_t::mapped_type wI = wfs[PAIR(P_adc_I,c)],
                                   wQ = wfs[PAIR(P_adc_Q,c)];

                std::vector<double>::iterator
                        I = wI->begin(),
                        Q = wQ->begin();

                for(size_t i = 0; i < DAC_COUNT; ++i, ++I, ++Q) {
			*I = zoom_v_scale[c] * (*S++ / 32767.0);
			*Q = zoom_v_scale[c] * (*S++ / 32767.0);

		/*
			Starting from bitfile rev. 88 the "host interfaces now have their binary points
			aligned with the most significant bit of each 16-bit field of interface.
			So the cav, fwd, rev and ref readbacks are doubled." Nathan 7/27/2012
		*/

		if( Log >= SCOPE_LOG_LEVEL )
			{
			*pOf[c] << dec << setw(6) << i << " zoom_v_scale = " << zoom_v_scale << " ";
			*pOf[c] << dec << "I = " << setw(8) << *I << " ";
			*pOf[c] << dec << "Q = " << setw(8) << *Q << " ";
			}

                    if( i%2 ) {
                        *I *= -1;
                        *Q *= -1;
			if( Log >= SCOPE_LOG_LEVEL )
				{
				*pOf[c] << dec << " -I = " << setw(8) << *I << " ";
				*pOf[c] << dec << " -Q = " << setw(8) << *Q << " ";
				}
                    }
		if(  Log >= SCOPE_LOG_LEVEL ) *pOf[c] << endl;
                }
		if( Log >= SCOPE_LOG_LEVEL )
			{
			cout << "Closing I/Q data file: " << &filename[c][0] << endl;
			pOf[c]->close();
			delete pOf[c];
			}

                AERR(doCallbacksFloat64Array(&(*wI)[0], DAC_COUNT, P_adc_I, c));
                AERR(doCallbacksFloat64Array(&(*wQ)[0], DAC_COUNT, P_adc_Q, c));
            }

	// Retrigger the scope data acquisition
	okFrontPanel_UpdateWireIns(H);

	// If the FB_Trig is requested ... 
	if( FB_TrigReq )
		{
		// Wait until previous ramp finished
		ii = 0;
		do
			{
			okFrontPanel_UpdateWireOuts(H);
			jj = okFrontPanel_GetWireOutValue(H, EP_WireOut_StatusBits); // EP_WireOut_StatusBits = 0x20, mask RampBusy = 0x02
			ii++;
			}
		while( ii < 100 && (jj && STATUS_FB_Busy));

		// Comment out for testing trig from external hardware only 08/12/2012

		HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS, ACTIONS_FB_SETPOINT_RAMP_TRIG) );
		okFrontPanel_UpdateTriggerOuts(H);

		// cout << "Commented out 08/20/12 : Update -> ActivateTriggerIn: FB Ramp: EP=0x" << hex << setw(2) << EP_ACTIONS;
		// cout << " - Val=0x" << setfill('0') << setw(2) << ACTIONS_FB_SETPOINT_RAMP_TRIG << dec << endl;
		//
		// Obs! ACTIONS_FB_SETPOINT_RAMP_TRIG MUST BE TRIGGERED ONCE AFTER WRITING TO THE TABLE
		// THIS TRIG OBVIOUSLY ACTUALLY TRIGGERS WRITE ALSO TO SOME OTHER BUFFERS !!!
		//
		// Don't trigger if external trigger mode!
		// FB_TrigReq = 1;
		}
	else
		okFrontPanel_UpdateWireOuts(H);

	// If data acquisition in run mode

	int run_mode = 0;

	AERR(getIntegerParam(P_scope_trigger_run, &run_mode));

	if( run_mode )
		HWERR2(pasynUser, okFrontPanel_ActivateTriggerIn(H, EP_ACTIONS, ACTIONS_ADC_START) );

	// else stop mode = don't retrigger
        }

        double dT = epicsTime::getCurrent() - start;

        double dTrig = start - lastTrig;
        lastTrig = start;

        AERR(setDoubleParam(P_adc_time, dT*1000.0));
        AERR(setDoubleParam(P_trig_time, dTrig*1000.0));
        int cnt = 0;
        AERR(getIntegerParam(P_adc_count, &cnt));
        AERR(setIntegerParam(P_adc_count, cnt+1));

    }
    // --- End of if done[0] = 1;
}

void llrfPort::report(FILE *fp, int details)
{
    asynPortDriver::report(fp, details);

    fprintf(fp, "Serial#: %s\n", serial.c_str());
}

AsynUser::AsynUser(const char *portName, int addr)
{
    usr = pasynManager->createAsynUser(&AsynUser::procCB, NULL);
    if(!usr)
        throw std::bad_alloc();
try{
    AERR(pasynManager->connectDevice(usr, portName, addr));

    usr->userPvt = (void*)this;
}catch(...){
    pasynManager->freeAsynUser(usr);
    throw;
}
}

AsynUser::AsynUser(asynUser *other)
    :usr(other)
{}

AsynUser::~AsynUser()
{
    if(!usr)
        return;
    pasynManager->freeAsynUser(usr);
}

const char* AsynUser::name() throw()
{
    const char *ret=0;
    if(pasynManager->getPortName(usr, &ret)!=asynSuccess)
        return "<not connected>";
    return ret;
}

asynUser*
AsynUser::release()
{
    asynUser *t=usr;
    usr=0;
    return t;
}

void
AsynUser::process()
{
    asynPrint(usr, ASYN_TRACE_FLOW, "%s: %s\n", name(), BOOST_CURRENT_FUNCTION);
}

void AsynUser::procCB(asynUser *usr)
{
    AsynUser *inst = (AsynUser*)usr->userPvt;
try {
    inst->process();

} catch(std::exception& e) {
    asynPrint(usr, ASYN_TRACE_ERROR, "%s: %s unhandled exception '%s'\n",
              inst->name(), BOOST_CURRENT_FUNCTION, e.what());
}
}

extern "C"
void createOKPort(const char *port, const char *serial, int debug)
{
    // can't delete it anyway...
    new llrfPort(port, serial, debug);
}

#include <epicsExport.h>
#include <iocsh.h>

/* createOKPort */
static const iocshArg createOKPortArg0 = { "portname",iocshArgString};
static const iocshArg createOKPortArg1 = { "serial#",iocshArgString};
static const iocshArg createOKPortArg2 = { "debug",iocshArgInt};
static const iocshArg * const createOKPortArgs[] = {&createOKPortArg0,&createOKPortArg1,&createOKPortArg2};
static const iocshFuncDef createOKPortFuncDef = {"createOKPort",3,createOKPortArgs};
static void createOKPortCallFunc(const iocshArgBuf *args)
{
    createOKPort(args[0].sval,args[1].sval, args[2].ival);
}

static
void llrfPortReg()
{
    iocshRegister(&createOKPortFuncDef,createOKPortCallFunc);
}

epicsExportRegistrar(llrfPortReg);
