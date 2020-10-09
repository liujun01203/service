cd ../
pn=`pwd | xargs -i basename {}`
wd=`grep watchdog_conf_file "./etc/"$pn"_ccd.conf"|grep -v '#'|wc -l`
cd ./op
if [ $wd -eq 0 ]
then
	./start_ccd.sh
	./start_mcd.sh
	./start_dcc.sh
else
	./start_watchdog.sh
fi	
