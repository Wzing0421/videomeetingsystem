#include<parameter.h>
#include<logger.h>
#include<utils.h>
#include<signal.h>
#include<iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include"monitor_server.h"
#define LOG_FILE "receive_driver.log"
#ifdef CONFIG_FILE
#undef CONFIG_FILE
#endif
#define CONFIG_FILE "../RecvDriver/config.json"

using namespace std;

Parameter g_parameter;
Logger g_logger;

void* MonitorFunc(void* param){
    MonitorServer* monitor_server = (MonitorServer*)param;
    monitor_server->WorkLoop();
	
    return (void*)(0);
}

void checkLicenseValid(){
    FILE *stream1, *stream2;
    char cfg1[21];
    char cfg2[21];
    stream1= fopen("/sys/fsl_otp/HW_OCOTP_CFG0", "r");
    stream2 = fopen("/sys/fsl_otp/HW_OCOTP_CFG1", "r");
    fgets(cfg1, 21, stream1);
    fgets(cfg2, 21, stream2);
    fclose(stream1);
    fclose(stream2);
    stpncpy(cfg1+10, cfg2, 10);
    cfg1[19]='\0';
    #ifdef KEY
    if (!strcmp(cfg1, KEY)){
        printf("success access\n");    
    }else{
    #endif 
        while(1){
        usleep(10000);
        }
    #ifdef KEY
    }
    #endif 
}

int main(){
    checkLicenseValid();
    cout << "Start RecvDriver" << endl;

    g_parameter.LoadConfig(CONFIG_FILE);
    g_logger.Init(LOG_FILE);
    //create subthreasd---monitor server
    MonitorServer monitor_server;
    if (! monitor_server.StartServer()){
        cout << "monitor server start service failed!" << endl;
        return -1;
    }
    
#ifdef PTHREAD
    pthread_t monitor_thread;
#else
	thread monitor_thread;
#endif

    if(! StartThread(&monitor_thread, MonitorFunc, &monitor_server)){
        cout << "start monitor thread failed!\n";
        return -1;
    }

    //wait for monitor subthread to exit
    JoinThread(monitor_thread);
    cout << "Exit RecvDriver...." << endl;
    return 0;
}

