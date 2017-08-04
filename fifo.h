class __FIFO : public Algorithm
{
public: //缓存基本操作

    //初始化
    __FIFO()
    {
    }

    ~__FIFO()
    {
    }

    void map_operation(u_int64_t key, cache_c *ctx)
    {
        list_entry *cacheblk = ctx->cache_blk;
        char map_key[40];
        //构造map_key
        get_map_key(map_key,ctx->ti->hostname,ctx->ti->disknum,key);
        //cout << map_key << endl;
        
        unordered_map<string,struct list_entry *>::iterator got = cache_map.find(map_key);

        if(got == cache_map.end())
        {//未命中
            if(cache_map.size() == ctx->block_num_conf)
            {//缓存满，需要替换
                
                //写ssd
                ctx->stat->write_ssd();
                cache_map.erase(ctx->lru->map_key);

                ctx->lru->next = ctx->mru;
                ctx->mru->pre = ctx->lru;
                ctx->mru = ctx->lru;
                ctx->lru = ctx->lru->pre;
                ctx->lru->next = ctx->mru->pre = NULL;
                ctx->mru->block_id = key;
                ctx->mru->access_cnt = 1;
                ctx->mru->pre_access = ctx->stat->total_num;
                strcpy(ctx->mru->map_key,map_key);
                
                cache_map[map_key] = ctx->mru;
            }
            else
            {
                struct list_entry *  le = &cacheblk[cache_map.size()];
                le->lbn = cache_map.size();

                if(ctx->mru == NULL)
                {
                    le->next = le->pre = NULL;
                    le->block_id = key; 
                    le->access_cnt = 1;
                    le->pre_access = ctx->stat->total_num;
                    strcpy(le->map_key,map_key);
                    ctx->mru = ctx->lru = le;
                    cache_map[map_key] = le;
                }
                else
                {
                    le->pre = NULL;
                    le->block_id = key; 
                    le->access_cnt = 1;
                    le->pre_access = ctx->stat->total_num;

                    le->next = ctx->mru;
                    ctx->mru->pre = le;
                    strcpy(le->map_key,map_key);
                    ctx->mru = le;
                    cache_map[map_key] = le;
                    
                }
                le = NULL;
                //写ssd
                ctx->stat->write_ssd();
            }
        }
        else
        {//命中
            struct list_entry *  le = got->second;
            le->access_cnt++;
            le->pre_access = ctx->stat->total_num;
            ctx->stat->hit_num++;
            if(ctx->ti->type == 1){
                //写ssd
                ctx->stat->write_ssd();
            }
            le = NULL;

        }
       
//lru链表测试
#if DEBUG
    struct list_entry *le1 = ctx->mru;
    while(le1 != NULL){
        cout << le1->block_id << "\t";
        le1 = le1->next;
    }
    cout << endl;
#endif
    }

};