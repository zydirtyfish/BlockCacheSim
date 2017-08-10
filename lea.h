#include <list>

class __LEA : public Algorithm
{

private://缓存基本信息

	u_int64_t current_index;

public: //缓存基本操作

    //初始化
    __LEA()
    {
    	current_index = 0;
    }

    ~__LEA()
    {
    }

    void map_operation(u_int64_t key, cache_c *ctx,char *map_key)
    {
        list_entry *cache_entry = ctx->cache_blk;

        unordered_map<string,struct list_entry *>::iterator got = cache_map.find(map_key);

        if(got == cache_map.end())
        {//未命中
            int map_size = cache_map.size();
            //cout << cache_map.size() << endl;
            if(map_size == ctx->block_num_conf)
            {//缓存满，需要替换

                int victim = find_victim(ctx);

                if(victim == -1)
                    return;

                cache_map.erase(cache_entry[victim].map_key);
                
                cache_entry[victim].block_id = key;
                cache_entry[victim].access_cnt = 1;
                cache_entry[victim].pre_access = ctx->stat->total_num;
                cache_entry[victim].avg_pre = 0;
                cache_entry[victim].io_type = ctx->ti->type;
                cache_entry[victim].io_size = ctx->ti->size;
                strcpy(cache_entry[victim].map_key , map_key);

                cache_map[map_key] = &cache_entry[victim];


                //写ssd
                ctx->stat->write_ssd();
            }
            else
            {
                cache_entry[map_size].block_id = key;
                cache_entry[map_size].access_cnt = 1;
                cache_entry[map_size].pre_access = ctx->stat->total_num;
                cache_entry[map_size].avg_pre = 0;
                cache_entry[map_size].lbn = map_size;
                cache_entry[map_size].io_type = ctx->ti->type;
                cache_entry[map_size].io_size = ctx->ti->size;
                strcpy(cache_entry[map_size].map_key , map_key);
                
                cache_map[map_key] = &cache_entry[map_size];

                //写ssd
                ctx->stat->write_ssd();
            }
        }
        else
        {//命中
            struct list_entry *  le = got->second;

            le->avg_pre = ((u_int64_t)((le->avg_pre)*(le->access_cnt)) + (u_int64_t)(ctx->stat->total_num - le->pre_access))/(le->access_cnt + 1);
            le->access_cnt++;
            le->pre_access = ctx->stat->total_num;
            le->io_type = ctx->ti->type;
            le->io_size = ctx->ti->size;

            ctx->stat->hit_num++;
            if(ctx->ti->type == 1){
                //写ssd
                ctx->stat->write_ssd();
            }
            le = NULL;
        }
       
//lru链表测试
#if DEBUG
    int cs = cache_map.size();
    for(int i=0 ; i < cs ; i++)
    {
        cout << cache_entry[i].block_id << "\t";
    }
    cout << endl;
#endif
    }

    int find_victim(cache_c *ctx)
    {
    	list_entry *cache_entry = ctx->cache_blk;
		int result = -1;
        if(cache_entry[current_index].access_cnt == 1)
        {
            if(ctx->stat->total_num - cache_entry[current_index].pre_access > ctx->PARA)
            {
                result = current_index;
            }
        }
        else if(ctx->stat->total_num - cache_entry[current_index].pre_access > ctx->K*cache_entry[current_index].avg_pre)
        {
            result = current_index;
        }

        current_index = (current_index+1) % ctx->block_num_conf;
        return result;
    }


};