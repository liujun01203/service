#!/bin/bash

cd ../../
pn=`pwd | xargs -i basename {}`
ps -ef | grep $pn

