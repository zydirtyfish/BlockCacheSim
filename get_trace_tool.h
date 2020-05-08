#ifndef _GET_TRACE_TOOL_H
#define _GET_TRACE_TOOL_H

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include "trace_info.h"
#include "common_types.h"
#include "clog.h"

using namespace std;

class GetTraceTool
{
public:
	GetTraceTool(const char *fn);
	~GetTraceTool();
	TraceInfo * get_ti();
	double get_process_ratio();
	uint64_t get_rec_cnt();

private:
	char filename[200];
	FILE *fp;
	TraceInfo *ti;
	uint64_t total_cnt;
	uint64_t current_cnt;
};
#endif
