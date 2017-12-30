TOP = ..

INC-BDD  = -I${TOP}/bdd
INC-UTIL = -I${TOP}/util
INC-NET = -I${TOP}/network

MY-INC = ${INCGCC} ${INC-BDD} ${INC-UTIL} ${INC-NET}

MYPACKAGE = \
network \
bdd \
util \
#Cudd \

CUDDTOP = ../cudd-2.4.2

MYLIBS = \
${TOP}/network/libnetwork.a \
${TOP}/bdd/libbdd.a \
${TOP}/util/libutil.a \
#${TOP}/Cudd/libCudd.a \
#${CUDDTOP}/cudd/libcudd.a \
#${CUDDTOP}/obj/libobj.a \
#${CUDDTOP}/st/libst.a \
#${CUDDTOP}/util/libutil.a \
#${CUDDTOP}/mtr/libmtr.a \


###########################3

## -m32 or -m64
BTYPE = -m32

CXX = g++

TARGET = ger-program

CXXFLAGS = -std=c++11 -Wall -O3 -pipe
# CXXFLAGS = -std=c++11 -Wall -O3 -pipe -g 
#CXXFLAGS = -Wall -O2 -pipe
#CXXFLAGS = -g -Wall -DGCC "-DCOMPILED_DATE=\"`date`\""

SRCS = main.cpp
OBJS := $(SRCS:.cpp=.o) #SRCSの各ファイルのサフィックスの.cppを.oに変換


.cpp.o:
	$(CXX) $(BTYPE) $(CXXFLAGS) -c $< ${MY-INC}



$(TARGET): $(OBJS)
	$(CXX) $(BTYPE) -o $@ $(OBJS) ${MYLIBS}  -lstdc++ -lm


clean:
	rm -f $(TARGET) $(OBJS) *.*~ \#*\# 
