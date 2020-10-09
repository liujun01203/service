cd ../
pn=`pwd | xargs -i basename {}`
cd ./bin
pid="`cat $pn"_watchdog.pid"`|`cat $pn"_ccd.pid"`|`cat $pn"_mcd.pid"`|`cat $pn"_dcc.pid"`"
ps -ef|egrep $pid|grep -v grep
#echo $pid
