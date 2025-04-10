#ifndef AWGPORT_H
#define AWGPORT_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

#include <epicsTypes.h>
#include <epicsThread.h>
#include <epicsGuard.h>
#include <epicsTime.h>

#include <asynPortDriver.h>

#include "registers.h"

#include <okFrontPanelDLL.h>

class llrfPort : public asynPortDriver
{
public:
    llrfPort(const char *portName, const char *serial, int debug);
    virtual ~llrfPort();

    virtual void report(FILE *fp, int details);
    virtual asynStatus connect(asynUser *pasynUser);
    virtual asynStatus disconnect(asynUser *pasynUser);
    virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual);
    virtual asynStatus readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int *eomReason);
    virtual asynStatus readInt16Array(asynUser *pasynUser, epicsInt16 *value, size_t nElements, size_t *nIn);
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);

    virtual asynStatus writeFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements);
    virtual asynStatus readFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements, size_t *nIn);

//protected:
    // Parameter IDs must be contigious
#define FIRST_CMD P_model

    // static info
    int P_model;
    int P_id;
    int P_major, P_minor;
    int P_serial;

    // software info
    int P_cnt_t;
    int P_adc_count;
    int P_adc_time;
    int P_trig_time;

    int P_rf_clk;

    // commands
    int P_load;
    int P_reset;
    int	P_sdramReset;
    int P_update;

    // settings
    int P_bit_dir;
    int P_bit_file;

    int P_cav_type;

/********************************************************/
/*							*/
/* March 1, 2012					*/
	int	P_ff_dutycycle;
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* March 14, 2012					*/
	int	P_WinRB_EP00;
	int	P_WinRB_EP01;
	int	P_WinRB_EP02;
	int	P_WinRB_EP03;
	int	P_WinRB_EP04;
	int	P_WinRB_EP05;
	int	P_WinRB_EP06;
	int	P_WinRB_EP07;
	int	P_WinRB_EP08;
	int	P_WinRB_EP09;
	int	P_WinRB_EP10;
	int	P_WinRB_EP11;
	int	P_WinRB_EP12;
	int	P_WinRB_EP13;
	int	P_WinRB_EP14;
	int	P_WinRB_EP15;
	int	P_WinRB_EP16;
	int	P_WinRB_EP17;
/********************************************************/
/*							*/
/* June 6, 2012						*/
	int	P_WinRB_EP18;
	int	P_WinRB_EP19;
	int	P_WinRB_EP20;
	int	P_WinRB_EP21;
	int	P_WinRB_EP22;
	int	P_WinRB_EP23;
	int	P_WinRB_EP24;
	int	P_WinRB_EP25;
	int	P_WinRB_EP26;
	int	P_WinRB_EP27;
	int	P_WinRB_EP28;
	int	P_WinRB_EP29;
	int	P_WinRB_EP30;
	int	P_WinRB_EP31;
	int	P_WinRB_EP32;
	int	P_WinRB_EP33;
	int	P_WinRB_EP34;
	int	P_WinRB_EP35;
	int	P_WinRB_EP36;

/********************************************************/
/*							*/
/* September 17, 2012					*/
	int	P_WinRB_EP37;
	int	P_WinRB_EP38;
	int	P_WinRB_EP39;
	int	P_WinRB_EP40;
	int	P_WinRB_EP41;
	int	P_WinRB_EP42;
	int	P_WinRB_EP43;
	int	P_WinRB_EP44;
	int	P_WinRB_EP45;
	int	P_WinRB_EP46;
	int	P_WinRB_EP47;
	int	P_WinRB_EP48;
	int	P_WinRB_EP49;
	int	P_WinRB_EP50;
	int	P_WinRB_EP51;
	int	P_WinRB_EP52;
	int	P_WinRB_EP53;
	int	P_WinRB_EP54;
	int	P_WinRB_EP55;
	int	P_WinRB_EP56;
	int	P_WinRB_EP57;
	int	P_WinRB_EP58;
	int	P_WinRB_EP59;
	int	P_WinRB_EP60;
	int	P_WinRB_EP61;
	int	P_WinRB_EP62;
	int	P_WinRB_EP63;

