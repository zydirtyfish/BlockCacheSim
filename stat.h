#ifndef STAT__
#define STAT__
class Stat
{
public:
    u_int64_t total_num;
    u_int64_t hit_num;
    u_int64_t read_num;
    u_int64_t ssd_write;

    //ssd_write 0.8ms;ssd_read;0.2ms;SATA_write 6ms;SATA_read 14ms

    /*高级统计项*/
    u_int64_t throughput;/*流量统计*/
    u_int64_t uni_data;/*唯一数据量*/
    u_int64_t re_access_data;/*再次访问的数据*/
    u_int64_t freq[20];/*频次分布*/
    double freq_cdf[20];/*累计分布*/
    u_int64_t reuse_dis[40];/*重用距离分布*/
    double reuse_dis_cdf[40];/*累计分布*/

    double latency;


    Stat()
    {
        latency = total_num = hit_num = read_num =ssd_write = 0;
        throughput = uni_data = re_access_data = 0;
        for(int i = 0; i< 20 ; i++)
        {
        	freq[i] = freq_cdf[i] = reuse_dis[i] = reuse_dis_cdf[i] = 0;
        }
        for(int i= 20 ;i < 40;i++)
        {
			reuse_dis_cdf[i] = reuse_dis[i] = 0;
        }
    }

    ~Stat()
    {
        
    }

    u_int64_t get_total_num()
    {
        return total_num;
    }
    
    double get_hit_ratio()
    {
        return hit_num * 100.0 / total_num;
    }

    double get_read_ratio()
    {
        return read_num * 100.0 / total_num;
    }

    u_int64_t get_ssd_write()
    {
        return ssd_write;
    }

    void write_ssd()
    {
        ssd_write++;
    }

    void cache_hit()
    {
        hit_num++;
    }

    void add_latency(int type)
    {
        switch(type)
        {//ssd_write 0.8ms;ssd_read;0.2ms;SATA_write 6ms;SATA_read 14ms
            case 1://读ssd
                latency+=0.2;
                break;
            case 2://写ssd
                latency+=0.8;
                break;
            case 3://读磁盘
                latency+=14;
                break;
            case 4://写磁盘
                latency+=6;
                break;
        }
    }

    double get_latency()
    {
        return latency;
    }
};
#endif//STAT__