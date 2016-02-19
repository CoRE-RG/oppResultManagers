#
# OMNeT++/OMNEST Makefile for liboppResultManagers
#
# This file was generated with the command:
#  opp_makemake -f --deep --make-so -e cc -XSQLite -XpostgreSQL -DWITH_MULTIPLE_COMMON
#

# Name of target to be created (-o option)
TARGET = liboppResultManagers$(SHARED_LIB_SUFFIX)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I. \
    -Icheck \
    -Icommon \
    -Ieventlog \
    -Ieventlog/gcta \
    -Imultiple \
    -Iutilities

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS =

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc, .msg and .sm files
OBJS = \
    $O/check/cCheckOutputVectorManager.o \
    $O/common/cDatabaseOutputVectorManager.o \
    $O/eventlog/gcta/GCTAEventlogManager.o \
    $O/multiple/cMultipleOutputScalarManager.o \
    $O/multiple/cMultipleOutputVectorManager.o \
    $O/utilities/HelperFunctions.o

# Message files
MSGFILES =

# SM files
SMFILES =

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppenvir$D $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS) -DWITH_MULTIPLE_COMMON $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)
SMCOPTS =

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ($(MAKECMDGOALS),depend)
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# <<<
#------------------------------------------------------------------------------

# Main target
all: $O/$(TARGET)
	$(Q)$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	@echo Creating shared library: $@
	$(Q)$(SHLIB_LD) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS) $(LDFLAGS)
	$(Q)$(SHLIB_POSTPROCESS) $O/$(TARGET)

.PHONY: all clean cleanall depend msgheaders smheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc $(MSGCOPTS) $?

%_sm.cc %_sm.h: %.sm
	$(qecho) SMC: $<
	$(Q)$(SMC) -c++ -suffix cc $(SMCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

smheaders: $(SMFILES:.sm=_sm.h)

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f oppResultManagers oppResultManagers.exe liboppResultManagers.so liboppResultManagers.a liboppResultManagers.dll liboppResultManagers.dylib
	$(Q)-rm -f ./*_m.cc ./*_m.h ./*_sm.cc ./*_sm.h
	$(Q)-rm -f check/*_m.cc check/*_m.h check/*_sm.cc check/*_sm.h
	$(Q)-rm -f common/*_m.cc common/*_m.h common/*_sm.cc common/*_sm.h
	$(Q)-rm -f eventlog/*_m.cc eventlog/*_m.h eventlog/*_sm.cc eventlog/*_sm.h
	$(Q)-rm -f eventlog/gcta/*_m.cc eventlog/gcta/*_m.h eventlog/gcta/*_sm.cc eventlog/gcta/*_sm.h
	$(Q)-rm -f multiple/*_m.cc multiple/*_m.h multiple/*_sm.cc multiple/*_sm.h
	$(Q)-rm -f utilities/*_m.cc utilities/*_m.h utilities/*_sm.cc utilities/*_sm.h

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES) $(SM_CC_FILES)  ./*.cc check/*.cc common/*.cc eventlog/*.cc eventlog/gcta/*.cc multiple/*.cc utilities/*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/check/cCheckOutputVectorManager.o: check/cCheckOutputVectorManager.cc \
	check/cCheckOutputVectorManager.h
$O/common/cDatabaseOutputVectorManager.o: common/cDatabaseOutputVectorManager.cc \
	common/cDatabaseOutputVectorManager.h \
	utilities/HelperFunctions.h
$O/eventlog/gcta/GCTAEventlogManager.o: eventlog/gcta/GCTAEventlogManager.cc \
	eventlog/gcta/GCTAEventlogManager.h
$O/multiple/cMultipleOutputScalarManager.o: multiple/cMultipleOutputScalarManager.cc \
	multiple/cMultipleOutputScalarManager.h
$O/multiple/cMultipleOutputVectorManager.o: multiple/cMultipleOutputVectorManager.cc \
	multiple/cMultipleOutputVectorManager.h
$O/utilities/HelperFunctions.o: utilities/HelperFunctions.cc \
	utilities/HelperFunctions.h

