#Try to detect INET if variable is not set
ifndef INET_PROJ
    ifneq ($(wildcard ../inet),)
        INET_PROJ=../../inet
    endif
endif
#Try to detect FiCo4OMNeT if variable is not set
ifndef FICO4OMNET_PROJ
    ifneq ($(wildcard ../FiCo4OMNeT),)
        FICO4OMNET_PROJ=../../FiCo4OMNeT
    endif
endif

all: checkmakefiles src/oppresultmanagers/features.h 
	cd src && $(MAKE)

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile src/oppresultmanagers/features.h
	
DBG_SUFFIX=""
ifneq (,$(findstring debug, $(MODE)))
	DBG_SUFFIX="_dbg"
endif

MAKEMAKE_OPTIONS := -f --deep --no-deep-includes -I.

OPPRESULTMANAGERS_PCAPNG_ENABLED := $(shell opp_featuretool -q isenabled PCAPNGEventlog_common; echo $$?)
ifeq ($(OPPRESULTMANAGERS_PCAPNG_ENABLED),0)
    ifdef INET_PROJ
        MAKEMAKE_OPTIONS += -I$(INET_PROJ)/src/ -L$(INET_PROJ)/src -lINET$(DBG_SUFFIX) -KINET_PROJ=$(INET_PROJ)
    endif

    ifdef FICO4OMNET_PROJ
        MAKEMAKE_OPTIONS += -I$(FICO4OMNET_PROJ)/src/ -L$(FICO4OMNET_PROJ)/src -lFiCo4OMNeT$(DBG_SUFFIX) -KFICO4OMNET_PROJ=$(FICO4OMNET_PROJ)
    endif
endif

makefiles: src/oppresultmanagers/features.h makefiles-so

makefiles-so:
	@FEATURE_OPTIONS=$$(opp_featuretool options -f -l -c) && cd src && opp_makemake --make-so $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

makefiles-lib:
	@FEATURE_OPTIONS=$$(opp_featuretool options -f -l -c) && cd src && opp_makemake --make-lib $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

makefiles-exe:
	@FEATURE_OPTIONS=$$(opp_featuretool options -f -l -c) && cd src && opp_makemake $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi

# generate an include file that contains all the WITH_FEATURE macros according to the current enablement of features
src/oppresultmanagers/features.h: $(wildcard .oppfeaturestate) .oppfeatures
	@opp_featuretool defines >src/oppresultmanagers/features.h

doxy:
	doxygen doxy.cfg