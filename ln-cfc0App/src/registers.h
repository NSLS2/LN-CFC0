#ifndef REGISTERS_H
#define REGISTERS_H

#define	OPAL_KELLY_BIT_FILENAME	"llrf_xem_SRC_Rev19.bit"
#define	OPAL_KELLY_LOG_FILENAME "llrf_log.txt"

#define	LOGGING_ENABLED		0

#define	SCOPE_LOG_LEVEL		3

#define	OPAL_KELLY_SERIAL_NO	"13130005RX"

// #define	M_PI		3.1415926535897932384626433	

// #define NUM_CHANNELS		       10
// OCT. 9, 2013 expanded to 8 channels ...
#define NUM_CHANNELS 16
#define NUM_CCHN     (NUM_CHANNELS/2)

#define	N_RBVALS		       37
#define	N_WOVALS		       32

#define	FILENAME_FF_TABLE_IQ	"FF_DATA_IQ.TXT"
#define	FILENAME_FF_TABLERAW	"FF_DATA_RAW.TXT"
#define	FILENAME_FB_TABLE_IQ	"FB_DATA_IQ.TXT"
#define	FILENAME_FB_TABLERAW	"FB_DATA_RAW.TXT"

#define	FILENAME_SCOPE_RAWDATA	"SCOPE_DATA_RAW.TXT"
#define	FILENAME_SCOPE_IQDATA	"SCOPE_DATA_IQ_"

#define	DAC_RESOLUTION_BITS	       16
#define	DAC_MAXVALUE		((1 << (DAC_RESOLUTION_BITS-1)) - 1)


#define	FF_TABLE_LENGTH		      512
#define	FF_TABLE_SIZE_BYTES	(FF_TABLE_LENGTH*2 * sizeof(short))

// For pipe transfers
#define BLOCK_SIZE		     1024

/* WireIn */

#define EP_SDRAM			0
#define SDRAM_READ		   (1<<0)
#define SDRAM_WRITE		   (1<<1)
#define SDRAM_RESET		   (1<<2)
#define SDRAM_CLR_EXTRIG	   (1<<3)
#define SDRAM_PAGE_mask		   0xe000 /* bits 15:13 */
#define SDRAM_PAGE_shft		      13

/*-------------------------------------------------*/
/* MAR. 21, 2017: Added Equench using WireIns      */
/* EP 0x1e as Most Significant Word,               */
/* EP 0x1d as Least Significant Word               */
/* Multiplexor(selector) WireIn EP 0x01,bits 4,3,2 */
/* Strobe WireIn: Trigger In EP 41 bit 13, 0x0d    */
/* Readbacks from WireOuts                         */
/* EP 0x3d as Most Significant Word,               */
/* EP 0x3c as Least Significant Word               */
/* Strobe WireOut: Trigger In EP 41 bit 14, 0x0e   */

#define EP_EQ_MUX                    0x01
#define EP_EQ_EP1                       0
#define EP_EQ_EP2                       1
#define EP_EQ_EP3                       2
#define EP_EQ_EP4                       3
#define EP_EQ_EP5                       4
#define EP_EQ_EP6                       5
#define EP_EQ_EP7                       6
#define EP_EQ_EP8                       7

#define EP_EQ_Win_MSW                0x1e
#define EP_EQ_Win_LSW                0x1d

