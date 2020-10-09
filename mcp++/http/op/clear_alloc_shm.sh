echo "remove alloc shm..."
mqs=`grep "shmkey" ../etc/*.conf|awk -F'=' '{printf "0x%x\n", $2}'`
for mq in `echo $mqs`
do
	echo "ipcrm -M $mq"
	ipcrm -M $mq
done		
sems=`grep "semname" ../etc/*.conf|awk -F'=' '{printf "/dev/shm/sem.%s\n", substr($2,3)}'`
for sem in `echo $sems`
do
	echo "rm -f $sem"
	rm -f $sem
done		
