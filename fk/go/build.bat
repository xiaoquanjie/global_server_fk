set cur_pwd=%cd%
set GOPATH=%cur_pwd%

cd %cur_pwd%\src\new_deploy\main
go build -o %cur_pwd%\build\new_deploy.exe main.go