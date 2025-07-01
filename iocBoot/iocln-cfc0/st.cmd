#!../../bin/linux-x86_64/ln-cfc0

#- You may have to change ln-cfc0 to something else
#- everywhere it appears in this file

< envPaths

epicsEnvSet("EPICS_CA_MAX_ARRAY_BYTES", "33554434")

cd "${TOP}"

epicsEnvSet("EPICS_CA_AUTO_ADDR_LIST","NO")
epicsEnvSet("EPICS_CA_ADDR_LIST","10.0.153.255")
epicsEnvSet("EPICS_CAS_AUTO_BEACON_ADDR_LIST","NO")
epicsEnvSet("EPICS_CAS_BEACON_ADDR_LIST","10.0.153.255")
epicsEnvSet("EPICS_TZ", "EST5EDT,M3.2.0/2,M11.1.0/2")

## Register all support components
dbLoadDatabase "dbd/ln-cfc0.dbd"
ln_cfc0_registerRecordDeviceDriver pdbbase

pwd
#
# 121000031B was the OpalKelly of linac version - went as spare for linac
# on OCT. 1, 2014
#
#system("$(TOP)/bin/linux-x86/okLoad_bitfile 121000031B llrf_xem20140328_SR.bit")
system("export LD_LIBRARY_PATH=$(EPICS_BASE)/lib/linux-x86_64:$(TOP)/lib/linux-x86_64:$LD_LIBRARY_PATH;$(TOP)/bin/linux-x86_64/okLoad_bitfile 13130005SL llrf_xem20141112_SR.bit")

okls()

#createOKPort("rfport","121000031B")
createOKPort("rfport","13130005SL")

## Load record instances
dbLoadRecords("db/scaleF.db", "PORT=rfport, S1=LN-RF, S2=RF, D1=CFC:0, SD0=Ref, SD2=Cav")
dbLoadRecords("db/okllrf.db", "PORT=rfport, S1=LN-RF, S2=RF, D1=CFC:0, SD0=Ref, SD2=Cav, SD6=RAM, D3=Osc:1, TBL1=FF, TBL2=FB, BFN=$(TOP)/BITFLD.TXT")
dbLoadRecords("db/okWireIn.db", "PORT=rfport, S1=LN-RF, D1=CFC:0, D9=NA, DA=PG, TBL2=FB")
dbLoadRecords("db/okWireInRB.db", "PORT=rfport, S1=LN-RF, S2=RF, D1=CFC:0, SD1=Tnr, D3=Osc:1, TBL1=FF, TBL2=FB")
dbLoadRecords("db/okWireOut.db", "PORT=rfport, S1=LN-RF, D1=CFC:0, SD0=Ref, SD1=Tnr, SD2=Cav, SD3=Fwd, SD4=Rfl, SD6=RAM, SD7=TS, SD8=EQ, TBL1=FF, TBL2=FB, RD=Xmtr:A0")
dbLoadRecords("db/okTbl.db", "PORT=rfport, S1=LN-RF, D1=CFC:0, D2=Cav, TBL=FF, SD0=Ref")
dbLoadRecords("db/okTrigIn.db", "PORT=rfport, S1=LN-RF, D1=CFC:0, D9=NA, DA=PG, Sc=CFC:S, SD7=TS")

dbLoadRecords("db/okadc.db", "PORT=rfport, S1=LN-RF, D1=CFC:0, SD0=Ref, SD1=Ref, ADDR=0")
dbLoadRecords("db/okadc.db", "PORT=rfport, S1=LN-RF, D1=CFC:0, SD0=Ref, SD1=Cav, ADDR=1")
dbLoadRecords("db/okadc.db", "PORT=rfport, S1=LN-RF, D1=CFC:0, SD0=Ref, SD1=Fwd, ADDR=2")
dbLoadRecords("db/okadc.db", "PORT=rfport, S1=LN-RF, D1=CFC:0, SD0=Ref, SD1=Rfl, ADDR=3")
dbLoadRecords("db/okadc.db", "PORT=rfport, S1=LN-RF, D1=CFC:0, SD0=Ref, SD1=Drv, ADDR=4")

dbLoadRecords("db/okmeasure.db", "S1=LN-RF, D1=CFC:0, D2=Cav, SD2=Cav, TBL1=FF, TBL2=FB")

dbLoadRecords("db/okSNo.db","S1=LN-RF,D1=CFC:0")
dbLoadRecords("db/iocAdminSoft.db", "IOC=RF-CT{$(IOC)}")
dbLoadRecords("db/reccaster.db", "P=RF-CT{${IOC}-RC}")
dbLoadRecords("db/asynRecord.db", "P=LN-RF{CFC:0},R=port, PORT=rfport, ADDR=0, OMAX=40, IMAX=40")
dbLoadRecords("db/save_restoreStatus.db", "P=RF-CT{$(IOC)}")

dbLoadRecords("db/MBM-Ali.db")

asSetFilename("/cf-update/acf/default.acf")

# Auto save/restore
save_restoreSet_status_prefix("RF-CT{$(IOC)}")

# ensure directories exist
system("install -d ${TOP}/as")
system("install -d ${TOP}/as/req")
system("install -d ${TOP}/as/save")

set_savefile_path("${TOP}/as","/save")
set_requestfile_path("${TOP}/as","/req")

set_pass0_restoreFile("cfc_settings.sav")
set_pass0_restoreFile("cfc_values.sav")
set_pass1_restoreFile("cfc_values.sav")
set_pass1_restoreFile("cfc_waveforms.sav")

##asynSetTraceMask("rfport", -1, 0xff)
##asynSetTraceIOMask("rfport", -1, 0xff)

cd "${TOP}/iocBoot/${IOC}"
iocInit

dbl > records.dbl

system "cp records.dbl /cf-update/$HOSTNAME.$IOCNAME.dbl"

makeAutosaveFileFromDbInfo("${TOP}/as/req/cfc_settings.req", "autosaveFields_pass0")
makeAutosaveFileFromDbInfo("${TOP}/as/req/cfc_values.req", "autosaveFields")
makeAutosaveFileFromDbInfo("${TOP}/as/req/cfc_waveforms.req", "autosaveFields_pass1")

create_monitor_set("cfc_settings.req", 5 , "")
create_monitor_set("cfc_values.req", 5 , "")
create_monitor_set("cfc_waveforms.req", 5 , "")

caPutLogInit("10.0.152.133:7004", 1)
dbpf("RF-CT{$(IOC)}:READACF.SCAN", "10 second")
