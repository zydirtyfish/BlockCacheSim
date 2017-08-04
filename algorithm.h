#include "cache-sim.h"

class Algorithm
{
public: //缓存基本信息
    unordered_map<string,struct list_entry *> cache_map; //缓存索引

public: //缓存基本操作

    virtual void map_operation(u_int64_t key, cache_c *ctx){};
    
     //分块
    struct bio * div_block(struct cache_c *ctx)
    {
        struct bio *head = NULL,*tail=NULL;
        u_int64_t begin = ctx->ti->offset / (ctx->block_size_conf) ; 
        u_int64_t end = (ctx->ti->offset + ctx->ti->size ) / (ctx->block_size_conf) ; 

        if((ctx->ti->offset + ctx->ti->size ) % ctx->block_size_conf == 0 && (ctx->ti->offset + ctx->ti->size ) >= ctx->block_size_conf)
        {
            end--;
        }
        
        for(u_int64_t i = begin ; i <= end ; i++ )
        {
            struct bio *tmp = (struct bio *)malloc(sizeof(bio));
            tmp->block_id = i;
            tmp->size = 1;
            tmp->next = NULL;
    
            if(head == NULL)
            {
                head = tmp;
                tail = tmp;
            }
            else
            {
                tail->next = tmp;
                tail = tmp;
            }
            tmp = NULL;
        }
        return head;
    }

    void kernel(cache_c *ctx)
    {
        struct bio *head = div_block(ctx);
        if(1 == ctx->ti->type){//如果是写操作
            while(head != NULL)
            {
                ctx->stat->total_num++;
                write(head->block_id,ctx);
                head = head->next;
            } 
        }else{//如果是读操作
            while(head != NULL)
            {
                ctx->stat->read_num++;
                ctx->stat->total_num++;

                read(head->block_id,ctx);
                head = head->next;
            }
        }
    }

    void read(u_int64_t block_id,struct cache_c *ctx)
    {
        map_operation(block_id,ctx);
    }

    void write(u_int64_t block_id,struct cache_c *ctx)
    {
        switch(ctx->write_algorithm_conf)
        {
            case 0: //write through
                break;
            case 1: //write back
                map_operation(block_id, ctx);
                break;
            case 2: //write around
                break;
        }
        
    }
    

    void get_map_key(char *map_key,char *hostname,int disknum,u_int64_t block_id)
    {
        //构造map_key
        char of[25];
        strcpy(map_key,hostname);
        strcat(map_key,",");

        char diskNumber[5];
        memset(diskNumber,'\0',sizeof(diskNumber));
        sprintf(diskNumber,"%d",disknum);
        strcat(map_key,diskNumber);
        strcat(map_key,",");

        sprintf(of,"%llu",block_id);
        strcat(map_key,of);

        //cout << map_key << endl;
    }

};