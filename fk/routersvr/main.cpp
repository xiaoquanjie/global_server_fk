#include "routersvr/router_svr.h"
#include <iostream>
#include "protolib/src/routersvr_config.pb.h"
using namespace std;

int main(int argc, char* argv[]) {
	RouterAppSgl::mutable_instance().Init(argc, argv);
	RouterAppSgl::mutable_instance().Run();
	return 0;
}