/********************************************************/
/*							*/
/* Apr 5, 2012						*/
/* Debugging scope channel multiplexer			*/

	int	P_WinADCMUX;

/*							*/
/********************************************************/
/********************************************************/
/* Apr 23, 2012						*/

	int	P_Win_Tuner_PhOffset;
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* March 19, 2012					*/
	int	P_WO_EP20;
	int	P_WO_EP21;
	int	P_WO_EP22;
	int	P_WO_EP23;
	int	P_WO_EP24;
	int	P_WO_EP25;
	int	P_WO_EP26;
	int	P_WO_EP27;
	int	P_WO_EP28;
	int	P_WO_EP29;
	int	P_WO_EP2a;
	int	P_WO_EP2b;
	int	P_WO_EP2c;
	int	P_WO_EP2d;
	int	P_WO_EP2e;
	int	P_WO_EP2f;
	int	P_WO_EP30;
	int	P_WO_EP31;
	int	P_WO_EP32;
	int	P_WO_EP33;
	int	P_WO_EP34;
	int	P_WO_EP35;
	int	P_WO_EP36;
	int	P_WO_EP37;
	int	P_WO_EP38;
	int	P_WO_EP39;
	int	P_WO_EP3a;
	int	P_WO_EP3b;
	int	P_WO_EP3c;
	int	P_WO_EP3d;
	int	P_WO_EP3e;
	int	P_WO_EP3f;
/*							*/
/********************************************************/
/* Oct. 19, 2012					*/
	int	P_TO_EP60;
/*							*/
/********************************************************/
/*							*/
/* March 1, 2012					*/
	int	P_ff_enable;
	int	P_scope_trigger;
	int	P_scope_trigger_run; // Run = 1, Stop = 0
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* April 9, 2012					*/

	int	P_zoom_selector;
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* April 13, 2012					*/

	int	P_rampinterval_SP;
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* September 25, 2020					*/

	int	P_naFreq_SP;
        int     P_naPII_SP;
        int     P_naPIC0_SP;
        int     P_naPIC1_SP;
        int     P_naIntensity_SP;
        int     P_na_I;
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* October 14, 2020					*/

	int	P_PG_Enbl_SP;
        int     P_PG_AltKp_SP;
        int     P_PG_AltKi_SP;
        int     P_PG_AltRotI_SP;
        int     P_PG_AltRotQ_SP;
        int     P_PG_SwTime_SP;
        int     P_PG_GO;
/*							*/
/********************************************************/

/********************************************************/
/*							*/
/* April 13, 2012					*/

	int	P_fbramptrig;
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* April 23, 2012 TrigIns				*/
	int	P_Force_RFInhTrip;
	int	P_Reset_RFInh;
	int	P_Force_AVAGO2Trip;
	int	P_Reset_AVAGO2;
	int	P_Force_PPSTrip;
	int	P_Reset_PPS;
	int	P_Force_QUENCHTrip;
	int	P_Reset_QUENCH;
	int	P_Force_SoftTrip;
	int	P_Reset_SoftTrip;
	int	P_Force_VacTrip;
	int	P_Reset_VacTrip;
	int	P_na_Start;
	int	P_na_Stop;

/* Dec. 3, 2012						*/
	int	P_TSNext;
	int	P_TSReset;
/*							*/
/* Aug. 24, 2016: Activate Limiter			*/
	int	P_Force_Limit;
/*							*/
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* Sep. 24, 2018					*/

	int	P_TopTrDly;
