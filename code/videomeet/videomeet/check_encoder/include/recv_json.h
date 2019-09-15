#include<iostream>
#include<string>
#include <fstream> 
#include<sstream>
#include<map>
#include<stdio.h>
#include"rapidjson/document.h"
#include"rapidjson/stringbuffer.h"
#include"rapidjson/writer.h"
#include "rapidjson/filereadstream.h"
#include"rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"  
using namespace std;
using namespace rapidjson;
bool check_file(string path)
{  
    fstream fin;
	fin.open(path);
    if(!fin)
    {
        cout<<"open faild"<<endl;
        return false;
    }
    cout<<"open sucessful"<<endl;
    return true;
}
void read_config(string path, uint8_t&mode)
{
	cout << "read configure" << endl;
        //char path1[100];
        //int i; 
        //for(i=0;i<path.length();i++)
            //path1[i] = path[i];
        //path1[i] = '\0';
	fstream fin;
	fin.open(path);
	string str;
	string str_in = "";
	while (getline(fin, str))    //一行一行地读到字符串str_in中  
	{
		str_in = str_in + str + '\n';
	}
	//解析并打印出来  
	//cout << str_in << endl;
	uint8_t channel;
	Document document;
	document.Parse<0>(str_in.c_str());
	//Value &node1 = document["author"];
	Value &ch1 = document["encode_id"];
	int id = ch1.GetInt();
	cout << "encode_id:" << "  " << id << endl;
	mode = (uint8_t)(id);
	cout << "encode_id:" << "  " << mode << endl;
}
void get_Encoder_IP(string path, uint8_t mode, string &destip){
        cout << "read id2ip_de configure" << endl;
	char path1[100];
        int i; 
        for(i=0;i<path.length();i++)
            path1[i] = path[i];
        path1[i] = '\0';
	fstream fin;
	fin.open(path1);
	string str;
	string str_in = "";
	while (getline(fin, str))    //一行一行地读到字符串str_in中  
	{
		str_in = str_in + str + '\n';
	}
	
	
	Document document;
	document.Parse<0>(str_in.c_str());
        
        int num = (int)mode;
        string dest;
        if(num<10){
            dest = "00" + std::to_string(num);
        }
        else if(num<100){
            dest = "0" + std::to_string(num);
        }
        else{
            dest = std::to_string(num);
        }
        
	cout<<dest<<endl;
        const char * dd = dest.c_str();
        Value &boxID = document[dd];
        if(!boxID.IsObject() || !boxID.HasMember("encode_ip")){
           cout<<" wrong configure!"<<endl;
        } 
        Value &vv = boxID["encode_ip"];
        destip = vv.GetString(); 
        cout<<destip<<endl;
}
