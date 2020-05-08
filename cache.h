#ifndef _CACHE_H
#define _CACHE_H

#include <iostream>
#include <unordered_map>
#include <string.h>
#include "list_entry.h"
#include "cache_c.h"

using namespace std;
class Cache
{
public:
	ListEntry *cache_space;
	ListEntry *free_space_head;
	ListEntry *free_space_tail;
	ListEntry *lru_end;
	ListEntry *mru_end;
	int cache_algorithm;
	int clean_region;
	int cache_size;
	unordered_map<string, ListEntry *> cache_index;
	
public:
	Cache(int cache_size,Ctx *ctx);
	~Cache();
	ListEntry *get_free_space();
	void *reclaim_space(ListEntry *);
	ListEntry * get(char *key);
	uint64_t set(char *key, uint64_t block_id, int io_type,char *eviction_key);
	ListEntry *get_eviction();
};
#endif
