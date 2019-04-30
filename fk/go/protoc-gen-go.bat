set cur_pwd=%cd%
set GOPATH=%cur_pwd%

cd %cur_pwd%\src\github.com\golang\protobuf\protoc-gen-go
go install