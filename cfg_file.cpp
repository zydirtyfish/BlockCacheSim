#include "cfg_file.h"

CfgFile _cfg;

CfgFile::CfgFile()
{
}

CfgFile::~CfgFile()
{

}

void CfgFile::init_cfg(const char *cfg_name)
{
	char tmp[LINE_LENGTH];
	char tmp_num[10];
	int cnt = 0 , idx = 0;
	get(cfg_name,"block_size_conf",tmp);
	_block_size_conf = atoi(tmp);

	get(cfg_name,"io_trace",_io_trace);

	get(cfg_name,"para",tmp);
	_para = atof(tmp);

	get(cfg_name,"k",tmp);
	_k = atof(tmp);

	get(cfg_name,"algorithm_list",tmp);
	while(tmp[cnt] != '\0' && tmp[cnt] != '\n')
	{
		if(tmp[cnt] == ',')
		{
			tmp_num[idx] = '\0';
			_algorithm_list.push_back(atoi(tmp_num));
			idx = 0;
		}
		else
		{
			tmp_num[idx++] = tmp[cnt];
		}
		cnt ++;
	}
	tmp_num[idx] = '\0';
	_algorithm_list.push_back(atoi(tmp_num));
	cnt = idx = 0;
	/*
	for(int i = 0 ; i < _algorithm_list.size() ; i++)
	{
		printf("%d\t",_algorithm_list[i]);
	}
	printf("\n");
	*/
	
	get(cfg_name,"cache_size_list",tmp);
	while(tmp[cnt] != '\0' && tmp[cnt] != '\n')
	{
		if(tmp[cnt] == ',')
		{
			tmp_num[idx] = '\0';
			_cache_size_list.push_back(atoi(tmp_num));
			idx = 0;
		}
		else
		{
			tmp_num[idx++] = tmp[cnt];
		}
		cnt ++;
	}
	tmp_num[idx] = '\0';
	_cache_size_list.push_back(atoi(tmp_num));
	idx = 0;
	
	/*
	for(int i = 0 ; i < _cache_size_list.size() ; i++)
	{
		printf("%d\t",_cache_size_list[i]);
	}
	printf("\n");
	*/
}

void CfgFile::get(const char *key_path, const char *para_name,char *result)
{
	ifstream fin(key_path); 
	if(fin)
    {
		char str[LINE_LENGTH];
		while( fin.getline(str,LINE_LENGTH) )
		{
			if(str[0] == '#' || str[0] == '\0')//注释以#开头
				continue;
			int cnt = 0;
			char tmp[2][LINE_LENGTH];
			//以等号分割配置文件
			const char *sep = "="; //可按多个字符来分割
			char *p;
			p = strtok(str, sep);
			strcpy(tmp[cnt],p);
			while(p){
				p = strtok(NULL, sep);
				cnt++;
				if(p)
					strcpy(tmp[cnt],p);
				else
					break;
			}
			if(cnt == 2)
			{
				if(strcmp(tmp[0],para_name) == 0)
					strcpy(result,tmp[1]);
			}
		}
		fin.close();
	}
	else{
		cout << "can't open config file named '" << key_path << "'" <<endl;
	}
}
