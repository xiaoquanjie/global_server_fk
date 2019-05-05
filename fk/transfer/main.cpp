#include "transfer/transfer.h"

int main(int argc, char* argv[]) {
	TransferAppSgl::mutable_instance().Init(argc, argv);
	TransferAppSgl::mutable_instance().Run();
	return 0;
}