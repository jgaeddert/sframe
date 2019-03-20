
all:

CFLAGS  := -Wall -O2 -std=c++11 -I./include
LDFLAGS := -lc -lm -lliquid -lfftw3f

lib := src/sframe.o src/sframegen.o src/sframesync.o src/sdetect.o

prog :=						\
	examples/sframe_example			\
	examples/sframesync_benchmark_example	\
	examples/sframesync_timing_example	\

headers := $(patsubst src/%.o,include/%.hh,${lib})

${lib} : %.o : %.cc ${headers}
	g++ ${CFLAGS} -c -o $@ $<

${prog} : % : %.cc ${lib}
	g++ ${CFLAGS} -o $@ $< ${lib} ${LDFLAGS}

all: ${prog}

clean:
	rm -f ${lib} ${prog}

