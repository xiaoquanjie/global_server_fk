#include "transfersvr/transfersvr.h"
#include <iostream>
#include "protolib/src/transfersvr_config.pb.h"
using namespace std;

int main(int argc, char* argv[]) {
	TransferAppSgl::mutable_instance().Init(argc, argv);
	TransferAppSgl::mutable_instance().Run();
	return 0;
}