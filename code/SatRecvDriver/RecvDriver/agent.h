#ifndef AGENT_H_
#define AGENT_H_
#include<string>
int GetMemory();
int GetCpu();
#ifdef LINUX
void RunCommand(const char* command);
std::string GetOriginalNetmask();
#endif
#endif
