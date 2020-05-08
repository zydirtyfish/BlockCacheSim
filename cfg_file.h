#ifndef _CFG_FILE_H
#define _CFG_FILE_H
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <vector>

using namespace std;

#define LINE_LENGTH 500

class CfgFile{
public:
	CfgFile();
	~CfgFile();
	void get(const char *key_path,const char *para_name, char *rst);
	void init_cfg(const char *filename);

public:
	int _block_size_conf;
	char _io_trace[LINE_LENGTH];
	vector<int> _cache_size_list;
	vector<int> _algorithm_list;
	double _para;
	double _k;
};

extern CfgFile _cfg;
#endif
