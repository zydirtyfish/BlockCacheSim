#include "lru_cache.h"

LruCache::LruCache(int cs, Ctx *ctx)
{
	cache_size = cs;
	cache_space = (ListEntry *)malloc(sizeof(ListEntry) * cs);
	lru = (ListEntry *)malloc(sizeof(ListEntry));
	mru = (ListEntry *)malloc(sizeof(ListEntry));
	mru->pre = lru->next = NULL;
	mru->next = lru;
	lru->pre = mru;
}

LruCache::~LruCache()
{
	free(mru);
	free(lru);
	free(cache_space);
}

void LruCache::map_operation(TraceInfo *ti,Ctx *ctx)
{
	auto it = cache_index.find(ti->map_key);
	if(it == cache_index.end())
	{/*cache miss*/
		ctx->stat->ssd_write ++;
		if(cache_index.size() == cache_size)
		{/*make replacemant*/
			cache_index.erase(lru->pre->map_key);
			ListEntry *eviction = lru->pre;
			eviction->pre->next = eviction->next;
			eviction->next->pre = eviction->pre;
			eviction->offset = ti->offset;
			strcpy(eviction->map_key,ti->map_key);
			eviction->next = mru->next;
			eviction->pre = mru;
			mru->next->pre = eviction;
			mru->next = eviction;
			cache_index[ti->map_key] = eviction;
		}
		else
		{
			ListEntry * le = &cache_space[cache_index.size()];
			cache_index[ti->map_key] = le;
			le->offset = ti->offset;
			strcpy(le->map_key,ti->map_key);
			mru->next->pre = le ;
			le->next = mru->next;
			le->pre = mru;
			mru->next = le;
		}
	}
	else
	{
		ctx->stat->ssd_hit ++;
		ListEntry *le = it->second;
		if(ti->type == 1)
			ctx->stat->ssd_write ++;

		le->pre->next = le->next;
		le->next->pre = le->pre;

		le->next = mru->next;
		le->pre = mru;
		mru->next->pre = le;
		mru->next = le;
	}
}
