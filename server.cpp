#include"XTcp.h"
#include<stdlib.h>
#include<thread>
#include<iostream>


#include<string.h>
#include<string>
#include<regex>
//using namespace std;
using std::string;
using std::regex;
using std::smatch;


class TcpThread
{
public:
	void Main()
	{
		char buf[10000] = { 0 };
		
		int recvlen = client.Recv(buf, sizeof(buf) - 1);
		if (recvlen <= 0) 
		{
			client.Close();
			delete this;
			return;
		}
		buf[recvlen]='\0';

		printf("========recv========\n%s=========================\n",buf);


		//GET /index.html HTTP/1.1
		//Host: 192.168.3.69
		//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; rv:51.0) Gecko/20100101 Fi
		//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
		//Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3
		//Accept-Encoding: gzip, deflate
		//DNT: 1
		//Connection: keep-alive
		//Upgrade-Insecure-Requests: 1


		using std::string;
		using std::regex;
		

		string src = buf;
		
		string pattern = "^([A-Z]+) /([a-zA-Z0-9]*([.].*)) HTTP/1";
		regex r(pattern);
		smatch mas;
		
		regex_search(src, mas, r);
		if (mas.size() == 0)
		{
			printf("%s failed\n", pattern.c_str());
			Close();
		}
		string type = mas[1];
		string path = "/";
		path+=mas[2];
		string filetype=mas[3];
	       if(filetype.size() > 0)
	        filetype = filetype.substr(1,filetype.size()-1);
				printf("type:[%s]\npath:[%s]\nfiletype:[%s]",
				type.c_str(),
				path.c_str(),
				filetype.c_str()
				);
	       

		if (type != "GET")
		{
			printf("Not GET!!!\n");
			Close();
			return;
		}
		string filename = path;
		if (path == "/")
		{
			filename = "/index.html";

		}

		string filepath = "www";
		filepath += filename;
		if(filetype=="php")
		{
			string cmd= "php-cgi";
			cmd+=filepath;
			cmd+= " > ";
			filepath+=".html";
			cmd += filepath;
			printf("%s",cmd.c_str());
			system(cmd.c_str());
		}

	

		FILE* fp = fopen(filepath.c_str(), "rb");
		if (fp == NULL)
		{
			printf("open file %s failed\n",filepath.c_str());
			Close();
			return;
		}

		//��ȡ�ļ���С
		fseek(fp, 0, SEEK_END);
		int filesize = ftell(fp);
		fseek(fp, 0, 0);
		printf("file size is %d\n", filesize);



		string rmsg = "";
		rmsg = "HTTP/1.1 200 OK\r\n";
		rmsg += "Server: XHttp\r\n";
		rmsg += "Content-Type: text/html\r\n";
		rmsg += "Content-Length: ";
		char bsize[128] = { 0 };
		sprintf(bsize, "%d", filesize);
		rmsg += bsize;
		
		//rmsg += "10\r\n";
		rmsg += "\r\n\r\n";
		//rmsg += "0123456789";

		int sendsize=client.Send(rmsg.c_str(), rmsg.size());
		printf("sendsize = %d\n", sendsize);
		printf("=====send=====\n%s\n==============\n", rmsg.c_str());
		
		for (;;)
		{
			int len = fread(buf, 1, sizeof(buf), fp);
			if (len <= 0) break;
			int re=client.Send(buf, len);
			if (re <= 0) break;
		}
		
		
		client.Close();
		delete this;
	}

	XTcp client;

	void Close()
	{
		client.Close();
		delete this;
		return;
	}

};



int main(int argc, char* argv[])
{
	unsigned short port = 8080;
	if (argc > 1)
	{
		port = atoi(argv[1]);
	}

	XTcp server;
	server.CreateSocket();
	server.Bind(port);
	for (;;)
	{
		XTcp client = server.Accept();

		TcpThread* th = new TcpThread;
		th->client = client;
		std::thread sth(&TcpThread::Main, th);
		sth.detach();
	}
	server.Close();

	return 0;
}