/*							*/
/********************************************************/
    int P_limiter;
    int P_ff_mode;
    int P_RefPr_En;

    int P_scope_zoom, P_scope_dly;

    int P_fb;
    int P_gain_I, P_gain_P;

    int P_loop_phase;

    int P_ff_T;

    int P_sp_I, P_sp_Q;
    int	P_sp_IPVAL, P_sp_QPVAL;

/*------------------------------------------------------*/
/********************************************************/
/* JUNE 4, 2015: Added second set of I/Q setpoints      */
/* for phase toggling by 100 deg (G.Wang experiment     */
/* EP 0, bit 0x08 enables toggling (the second set)     */
/* EP 0x15, 0x16 are the second set of values           */

    int P_spI2, P_spQ2;
    int P_EnblI2Q2;

/********************************************************/
/*------------------------------------------------------*/
/*------------------------------------------------------*/
/********************************************************/
/* MAR 15, 2016: Added two signed integers for          */
/* debugging the Limiter: Ep 0x17 = Limiter Level       */
/* Ep 0x1A = Number of clocks                           */

    int P_LimLev, P_NLimClks;

/********************************************************/
/*------------------------------------------------------*/
/*------------------------------------------------------*/
/* MAR. 21, 2017: Added Equench using WireIns           */
/* EP 0x1e as Most Significant Word,                    */
/* EP 0x1d as Least Significant Word                    */
/* Multiplexor(selector) WireIn EP 0x01,bits 4,3,2      */
/* Strobe WireIn: Trigger In EP 41 bit 13, 0x0d         */
/* Readbacks from WireOuts                              */
/* EP 0x3d as Most Significant Word,                    */
/* EP 0x3c as Least Significant Word                    */
/* Strobe WireOut: Trigger In EP 41 bit 14, 0x0e        */

    int P_EQ_EP1_SP;
    int P_EQ_EP2_SP;
    int P_EQ_EP3_SP;
    int P_EQ_EP4_SP;
    int P_EQ_EP5_SP;
    int P_EQ_EP6_SP;
    int P_EQ_EP7_SP;
    int P_EQ_EP8_SP;
    int P_EQ_EP9_SP;
    int P_EQ_EPA_SP;
    int P_EQ_EPB_SP;
    int P_EQ_EPC_SP;
    int P_EQ_EPD_SP;
    int P_EQ_EPE_SP;
    int P_EQ_EPF_SP;

    int P_EQ_EP1_RB;
    int P_EQ_EP2_RB;
    int P_EQ_EP3_RB;
    int P_EQ_EP4_RB;
    int P_EQ_EP5_RB;
    int P_EQ_EP6_RB;
    int P_EQ_EP7_RB;
    int P_EQ_EP8_RB;
    int P_EQ_EP9_RB;
    int P_EQ_EPA_RB;
    int P_EQ_EPB_RB;
    int P_EQ_EPC_RB;
    int P_EQ_EPD_RB;
    int P_EQ_EPE_RB;
    int P_EQ_EPF_RB;

    int P_EQ_Wr;
    int P_EQ_Rd;
    int P_EQ_Enbl;
/*------------------------------------------------------*/
/*------------------------------------------------------*/
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*======================================================*/
    int P_ff_IQ;

/* SEP. 25. 2017: Added External Circular Buffer Trigger*/
/* Enable (bit 9 to EP SDRAM                            */

    int P_EnblExtCbufTr;

    int P_fb_IQ;

    // readings

    int P_ref_phase;

    int P_adc_T;

    // 8x
    int P_adc_I, P_adc_Q;

    int	P_TS_RdAll;
    int P_TS_RdWord0_1;
    int	P_TS_RdWord1_0;
    int P_TS_RdWord2_0;

    int P_ramRstEn;
    int	P_ramRdEn;
    int	P_ramWrEn;
    int P_ramArm;
    int P_ramClrExTrig;

    int P_CircBufSWTripA;
    int P_CircBufSWTripB;
    int	P_ram_wrap;
    int P_ram_Lastpage;
    int P_ram_TrSrc;
    int P_circ_IQ;
    int	P_ram_IQ;
    int P_ram_I;
    int P_ram_Q;
    int P_ram_TS;
    int P_ok_SN;

