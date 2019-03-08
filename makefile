
all:

LDFLAGS := -lc -lm -lliquid

lib := sframe.o sframegen.o sframesync.o
prog := sframe_example

${lib} : %.o : %.cc %.hh
	g++ -Wall -O2 -c -o $@ $<

${prog} : % : %.cc ${lib}
	g++ -Wall -O2 -o $@ $< ${lib} ${LDFLAGS}

all: ${prog}
