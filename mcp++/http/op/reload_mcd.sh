cd ../
pn=`pwd | xargs -i basename {}`
cd ./bin
kill -10 `cat $pn"_mcd.pid"`