#define EP_EQ_Enbl                   0x01
#define EP_EQ_EnbleBIT               0x40
#define EP_EQ_Stat                   0x20
#define EP_EQ_StatBIT                0x40
/*-------------------------------------------------*/
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*-------------------------------------------------*/
/* OCT. 14, 2020: Added PulsedGains using WireIns  */
/* EP 0x01  bit 0,1: Circuit Enable, Repeat Enable */
/* EP 0x18: alt_ki                                 */
/* EP 0x19: alt_rot_i                              */
/* EP 0x1b: switch time                            */
/* EP 0x1c: alt_rot_q                              */
/* TRIG IN 0x41 bit 12: GO                         */
/*=================================================*/
#define EP_PG_ENBL                   0x01
#define EP_PG_CircEnblBIT            0x01
#define EP_PG_ReptEnblBIT            0x02
#define EP_PG_ALTKi                  0x18
#define EP_PG_ALTROT_I               0x19
#define EP_PG_ALTROT_Q               0x1c
#define EP_PG_SWTIME                 0x1b
/*-------------------------------------------------*/
/* JUNE 4, 2015: Added second set of I/Q setpoints */
/* for phase toggling by 100 deg (G.Wang experiment*/
/* EP 0, bit 0x08 enables toggling (the second set)*/
#define PHS_TOGGLE_ENBL            (1<<8)
/*-------------------------------------------------*/
/*-------------------------------------------------*/
/* SEP. 25, 2017: Added External Circular Buffer   */
/* Trigger Enable (bit 9) = 0x200                  */
#define EXT_CBUFTR_ENBL            (1<<9)
/*-------------------------------------------------*/

#define EP_LENGTH		       2

#define	EP_CONTROL_0x02		    0x02
#define CONTROL_ZOOM_mask	  0x001f

#define EP_CONTROL		       3
#define CONTROL_ADCMUX_mask	  0x0007
#define CONTROL_CAVITY_TYPE	  0x0008
#define CONTROL_DEBUG_mask	  0x0070
#define CONTROL_DEBUG_shft	       4
/********************************************************/
/*							*/
/* March 1, 2012					*/
/* #define CONTROL_TRIG_SRC	  0x0080		*/
#define	CONTROL_REFPR_EN	  0x0080
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* March 1, 2012					*/
#define	CONTROL_FF_DUTY_CYCLE	  0x0200
/*							*/
/* Also called RETRIGGERING by Nathan			*/
/* Previously called CONTROL_TEST by Michael		*/
/*							*/
/********************************************************/

#define	EP_TUNER_PHASE_OFFSET	   0x04

#define CONTROL_DAC_WRITE_mask	 0x0c00
#define CONTROL_DAC_WRITE_shft	     10
#define CONTROL_DAC_READ	 0x1000
#define CONTROL_FF_ENABLE	 0x2000
#define CONTROL_LIMITER		 0x4000
#define CONTROL_FB		 0x8000

#define EP_GAIN_INT		      5

#define EP_ROT_I		      6
#define EP_ROT_Q		      7

/* Removed, set to Not assigned: Nathan, March 14, 2012	*/
// #define EP_SP_I			      8
// #define EP_SP_Q			      9
/*------------------------------------------------------*/
/* Reintroduced in v.77: Nathan, June 14, 2012		*/
/* for storage ring and linac				*/
/* i.e other than booster				*/
#define EP_SP_I			      8
#define EP_SP_Q			      9

/*-------------------------------------------------*/
/* JUNE 4, 2015: Added second set of I/Q setpoints */
/* for phase toggling by 100 deg (G.Wang experiment*/
/* EP 0, bit 0x08 enables toggling (the second set)*/
/* EP 0x15, 0x16 are the second set of values      */
#define EP_SP_I2                   0x15
#define EP_SP_Q2                   0x16
/*-------------------------------------------------*/
/* MAR 15, 2016: Added two signed integers for     */
/* debugging the Limiter: Ep 0x17 = Limiter Level  */
/* Ep 0x1A = Number of clocks                      */
#define EP_LIMLEVEL                0x17
#define EP_NLIMCLKS                0x1A
/*-------------------------------------------------*/

#define EP_1WIRE_CMD		     10

/********************************************************/
/*							*/
/* March 6, 2012					*/
#define	CONTROL_1WIRE_magic	   0x33
/*							*/
/********************************************************/
/********************************************************/
/*							*/
/* March 5, 2012					*/
#define	CONTROL_1WIRE_romid	  0x43
#define	CONTROL_1WIRE_mask	0xffff
/*							*/
/********************************************************/
/* March 14, 2012					*/
#define	EP_LLRF_XEM		  0x0b	// FB ramp interval between waypoints [clock cycles*16]
					// 0 ... 32767: 1 = each point will be output after 16 of 40MHz clock cycles
					// Recommended value = 50: Nathan, March 14, 2012
