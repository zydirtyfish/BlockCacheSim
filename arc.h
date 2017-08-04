class __ARC : public Algorithm
{
private://缓存基本信息
    
    struct list_entry *hot_t1; //fifo的hot端
    struct list_entry *cold_t1; //fifo的cold端
    struct list_entry *hot_b1; //fifo的hot端
    struct list_entry *cold_b1; //fifo的cold端
    struct list_entry *hot_t2; //fifo的hot端
    struct list_entry *cold_t2; //fifo的cold端
    struct list_entry *hot_b2; //fifo的hot端
    struct list_entry *cold_b2; //fifo的cold端

    unordered_map<string,struct list_entry *> cache_map_t1; //缓存索引
    unordered_map<string,struct list_entry *> cache_map_b1; //缓存索引
    unordered_map<string,struct list_entry *> cache_map_t2; //缓存索引
    unordered_map<string,struct list_entry *> cache_map_b2; //缓存索引

    int p;
    int del;
    u_int64_t c;

public: //缓存基本操作

    //初始化
    __ARC(cache_c *ctx)
    {
        hot_t1 = cold_t1 = NULL;
        hot_b1 = cold_b1 = NULL;
        hot_t2 = cold_t2 = NULL;
        hot_b2 = cold_b2 = NULL;
        p = 0 ;
        c = ctx->block_num_conf;
    }

    ~__ARC()
    {
        //销毁lru链表
        struct list_entry *le1;
        while(hot_t1 != NULL)
        { 
            le1 = hot_t1;
            hot_t1 = le1->next;
            free(le1);
            le1 = NULL;
        }
        while(hot_t2 != NULL)
        { 
            le1 = hot_t2;
            hot_t2 = le1->next;
            free(le1);
            le1 = NULL;
        }
        while(hot_b1 != NULL)
        { 
            le1 = hot_b1;
            hot_b1 = le1->next;
            free(le1);
            le1 = NULL;
        }
        while(hot_b2 != NULL)
        { 
            le1 = hot_b2;
            hot_b2 = le1->next;
            free(le1);
            le1 = NULL;
        }

        hot_t1 = cold_t1 = hot_t2 = cold_t2 = hot_b1 = hot_b2 = cold_b1 = cold_b2 = NULL;
        //销毁map
        cache_map_t1.clear();
        cache_map_t2.clear();
        cache_map_b1.clear();
        cache_map_b2.clear();
    }



    void map_operation(u_int64_t key, cache_c *ctx)
    {
        list_entry *cacheblk = ctx->cache_blk;
        char map_key[40];
        get_map_key(map_key,ctx->ti->hostname,ctx->ti->disknum,key);
        struct list_entry *le;

        int t1 = cache_map_t1.size();
        int t2 = cache_map_t2.size();
        int b1 = cache_map_b1.size();
        int b2 = cache_map_b2.size();

        unordered_map<string,struct list_entry *>::iterator got = cache_map_t1.find(map_key);
        if(got == cache_map_t1.end())
        {//t1未命中 
           unordered_map<string,struct list_entry *>::iterator got2 = cache_map_t2.find(map_key);
           if(got2 == cache_map_t2.end())
           {//t2未命中
                unordered_map<string,struct list_entry *>::iterator got3 = cache_map_b1.find(map_key);
                if(got3 == cache_map_b1.end())
                {//b1中未命中
                    unordered_map<string,struct list_entry *>::iterator got4 = cache_map_b2.find(map_key);
                    if(got4 == cache_map_b2.end())
                    {//都未命中---case4
                        if(t1+b1 == c)
                        {
                            if(t1 < c)
                            {//删除 b1的lru端
                                del_cold(&cache_map_b1,&hot_b1,&cold_b1,&le);
                                if(le != NULL)
                                {
                                    free(le);
                                }
                                replace(p,4);
                            }
                            else
                            {//b1是空的,删除t1的lru端
                                del_cold(&cache_map_t1,&hot_t1,&cold_t1,&le);
                                if(le != NULL)
                                {
                                    free(le);
                                }
                            }
                        }
                        else if( t1 + b1 < c)
                        {//t1+b1 < c
                            if(t1+t2+b1+b2 >= c)
                            {
                                if(t1+t2+b1+b2 == 2*c)
                                {
                                    //删除b2的lru端
                                    del_cold(&cache_map_b2,&hot_b2,&cold_b2,&le);
                                    if(le!=NULL)
                                    {
                                        free(le);
                                    }
                                }
                                replace(p,4);
                            }
                        }

                        struct list_entry *lie = (struct list_entry *)malloc(sizeof(struct list_entry));
                        lie->pre = NULL;
                        lie->next = hot_t1;
                        lie->block_id = key;
                        lie->access_cnt = 1;
                        lie->pre_access = ctx->stat->total_num;
                        strcpy(lie->map_key , map_key);
                        
                        add_hot(&cache_map_t1,&hot_t1,&cold_t1,lie);
                    }
                    else
                    {//缓存未命中，但b2命中---case3
                        le = got4->second;

                        if(b2 >= b1)
                        {
                            del = 1;
                        }
                        else
                        {   
                            del = b1 / b2;
                        }

                        if(p-del > 0)
                        {
                            p -= del;
                        }
                        else
                        {
                            p = 0;
                        }

                        replace(p,3);

                        del_poi(&cache_map_b2,&hot_b2,&cold_b2,le);
                        add_hot(&cache_map_t2,&hot_t2,&cold_t2,le);
                    }
                }
                else
                {//缓存未命中，但b1命中---case2
                    le = got3->second;

                    if(b1 >= b2)
                    {
                        del = 1;
                    }
                    else
                    {
                        del = b2 / b1;
                    }

                    if(p+del < c)
                    {
                        p += del;
                    }
                    else
                    {
                        p = c;
                    }

                    replace(p,2);

                    del_poi(&cache_map_b1,&hot_b1,&cold_b1,le);
                    add_hot(&cache_map_t2,&hot_t2,&cold_t2,le);
                }

                //写ssd
                ctx->stat->write_ssd();
           }
           else
           {//t2命中---case1
                le = got2->second;
               
                del_poi(&cache_map_t2,&hot_t2,&cold_t2,le);
                add_hot(&cache_map_t2,&hot_t2,&cold_t2,le);

                ctx->stat->hit_num++;
                if(ctx->ti->type == 1){
                    //写ssd
                    ctx->stat->write_ssd();
                }
           }
        }
        else
        {//t1命中---case1
            le = got->second;
            del_poi(&cache_map_t1,&hot_t1,&cold_t1,le);
            add_hot(&cache_map_t2,&hot_t2,&cold_t2,le);
            
            ctx->stat->hit_num++;
            if(ctx->ti->type == 1){
                //写ssd
                ctx->stat->write_ssd();
            }
        }
       
//lru链表测试
#if DEBUG
    display();
#endif

    }

    void replace(int pp, int flag)
    {
        struct list_entry *le;
        int tmp = cache_map_t1.size();
        if((tmp != 0)&&((tmp > pp) || (flag == 3 && tmp == p)))
        {
            del_cold(&cache_map_t1,&hot_t1,&cold_t1,&le);
            add_hot(&cache_map_b1,&hot_b1,&cold_b1,le);
        }
        else
        {
            del_cold(&cache_map_t2,&hot_t2,&cold_t2,&le);
            add_hot(&cache_map_b2,&hot_b2,&cold_b2,le);
        }
    }

    void del_cold(unordered_map<string,struct list_entry *> *cache_map, struct list_entry **hot, struct list_entry **cold,struct list_entry **le)
    {
        *le = *cold;
        if(*cold != NULL)
        {
            (*cache_map).erase((*le)->map_key);
            if((*le)->pre != NULL){
                (*le)->pre->next = (*le)->next;
                *cold = (*le)->pre;
                if(*cold != NULL)
                {
                    (*cold)->next = NULL;
                }
            }else{
                *hot = *cold = NULL;
            }
        }
    }

    void del_poi(unordered_map<string,struct list_entry *> *cache_map, struct list_entry **hot, struct list_entry **cold, struct list_entry *le)
    {
        if(le != NULL)
        {
            (*cache_map).erase(le->map_key);
            if(le->pre != NULL){
                le->pre->next = le->next;
            }else{
                *hot = le->next;
            }
            if(le->next != NULL){
                le->next->pre = le->pre;
            }else{
                *cold = le->pre;
            }
            if(*hot != NULL)
            {
                (*hot)->pre = NULL;
            }
            if(*cold != NULL)
            {
                (*cold)->next = NULL;
            }
        }
    }

    void add_hot(unordered_map<string,struct list_entry *> *cache_map, struct list_entry ** hot, struct list_entry ** cold,struct list_entry *le)
    {
        if(le != NULL)
        {
            (*cache_map)[le->map_key] = le;
            le->next = *hot;
            if(*cold == NULL)
            {
                *cold = le;
            }
            
            if(*hot != NULL)
            {
                (*hot)->pre = le;
            }
            *hot = le;
            (*hot)->pre = NULL;
        }
    }

    void display()
    {

        struct list_entry *le1;

        le1 = hot_b2;
        cout << "b2 :" << "\t" ;
        while(le1 != NULL)
        { 
            cout << le1->block_id << "\t";
            le1 = le1->next;
        }
        le1 = hot_t2;
        cout << "t2 :" << "\t" ;
        while(le1 != NULL)
        { 
            cout << le1->block_id << "\t";
            le1 = le1->next;
        }
        le1 = hot_t1;
        cout << "t1 :" << "\t" ;
        while(le1 != NULL)
        { 
            cout << le1->block_id << "\t";
            le1 = le1->next;
        }
        le1 = hot_b1;
        cout << "b1 :" << "\t" ;
        while(le1 != NULL)
        { 
            cout << le1->block_id << "\t";
            le1 = le1->next;
        }
        cout << cache_map_b2.size() <<" "<<cache_map_t2.size() << " "<<cache_map_t1.size()<<" "<<cache_map_b1.size();
        cout << endl;
    }

};