#!/bin/bash

echo "ipcrm -M 287454098"
ipcrm -M 287454098
echo "ipcrm -M 287454099"
ipcrm -M 287454099
echo "ipcrm -M 287954088"
ipcrm -M 287954088
echo "rm /dev/shm/sem.simple_http_mq1"
rm /dev/shm/sem.simple_http_mq1
echo "rm /dev/shm/sem.simple_http_mq2"
rm /dev/shm/sem.simple_http_mq2
echo "rm /dev/shm/sem.simple_http_shmalloc"
rm /dev/shm/sem.simple_http_shmalloc

