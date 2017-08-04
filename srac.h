#include <algorithm>

class __SRAC : public Algorithm
{
private://缓存基本信息

    struct list_entry *ghost_mru; //ghost的hot端
    struct list_entry *ghost_lru; //ghost的cold端
    unordered_map<string,struct list_entry *> ghost_map; //ghost索引

    double cr;
    int c;
    int freq;

public: //缓存基本操作

    //初始化
    __SRAC(cache_c *ctx)
    {
        cr =  ctx->block_num_conf * 0.1;
        c =  ctx->block_num_conf;
        ghost_lru = ghost_mru =NULL;
        freq = 5;
    }

    ~__SRAC()
    {
    }

    void map_operation(u_int64_t key, cache_c *ctx,char *map_key)
    {
        list_entry *cacheblk = ctx->cache_blk;
        //cout << map_key << endl;

        unordered_map<string,struct list_entry *>::iterator got = cache_map.find(map_key);
        if(got == cache_map.end())
        {//未命中
            //cr = min(0.9*c,(cr+(c/cr)));

            unordered_map<string,struct list_entry *>::iterator got1 = ghost_map.find(map_key);
            
            if(got1 == ghost_map.end())
            {//ghost未命中
                struct list_entry *  le = (struct list_entry *)malloc(sizeof(struct list_entry));
                strcpy(le->map_key,map_key);

                add_ghost(le);
                if(ghost_map.size() > c)
                    rm_ghost_lru();

                return;
            }
            
            struct list_entry *  le = got1->second;

            if(le->access_cnt < 5)
            {
                le->access_cnt++;
                mv_to_ghost_mru(le);
                return;
            }
            rm_ghost(le);

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
                ctx->mru->access_cnt = freq+1;
                ctx->mru->pre_access = ctx->stat->total_num;
                strcpy(ctx->mru->map_key,map_key);
                cache_map[map_key] = ctx->mru;
            }
            else
            {
                //写ssd
                ctx->stat->write_ssd();

                struct list_entry *  le = &cacheblk[cache_map.size()];
                le->lbn = cache_map.size();

                if(ctx->mru == NULL)
                {
                    le->next = le->pre = NULL;
                    le->block_id = key; 
                    le->access_cnt = freq+1;
                    le->pre_access = ctx->stat->total_num;
                    strcpy(le->map_key,map_key);
                    ctx->mru = ctx->lru = le;
                    cache_map[map_key] = le;
                }
                else
                {
                    le->pre = NULL;
                    le->block_id = key; 
                    strcpy(le->map_key,map_key);
                    le->access_cnt = freq+1;
                    le->pre_access = ctx->stat->total_num;

                    le->next = ctx->mru;
                    ctx->mru->pre = le;
                    ctx->mru = le;
                    cache_map[map_key] = le;
                }
                le = NULL;
            }


        }
        else
        {//命中
            //cr = max(0.1*c,(cr-(c/(c-cr))));
            struct list_entry *  le = got->second;
            if(ctx->mru != le)
            {
                le->pre->next = le->next;

                if(le->next != NULL){
                    le->next->pre = le->pre;
                }else{
                    ctx->lru = le->pre;
                    ctx->lru->next = NULL;
                }

                le->next = ctx->mru;
                ctx->mru->pre = le;
                le->pre = NULL;
                ctx->mru = le;

            }
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

struct list_entry *le1 = ghost_mru;
cout << "ghost_list"<< "\t" << ghost_map.size() << "\t";
while(le1 != NULL){
    cout << le1->map_key << "\t";
    le1 = le1->next;
}
cout << endl;
le1 = ctx->mru;
cout << "entry_list"<< "\t"<< cache_map.size() << "\t";
while(le1 != NULL){
    cout << le1->map_key << "\t";
    le1 = le1->next;
}
cout << endl << endl;

add_ghost(le);
if(ghost_map.size() > cr)
    rm_ghost_lru();
#endif

    }

    void add_ghost(struct list_entry *le)
    {
        //添加索引
        ghost_map[le->map_key] = le;
        
        if(ghost_lru == NULL)
        {
            le->pre = le->next = NULL;
            ghost_lru = ghost_mru = le;
        }
        else
        {
            le->next = ghost_mru;
            ghost_mru->pre = le;
            le->pre = NULL;
            ghost_mru = le;
        }

    }

    void rm_ghost(struct list_entry *le)
    {
        //删除索引
        ghost_map.erase(le->map_key);
        
        if(ghost_lru == ghost_mru)
        {
            ghost_lru = ghost_mru = NULL;
            free(le);
            return;
        }
        
        if(le->pre != NULL)
            le->pre->next = le->next;
        else
            ghost_mru = le->next;

        if(le->next != NULL)
            le->next->pre = le->pre;
        else
            ghost_lru = le->pre;

        free(le);
    }

    void rm_ghost_lru()
    {
        ghost_map.erase(ghost_lru->map_key);
        if(ghost_lru == ghost_mru)
        {
            ghost_lru = ghost_mru = NULL;
            free(ghost_lru);
        }
        else
        {
            struct list_entry *le = ghost_lru;
            le->pre->next = NULL;
            ghost_lru = le->pre;
            free(le);
        }
    }

    void mv_to_ghost_mru(struct list_entry *le)
    {
        if(le->pre == NULL)
            return;
        
        if(le->next == NULL)
        {
            le->next = ghost_mru;
            ghost_mru->pre = le;
            le->pre = NULL;
            ghost_mru = le;
            return;
        }
        else
        {
            le->pre->next = le->next;
            le->next->pre = le->pre;

            le->next = ghost_mru;
            ghost_mru->pre = le;
            le->pre = NULL;
            ghost_mru = le;
        }
    }
};