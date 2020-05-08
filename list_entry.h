#ifndef _LIST_ENTRY_H
#define _LIST_ENTRY_H
#include "common_types.h"

struct ListEntry
{
    uint64_t access_cnt; /*access_cnt*/
    uint64_t next_access; //距离下一次访问的间隔，用于opt算法
    uint64_t next_time; /*next access time*/
    uint64_t pre_access; /*previous access time*/
	uint64_t avg_pre;
    uint64_t offset;/*logic block id*/
	uint32_t disknum;
    ListEntry *next;
    ListEntry *pre;
    int dirty; /*dirty or not*/
	char map_key[40];
};
#endif //_LIST_ENTRY_H
