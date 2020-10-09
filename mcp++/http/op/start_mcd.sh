cd ../
pn=`pwd | xargs -i basename {}`
cd ./bin
kill -15 `cat $pn"_mcd.pid"` > /dev/null 2>&1
./$pn"_mcd" "../etc/"$pn"_mcd.conf"
