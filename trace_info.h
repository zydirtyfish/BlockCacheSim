#ifndef _TRACE_INFO_H
#define _TRACE_INFO_H

#include "common_types.h"

typedef struct _TraceInfo
{
	int disknum;
	uint32_t size;
	uint32_t responsetime;
	uint64_t timestamp;
	uint64_t offset;
	char hostname[8];
	char io_type[8];
	uint32_t type;
	char map_key[40];
} TraceInfo;
#endif
