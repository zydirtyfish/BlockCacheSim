#ifndef _CLOG_H
#define _CLOG_H
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <mutex>
#include <vector>
#define LOG_ON 1
#define PRINT_INFO 0
#define CURRENT_LEVEL 1

using namespace std;
class CLog{
public:

	CLog(string fn);
	~CLog();
	char *get_msg(int level,string s);
	void log_clear();
	mutex *get_mtx();
	FILE *open_file();
	void close_file();

private:
	FILE *log_file;
	char file_name[20];
	vector<string> LEVEL;
	char log_msg[1000];
	mutex log_mtx;
};

extern CLog _clog; 
#if LOG_ON
#define LogWrite(level,s,arg...)\
    if(PRINT_INFO){fprintf(stdout,s,##arg);printf("\n");}\
    if(level >= CURRENT_LEVEL)\
    {_clog.get_mtx()->lock();\
    fprintf(_clog.open_file(),_clog.get_msg(level,s),##arg);\
    _clog.close_file();\
	_clog.get_mtx()->unlock();}
#else
#define LogWrite(level,f DMC_DEBUGs,arg...)
#endif

#endif
