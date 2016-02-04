#
# OMNeT++/OMNEST Makefile for liboppResultManagers
#
# This file was generated with the command:
#  opp_makemake -f --deep --make-so -lpqxx -lpq -DWITH_AVB_COMMON -DWITH_AS6802_COMMON -DWITH_IEEE8021Q_COMMON -DWITH_IPORE -DWITH_POSTGRESQL_COMMON
#

# Name of target to be created (-o option)
TARGET = liboppResultManagers$(SHARED_LIB_SUFFIX)

# C++ include paths (with -I)
INCLUDE_PATH = -I. -IMultiple -IPostgreSQL

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS =  -lpqxx -lpq

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = $O/PostgreSQL/cPostgreSQLOutputScalarMgr.o $O/PostgreSQL/cPostgreSQLOutputVectorMgr.o $O/PostgreSQL/opppostgresqlutils.o

# Message files
MSGFILES =

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

COPTS = $(CFLAGS) -DWITH_AVB_COMMON -DWITH_AS6802_COMMON -DWITH_IEEE8021Q_COMMON -DWITH_IPORE -DWITH_POSTGRESQL_COMMON $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)

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

.PHONY: all clean cleanall depend msgheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f oppResultManagers oppResultManagers.exe liboppResultManagers.so liboppResultManagers.a liboppResultManagers.dll liboppResultManagers.dylib
	$(Q)-rm -f ./*_m.cc ./*_m.h
	$(Q)-rm -f Multiple/*_m.cc Multiple/*_m.h
	$(Q)-rm -f PostgreSQL/*_m.cc PostgreSQL/*_m.h

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc Multiple/*.cc PostgreSQL/*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/PostgreSQL/cPostgreSQLOutputScalarMgr.o: PostgreSQL/cPostgreSQLOutputScalarMgr.cc \
	PostgreSQL/cPostgreSQLOutputScalarMgr.h \
	PostgreSQL/opppostgresqlutils.h
$O/PostgreSQL/cPostgreSQLOutputVectorMgr.o: PostgreSQL/cPostgreSQLOutputVectorMgr.cc \
	PostgreSQL/cPostgreSQLOutputVectorMgr.h \
	PostgreSQL/opppostgresqlutils.h
$O/PostgreSQL/opppostgresqlutils.o: PostgreSQL/opppostgresqlutils.cc \
	PostgreSQL/opppostgresqlutils.h

