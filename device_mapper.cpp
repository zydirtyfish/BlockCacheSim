#include "device_mapper.h"

DeviceMapper::DeviceMapper(Ctx *ctx)
{
	memory_cache = new Cache(ctx->cache_size / 1000, ctx);
	if(ctx->cache_algorithm == 1 || ctx->cache_algorithm == 2)
	{
		ssd_cache = new LruCache(ctx->cache_size, ctx);
	}
	else
	{
		ssd_cache = new LeaCache(ctx->cache_size, ctx);
	}
	ti_tmp = new TraceInfo;
}

DeviceMapper::~DeviceMapper()
{
	delete ti_tmp;
	delete ssd_cache;
	delete memory_cache;
}

void DeviceMapper::kernel(Ctx *ctx)
{
	uint64_t begin = ctx->ti->offset / (_cfg._block_size_conf);
	uint64_t end = (ctx->ti->offset + ctx->ti->size - 1) / (_cfg._block_size_conf);
	for(uint64_t i = begin ; i <= end ; i ++)
	{
		ctx->stat->total_cnt ++;
		ctx->ti->offset = i;
		//ctx->ti->disknum = 0 ;
		get_map_key(ctx->ti->map_key,ctx->ti->hostname,ctx->ti->disknum,i);
		if(static_cast<int>(ctx->ti->type) == 1)
		{/*it is a write*/
			write(ctx);
		}
		else
		{/*it is a read*/
			read(ctx);
		}
	}
}

void DeviceMapper::write(Ctx *ctx)
{
	if(memory_cache->get(ctx->ti->map_key))
	{/*memory write hit*/
		ctx->stat->memory_hit ++;
		memory_cache->set(ctx->ti->map_key,ctx->ti->offset,1,ti_tmp->map_key);
	}
	else
	{/*memory write miss*/
		int of = memory_cache->set(ctx->ti->map_key,ctx->ti->offset,1,ti_tmp->map_key);
		if(of != 0)
		{
			ti_tmp->offset = of;
			ti_tmp->type = 1;
			ssd_cache->map_operation(ti_tmp,ctx);
		}
	}
}

void DeviceMapper::read(Ctx *ctx)
{
	ctx->stat->read_cnt ++;
	if(memory_cache->get(ctx->ti->map_key))
	{/*memory read hit*/
		ctx->stat->memory_hit ++;
		memory_cache->set(ctx->ti->map_key,ctx->ti->offset,0,ti_tmp->map_key);
	}
	else
	{/*memory read miss*/
		ti_tmp->offset = ctx->ti->offset;
		ti_tmp->type = 0;
		strcpy(ti_tmp->map_key,ctx->ti->map_key);
		ssd_cache->map_operation(ti_tmp,ctx);
		int of = memory_cache->set(ctx->ti->map_key,ctx->ti->offset,0,ti_tmp->map_key);
		if(of != 0)
		{
			ti_tmp->offset = of;
			ti_tmp->type = 1;
			ssd_cache->map_operation(ti_tmp,ctx);
		}
	}
}

void DeviceMapper::get_map_key(char *map_key, char *hostname, int disknum, uint64_t block_id)
{
	char of[25];
	strcpy(map_key,hostname);
	strcat(map_key,",");

	char diskNumber[5];
	memset(diskNumber,'\0',sizeof(diskNumber));
	sprintf(diskNumber,"%d",disknum);
	strcat(map_key,diskNumber);
	strcat(map_key,",");

	sprintf(of,"%llu",block_id);
	strcat(map_key,of);
}
