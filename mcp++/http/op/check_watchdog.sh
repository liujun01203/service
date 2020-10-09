cd ../
pn=`pwd | xargs -i basename {}`
pid=`cat ./bin/$pn"_watchdog.pid"`
cn=`ps -ef|grep $pid|grep -v grep|wc -l`
if [ $cn -lt 1 ]
then
	cd ./op
	./start_watchdog.sh
fi		
