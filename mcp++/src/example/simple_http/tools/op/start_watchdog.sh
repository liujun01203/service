#!/bin/bash

ulimit -n 200000

cd ../../
pn=`pwd | xargs -i basename {}`
cd ./bin
./$pn"_watchdog"

