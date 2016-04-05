all: checkmakefiles src/oppresultmanagers/features.h 
	cd src && $(MAKE)

clean: checkmakefiles
	cd src && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile src/oppresultmanagers/features.h

INET_PROJ=../../inet

MAKEMAKE_OPTIONS := -f --deep --no-deep-includes -I.

OPPRESULTMANAGERS_PCAPNG_ENABLED := $(shell ./oppresultmanagers_featuretool -q isenabled PCAPNGEventlog_common; echo $$?)
ifeq ($(OPPRESULTMANAGERS_PCAPNG_ENABLED),0)
    MAKEMAKE_OPTIONS += -I$(INET_PROJ)/src/ -DINET_IMPORT -L$(INET_PROJ)/src -lINET -KINET_PROJ=$(INET_PROJ)
endif

makefiles: src/oppresultmanagers/features.h makefiles-so

makefiles-so:
	@FEATURE_OPTIONS=$$(./oppresultmanagers_featuretool options -f -l) && cd src && opp_makemake --make-so $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

makefiles-lib:
	@FEATURE_OPTIONS=$$(./oppresultmanagers_featuretool options -f -l) && cd src && opp_makemake --make-lib $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

makefiles-exe:
	@FEATURE_OPTIONS=$$(./oppresultmanagers_featuretool options -f -l) && cd src && opp_makemake $(MAKEMAKE_OPTIONS) $$FEATURE_OPTIONS

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
	@./oppresultmanagers_featuretool defines >src/oppresultmanagers/features.h

doxy:
	doxygen doxy.cfg