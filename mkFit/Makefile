##### Random stuff #####
#
# To disable vectorization set USER_CXXFLAGS := -no-simd -no-vec
# Setting only one of the above has little effect.
# Note, this also screws-up prefetching so it's a lousy deal.
#
# -opt-prefetch-distance=64,8

include ../Makefile.config

CPPEXTRA := -I.. ${USER_CPPFLAGS} ${DEFS}
LDEXTRA  := -ltbb ${USER_LDFLAGS}

CPPFLAGS := ${CPPEXTRA} ${CPPFLAGS}
CXXFLAGS += ${USER_CXXFLAGS}
LDFLAGS  += ${LDEXTRA}

CPPFLAGS_NO_ROOT += ${CPPEXTRA}
LDFLAGS_NO_ROOT  += ${LDEXTRA}

.PHONY: all clean distclean echo

all: default

LIB_MKFIT := ../lib/libMkFit.so

TGTS := mkFit ${LIB_MKFIT}

ifdef KNC_BUILD
  TGTS += mkFit-mic
endif

auto-genmplex: GenMPlexOps.pl
	./GenMPlexOps.pl && touch $@

AUTO_TGTS := auto-genmplex

default: ${AUTO_TGTS} ${TGTS}

clean:
	rm -f ${TGTS} *.d *.o *.om Ice/*.d Ice/*.o Ice/*.om
	rm -rf mkFit.dSYM

distclean: clean
	rm -f *.optrpt Ice/*.optrpt
	rm -f ${AUTO_TGTS} *.ah

echo:
	@echo "CXX      = ${CXX}"
	@echo "CPPFLAGS = ${CPPFLAGS}"
	@echo "CXXFLAGS = ${CXXFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "EXES     = ${EXES}"


################################################################

MKFSRCS := $(wildcard *.cc) $(wildcard Ice/*.cc)
MKFHDRS := $(wildcard *.h)

MKFOBJS := $(MKFSRCS:.cc=.o)
MKFDEPS := $(MKFSRCS:.cc=.d)

ALLOBJS := ${MKFOBJS}
LIBOBJS := $(filter-out mkFit.o, ${ALLOBJS})

${MKFDEPS}: auto-genmplex

ifeq ($(filter clean distclean, ${MAKECMDGOALS}),)
include ${MKFDEPS}
endif

mkFit: ${ALLOBJS}
	${CXX} ${CXXFLAGS} ${VEC_HOST} ${LDFLAGS} ${ALLOBJS} -o $@ ${LDFLAGS_HOST} -L../lib -lMicCore -Wl,-rpath,../lib,-rpath,./lib

${LIB_MKFIT}: ${LIBOBJS}
	${CXX} ${CXXFLAGS} ${VEC_HOST} ${LIBOBJS} -shared -o $@ ${LDFLAGS_HOST} ${LDFLAGS}

ifdef WITH_ROOT
fittestMPlex.o : CPPFLAGS += $(shell root-config --cflags)

TFile.h: 
	echo "Using dummy rule for TFile.h"

TTree.h: 
	echo "Using dummy rule for TTree.h"
endif

${MKFOBJS}: %.o: %.cc %.d
	${CXX} ${CPPFLAGS} ${CXXFLAGS} ${VEC_HOST} -c -o $@ $<

### MIC build, icc only

ifdef KNC_BUILD

MKFOBJS_MIC := $(MKFOBJS:.o=.om)

ALLOBJS_MIC := $(ALLOBJS:.o=.om) 

mkFit-mic: ${ALLOBJS_MIC}
	${CXX} ${CXXFLAGS} ${VEC_MIC} ${LDFLAGS_NO_ROOT} ${ALLOBJS_MIC} -o $@ ${LDFLAGS_MIC} -L../lib -lMicCore-mic -Wl,-rpath=../lib,-rpath=./lib

${MKFOBJS_MIC}: %.om: %.cc %.d
	${CXX} ${CPPFLAGS_NO_ROOT} ${CXXFLAGS} ${VEC_MIC} -c -o $@ $<

endif
