#ifndef _HISTORY_ENTRY_H
#define _HISTORY_ENTRY_H
struct HistoryEntry
{
	HistoryEntry *pre;
	HistoryEntry *next;
	uint64_t access_cnt;
	uint64_t pre_access;
	char map_key[40];
};
#endif
