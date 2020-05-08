#ifndef _DEVICE_MAPPER_H
#define _DEVICE_MAPPER_H
#include "cfg_file.h"
#include "cache_c.h"
#include "cache.h"
#include "ssd_cache.h"
#include "lru_cache.h"
#include "lea_cache.h"
#include "clog.h"

class DeviceMapper
{
public:
	DeviceMapper(Ctx *ctx);
	~DeviceMapper();
	void kernel(Ctx *ctx);
	void read(Ctx *ctx);
	void write(Ctx *ctx);
	void get_map_key(char *map_key,char *hostname, int disknum,uint64_t block_id);

public:
	Cache *memory_cache;
	SSDCache *ssd_cache;
	TraceInfo *ti_tmp;
};
#endif
