cd ../
pn=`pwd | xargs -i basename {}`
cd ./bin
kill -15 `cat $pn"_ccd.pid"` > /dev/null 2>&1
./$pn"_ccd" "../etc/"$pn"_ccd.conf"
