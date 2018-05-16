#include <list>
#include <math.h>

struct stat_map_entry
{
    u_int64_t last_access;
    u_int64_t access_times;
};

class __Astat : public Algorithm
{

private://缓存基本信息

    unordered_map<string,struct stat_map_entry *> io_map; //统计map

public: //缓存基本操作

    //初始化
    __Astat()
    {
    }

    ~__Astat()
    {
        for(unordered_map<string,struct stat_map_entry *>::iterator it=io_map.begin();it != io_map.end();++it)
        {
            struct stat_map_entry *s = it->second;
            free(s);
            s = NULL;
        }
    }

    void map_operation(u_int64_t key, cache_c *ctx,char *map_key)
    {
        unordered_map<string,struct stat_map_entry *>::iterator it = io_map.find(map_key);
        if(it != io_map.end())
        {
            struct stat_map_entry *s = it->second;
            s->access_times++;

            /*构建重用距离分布*/
            int index_tmp = log(ctx->stat->total_num - s->last_access) / log(2) + 1;
            if((ctx->stat->total_num - s->last_access)%2 == 0)
                index_tmp--;
            ctx->stat->reuse_dis[index_tmp]++;

            s->last_access = ctx->stat->total_num;
        }
        else
        {
            struct stat_map_entry* sme = (struct stat_map_entry *)malloc(sizeof(struct stat_map_entry));
            sme->access_times = 1;
            sme->last_access = ctx->stat->total_num;
            io_map[map_key] = sme;
        }
        
       
    }

    void stat_end(cache_c *ctx)
    {
	u_int64_t two = 0;
        for(unordered_map<string,struct stat_map_entry *>::iterator it=io_map.begin();it != io_map.end();++it)
        {
            ctx->stat->uni_data++;
            struct stat_map_entry *s = it->second;
            ctx->stat->throughput += s->access_times;
            ctx->stat->re_access_data += s->access_times - 1;
	    if(s->access_times <= 5&&s->access_times > 1)
	    {
		two++;	
	    }
            /*构建频次分布*/
            int index_tmp = log(s->access_times) / log(2) + 1;
            if(s->access_times % 2 == 0)
                index_tmp--;
            ctx->stat->freq[index_tmp]++;
        }

        /*构建频次累计分布*/
        double sum_tmp = 0;
        for(int i = 0; i < 40 ;i++)
        {
            sum_tmp += ctx->stat->freq[i];
            ctx->stat->freq_cdf[i] = sum_tmp;
        }
        for(int i = 0; i < 40 ;i++)
        {
            ctx->stat->freq_cdf[i] = ctx->stat->freq_cdf[i] *1.0 / sum_tmp;
        }

        /*构建重用距离累计分布*/
        sum_tmp = 0;
        for(int i = 0; i < 40 ;i++)
        {
            sum_tmp += ctx->stat->reuse_dis[i];
            ctx->stat->reuse_dis_cdf[i] = sum_tmp;
        }
        for(int i = 0; i < 40 ;i++)
        {
            ctx->stat->reuse_dis_cdf[i] = ctx->stat->reuse_dis_cdf[i] *1.0 / sum_tmp;
        }
	cout << two*100.0/io_map.size() << endl;
    }
 
};
