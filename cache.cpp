#include "cache.h"

Cache::Cache(int cs, Ctx *ctx)
{
	cache_size = cs;
	clean_region = cs / 6;
	cache_space = (ListEntry *)malloc(sizeof(ListEntry) * cache_size);

	cache_algorithm = ctx->cache_algorithm;
	free_space_head = (ListEntry *)malloc(sizeof(ListEntry));
	free_space_tail = (ListEntry *)malloc(sizeof(ListEntry));
	lru_end = (ListEntry *)malloc(sizeof(ListEntry));
	mru_end = (ListEntry *)malloc(sizeof(ListEntry));

	for(int i = 0 ; i < cache_size - 1 ; i ++)
	{
		cache_space[i].next = &cache_space[i+1];
		cache_space[i+1].pre = &cache_space[i];
	}
	cache_space[0].pre = free_space_head;
	free_space_head->next = &cache_space[0];
	cache_space[cache_size-1].next = free_space_tail;
	free_space_tail->pre = &cache_space[cache_size-1]; 
	mru_end->next = lru_end;
	lru_end->pre = mru_end; 
	mru_end->pre = lru_end->next = NULL;
}

Cache::~Cache()
{
	free(cache_space);
}

ListEntry *Cache::get_free_space()
{
	ListEntry *rst = NULL;
	if(free_space_head->next != free_space_tail)
	{
		rst = free_space_head->next;
		rst->pre->next = rst->next;
		rst->next->pre = rst->pre;
		rst->next = rst->pre = NULL;
		rst->dirty = 0;
	}
	else
	{
		rst = get_eviction();
	}
	return rst;
}

ListEntry *Cache::get_eviction()
{
	ListEntry *rst = NULL;
	if(cache_algorithm == 1){/*lru*/
		rst = lru_end->pre;
		reclaim_space(rst);
	}
	else{/*CFLRU*/
		int cnt = clean_region;
		rst = lru_end->pre;
		while(cnt--)
		{
			if(!rst->dirty)
				break;
			rst = rst->pre;
		}
		if(rst->dirty)
			rst = lru_end->pre;
		reclaim_space(rst);
	}
	return rst;
}

void *Cache::reclaim_space(ListEntry *le)
{
	cache_index.erase(le->map_key);
	le->pre->next = le->next;
	le->next->pre = le->pre;
}

ListEntry *Cache::get(char *key)
{
	auto it = cache_index.find(key);
	if(it != cache_index.end())
	{
		return it->second;
	}
	return NULL;
}

uint64_t Cache::set(char *key, uint64_t block_id, int io_type, char *eviction_key)
{
	auto it = cache_index.find(key);
	ListEntry *le = NULL;
	uint64_t rst = 0;
	if(it != cache_index.end())
	{
		le = it->second;
		if(io_type)
			le->dirty = 1;
		le->pre->next = le->next;
		le->next->pre = le->pre;
		le->next = mru_end->next;
		le->pre = mru_end;
		mru_end->next->pre = le;
		mru_end->next = le;
	}
	else
	{
		le = get_free_space();
		if(le->dirty)
		{
			strcpy(eviction_key,le->map_key);
			rst = le->offset;
			le->dirty = 0;
		}
		cache_index[key] = le;
		le->offset = block_id;
		strcpy(le->map_key,key);
		le->next = mru_end->next;
		le->pre = mru_end;
		mru_end->next->pre = le;
		mru_end->next = le;			
		le->access_cnt = 1;
		if(io_type)
			le->dirty = 1;
		else
			le->dirty = 0;
	}
	return rst;
}