#define LAST_CMD P_ok_SN

    const char* retParamName(int idx=0) {return retParamName(0,idx);}
    const char* retParamName(int list, int idx) {
        const char *ret=0;
        if(getParamName(list,idx,&ret)!=asynSuccess)
            return "<invalid>";
        return ret;
    }

private:
    static void shutdown(void* raw);
    void stop();

    void update(asynUser*);

    void SDRAM_Reset(asynUser *pasynUser);
    void SDRAM_RstEn(asynUser *pasynUser, int enable);
    void SDRAM_RdEn(asynUser *pasynUser, int enable);
    void SDRAM_WrEn(asynUser *pasynUser, int enable);
    int  SDRAM_Rd(asynUser *pasynUser);
    void SDRAM_RstExTrig(asynUser *pasynUser, int enable);

    std::string serial;

    okFrontPanel_HANDLE H;
    typedef ok_ErrorCode error_t;

    void hwError(error_t err, const char* file, int line,
                 asynUser *usr=0);

    void init(asynUser *pasynUser);

    typedef std::vector<epicsInt16> scratch_t;
    scratch_t scratch;

    bool connected[NUM_CHANNELS];
    bool open;
    bool nextdac;

    epicsTime lastTrig;

    double zoom_v_scale[NUM_CHANNELS];

    // indexed by param, channel pair (ie P_adc_I, 3)
    typedef std::map<std::pair<int,int>, std::vector<double>* > wfs_t;
    wfs_t wfs;

    bool zombie; // brains...

	double		FFI[DAC_COUNT];
	double		FFQ[DAC_COUNT];
	double		FBI[DAC_COUNT];
	double		FBQ[DAC_COUNT];

	int		FB_TrigReq;
 	int		Log;

	short		*pCirc0;

struct
	timespec	ts[3];

	char		bitfilename[256];
	char		bitdirname[256];

        short           naBuf[128];

        short           ascratch[NUM_CCHN][DAC_COUNT][2];
        short           cscratch[NUM_CCHN][DAC_COUNT][2];
        complex<double> cchn[NUM_CCHN][DAC_COUNT];        
};

#include <boost/current_function.hpp>

#define FLOW(USR) asynPrint(USR, ASYN_TRACE_FLOW, "%s: %s\n", portName, BOOST_CURRENT_FUNCTION)

/* translate c++ exceptions into asyn errors */

#define CATCHALL(USR) catch(std::exception& e) { \
asynPrint(USR, ASYN_TRACE_ERROR, "%s: %s: %s\n", portName, BOOST_CURRENT_FUNCTION, e.what()); \
return asynError; }

#define CATCHALL_CONTINUE(USR) catch(std::exception& e) { \
asynPrint(USR, ASYN_TRACE_ERROR, "%s: %s: %s\n", portName, BOOST_CURRENT_FUNCTION, e.what()); \
return; }

/* and vis versa */

#define AERR(RET) do { if((RET)!=asynSuccess) { \
std::ostringstream msg; msg<<portName<<": "<<BOOST_CURRENT_FUNCTION<<": on line "<<__LINE__; \
        throw std::runtime_error(msg.str()); } } while(0)

//! Container for asynUser
class AsynUser {
    asynUser *usr;

    AsynUser(const AsynUser&);
    AsynUser& operator=(const AsynUser&);
public:
    AsynUser(const char* port, int addr=-1);
    AsynUser(asynUser* other);

    virtual ~AsynUser();

    asynUser* release();

    const char *name() throw();

    operator asynUser*(){return usr;}

    asynUser* operator->(){return usr;}
    asynUser& operator*(){return *usr;}

    virtual void process();

private:
    static void procCB(asynUser *);
};

#endif // AWGPORT_H
