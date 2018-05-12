#!/bin/bash

set -e

function test_compile {                                                                                                                                                                                                          

echo ------------------------------------
echo ------------------------------------
echo Compiling with $1
echo ------------------------------------

cd build/
make     
sudo make install
cd ..

tools/testers/algorithm-tester.pl -alg dummyDijkstra -pguser postgres


#cd build
#make doc
#cd ..

#dropdb --if-exists ___pgr___test___                                                                                                                                                                                              
#createdb  ___pgr___test___                                                                                                                                                                                                       
#sh ./tools/testers/pg_prove_tests.sh vicky                                                                                                                                                                                      
#dropdb  ___pgr___test___                                                                                                                                                                                                         
}