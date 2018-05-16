# BlockCacheSim

### Download & Compile

* Download via the following command.
```
git clone https://github.com/zydirtyfish/BlockCacheSim.git
```

* Enter the BlockCacheSim directory, and use command ```make``` to compile.

* Use command ```make run``` to execute the sample program.

### To download the trace file

* The sample program uses the trace file ```example.csv``` under the directory ```trace```. You can get more trace file from [UMassTraceRepository](http://traces.cs.umass.edu/index.php/Storage/Storage) or [SNIA Website](http://iotta.snia.org/tracetypes/3). You can match the trace file format by adjusting the trace_type in the configuration file.

### Configuration file
* All the configuration information of BlockCacheSim is in the file ```config```, it uses equals sign to connnect the config attributes and their values. 
* Comments begin with #
```
#algorithm type
algorithm_type=0
#cache size
block_num_conf=65536
#block size
block_size_conf=4096
#write strategy
write_algorithm_conf=1

#the name of the log file
log_prefix=./trace/example.csv
```

The program entry is in the main function of ```cache-sim.cpp```. It can be found that the configuration file is passed in through the command line parameters. That is we should using the following command to execute the program.
```
./cache-sim.o config
```

### To statistic the basic information of traces
```
./cache-sim.o config 100
```

### The basic structure of BlockCacheSim

- The initialization funtion ```init_cache()```.
- The destruction function ```destroy_cache()```.
- The function ```main()```  is the entrance to the program which calls the method ```exec()``` of the instance of class ```Run```.
- The method ```exec()``` calls the method ```kernel()``` of the instance of class ```Algorithm```. ```kernel()``` is the core method of BlockCacheSim.
- Different cache replacement algorithm classes inherit the parent class ```Algorithm``` and implemente the virtual function ```map_operation()``` of the parent class. The function ```kernel()```  implements different replacement strategies by calling different ```map_operation()``` functions. 
- This is similar to the implementation of [flashcache](https://github.com/facebookarchive/flashcache) (an open source block level cache for flash cache). The duty of function ```map_operation()``` is similar to the duty of flashcache, and the function ```kernel()``` is similar to the device-mapper layer.

<!--### The results
![image](http://onx1obrfu.bkt.clouddn.com/joystorage/blogs/缓存模拟器-cache-sim1.jpg)-->

### Sample output
```
<---------------------Statistical Reuslts--------------------->
File Name:./trace/example.csv
throughput	14.00GB
unique data	2.18GB
re-access data	11.82GB
read ratio	76.17
----------reuse_dis_cdf----------
1KB	2KB	4KB	8KB	16KB	32KB	64KB	128KB	256KB	512KB	1MB	2MB	4MB	8MB	16MB	32MB	64MB	128MB	256MB	512MB	1GB	2GB	4GB	
0.00	0.01	0.02	0.05	0.08	0.09	0.10	0.12	0.15	0.18	0.19	0.20	0.21	0.51	0.81	0.83	0.85	0.87	0.89	0.91	0.93	0.97	1.00	
----------frequency_cdf----------
1	2	4	8	16	32	64	128	256	512	1024	2048	4096	8192	16384	32768	
0.00	0.83	0.91	0.96	0.98	0.98	0.98	0.99	1.00	1.00	1.00	1.00	1.00	1.00	1.00	1.00	

--------------------------------------------------------------------------------
cache_size: 10000blocks	write_policy: write through
--------------------------------------------------------------------------------
name		hit_ratio		ssd_write
--------------------------------------------------------------------------------
LRU		18.69		3601641
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
cache_size: 10000blocks	write_policy: write through
--------------------------------------------------------------------------------
name		hit_ratio		ssd_write
--------------------------------------------------------------------------------
OPT		63.73		1990351
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
cache_size: 10000blocks	write_policy: write through
--------------------------------------------------------------------------------
name		hit_ratio		ssd_write
--------------------------------------------------------------------------------
ARC		32.85		3080631
--------------------------------------------------------------------------------
```
<!--![image](http://onx1obrfu.bkt.clouddn.com/joystorage/blogs/缓存模拟器-cache-sim2.jpg)-->

### To add algorithms
- First create a replacement algorithm header file ```x.h``` according to the ```lru.h``` template, and then modify the logic of the function ```map_operation()```.
- After modifying the header file ```x.h```, add the line ```#define X n``` to the location of the cache algorithm configuration area in the header file ```cache-sim.h```. ```X``` is the name of the algorithm and ```n``` is a positive integer, but it cannot be repeated with the previously defined integer.
- Finally modify the header file ```run.h```. Firstly it needs to import the header file ```#include "x.h"```. Then add an ```X``` object (```__X * x```) as a private variable for the class ```Run```, and then modify the initialization part of function ```RUN()```. Lastly modified the cache operation part of class ```RUN``` (```exec()```).

### Update
BlockCacheSim will also implement more algorithms proposed recently, so stay tuned!

### About us
<!--Yu Zhang-->

### Statement
Non-commercial reprint please indicate the author and source. Commercial reprint please contact the author himself.

<!-- 非商业转载请注明作者及出处。商业转载请联系作者本人-->
