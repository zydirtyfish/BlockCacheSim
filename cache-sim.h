#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unordered_map>

#include <fstream>

#include <time.h>
#include <typeinfo>

using namespace std;

//my_head
#include "stat.h"

#define SCSI_SN_LEN 16
#define MAX_HOUR 48
#define DEBUG 0

//---缓存算法配置区---
#define FIFO 0
#define LRU 1
#define OPT 2
#define ARC 3
#define LARC 4
#define SRAC 5


#ifndef LIST_ENTRY__
#define LIST_ENTRY__
struct list_entry
{
    u_int64_t access_cnt;  //共访问了多少次
    u_int64_t next_access; //距离下一次访问的间隔，用于opt算法
    u_int64_t next_time; //下次访问的时间
    u_int64_t pre_access; //上一次访问的时间
    u_int64_t lbn;  //对应于缓存中的逻辑地址 是0-block_num_conf之间的整数

    u_int64_t block_id;//逻辑块号
    u_int64_t avg_pre; //平均重用距离
    struct list_entry *next;
    struct list_entry *pre;
    int io_type;
    int io_size; // 原请求大小
    int dirty; //是否为脏数据
    char map_key[40];
};
#endif //LIST_ENTRY__

#ifndef CACHE_C__
#define CACHE_C__
struct cache_c
{//缓存运行时的上下文context
    int algorithm_type; /*缓存替换算法类型*/
    int block_size_conf; /*块大小*/
    int write_algorithm_conf; /*写策略*/
    int log_start;
    int log_num;
    Stat *stat; /*统计信息*/
    struct trace_inf *ti; /*当前的IO任务ti*/
    struct list_entry *cache_blk; /*缓存空间*/
    struct list_entry *lru; /*lru端*/
    struct list_entry *mru; /*mru端*/

    int PARA;
    int K;

    u_int64_t block_num_conf;/*缓存大小*/
    char log_prefix[500];/*日志文件的前缀*/
    FILE *outfile[10];/*输出文件*/
    char cache_name[10];
};
#endif//CACHE_C__

#ifndef BIO__
#define BIO__
struct bio
{
    u_int64_t block_id; //块号
    int size;   //请求的大小；单位是块
    struct bio *next;
};
#endif//BIO__

#ifndef STACK_ENTRY__
#define STACK_ENTRY__
struct stack_entry
{
    int index;
    struct stack_entry *next;
};
#endif//STACK_ENTRY__