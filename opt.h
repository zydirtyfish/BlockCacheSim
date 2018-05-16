#include <list>
#include <map>
#include <math.h>

struct next_acc
{
    u_int64_t num;
    struct next_acc *next;
};

struct map_entry
{
    int acc_num;
    struct next_acc *head;
    struct next_acc *tail;
};

class __OPT : public Algorithm
{   //opt时间最优算法
    //且处理时间为log(n)级别
    //得到opt绝对最优解，内存空间充足情况下使用

private://缓存基本信息

    unordered_map<string,struct map_entry *> io_map; //预处理map
    map<u_int64_t,struct stack_entry *> opt_stack; //opt_stack

    u_int64_t pre_cnt;
    u_int64_t pre_read;
    u_int64_t total_rec;

public: //缓存基本操作

    //初始化
    __OPT()
    {
        pre_cnt = 0;
        pre_read = 0;
        total_rec = 1.8446744*1e19;
    }

    ~__OPT()
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

                int victim = find_victim();

                if(victim == -1)
                    return;

                del_from_opt_stack(cache_entry[victim].next_access , victim);

                cache_map.erase(cache_entry[victim].map_key);
                
                cache_entry[victim].block_id = key;
                cache_entry[victim].access_cnt = 1;
                cache_entry[victim].pre_access = ctx->stat->total_num;
                cache_entry[victim].next_access = find_next(map_key);
                cache_entry[victim].avg_pre = 0;
                cache_entry[victim].io_type = ctx->ti->type;
                cache_entry[victim].io_size = ctx->ti->size;
                strcpy(cache_entry[victim].map_key , map_key);

                cache_map[map_key] = &cache_entry[victim];

                add_into_opt_stack(cache_entry[victim].next_access , victim);

                //写ssd
                ctx->stat->write_ssd();
            }
            else
            {
                cache_entry[map_size].block_id = key;
                cache_entry[map_size].access_cnt = 1;
                cache_entry[map_size].pre_access = ctx->stat->total_num;
                cache_entry[map_size].avg_pre = 0;
                cache_entry[map_size].next_access = find_next(map_key);
                cache_entry[map_size].lbn = map_size;
                cache_entry[map_size].io_type = ctx->ti->type;
                cache_entry[map_size].io_size = ctx->ti->size;
                strcpy(cache_entry[map_size].map_key , map_key);
                
                cache_map[map_key] = &cache_entry[map_size];

                add_into_opt_stack(cache_entry[map_size].next_access , map_size);

                //写ssd
                ctx->stat->write_ssd();
            }
        }
        else
        {//命中
            struct list_entry *  le = got->second;

            del_from_opt_stack(le->next_access , le->lbn);

            le->avg_pre = ((u_int64_t)((le->avg_pre)*(le->access_cnt)) + (u_int64_t)(ctx->stat->total_num - le->pre_access))/(le->access_cnt + 1);
            le->access_cnt++;
            le->pre_access = ctx->stat->total_num;
            le->next_access = find_next(map_key);
            le->io_type = ctx->ti->type;
            le->io_size = ctx->ti->size;

            add_into_opt_stack(le->next_access , le->lbn);

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

    int find_victim()
    {
        return find_opt_victim();
    }

    void add_into_opt_stack(u_int64_t next , int index)
    {
         map<u_int64_t,struct stack_entry *>::iterator it = opt_stack.find(next);
         if(it != opt_stack.end())
         {
            struct stack_entry * tmp1 = it->second;
            struct stack_entry * tmp2 = it->second;
            while(tmp1 != NULL)
            {
                tmp2 = tmp1;
                tmp1 = tmp2->next;
            }
            struct stack_entry * tmp = (struct stack_entry *)malloc(sizeof(stack_entry));
            tmp->index = index;
            tmp->next = NULL;
            //opt_stack[next] = tmp;

            tmp2->next = tmp;
         }
         else
         {
             struct stack_entry * tmp = (struct stack_entry *)malloc(sizeof(stack_entry));
             tmp->index = index;
             tmp->next = NULL;
             opt_stack[next] = tmp;
         }
    }

    void del_from_opt_stack(u_int64_t next , int index)
    {
        map<u_int64_t,struct stack_entry *>::iterator it = opt_stack.find(next);
        if(it != opt_stack.end())
        {
            struct stack_entry * tmp1 = it->second;
            struct stack_entry * tmp2 = it->second;

            if(tmp1->index == index)
            {
                if(tmp1->next == NULL)
                {
                    opt_stack.erase(it);
                }
                else
                {
                    it->second = tmp1->next;
                    free(tmp1);
                }
                return;
            }

            while(tmp1 != NULL)
            {
                if(tmp1->index == index)
                {
                    tmp2->next = tmp1->next;
                    free(tmp1);
                    return;
                }
                tmp2 = tmp1;
                tmp1 = tmp2->next;
            }
        }
    }

    int find_opt_victim()
    {
        map<u_int64_t,struct stack_entry*>::reverse_iterator it = opt_stack.rbegin();
        struct stack_entry * tmp = it->second;
        int result = -1;
        result = tmp->index;
        if(tmp->next == NULL)
        {
            opt_stack.erase(it->first);
            free(tmp);
        }
        else
        {
            it->second = tmp->next;
            free(tmp);
        }
        return result;
    }

    u_int64_t find_next(char *map_key)
    {   
        unordered_map<string,struct map_entry*>::iterator it = io_map.find(map_key);
        struct next_acc *nt = NULL;
        struct map_entry *me = NULL;
        if(it != io_map.end())
        {
            u_int64_t result = 0;
            me = it->second;
            if(me->head == NULL)
            {
                result = total_rec + 1;
            }
            else{
                nt = me->head;
                me->head = nt->next;
                delete nt;
                if(me->head != NULL)
                {
                    result = me->head->num;
                }
                else
                {
                    result = total_rec + 1;
                }
            }

            return result;
        }
        else
        {
            cout << "Severe Error!!!" << endl;
            exit(0);
        }
    }

    //初始化io_list
    int init_io_list(cache_c *ctx)
    {
        struct bio *head = div_block(ctx);
        while(head != NULL)
        {
            char map_key[40];
            get_map_key(map_key,ctx->ti->hostname,ctx->ti->disknum,head->block_id);
            pre_cnt++;
            if(ctx->ti->type == 0)
                pre_read++;
            unordered_map<string,struct map_entry*>::iterator it = io_map.find(map_key);
            struct next_acc *nt = NULL;
            struct map_entry *me = NULL;

            if(it == io_map.end())
            {
                me = (struct map_entry*)malloc(sizeof(struct map_entry));
                nt = (struct next_acc *)malloc(sizeof(struct next_acc));
                nt->num = pre_cnt;
                nt->next = NULL;
                me->head = me->tail = nt;
                me->acc_num = 1;
                io_map[map_key] = me;
            }else{
                nt = (struct next_acc *)malloc(sizeof(struct next_acc));
                nt->num = pre_cnt;
                nt->next = NULL;
                me = it->second;
                me->tail->next = nt;
                me->tail = nt;
                me->acc_num++;
            }

            head = head->next;
        } 
        return io_map.size();
    }

    /*show fre*/
    void show_io_list(){
    }

    u_int64_t get_pre_cnt()
    {
        return pre_cnt;
    }

};