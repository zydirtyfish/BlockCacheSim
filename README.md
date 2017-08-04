# cache-sim_v1.0

### 下载与编译
通过下列命令下载

```
git clone https://github.com/zydirtyfish/cache-sim_v1.0.git
```

进入目录，make进行编译

```
make
```

输入下列命令可运行demo程序

```
make run
```

### trace下载
demo程序中分析的是trace文件夹的下的example文件，更多的trace文件可在网站[SNIA网站](http://iotta.snia.org/tracetypes/3)上下载。

### 配置文件
缓存所有的配置信息在config文件中，配置文件属性与值之间以等号连接，注释以#开头
```
#缓存替换算法的类型
algorithm_type=0
#缓存的大小
block_num_conf=65536
#块大小
block_size_conf=4096
#写策略
write_algorithm_conf=1
#日志的开始
log_start=0
#每次读取的日志数
log_num=1
#日志文件的前缀
log_prefix=./trace/example.csv

#懒惰参数
PARA=4
k=16
```

程序入口在cache-sim.cpp中的main函数，可以发现，配置文件通过命令行参数传入，即通过下列命令运行
```
./cache-sim.o config
```
### 缓存的初始化

```
init_cache()//函数读取配置文件并对缓存进行初始化
destroy_cache()//销毁缓存
```


### 运行缓存
main()函数通过调用Run对象的exec()方法运行缓存,exec()函数调用了Algorithm类中的kernel()方法。因此kernel()是整个cache-sim的核心方法。

不同的缓存替换算法都继承了Algorithm这个算法父类，并且实现了父类中的虚函数map_operation()，kernel()函数通过调用不同的map_operation()实现不同的替换策略，这一点类似于flashcache的实现，flashcache实现的就是map_operation()所实现的功能，而kernel()则类似于device-mapper层提供的转发功能。

### 关于我们
张煜

华中科技大学
武汉光电国家实验室
计算机系统结构
2016级研究生

### 更新
cache-sim还将会实现更多的高级统计项，敬请期待！
