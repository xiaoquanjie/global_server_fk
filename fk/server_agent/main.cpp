#include "server_agent/server_agent.h"

int main(int argc, char* argv[]) {
	AgentApplicationSgl.Init(argc, argv);
	AgentApplicationSgl.Run();
	return 0;
}