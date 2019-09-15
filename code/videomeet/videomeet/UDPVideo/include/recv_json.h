#include<iostream>
#include<string>
#include <fstream> 
#include<sstream>
#include<map>
#include"rapidjson/document.h"
#include"rapidjson/stringbuffer.h"
#include"rapidjson/writer.h"
#include "rapidjson/filereadstream.h"
#include"rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"  
using namespace std;
using namespace rapidjson;
void read_config(string path, uint8_t&mode)
{
	cout << "read configure" << endl;
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
