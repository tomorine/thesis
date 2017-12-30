## object files ##

## -m32 or -m64
BTYPE = -m32

CFLAGS = -std=c++11 -g -Wall -DGCC
# CFLAGS = -std=c++11 -g -Wall -DGCC -Wno-sign-compare
# CFLAGS = -O3 -std=c++11 -g -Wall -DGCC

CXX = g++

SRCS = node.cpp network.cpp general.cpp utility.cpp

OBJS = ${SRCS:.cpp=.o}

## header files ##

TOP = ..

INC-NET  = -I${TOP}/network
INC-BDD  = -I${TOP}/bdd
#INC-GER  = -I${TOP}/ger-base
INC-UTIL = -I${TOP}/util
#INC-DECOMP = -I${TOP}/decomp

#MY-INC = ${INCGCC} ${INC-NET} ${INC-BDD} ${INC-GER} ${INC-UTIL} ${INC-DECOMP}
MY-INC = ${INCGCC} ${INC-NET} ${INC-BDD} ${INC-GER} ${INC-UTIL}

INC-G++ = -I/usr/gnu/include/g++ -I/usr/gnu/sparc-sun-solaris2.6/include

## libraries ##

ARCS = libnetwork.a
SO = libnetwork.so


.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) $(BTYPE) $(CFLAGS) -c $< ${MY-INC}

#all: ${SO} ${ARCS}
all: ${ARCS}

${ARCS}: ${OBJS}
	ar cr $@ ${OBJS}
	ranlib $@

${SO}: ${OBJS}
	$(CXX) $(BTYPE) -G -o $@ ${OBJS} ${MY-INC} 

depend:
	makedepend ${SRCS} -DGCC ${MY-INC} ${INC-G++}

clean:
	rm -f *.bak *.o *.*~ \#*\# *.a *.so

