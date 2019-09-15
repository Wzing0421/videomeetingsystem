#include<utils.h>
#include"agent.h"
#ifdef LINUX
struct CPU_OCCUPY         //定义一个cpu occupy的结构体
{
	char name[20];     //定义一个char类型的数组名name有20个元素
	unsigned int user; //定义一个无符号的int类型的user
	unsigned int nice; //定义一个无符号的int类型的nice
	unsigned int system;//定义一个无符号的int类型的system
	unsigned int idle; //定义一个无符号的int类型的idle
};
struct MEM_OCCUPY         //定义一个mem occupy的结构体
{
	char name[20];      //定义一个char类型的数组名name有20个元素
	unsigned long total;
	char name2[20];
	unsigned long free;
};

std::string GetOriginalNetmask(){
    std::string result = "";
    FILE* fp = fopen("/etc/network/interfaces", "r");
    if(!fp){
        printf("open file /etc/network/interfaces failed!\n");
        return result;
    }
    char config_buf[5000];
    char ret[100];
    int len = fread(config_buf, 1, 5000, fp);
    char* pattern = "netmask";
    for(int i = len - 6; i >= 0; i --){
        int j = 0;
        for(; j < 7; j ++){
            if(config_buf[i + j] != pattern[j])
              break;
        }
        if(j == 7){
            i += j;
            while(i >= 0 && (config_buf[i] == ' ' || config_buf[i] == '\n'))
              i ++;
            if(i == len){
                break;
            }
            j = 0;
            while(i >= 0 && config_buf[i] != '\n' && config_buf[i] != ' '){
                ret[j++] = config_buf[i++];
            }
            ret[j] = '\0';
            break;
        }
    } 
    fclose(fp);
    result = std::string(ret);
    if(result.length() > 15 || result.length() <= 0){
        printf("can not find netmask!\n");
    }
    return result;
}

void RunCommand(const char* command){
	char result_buf[1000];
	int rc = 0;
	FILE* fp;

	fp = popen(command, "r");
	if (fp == NULL){
		perror("command failed!\n");
		return;
	}
	while (fgets(result_buf, sizeof(result_buf), fp) != NULL){
		if ('\n' == result_buf[strlen(result_buf) - 1]){
			result_buf[strlen(result_buf) - 1] = '\0';
		}
		printf("command %s output %s\n", command, result_buf);
	}
	rc = pclose(fp);
	if (-1 == rc){
		perror("close fp failed!\n");
		return;
	}
	else{
		printf("command %s child process return status = %d, command return status = %d\n",
			command, rc, WEXITSTATUS(rc));
	}
}

#else
#include<Windows.h>
///时间转换  
static uint64_t FileTime2UTC(const FILETIME* ftime)
{
	LARGE_INTEGER li;
	assert(ftime);
	li.LowPart = ftime->dwLowDateTime;
	li.HighPart = ftime->dwHighDateTime;
	return li.QuadPart;
}

static __int64 CompareFileTime(FILETIME time1, FILETIME time2)
{
	__int64 a = time1.dwHighDateTime << 32 | time1.dwLowDateTime;  
	__int64 b = time2.dwHighDateTime << 32 | time2.dwLowDateTime;  
	return (b - a);  
}  

/// 获得CPU的核数  
static int GetProcessorNumber()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return (int)info.dwNumberOfProcessors;
}

#endif

int GetMemory()
{
#ifdef LINUX
	FILE *fd;
	char buff[256];
	MEM_OCCUPY *tempMem = new MEM_OCCUPY;
	fd = fopen("/proc/meminfo", "r");
	fgets(buff, sizeof(buff), fd);
	sscanf(buff, "%s %u %s", tempMem->name, (unsigned int*)&tempMem->total, tempMem->name);
	fgets(buff, sizeof(buff), fd);
	sscanf(buff, "%s %u %s", tempMem->name2, (unsigned int*)&tempMem->free, tempMem->name2);
	fclose(fd);
	long long tempTotal = tempMem->total;
	long long tempFree = tempMem->free;
	printf("total is %u and free is %u\n", tempTotal, tempFree);
	long long memUsage = 100 - (100 * tempFree) / tempTotal;
	int re = (int)memUsage;
	if (re <0 || re >100)
		return -1;
	return re;
#else
	MEMORYSTATUS ms;
	::GlobalMemoryStatus(&ms);
	return ms.dwMemoryLoad;
#endif
}


int GetCpu()
{
#ifdef LINUX
	static CPU_OCCUPY lastCpuUsage;
	FILE *fd;
	//int n;
	char buff[256];
	fd = fopen("/proc/stat", "r");
	fgets(buff, sizeof(buff), fd);
	CPU_OCCUPY *tempCpu = new CPU_OCCUPY;
	sscanf(buff, "%s %u %u %u %u", tempCpu->name, &tempCpu->user, &tempCpu->nice, &tempCpu->system, &tempCpu->idle);
	fclose(fd);
	long long lastTime = (lastCpuUsage.idle + lastCpuUsage.nice + lastCpuUsage.system + lastCpuUsage.user);
	long long  thisTime = (tempCpu->idle + tempCpu->nice + tempCpu->system + tempCpu->user);
	long long diff = (tempCpu->system + tempCpu->user - lastCpuUsage.system - lastCpuUsage.user);
	int usage = (diff * 100) / (thisTime - lastTime);
	lastCpuUsage.idle = tempCpu->idle;
	memcpy(&lastCpuUsage.name, &tempCpu->name, 20);
	lastCpuUsage.nice = tempCpu->nice;
	lastCpuUsage.system = tempCpu->system;
	lastCpuUsage.user = tempCpu->user;
	delete tempCpu;
	if (usage <0 || usage >100)
		return -1;
	return usage;
#else

	//cpu数量
	static int processor_count_ = -1;
	//上一次的时间  
	static int64_t last_time_ = 0;
	static int64_t last_system_time_ = 0;

	FILETIME now;
	FILETIME creation_time;
	FILETIME exit_time;
	FILETIME kernel_time;
	FILETIME user_time;
	int64_t system_time;
	int64_t time;
	int64_t system_time_delta;
	int64_t time_delta;

	int cpu = -1;
	if (processor_count_ == -1)
	{
		processor_count_ = GetProcessorNumber();
	}
	GetSystemTimeAsFileTime(&now);

	if (!GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time,
		&kernel_time, &user_time))
	{
		// We don't assert here because in some cases (such as in the Task   Manager)
		// we may call this function on a process that has just exited but   we have
		// not yet received the notification.  
			return -1;
	}
	system_time = (FileTime2UTC(&kernel_time) + FileTime2UTC(&user_time)) / processor_count_;
	time = FileTime2UTC(&now);

	if ((last_system_time_ == 0) || (last_time_ == 0))
	{
		// First call, just set the last values.  
		last_system_time_ = system_time;
		last_time_ = time;
		return -1;
	}

	system_time_delta = system_time - last_system_time_;
	time_delta = time - last_time_;

	assert(time_delta != 0);

	if (time_delta == 0)
		return -1;

	// We add time_delta / 2 so the result is rounded.  
	cpu = (int)((system_time_delta * 100 + time_delta / 2) / time_delta);
	last_system_time_ = system_time;
	last_time_ = time;
	return cpu;
#endif
}



