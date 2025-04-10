TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = ln-cfc0
# ln-cfc0.dbd will be created and installed
DBD += ln-cfc0.dbd

# ln-cfc0.dbd will be made up from these files:
ln-cfc0_DBD += base.dbd

# Include dbd files from all support applications:
ln-cfc0_DBD += menuScan.dbd
ln-cfc0_DBD += system.dbd
ln-cfc0_DBD += asyn.dbd
ln-cfc0_DBD += calcSupport.dbd
ln-cfc0_DBD += okSupport.dbd
ln-cfc0_DBD += asSupport.dbd
ln-cfc0_DBD += devFileWaveform.dbd
ln-cfc0_DBD += devIocStats.dbd
ln-cfc0_DBD += okSNoASub.dbd
ln-cfc0_DBD += caPutLog.dbd
ln-cfc0_DBD += stdSupport.dbd
ln-cfc0_DBD += reccaster.dbd

USR_CPPFLAGS += -I$(OK)  -D_X86_64_ -DUNIX -DLINUX

# Add all the support libraries needed by this IOC
ln-cfc0_LIBS += llrfapp std asyn autosave devIocStats calc sscan caPutLog seq pv reccaster pvDatabase qsrv pvaClient pvAccessIOC pvAccess pvAccessCA nt pvData

ln-cfc0_SYS_LIBS += okFrontPanel

USR_LDFLAGS += -L$(OK)

# ln-cfc0_registerRecordDeviceDriver.cpp derives from ln-cfc0.dbd
ln-cfc0_SRCS += ln-cfc0_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
ln-cfc0_SRCS_DEFAULT += ln-cfc0Main.cpp
ln-cfc0_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#ln-cfc0_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

LIBRARY_IOC += llrfapp

llrfapp_SRCS += okiface.cpp
llrfapp_SRCS += llrfPort.cpp
llrfapp_SRCS += calc.c
llrfapp_SRCS += devFileWaveform.c
llrfapp_SRCS += devFileWaveform0.c
llrfapp_SRCS += okSNoASub.c

# Link QSRV (pvAccess Server) if available
ifdef EPICS_QSRV_MAJOR_VERSION
    ln-cfc0_LIBS += qsrv
    ln-cfc0_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)
    ln-cfc0_DBD  += PVAServerRegister.dbd
    ln-cfc0_DBD  += qsrv.dbd
endif

# Finally link to the EPICS Base libraries
ln-cfc0_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

