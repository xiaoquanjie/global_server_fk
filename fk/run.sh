#!/bin/sh

root_path=`pwd`
#export LD_LIBRARY_PATH=/usr/local/lib
#echo library path: ${root_path}/protobuflib/linux


start()
{
   SVR_NAME=$1
   echo "starting ${SVR_NAME}.........."  
   ./blade-bin/${SVR_NAME}/${SVR_NAME} -D --conf_dir ${root_path}/conf/
}

svr_array=(routersvr connsvr)

if [ $# == 1 ];then
   start $1
else
   cat svr_list.txt |
   while read line
   do
     if [ -n "$line" ];then
        start $line
     fi
   done
fi

