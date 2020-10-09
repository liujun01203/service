cd ../
pn=`pwd | xargs -i basename {}`
wd=`grep watchdog_conf_file "./etc/"$pn"_ccd.conf"|grep -v '#'|wc -l`
cd ./op
if [ $wd -eq 0 ]
then
	./check_ccd.sh
	./check_mcd.sh
	./check_dcc.sh
else
	./check_watchdog.sh
fi	
