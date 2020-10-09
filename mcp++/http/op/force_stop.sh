cd ../
pn=`pwd | xargs -i basename {}`
cd ./bin
kill -9 `cat $pn"_ccd.pid"`
kill -9 `cat $pn"_mcd.pid"`
kill -9 `cat $pn"_dcc.pid"`
kill -9 `cat $pn"_watchdog.pid"`