#define	EP_RAMPINTERVAL_SP	0x0b
/********************************************************/
/*							*/
/* March 13, 2012					*/
#define	EP_T_SCOPE_START	  0x10
/*							*/
/********************************************************/
#define EP_DELAY		    11

/********************************************************/
/*                                                      */
/* SEP. 21, 2018, C. Marques                            */
#define	EP_Tr_DLY	         0x0C
/*                                                      */
/********************************************************/
/********************************************************/
/*                                                      */
/* SEP. 25, 2020, C. Marques                            */
#define	EP_NA_FREQ	          0x11
#define	EP_NA_PII	          0x12
#define	EP_NA_PIC0	          0x13
#define	EP_NA_PIC1	          0x14
#define EP_NA_INTENSITY           0x03
/*                                                      */
/********************************************************/

/* WireOut */
#define	EP_WireOut_StatusBits	 0x20
#define	STATUS_FB_Busy		 0x02	// Bit No 03
#define	STATUS_FF_Busy		 0x04	// Bit No 03
#define	STATUS_RF_INHIBIT      0x0020	// Bit No 05

#define	EP_WireIn_Mux_RB	 0x21
#define	EP_ffImon_Setpoint_RB	 0x22
#define	EP_ffQmon_Setpoint_RB	 0x23
#define	EP_RAM_LASTPAGE		 0x2d
#define	EP_2fWIRE		 0x2f

#define	EP_WireOut_cavImon	 0x30
#define	EP_WireOut_cavQmon	 0x31
#define	EP_WireOut_fbImon	 0x38
#define	EP_WireOut_fbQmon	 0x39

#define EP_REF_A		 0x36
#define EP_REF_B		 0x37
#define EP_REF_C		 0x38
#define EP_REF_D		 0x39

#define EP_1WIRE_RES_0		 0x3a
#define EP_1WIRE_RES_1		 0x3b
#define EP_1WIRE_RES_2		 0x3c
#define EP_1WIRE_RES_3		 0x3d

/*-------------------------------------------------*/
/* MAR. 21, 2017: Added Equench using WireIns      */
/* EP 0x1e as Most Significant Word,               */
/* EP 0x1d as Least Significant Word               */
/* Multiplexor(selector) WireIn EP 0x01,bits 4,3,2 */
/* Strobe WireIn: Trigger In EP 41 bit 13, 0x0d    */
/* Readbacks from WireOuts                         */
/* EP 0x3d as Most Significant Word,               */
/* EP 0x3c as Least Significant Word               */
/* Strobe WireOut: Trigger In EP 41 bit 14, 0x0e   */

#define EP_EQ_WOut_MSW                0x3d
#define EP_EQ_WOut_LSW                0x3c
/*-------------------------------------------------*/
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*=================================================*/

/* TriggerIn */

#define EP_ACTIONS		 0x40

#define	ACTIONS_FORCE_SOFT_TRIP	0x01
#define	ACTIONS_RESET_SOFT_TRIP	0x02

#define	ACTIONS_FFTRIG	         0x03
#define	ACTIONS_DATA_WR	         0x06

#define	ACTIONS_FORCE_RF_INHIBIT 0x09
#define	ACTIONS_RESET_RF_INHIBIT 0x0a

#define	ACTIONS_FORCE_QUENCH	0x0d
#define	ACTIONS_RESET_QUENCH	0x0e

#define ACTIONS_RESET		    0
#define ACTIONS_ADC_START	    4 /* Scope acquisition trigger: Nathan, March 14, 2012	*/
#define ACTIONS_RESET_ADC_POINTER   5
#define ACTIONS_RESET_DAC_POINTER   6
#define ACTIONS_RAM_START	    7
#define	ACTIONS_FB_SETPOINT_RAMP_TRIG	8

#define	ACTIONS_CIRCA_TRIP	 0x0b
#define	ACTIONS_CIRCB_TRIP	 0x0c

#define ACTIONS_UPDATE_DS1825	   15

#define EP_ACTIONS_EP0x41	0x41

