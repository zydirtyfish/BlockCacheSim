struct history_entry
{
    struct history_entry * pre;
    struct history_entry *next;
    u_int64_t access_cnt;
    u_int64_t pre_access;
    char map_key[40];
};

class __LEA : public Algorithm
{
private://缓存基本信息
    u_int64_t current_block;/*记录当前块，用于计算重用距离*/
    struct list_entry *lru;
    struct list_entry *mru;
    struct history_entry *hlru;
    struct history_entry *hmru;
    unordered_map<string, struct history_entry *> history_map; //缓存索引

public: //缓存基本操作

    //初始化
    __LEA()
    {
    	current_block = 0;
        lru = (struct list_entry*)malloc(sizeof(struct list_entry));
        mru = (struct list_entry*)malloc(sizeof(struct list_entry));
        hlru = (struct history_entry*)malloc(sizeof(struct history_entry));
        hmru = (struct history_entry*)malloc(sizeof(struct history_entry));
        lru->next = mru->pre = NULL;
        hlru->next = hmru->pre = NULL;
        mru->next = lru;
        lru->pre = mru;
        hmru->next = hlru;
        hlru->pre = hmru;
    }

    ~__LEA()
    {}

    void map_operation(u_int64_t key, struct cache_c *ctx,char *map_key)
    {
        struct list_entry *le;
        auto got = cache_map.find(map_key);
        //cout << cache_map.size() << endl;
        //cout << "123" << endl;
        current_block++;
        if(got == cache_map.end())
        {//未命中 情况(1)
            if(cache_map.size() == ctx->block_num_conf)
            {//缓存满，需要替换 
                le = lru->pre;
                auto got2 = history_map.find(map_key); 
                if(got2 == history_map.end())
                {//情况(1)-(a) L1与L2都未命中
                    if(can_evict_1(le,map_key,ctx))
                    {
                        replacement(key,ctx,map_key);
                    }
                    else
                    {
                        struct history_entry *he = (struct history_entry*)malloc(sizeof(struct history_entry));
                        he->access_cnt = 1;
                        his(map_key,he,ctx->block_num_conf);
                    }
                }
                else
                {//情况(1)-(b) L1未命中，L2命中
                    struct history_entry *he = got2->second;
                    he = remove_his_entry(he);
                    if(can_evict_2(le,10,ctx))
                    {
                        he->access_cnt = lru->pre->access_cnt;
                        his(lru->pre->map_key,he,ctx->block_num_conf);
                        replacement(key,ctx,map_key);
                    }
                    else
                    {
                        he->access_cnt++;
                        his(map_key,he,ctx->block_num_conf);
                    }
                }
            }
            else
            {//缓存未满，填充缓存
                //current_block++;
                le = (struct list_entry *)malloc(sizeof(struct list_entry));
                le->block_id = key;
                le->access_cnt = ctx->PARA;
                le->pre_access = current_block;
                le->avg_pre = 0;
                le->io_type = ctx->ti->type;
                le->io_size = ctx->ti->size;
                strcpy(le->map_key, map_key);
                cache_map[map_key] = le;
                insert_mru(le);
                //写ssd
                ctx->stat->write_ssd();
                //cout <<mru->next->map_key << "\t" << cache_map.size()<< endl;
            }
        }
        else
        {//命中 情况(2)
            le = got->second;
            le->avg_pre = (current_block - le->pre_access + 1);
            le->access_cnt++;
            le->pre_access = current_block;
            le->io_type = ctx->ti->type;
            le->io_size = ctx->ti->size;
            //更新统计信息
            ctx->stat->hit_num++;
            if(ctx->ti->type == 1)
            {
                //写ssd
                ctx->stat->write_ssd();
            }
            le = NULL;
        }
 

//lru链表测试
#if DEBUG
        cout << map_key << endl;
        le = mru->next;
        while (le->next != NULL)
        {
            cout <<le->map_key << " ";
            le = le->next;
        }
        cout << "------";
        struct history_entry *hee = hmru->next;
        while (hee->next != NULL)
        {
            cout <<hee->map_key << " ";
            hee = hee->next;
        }
        cout << endl << endl;
#endif
    }

    void insert_mru(struct list_entry *le)
    {/*在mru端插入数据，仅在缓存为填满时使用*/
        le->next = mru->next;
        le->pre = mru;
        mru->next = le;
        le->next->pre = le;
    }

    struct list_entry * remove_lru()
    {
        struct list_entry *tmp = lru->pre;
        lru->pre->pre->next = lru;
        lru->pre = lru->pre->pre;
        tmp->next = tmp->pre = NULL;
        return tmp;
    }


    void his(char *map_key,struct history_entry *he,u_int64_t cache_size)
    {
        he->pre_access = current_block;
        strcpy(he->map_key,map_key);
        if(history_map.size() < cache_size)
        {
            move_to_his_mru(he);
        }
        else
        {
            free(remove_his_lru());
            move_to_his_mru(he);
        }
    }

    void move_to_his_mru(history_entry *he)
    {
        he->next = hmru->next;
        he->pre = hmru;
        he->pre->next = he;
        he->next->pre = he;
        history_map[he->map_key] = he;
    }

    struct history_entry *remove_his_lru()
    {
        struct history_entry *he = hlru->pre;
        he->pre->next = he->next;
        he->next->pre = he->pre;
        history_map.erase(he->map_key);
        return he;
    }

    struct history_entry *remove_his_entry(history_entry *he)
    {
        history_map.erase(he->map_key);
        he->pre->next = he->next;
        he->next->pre = he->pre;
        return he;
    }

    void replacement(u_int64_t key, struct cache_c *ctx,char *map_key)
    {/*代表一次替换动作*/
        cache_map.erase(lru->pre->map_key);
        free(remove_lru());

        struct list_entry *le = (struct list_entry *)malloc(sizeof(struct list_entry));
        le->block_id = key;
        le->access_cnt = ctx->PARA;
        le->pre_access = current_block;
        le->avg_pre = 0;
        le->io_type = ctx->ti->type;
        le->io_size = ctx->ti->size;
        strcpy(le->map_key, map_key);
        cache_map[map_key] = le;
        insert_mru(le);

        //current_block++;
        //写ssd
        ctx->stat->write_ssd();
        
    }
    int can_evict_1(struct list_entry *le,char * map_key, struct cache_c *ctx)
    {//判断情况(1)-(a)能否替换
        if(le->access_cnt > 0)
        {
            insert_mru(remove_lru());
            return 0;
        }
        return 1;
    }
    int can_evict_2(struct list_entry *le_lru,int area,struct cache_c *ctx)
    {//判断情况(1)-(b)能否替换
        struct list_entry *le = le_lru;
        for(int pp = 0 ; pp < area; pp++){
            le->access_cnt >> 1;
            if(le->avg_pre == 0 || (current_block - le->pre_access + 1 > (ctx->K*le->access_cnt) * le->avg_pre))
            {
                return 1;
            }
            insert_mru(remove_lru());
            le = lru->pre;
        }
        return 0;
    }
};
