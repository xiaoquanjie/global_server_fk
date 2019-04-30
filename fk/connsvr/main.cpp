#include "connsvr/conn_svr.h"
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
	ConnApplicationSgl.Init(argc, argv);
	ConnApplicationSgl.Run();
	return 0;
}