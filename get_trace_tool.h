#include "cache-sim.h"

struct trace_inf {
	int disknum;
	u_int32_t size;
	u_int32_t responsetime;
	u_int64_t timestamp;
	u_int64_t offset;
	char hostname[8];
	char io_type[8];
    u_int32_t type;//0表示读请求
};

class get_trace_tool
{
private:
    char filename[200];
    FILE *fp;
    struct trace_inf *ti;
    u_int64_t total_rec;
    u_int64_t curr_rec;

public:
    //get_trace_tool init
    get_trace_tool(const char * fn)
    {
        strcpy(filename,fn);
        total_rec = get_rec_cnt();
        fp = NULL;
        fp = open_file(filename);
        ti = (struct trace_inf *)malloc(sizeof(trace_inf));
        curr_rec = 0;
    }

    ~get_trace_tool()
    {
        close_file(fp);
        fp = NULL;
        free(ti);
        ti = NULL;
    }

    //open a file
    FILE *open_file(char *fn)
    {
        return fopen(fn, "r");
    }

    //close a file
    void close_file(FILE *fp)
    {
        fclose(fp);
    }

    //get next trace_inf
    struct trace_inf *get_ti(bool output)
    {
        if(!feof(fp))
        {
            curr_rec++;
            fscanf(fp, "%llu,%[^,],%d,%[^,],%llu,%lu,%lu\n", &ti->timestamp, &ti->hostname, &ti->disknum, &ti->io_type, &ti->offset, &ti->size, &ti->responsetime);
            //cout << ti->timestamp << " " << ti->hostname << " " << ti->disknum << " " << ti->io_type << " " <<ti->offset << " " <<ti->size << " "<<ti->responsetime << endl;
            if(strcmp(ti->io_type,"Write")==0)
                ti->type = 1;
            else
                ti->type = 0;
            if(output)
            {
                get_progress_ratio();
            }
            return ti;
        }
        else
        {
            return NULL;
        }
    }

    u_int64_t get_rec_cnt()
    {
        string tmp;
        u_int64_t n = 0;
        ifstream ifs;
        ifs.open(filename,ios::in); // ios::in 表示以只读方式读取文件
        while(getline(ifs,tmp))
        {
            n++;
        }
        ifs.close();
        return n;
    }

    void get_progress_ratio()
    {
        if(curr_rec % 10000 == 0)
        {
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            cout << "progress ratio:" << (u_int64_t)(curr_rec * 100.0) / total_rec << "%";
        }
    }

    u_int64_t get_total_rec()
    {
        return total_rec;
    }
    
};