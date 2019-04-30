#!/bin/sh

cur_pwd=`pwd`
export GOPATH=${cur_pwd}

echo ${cur_pwd}

cd ${cur_pwd}/src/new_deploy/main
go build -o ${cur_pwd}/build/new_deploy main.go
