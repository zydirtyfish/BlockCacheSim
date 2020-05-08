#ifndef _SSD_CACHE_H
#define _SSD_CACHE_H
#include <unordered_map>
#include "list_entry.h"
#include "cache_c.h"
#include "trace_info.h"
#include "clog.h"

using namespace std;
class SSDCache
{
public:
	unordered_map<string, ListEntry *> cache_index;
	ListEntry *cache_space;
	int cache_size;

public:
	SSDCache();
	~SSDCache();
	virtual void map_operation(TraceInfo *ti, Ctx *ctx)=0;
};
#endif
