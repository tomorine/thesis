#!/bin/bash
source ./run.sh

(cd ../bdd/
#make clean
run make
cd ../network/
make clean
run make
cd ../util/
#make clean
run make
cd ../main/
make clean
run make
./ger-program GerTemp.blif GerTemp.blif
)
