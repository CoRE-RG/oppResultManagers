#
# OMNeT++/OMNEST Makefile for liboppResultManagers
#
# This file was generated with the command:
#  opp_makemake -f --deep --make-so -e cc -lpqxx -lpq -lsqlite3 -DWITH_POSTGRESQL_COMMON -DWITH_SQLITE_COMMON -DWITH_MULTIPLE_COMMON
#

# Name of target to be created (-o option)
TARGET = liboppResultManagers$(SHARED_LIB_SUFFIX)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I. \
    -ICheck \
    -ICommon \
    -IMultiple \
    -IPostgreSQL \
    -ISQLite

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS =  -lpqxx -lpq -lsqlite3

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = \
    $O/Check/cCheckOutputVectorManager.o \
    $O/Common/cDatabaseOutputVectorManager.o \
    $O/Multiple/cMultipleOutputVectorManager.o \
    $O/Multiple/cMultipleOutputScalarManager.o \
    $O/PostgreSQL/cPostgreSQLOutputScalarManager.o \
    $O/PostgreSQL/cPostgreSQLOutputVectorManager.o \
    $O/PostgreSQL/cPorstgreSQLOutputManager.o \
    $O/SQLite/cSQLiteOutputScalarManager.o \
    $O/SQLite/cSQLiteOutputManager.o \
    $O/SQLite/cSQLiteOutputVectorManager.o

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

COPTS = $(CFLAGS) -DWITH_POSTGRESQL_COMMON -DWITH_SQLITE_COMMON -DWITH_MULTIPLE_COMMON $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
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
	$(Q)-rm -f Check/*_m.cc Check/*_m.h
	$(Q)-rm -f Common/*_m.cc Common/*_m.h
	$(Q)-rm -f Multiple/*_m.cc Multiple/*_m.h
	$(Q)-rm -f PostgreSQL/*_m.cc PostgreSQL/*_m.h
	$(Q)-rm -f SQLite/*_m.cc SQLite/*_m.h

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc Check/*.cc Common/*.cc Multiple/*.cc PostgreSQL/*.cc SQLite/*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/Check/cCheckOutputVectorManager.o: Check/cCheckOutputVectorManager.cc \
	Check/cCheckOutputVectorManager.h
$O/Common/cDatabaseOutputVectorManager.o: Common/cDatabaseOutputVectorManager.cc \
	Common/cDatabaseOutputVectorManager.h
$O/Multiple/cMultipleOutputScalarManager.o: Multiple/cMultipleOutputScalarManager.cc \
	Multiple/cMultipleOutputScalarManager.h
$O/Multiple/cMultipleOutputVectorManager.o: Multiple/cMultipleOutputVectorManager.cc \
	Multiple/cMultipleOutputVectorManager.h
$O/PostgreSQL/cPorstgreSQLOutputManager.o: PostgreSQL/cPorstgreSQLOutputManager.cc \
	PostgreSQL/cPorstgreSQLOutputManager.h
$O/PostgreSQL/cPostgreSQLOutputScalarManager.o: PostgreSQL/cPostgreSQLOutputScalarManager.cc \
	PostgreSQL/cPorstgreSQLOutputManager.h \
	PostgreSQL/cPostgreSQLOutputScalarManager.h
$O/PostgreSQL/cPostgreSQLOutputVectorManager.o: PostgreSQL/cPostgreSQLOutputVectorManager.cc \
	Common/cDatabaseOutputVectorManager.h \
	PostgreSQL/cPorstgreSQLOutputManager.h \
	PostgreSQL/cPostgreSQLOutputVectorManager.h
$O/SQLite/cSQLiteOutputManager.o: SQLite/cSQLiteOutputManager.cc \
	SQLite/cSQLiteOutputManager.h
$O/SQLite/cSQLiteOutputScalarManager.o: SQLite/cSQLiteOutputScalarManager.cc \
	SQLite/cSQLiteOutputManager.h \
	SQLite/cSQLiteOutputScalarManager.h
$O/SQLite/cSQLiteOutputVectorManager.o: SQLite/cSQLiteOutputVectorManager.cc \
	Common/cDatabaseOutputVectorManager.h \
	SQLite/cSQLiteOutputManager.h \
	SQLite/cSQLiteOutputVectorManager.h

