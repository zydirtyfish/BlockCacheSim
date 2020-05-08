#include <string>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include "common_types.h"
#include "clog.h"
#include "get_trace_tool.h"
#include "cfg_file.h"
#include "list_entry.h"
#include "cache_c.h"
#include "device_mapper.h"

#define FIFO 0
#define LRU 1
#define OPT 2
#define ARC 3
#define LARC 4
#define SRAC 5
#define MRU 6
#define ASTAT 100
#define LEA 101

using namespace std;
mutex mtx;

void io_process(int cache_algorithm, int cache_size)
{
	uint64_t cnt = 0;

	Ctx *ctx = new Ctx;
	ctx->stat = new Stat;
	ctx->stat->read_cnt = ctx->stat->total_cnt = ctx->stat->memory_hit =
		ctx->stat->ssd_hit = ctx->stat->memory_write = ctx->stat->ssd_write = 0;
	ctx->cache_algorithm =  cache_algorithm;
	ctx->cache_size = cache_size;
	ctx->para = _cfg._para;
	ctx->k = _cfg._k;

	LogWrite(1,"cache size: %d , algorithm : %d",cache_size,cache_algorithm);
	DeviceMapper *dm = new DeviceMapper(ctx);
	GetTraceTool *get_trace_tool = new GetTraceTool(_cfg._io_trace);
	TraceInfo *ti = get_trace_tool->get_ti();

	while(ti)
	{
		/*
		LogWrite(1,"%llu,%s,%d,%s,%llu,%u,%u",
					ti->timestamp,
					ti->hostname,
					ti->disknum,
					ti->io_type,
					ti->offset,
					ti->size,
					ti->responsetime);
		*/
		ctx->ti = ti;
		dm->kernel(ctx);
		ti = get_trace_tool->get_ti();
		cnt ++;
	}

	mtx.lock();
	LogWrite(1,"algorithm type : %d, cache size : %d, cnt : %llu", cache_algorithm,cache_size,cnt);
	FILE *fp = fopen("result","a+");
	fprintf(fp,"%d,%d,%.4lf,%.4lf,%.4lf,%llu\n",
				cache_algorithm,
				cache_size,
				(ctx->stat->memory_hit) * 100.0/ ctx->stat->total_cnt,
				(ctx->stat->ssd_hit) * 100.0/ ctx->stat->total_cnt,
				(ctx->stat->ssd_hit + ctx->stat->memory_hit) * 100.0/ ctx->stat->total_cnt,
				ctx->stat->ssd_write);
	fclose(fp);
	mtx.unlock();

	delete get_trace_tool;
	delete dm;
	delete ctx->stat;
	delete ctx;
}


int main(int argc, char *argv[])
{
	LogWrite(1,"cache simulator process start!!!");
	if(2 != argc)
	{
		LogWrite(2,"Usage: ./cbs_simulator config_file");
		exit(-1);
	}

	if( 0 != access(argv[1], F_OK))
	{
		LogWrite(2,"Error, config file %s not exist",argv[1]);
		exit(-1);
	}

	_cfg.init_cfg(argv[1]);

	FILE *fp = fopen("result","w+");
	fclose(fp);

	int size_num = _cfg._cache_size_list.size();
	int algorithm_num = _cfg._algorithm_list.size();
	thread simulator_threads[size_num * algorithm_num];
	int cnt = 0;
	for(int i = 0 ; i < algorithm_num ; i ++)
	{
		for(int j = 0 ; j < size_num ; j ++)
		{
			simulator_threads[cnt++] = thread(io_process,
											_cfg._algorithm_list[i],
											_cfg._cache_size_list[j]);
		}
	}
	
	cnt = 0;
	for(int i = 0 ; i < algorithm_num ; i ++)
	{
		for(int j = 0 ; j < size_num ; j ++)
		{
			simulator_threads[cnt++].join();
		}
	}


	return 0;
}
