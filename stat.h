#ifndef _STAT_H
#define _STAT_H
struct Stat
{
	uint64_t read_cnt;
	uint64_t total_cnt;
	uint64_t memory_hit;
	uint64_t ssd_hit;
	uint64_t memory_write;
	uint64_t ssd_write;
};
#endif
