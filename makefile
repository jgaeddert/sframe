
all:

CFLAGS  := -Wall -O2 -std=c++11 -I.
LDFLAGS := -lc -lm -lliquid -lfftw3f

lib := sframe.o sframegen.o sframesync.o sdetect.o
prog := examples/sframe_example examples/sframesync_benchmark_example

headers := $(patsubst %.o,%.hh,${lib})

${lib} : %.o : %.cc ${headers}
	g++ ${CFLAGS} -c -o $@ $<

${prog} : % : %.cc ${lib}
	g++ ${CFLAGS} -o $@ $< ${lib} ${LDFLAGS}

all: ${prog}

clean:
	rm -f ${lib} ${prog}

