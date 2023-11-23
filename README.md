# 常用工具类
1. arena：levelDB中用于分配跳表节点内存的一个简易内存管理器
2. skiplist：levelDB中跳表数据结构实现，做了简化处理以及接口增加
3. noncopyable: 不需要拷贝的类直接继承该类，实现编译期检查
4. CountDownLatch: 线程同步方法，用于一个线程等其他n线程执行到某一步后再继续执行
5. bloom: 布隆过滤器