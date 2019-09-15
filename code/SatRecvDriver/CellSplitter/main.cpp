#include <iostream>
#include <signal.h>
#include <utils.h>
#include <logger.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include"cell_splitter.h"
#define LOG_FILE "cell_splitter.log"
using namespace std;
Logger g_logger;
void* MonitorFunc(void* param){
    CellSplitter* cell_splitter = (CellSplitter*)param;
    cell_splitter->MonitorLoop();
    return (void*)(0);
}

void* RecvFunc(void* param){    
    CellSplitter* cell_splitter = (CellSplitter*)param;
    cell_splitter->ReceiveLoop();
    return (void*)(0);
}

void* SendFunc(void* param){    
    CellSplitter* cell_splitter = (CellSplitter*)param;
    cell_splitter->ProcessLoop();
    return (void*)(0);
}

/*void checkLicenseValid(){
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
    if (!strcmp(cfg1, KEY))
    {
        printf("success access\n");
    }else{
    #endif
        while(1){
        usleep(10000);
        }
    #ifdef KEY
    }
    #endif 
}*/

int main(int argc, char* argv[]){
    //checkLicenseValid();
    g_logger.Init(LOG_FILE);
    CellSplitter cell_splitter;
    if (! cell_splitter.Init(argc, argv)){
        cout << "Init CellSplitter failed!" << endl;
        g_logger.LogToFile("Init CellSplitter failed!");
        exit(2);
    }
#ifdef PTHREAD
	pthread_t monitor_thread, recv_thread, send_thread;
#else
	thread monitor_thread, recv_thread, send_thread;
#endif
    /*if (! StartThread(&monitor_thread, MonitorFunc, &cell_splitter)){
        cout << "Start monitor thread in Cell Splitter failed!" << endl;
        g_logger.LogToFile("Start monitor thread in Cell Splitter failed!");
        exit(2);
    }*/

    if (! StartThread(&recv_thread, RecvFunc, &cell_splitter)){
        cout << "Start receive thread in Cell Splitter failed!" << endl;
        g_logger.LogToFile("Start receive thread in Cell Splitter failed!");
        exit(2);
    }

    if (! StartThread(&send_thread, SendFunc, &cell_splitter)){
        cout << "Start send thread in Cell Splitter failed!" << endl;
        g_logger.LogToFile("Start send thread in Cell Splitter failed!");
        exit(2);
    }

	//JoinThread(monitor_thread);
	JoinThread(recv_thread);
	JoinThread(send_thread);
    return 0;
}