#define	ACTIONS_FORCE_VACTRIP	0x00
#define	ACTIONS_RESET_VACTRIP	0x01
#define	ACTIONS_FORCE_PPSTRIP	0x02
#define	ACTIONS_RESET_PPSTRIP	0x03
#define	ACTIONS_FORCE_AVAGO2	0x04
#define	ACTIONS_RESET_AVAGO2	0x05

#define	ACTIONS_TSNEXT		0x08
#define	ACTIONS_TSRESET		0x09

#define ACTIONS_LIMIT             15

/*-------------------------------------------------*/
/* OCT. 14, 2020: Added PulsedGains using WireIns  */
/* TRIG IN 0x41 bit 12: GO                         */
/*=================================================*/
#define ACTIONS_PG_GO             12
/*-------------------------------------------------*/
/* MAR. 21, 2017: Added Equench using WireIns      */
/* EP 0x1e as Most Significant Word,               */
/* EP 0x1d as Least Significant Word               */
/* Multiplexor(selector) WireIn EP 0x01,bits 4,3,2 */
/* Strobe WireIn: Trigger In EP 41 bit13= 8192DEC  */
/* Readbacks from WireOuts                         */
/* EP 0x3d as Most Significant Word,               */
/* EP 0x3c as Least Significant Word               */
/* Strobe WireOut: Trigger In EP 41 bit14=16384DEC */

#define ACTIONS_EQ_Strobe_WireIn            13
#define ACTIONS_EQ_Strobe_WireOut           14
/*-------------------------------------------------*/
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*=================================================*/

/* TriggerOut */

#define EP_STATUS		 0x60
#define STATUS_ADC_DONE		    1
#define STATUS_RAM_DONE		    2

//#define STATUS_ADC2_DONE	    2
//#define STATUS_FBRAMP_DONE	 0x02
#define STATUS_FBRAMP_DONE	 0x04

#define STATUS_AVAGO2_TRIG	 0x03
#define STATUS_PHJUMP_TRIG       0x04
#define	STATUS_PPS_EV		 0x05
#define	STATUS_RFINH_EV		 0x06

#define STATUS_ST_EV		 0x08
#define	STATUS_QUENCH_EV	 0x09
#define	STATUS_VAC_EV		 0x0a
#define	STATUS_FFRAMP_DONE	 0x0b

/* PipeIn */

#define EP_DAC			 0x80
#define DAC_LENGTH		 2048 /* bytes 					*/
#define DAC_COUNT	(DAC_LENGTH/4)/* 512 I/Q pairs (2x 2 bytes integers)	*/
// #define EP_SP		 0x81 /* Removed: Nathan, March 14, 2012	*/
#define EP_CFG			 0x82
//#define EP_OL			 0x83
#define EP_SP		 	 0x83 /* Nathan, March 14, 2012, FB table	*/

/* PipeOut */

#define EP_ADC			0xa0

#define ADC_LENGTH	      0x4000 /* 16384 bytes  = 8[channels]*2048 = 0x4000 */
#define ADC_COUNT	(ADC_LENGTH/4)
#define EP_CFG_RB	        0xa1
#define EP_NA_WF                0xb0
#define EP_RAM			0xb1

#define	CIRC_BYTES	  0x02000000 /* 32M bytes                               */
#define	CIRC_SHORTS       0x01000000 /* 16M short vals                          */
#define	CIRC_INT32        0x00800000 /*  8M int   vals                          */
#define	CIRC_DBL          0x00400000 /*  4M dbl   vals                          */

/****************************************************************************************/
/* OCT. 18, 2019 									*/
/*											*/
//#define	CIRC_BYTES	  0x20000000 /*512M bytes     	536 870 912			*/
//#define	CIRC_SHORTS       0x10000000 /*256M short vals	268 435 456			*/
//#define	CIRC_INT32        0x08000000 /*128M int   vals					*/
//#define	CIRC_DBL          0x04000000 /* 64M dbl   vals					*/
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


#define	CIRC_ADDR		   9 /* Must be set to the same addr in the record INP field!	*/

#define	EP_TS_WORD0		0x2a
#define	EP_TS_WORD1		0x2b
#define	EP_TS_WORD2		0x2c

#endif // REGISTERS_H
