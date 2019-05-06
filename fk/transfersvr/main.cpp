#include "transfersvr/transfer_svr.h"

int main(int argc, char* argv[]) {
	TransferAppSgl.Init(argc, argv);
	TransferAppSgl.Run();
	return 0;
}