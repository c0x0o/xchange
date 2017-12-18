# xchange

> This project is still under developing and further testing is required. So you should NOT use it in any of your business project.

## introduction

xchange is a C++ infrastructure library including(current master branch):

1. noncopyable base class
2. EventEmitter base class, a event publish/subscribe model, i love Javascript
3. Singleton template
4. LockFreeQueue
5. Mutex
6. Condition
7. Thread
8. Thread Pool based on lock-free structure

This library is designed for such purposes:

1. easy to use
2. easy to read
3. easy to understand

and also, other features will be added(including Network interface, IO, etc)

xchange是一个我自己的c++基础设施，其中的一些思想和思路参考了Boost和muduo。当前已经开发完成的功能有：

1. noncopyable基类
2. Singleton单例模板（参考Boost的实现）
3. EventEmitter基类，一个事件发布/订阅模型，JS天下无敌
4. 无锁队列
5. 互斥量
6. 条件量
7. 线程的基本封装，接口设计参考了C++11的Thread
8. 基于无锁结构的线程池，实现中不包含互斥量和条件量

总之就是要实现一个使用方便、代码可读性强、易于理解的C++基础设施，后面还会添加网络操作和IO操作的特性，敬请期待。目前还没有来得及写详细的文档，有兴趣的同学可以看看头文件和test目录下的小例子。
