#!/bin/bash

cd ../../
pn=`pwd | xargs -i basename {}`
cd ./bin
pinfo=$(ps -e | grep `cat $pn"_watchdog.pid"`)
if [ "$pinfo" = "" ]
then
	echo "Watchdog stopped! Try to restart!"
	cd ../tools/op
	./stop_watchdog.sh
	sleep 5
	./start_watchdog.sh
else
	echo "Watchdog running!"
fi

exit 0

