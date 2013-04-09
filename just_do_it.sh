#!/usr/bin/env bash

EXPECTED_ARGS=3

if [ $# -ne $EXPECTED_ARGS ]
then
    echo 'Got' $# 'argument(s), expected' $EXPECTED_ARGS
    echo 'Usage' $0 'operand1 operation operand2'
    exit 1
fi

#sudo insmod ker.ko

operation_device_num=`grep operation_device /proc/devices | awk '{print $1}'`
operand1_device_num=`grep operand1_device /proc/devices | awk '{print $1}'`
operand2_device_num=`grep operand2_device /proc/devices | awk '{print $1}'`
result_device_num=`grep result_device /proc/devices | awk '{print $1}'`

echo 'Major number of the operation device:' $operation_device_num
echo 'Major number of the first operand device:' $operand1_device_num
echo 'Major number of the second operand device:' $operand2_device_num
echo 'Major number of the result device:' $result_device_num

sudo mknod /dev/operand1 c $operand1_device_num 1
sudo chmod a+r+w /dev/operand1
sudo mknod /dev/operand2 c $operand2_device_num 1
sudo chmod a+r+w /dev/operand2
sudo mknod /dev/operation c $operation_device_num 1
sudo chmod a+r+w /dev/operation
sudo mknod /dev/result c $result_device_num 1
sudo chmod a+r+w /dev/result

echo $1 > /dev/operand1
echo $2 > /dev/operation
echo $3 > /dev/operand2

echo 'Content of operand1:'
cat /dev/operand1
echo
echo 'Content of operation:'
cat /dev/operation
echo
echo 'Content of operand2:'
cat /dev/operand2
echo
echo 'Content of result:'
cat /dev/result
echo

