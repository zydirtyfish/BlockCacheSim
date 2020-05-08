#include "get_trace_tool.h"

GetTraceTool::GetTraceTool(const char *fn)
{
	strcpy(filename,fn);
	total_cnt = get_rec_cnt();
	fp = fopen(filename,"r");
	ti = new TraceInfo;
	current_cnt = 0;
}

GetTraceTool::~GetTraceTool()
{
	fclose(fp);
	fp = NULL;
	delete ti;
	ti = NULL;
}

uint64_t GetTraceTool::get_rec_cnt()
{
	string tmp;
	uint64_t rst = 0;
	ifstream ifs;
	ifs.open(filename,ios::in);
	while(getline(ifs,tmp))
	{
		rst ++;
	}
	ifs.close();
	return rst;
}

double GetTraceTool::get_process_ratio()
{
	if(current_cnt % 500000 == 0)
	{
		return current_cnt * 100.0 / total_cnt;
	}
	return -1;
}

TraceInfo * GetTraceTool::get_ti()
{
	if(feof(fp))
	{
		LogWrite(1,"%s has been finished !",filename);
		return NULL;
	}

	current_cnt ++;
	fscanf(fp,"%llu,%[^,],%d,%[^,],%llu,%u,%u\n",
				&ti->timestamp,
				&ti->hostname,
				&ti->disknum,
				&ti->io_type,
				&ti->offset,
				&ti->size,
				&ti->responsetime);

	if(strcmp(ti->io_type,"Write") == 0)
		ti->type = 1;
	else
		ti->type = 0;

	memset(ti->map_key,'\0',sizeof(char)*40);
	if(get_process_ratio() != -1)
	{
		LogWrite(1,"%s has been processed %.2lf\%",filename,get_process_ratio());
	}
	return ti;
}
