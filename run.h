#include "get_trace_tool.h"
#include "algorithm.h"
#include "adv_stat.h"
#include "lru.h"
#include "fifo.h"
#include "arc.h"
#include "larc.h"
#include "opt.h"
#include "srac.h"

class RUN
{

private:
	get_trace_tool *gtt;
	char name[10];

	__Astat *astat;
	__LRU *lru;
	__FIFO *fifo;
	__ARC *arc;
	__LARC *larc;
	__OPT *opt;
	__SRAC *srac;

public:
	RUN(cache_c *ctx)
	{
		switch(ctx->algorithm_type)
		{//初始化算法类
			case LRU:
				strcpy(name,"LRU");
				lru = new __LRU();
				break;
			case FIFO:
				strcpy(name,"FIFO");
				fifo = new __FIFO();
				break;
			case ARC:
				strcpy(name,"ARC");
				arc = new __ARC(ctx);
				break;
			case LARC:
				strcpy(name,"LARC");
				larc = new __LARC(ctx);
				break;
			case SRAC:
				strcpy(name,"SRAC");
				srac = new __SRAC(ctx);
				break;
			case ASTAT:
				strcpy(name,"STAT");
				astat = new __Astat();
				break;
			case OPT:
				strcpy(name,"OPT");
				opt = new __OPT();
				cout << "----------------opt pre-progress begin----------------" << endl;
				int mapsize = 0;
				char FN[500];
				memset(FN,'\0',sizeof(FN));
				strcpy(FN,ctx->log_prefix);
				my_strpro(FN);
				
				cout << "processing file named : " << FN << endl;
				get_trace_tool *gtt_opt = new get_trace_tool(FN);
				ctx->ti = gtt_opt->get_ti(true);
				while(ctx->ti != NULL)
				{
					mapsize = opt->init_io_list(ctx);
					//读取下一行trace文件
					ctx->ti = gtt_opt->get_ti(true);
				}
				delete gtt_opt;
				cout << endl;
				cout << "mapsize " << mapsize << endl;
				cout << "----------------opt pre-process end----------------" << endl;
				break;
		}
		strcpy(ctx->cache_name,name);
	}

	~RUN()
	{
	}

	void exec(cache_c *ctx)
	{
		cout << "----------------process begin-----------" << endl;
		char FN[500];
		strcpy(FN,ctx->log_prefix);
		my_strpro(FN);
		cout << "processing file named : " << FN <<endl;
		gtt = new get_trace_tool(FN);
		ctx->ti = gtt->get_ti(true);

		while(ctx->ti != NULL)
		{
			switch(ctx->algorithm_type)
			{//缓存操作
				case LRU:
					lru->kernel(ctx);
					break;
				case FIFO:
					fifo->kernel(ctx);
					break;
				case ARC:
					arc->kernel(ctx);
					break;
				case LARC:
					larc->kernel(ctx);
					break;
				case SRAC:
					srac->kernel(ctx);
					break;
				case OPT:
					opt->kernel(ctx);
					break;
				case ASTAT:
					astat->kernel(ctx);
					break;
			}

			ctx->ti = gtt->get_ti(true);
		}
		delete gtt;
		cout << endl;
		cout << "----------------process end-----------" << endl;
	}

	void show_result(cache_c *ctx)
	{
		if(ctx->algorithm_type == ASTAT)
		{//如果是数据统计
			show_stat(ctx);
			return;
		}
		//显示结果
		cout << "\n--------------------------------------------------------------------------------" << endl;
		cout << "cache_size: "<<ctx->block_num_conf<<"blocks"<<"\t"<<"write_policy: write through"<< endl;
		cout << "--------------------------------------------------------------------------------" << endl;
		cout << "name" <<"\t\t" << "read_ratio" <<"\t\t" << "hit_ratio" << "\t\t"<< "ssd_write"<<endl;
		cout << "--------------------------------------------------------------------------------" << endl;
		cout << ctx->cache_name << "\t\t" << ctx->stat->get_read_ratio() << "%" << "\t\t" << ctx->stat->get_hit_ratio() << "%\t\t" << ctx->stat->get_ssd_write() << endl;
		cout << "--------------------------------------------------------------------------------" << endl;
	}

	char* my_strpro(char *dst)
	{
		char *tmp = dst;
		while(*tmp != '\r' && *tmp != '\0')
			tmp++;
		*tmp = '\0';
		return (dst);
	}

	void show_stat(cache_c *ctx)
	{	
		char name_tmp[500] = "./";
		strcat(name_tmp,ctx->log_prefix);
		cout << name_tmp << endl;
		my_strpro(name_tmp);
		cout << name_tmp << endl;
		FILE *fp = fopen(name_tmp,"a+");
		astat->stat_end(ctx);/*最后处理结果*/
		for(int i = 0; i < 40 ;i++)
        {
            fprintf(fp,"%.2lf\t",ctx->stat->reuse_dis_cdf[i]);
        }
        fprintf(fp,"\n");
        fclose(fp);
	}
};