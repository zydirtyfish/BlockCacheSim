#ifndef _CACHE_C_H
#define _CACHE_C_H
#include "trace_info.h"
#include "stat.h"

typedef struct cache_c
{//cache_ctx
    int cache_algorithm; /*algorithm type*/
    int write_algorithm_conf; /*写策略*/
	int cache_size;
	Stat *stat;
    TraceInfo *ti; /*当前的IO任务ti*/

    double para;
    double k;
    char cache_name[10];
} Ctx;
#endif//_CACHE_C_H
