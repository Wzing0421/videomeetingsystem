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
void read_config(string path, map<uint8_t, string>&m_map)
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
    fin.close();
	//解析并打印出来
	//cout << str_in << endl;
	uint8_t channel;
	Document document;
	document.Parse<0>(str_in.c_str());
	//Value &node1 = document["author"];
	Value &ch1 = document["1"];
	string ip1 = ch1.GetString();
	cout << "decode1_ip:" << "  " << ip1 << endl;
	channel = (uint8_t)1;
	m_map[channel] = ip1;
	Value &ch2 = document["2"];
	string ip2 = ch2.GetString();
	cout << "decode2_ip:" << "  " << ip2 << endl;
	channel = (uint8_t)2;
	m_map[channel] = ip2;
	Value &ch3 = document["3"];
	string ip3 = ch3.GetString();
	cout << "decode3_ip:" << "  " << ip3 << endl;
	channel = (uint8_t)3;
	m_map[channel] = ip3;
	Value &ch4 = document["4"];
	string ip4 = ch4.GetString();
	cout << "decode4_ip:" << "  " << ip4 << endl;
	channel = (uint8_t)4;
	m_map[channel] = ip4;
	Value &ch5 = document["5"];
	string ip5 = ch5.GetString();
	cout << "decode5_ip:" << "  " << ip5 << endl;
	channel = (uint8_t)5;
	m_map[channel] = ip5;
	Value &ch6 = document["6"];
	string ip6 = ch6.GetString();
	cout << "decode6_ip:" << "  " << ip6 << endl;
	channel = (uint8_t)6;
	m_map[channel] = ip6;
	Value &ch7 = document["7"];
	string ip7 = ch7.GetString();
	cout << "decode7_ip:" << "  " << ip7 << endl;
	channel = (uint8_t)7;
	m_map[channel] = ip7;
	Value &ch8 = document["8"];
	string ip8 = ch8.GetString();
	cout << "decode8_ip:" << "  " << ip8 << endl;
	channel = (uint8_t)8;
	m_map[channel] = ip8;
	Value &ch9 = document["9"];
	string ip9 = ch9.GetString();
	channel = (uint8_t)9;
	m_map[channel] = ip9;
	cout << "decode9_ip:" << "  " << ip9 << endl;
	Value &ch10 = document["10"];
	string ip10 = ch10.GetString();
	cout << "decode10_ip:" << "  " << ip10 << endl;
	channel = (uint8_t)10;
	m_map[channel] = ip10;
}
