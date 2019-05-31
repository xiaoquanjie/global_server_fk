#include "transfersvr/transfersvr.h"
#include <iostream>
#include "protolib/src/transfersvr_config.pb.h"
using namespace std;

int main(int argc, char* argv[]) {
	TransferAppSgl.Init(argc, argv);
	TransferAppSgl.Run();
	return 0;
}