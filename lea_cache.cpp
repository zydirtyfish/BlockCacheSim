#include "lea_cache.h"

LeaCache::LeaCache(int cs, Ctx *ctx)
{
	cache_size = cs;
	cache_space = (ListEntry *)malloc(sizeof(ListEntry) * cs);
	history_space = (HistoryEntry *)malloc(sizeof(HistoryEntry) * cs);
	lru = (ListEntry *)malloc(sizeof(ListEntry));
	mru = (ListEntry *)malloc(sizeof(ListEntry));
	hlru = (HistoryEntry *)malloc(sizeof(HistoryEntry));
	hmru = (HistoryEntry *)malloc(sizeof(HistoryEntry));
	hlru->next = hmru->pre = NULL;
	mru->next = lru;
	lru->pre = mru;
	hmru->next = hlru;
	hlru->pre = hmru;
	current_block = 0;
}

LeaCache::~LeaCache()
{
	free(hmru);
	free(hlru);
	free(mru);
	free(lru);
	free(cache_space);
}

void LeaCache::map_operation(TraceInfo *ti, Ctx *ctx)
{
	ListEntry *le;
	current_block++;
	auto it = cache_index.find(ti->map_key);
	if(it == cache_index.end())
	{//未命中 情况(1)
		if(cache_index.size() == cache_size)
		{//缓存满，需要替换
			le = lru->pre;
			auto it2 = history_map.find(ti->map_key);
			if(it2 == history_map.end())
			{//情况(1)-(a) L1与L2都未命中
				if(can_evict_1(le,ti->map_key,ctx))
				{
					replacement(ti->offset,ctx,ti->map_key);
				}
				else
				{
					his(ti->map_key);
				}
			}
			else
			{//情况(1)-(b) L1未命中，L2命中
				HistoryEntry *he = it2->second;
				he = remove_his_entry(he);
				if(can_evict_2(le,10,ctx))
				{
					he->access_cnt = lru->pre->access_cnt;
					strcpy(he->map_key,lru->pre->map_key);
					move_to_his_mru(he);
					replacement(ti->offset,ctx,ti->map_key);
				}
				else
				{
					he->access_cnt ++;
					move_to_his_mru(he);
				}
			}
		}
		else
		{//缓存未满，填充缓存
			le = &cache_space[cache_index.size()];
			le->offset = ti->offset;
			le->access_cnt = ctx->para;
			le->pre_access = current_block;
			le->avg_pre = 0;
			strcpy(le->map_key,ti->map_key);
			cache_index[ti->map_key] = le;
			insert_mru(le);
			ctx->stat->ssd_write ++;
		}
	}
	else
	{//命中 情况(2)
		le = it->second;
		le->avg_pre = (current_block - le->pre_access + 1);
		le->access_cnt++;
		le->pre_access = current_block;
		ctx->stat->ssd_hit++;
		if(ctx->ti->type == 1)
		{
			ctx->stat->ssd_write ++;
		}
		le = NULL;
	}
}

void LeaCache::insert_mru(ListEntry *le)
{/*在mru端插入数据，仅在缓存为填满时使用*/
	le->next = mru->next;
    le->pre = mru;
    mru->next = le;
    le->next->pre = le;
}

ListEntry *LeaCache::remove_lru()
{
    ListEntry *tmp = lru->pre;
    lru->pre->pre->next = lru;
    lru->pre = lru->pre->pre;
    tmp->next = tmp->pre = NULL;
    return tmp;
}

void LeaCache::his(char *map_key)
{
	HistoryEntry *he = NULL;	
    if(history_map.size() < cache_size)
    {
		he = &history_space[history_map.size()];
    }
    else
    {
        he = remove_his_lru();
    }
	he->access_cnt = 1;
   	he->pre_access = current_block;
    strcpy(he->map_key,map_key);
    move_to_his_mru(he);
}

void LeaCache::move_to_his_mru(HistoryEntry *he)
{
    he->next = hmru->next;
    he->pre = hmru;
    he->pre->next = he;
    he->next->pre = he;
    history_map[he->map_key] = he;
}

HistoryEntry *LeaCache::remove_his_lru()
{
    HistoryEntry *he = hlru->pre;
    he->pre->next = he->next;
    he->next->pre = he->pre;
    history_map.erase(he->map_key);
    return he;
}

HistoryEntry *LeaCache::remove_his_entry(HistoryEntry *he)
{
    history_map.erase(he->map_key);
    he->pre->next = he->next;
    he->next->pre = he->pre;
    return he;
}

void LeaCache::replacement(uint64_t key, Ctx *ctx,char *map_key)
{/*代表一次替换动作*/
    cache_index.erase(lru->pre->map_key);
    ListEntry *le = remove_lru();
    le->offset = key;
    le->access_cnt = ctx->para;
    le->pre_access = current_block;
    le->avg_pre = 0;
    strcpy(le->map_key, map_key);
    cache_index[map_key] = le;
    insert_mru(le);
	
	//写ssd
	ctx->stat->ssd_write++;
}

int LeaCache::can_evict_1(ListEntry *le,char * map_key, Ctx *ctx)
{/*判断情况(1)-(a)能否替换*/
    if(le->access_cnt > 0)
    {
        insert_mru(remove_lru());
        return 0;
    }
    return 1;
}

int LeaCache::can_evict_2(ListEntry *le_lru,int area,Ctx *ctx)
{/*判断情况(1)-(b)能否替换*/
    ListEntry *le = le_lru;
    for(int pp = 0 ; pp < area; pp++){
        le->access_cnt >> 1;
        if(le->avg_pre == 0 || (current_block - le->pre_access + 1 > (ctx-> k * le->access_cnt) * le->avg_pre))
        {
            return 1;
        }
        insert_mru(remove_lru());
        le = lru->pre;
    }
    return 0;
}
