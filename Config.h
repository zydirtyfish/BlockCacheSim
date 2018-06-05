//////////////////////////////////////////////////////////////////////////
// Config.h
// Author: zydirtyfish
// Date: 2017-07

#ifndef Config_h__
#define Config_h__
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>

using namespace std;

#define LINE_LENGTH 500

class Config {
public:
	Config()
	{

	}
	~Config()
	{

	}

	void Get(const char *key_path, const char *para_name,char *result)
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

};
#endif // Config_h__
