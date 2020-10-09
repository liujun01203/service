cd ../
pn=`pwd | xargs -i basename {}`
cd ./bin
kill -15 `cat $pn"_ccd.pid"`
