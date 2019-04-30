#!/bin/sh

is_running()
{
    SERVER_NAME=$1
    #proc_num=$(ps -ef | grep -w "bin/${SERVER_NAME}" | grep -v grep | wc -l)
    proc_num=$(ps -C  "${SERVER_NAME}" | sed  -e '1d' | wc -l)
    if [ ${proc_num} -gt 0 ];then
        echo "Server ${SERVER_NAME} has already running!"
        return 1
    else
        return 0
    fi
}

cat svr_list.txt | 
while read line
do
  if [ -n "$line" ];then
     is_running $line
  fi
done
