cd ../
pn=`pwd | xargs -i basename {}`
wd=`grep watchdog_conf_file "./etc/"$pn"_ccd.conf"|grep -v '#'|wc -l`
cd ./op
if [ $wd -eq 0 ]
then
	./stop_ccd.sh
	./stop_mcd.sh
	./stop_dcc.sh
else
	./stop_watchdog.sh
fi
