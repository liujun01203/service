echo "remove mq..."
mqs=`grep "shm_key" ../etc/mq*.conf|awk -F'=' '{printf "0x%x\n", $2}'`
for mq in `echo $mqs`
do
	echo "ipcrm -M $mq"
	ipcrm -M $mq
done		
sems=`grep "sem_name" ../etc/mq*.conf|awk -F'=' '{printf "/dev/shm/sem.%s\n", substr($2,3)}'`
for sem in `echo $sems`
do
	echo "rm -f $sem"
	rm -f $sem
done		
