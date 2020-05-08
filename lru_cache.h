#ifndef _LRU_CACHE_H
#define _LRU_CACHE_H
#include <unordered_map>
#include <string.h>
#include "cache_c.h"
#include "ssd_cache.h"
#include "list_entry.h"
#include "trace_info.h"
#include "history_entry.h"

using namespace std;

class LruCache : public SSDCache
{
public:
	LruCache(int cache_size, Ctx *ctx);
	~LruCache();
	void map_operation(TraceInfo *ti,Ctx *ctx);

public:
	ListEntry *lru;
	ListEntry *mru;
};
#endif
