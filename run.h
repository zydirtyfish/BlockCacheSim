#include "get_trace_tool.h"
#include "algorithm.h"
#include "adv_stat.h"
#include "lru.h"
#include "fifo.h"
#include "arc.h"
#include "larc.h"
#include "opt.h"
#include "lea.h"
#include "mru.h"
#include <math.h>

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
	__LEA *lea;
	__MRU *mru;

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
			case ASTAT:
				strcpy(name,"STAT");
				astat = new __Astat();
				break;
			case LEA:
				strcpy(name,"LEA");
				lea = new __LEA();
				break;
			case MRU:
				strcpy(name,"MRU");
				mru = new __MRU();
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
				ctx->ti = gtt_opt->get_ti(true,ctx);
				while(ctx->ti != NULL)
				{
					mapsize = opt->init_io_list(ctx);
					//读取下一行trace文件
					ctx->ti = gtt_opt->get_ti(true,ctx);
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
		ctx->ti = gtt->get_ti(true,ctx);

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
				case OPT:
					opt->kernel(ctx);
					break;
				case ASTAT:
					astat->kernel(ctx);
					break;
				case LEA:
					lea->kernel(ctx);
					break;
				case MRU:
					mru->kernel(ctx);
					break;
			}

			ctx->ti = gtt->get_ti(true,ctx);
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
		}
		else
		{//否则统计命中率
			show_hit(ctx);
		}
	}

	char* my_strpro(char *dst)
	{
		char *tmp = dst;
		while(*tmp != '\r' && *tmp != '\0')
			tmp++;
		*tmp = '\0';
		return (dst);
	}

	void show_hit(cache_c *ctx)
	{
		//显示结果
		cout << "\n--------------------------------------------------------------------------------" << endl;
		cout << "cache_size: "<<ctx->block_num_conf<<"blocks"<<"\t"<<"write_policy: write through"<< endl;
		cout << "--------------------------------------------------------------------------------" << endl;
		cout << "name" <<"\t\t" << "read_ratio" <<"\t\t" << "hit_ratio" << "\t\t"<< "ssd_write"<<endl;
		cout << "--------------------------------------------------------------------------------" << endl;
		cout << ctx->cache_name << "\t\t" << ctx->stat->get_read_ratio() << "%" << "\t\t" << ctx->stat->get_hit_ratio() << "%\t\t" << ctx->stat->get_ssd_write() << endl;
		cout << "--------------------------------------------------------------------------------" << endl;

		char name_tmp[500] = "";
		strcpy(name_tmp,ctx->out_prefix);
		my_strpro(name_tmp);

		char split_tmp[50] = "";
		strcpy(split_tmp,ctx->log_prefix);
  		const char *sep = "/"; //可按多个字符来分割
  		char *p;
  		char p_tmp[50];
		p = strtok(split_tmp, sep);
		while(p){
			p = strtok(NULL, sep);
			if(p!=NULL)
				strcpy(p_tmp,p);
		}

		strcat(name_tmp,p_tmp);
		my_strpro(name_tmp);


		if(ctx->algorithm_type == LEA)
		{
			FILE *fp = fopen(name_tmp,"a+");
			fprintf(fp,"\n--------------------------------------------------------------------------------\n");
			fprintf(fp,"cache_size: %llublocks\twrite_policy: write through\n",ctx->block_num_conf);
			fprintf(fp,"[Lazy Parameters]\tPARA:%d\tK:%.2lf\n",ctx->PARA,ctx->K);
			fprintf(fp,"--------------------------------------------------------------------------------\n");
			fprintf(fp,"name\t\thit_ratio\t\tssd_write\n");
			fprintf(fp,"--------------------------------------------------------------------------------\n");
			fprintf(fp,"%s\t\t%.2lf%\t\t%llu\n",ctx->cache_name,ctx->stat->get_hit_ratio(),ctx->stat->get_ssd_write());
			fprintf(fp,"--------------------------------------------------------------------------------\n");
			fclose(fp);
		}
		else
		{
			FILE *fp = fopen(name_tmp,"a+");
			fprintf(fp,"\n--------------------------------------------------------------------------------\n");
			fprintf(fp,"cache_size: %llublocks\twrite_policy: write through\n",ctx->block_num_conf);
			fprintf(fp,"--------------------------------------------------------------------------------\n");
			fprintf(fp,"name\t\thit_ratio\t\tssd_write\n");
			fprintf(fp,"--------------------------------------------------------------------------------\n");
			fprintf(fp,"%s\t\t%.2lf%\t\t%llu\n",ctx->cache_name,ctx->stat->get_hit_ratio(),ctx->stat->get_ssd_write());
			fprintf(fp,"--------------------------------------------------------------------------------\n");
			fclose(fp);
		}
	}

	void show_stat(cache_c *ctx)
	{	
		char name_tmp[500] = "";
		strcpy(name_tmp,ctx->out_prefix);
		my_strpro(name_tmp);

		char split_tmp[50] = "";
		strcpy(split_tmp,ctx->log_prefix);
  		const char *sep = "/"; //可按多个字符来分割
  		char *p;
  		char p_tmp[50];
		p = strtok(split_tmp, sep);
		while(p){
			p = strtok(NULL, sep);
			if(p!=NULL)
				strcpy(p_tmp,p);
		}

		strcat(name_tmp,p_tmp);
		my_strpro(name_tmp);

		FILE *fp = fopen(name_tmp,"a+");
		astat->stat_end(ctx);/*最后处理结果*/

		fprintf(fp,"<---------------------Statistical Reuslts--------------------->\n");
		fprintf(fp,"File Name:%s\n",ctx->log_prefix);
		fprintf(fp,"throughput\t%.2lfGB\n",ctx->stat->throughput*4.0 / (1024*1024));
		fprintf(fp,"unique data\t%.2lfGB\n",ctx->stat->uni_data*4.0 / (1024*1024));
		fprintf(fp,"re-access data\t%.2lfGB\n",ctx->stat->re_access_data*4.0 / (1024*1024));
		fprintf(fp,"read ratio\t%.2lf%\n",ctx->stat->get_read_ratio());
		char unit[4][4]={"KB","MB","GB","TB"};
		fprintf(fp,"----------reuse_dis_cdf----------\n");
		int tmpp = log(ctx->block_size_conf / 1024) / log(2);
		for(int i = 0; i < 40 ;i++)
		{
		    fprintf(fp,"%.0lf%s\t",pow(2,(i+tmpp)%10),unit[(i+tmpp)/10]);
		    if(ctx->stat->reuse_dis_cdf[i]==1)
			break;
		}
		fprintf(fp,"\n");
		for(int i = 0; i < 40 ;i++)
		{
		    fprintf(fp,"%.2lf\t",ctx->stat->reuse_dis_cdf[i]);
		    if(ctx->stat->reuse_dis_cdf[i]==1)
			break;
		}
		fprintf(fp,"\n");

		fprintf(fp,"----------frequency_cdf----------\n");
			for(int i = 0; i < 40 ;i++)
		{
		    fprintf(fp,"%.0lf\t",pow(2,i));
		    if(ctx->stat->freq_cdf[i]==1)
			break;
		}
		fprintf(fp,"\n");
			for(int i = 0; i < 40 ;i++)
		{
		    fprintf(fp,"%.2lf\t",ctx->stat->freq_cdf[i]);
		    if(ctx->stat->freq_cdf[i]==1)
			break;
		}
		fprintf(fp,"\n");
		fclose(fp);
	}
};
