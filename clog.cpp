#include "clog.h"

CLog _clog("mylog"); 

CLog::CLog(string fn){
	LEVEL.push_back("DEBUG");
	LEVEL.push_back("INFO");
	LEVEL.push_back("ERROR");
	strcpy(file_name,fn.c_str());
	log_mtx.lock();
	log_file = fopen(file_name,"w+");
	fclose(log_file);
	log_mtx.unlock();
}

CLog::~CLog(){

}

char *CLog::get_msg(int level,string s){
    time_t t = time(0);
    struct tm *local = localtime(&t);
    char time_buf[128];
    strftime(time_buf,64,"%Y/%m/%d %H:%M:%S",local);
    strcpy(log_msg,"[");
    strcat(log_msg,time_buf);
    strcat(log_msg,"] [");
    strcat(log_msg,LEVEL[level].c_str());
    strcat(log_msg,"] ");
    strcat(log_msg,s.c_str());
    strcat(log_msg,"\n");
    return log_msg;
}

void CLog::log_clear(){
	log_mtx.lock();
	log_file = fopen(file_name,"w");
	fclose(log_file);
	log_mtx.unlock();
}

mutex *CLog::get_mtx(){
	return &log_mtx;
}

FILE *CLog::open_file(){
	log_file = fopen(file_name,"a+");
	return log_file;
}

void CLog::close_file(){
	fflush(log_file);
	fclose(log_file);
}

/*
int main(){
	CLog *lg = new CLog("mylog");
	LogWrite(1,"%d\n",123);
	return 0;
}*/
