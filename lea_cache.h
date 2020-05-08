#ifndef _LEA_CACHE_H
#define _LEA_CACHE_H
#include <unordered_map>
#include <string.h>
#include "cache_c.h"
#include "ssd_cache.h"
#include "list_entry.h"
#include "trace_info.h"
#include "history_entry.h"

class LeaCache : public SSDCache
{
public:
	LeaCache(int cache_size, Ctx *ctx);
	~LeaCache();
	void map_operation(TraceInfo *ti, Ctx *ctx);
	void insert_mru(ListEntry *le);
	ListEntry *remove_lru();
	void his(char *map_key);
	void move_to_his_mru(HistoryEntry *he);
	HistoryEntry *remove_his_lru();
	HistoryEntry *remove_his_entry(HistoryEntry *he);
	void replacement(uint64_t key, Ctx *ctx,char *map_key);
	int can_evict_1(ListEntry *le,char * map_key, Ctx *ctx);
	int can_evict_2(ListEntry *le_lru,int area,Ctx *ctx);

public:
	ListEntry *lru;
	ListEntry *mru;
	HistoryEntry *hlru;
	HistoryEntry *hmru;
	HistoryEntry *history_space;
	unordered_map<string,HistoryEntry *> history_map;
	uint64_t current_block;/*记录当前块，用于计算重用距离*/
};
#endif
