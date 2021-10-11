## 基于libevent实现的线程池以及高并发Ftp服务器
* 本项目基于之前的test_thread_pool,将线程池应用到Ftp服务器里，实现高并发。整套代码在linux和windows环境下均可运行，并部署到阿里云服务器测试。
* 大量应用了C++11的新特性，面向对象风格。
  
### 线程池部分
...



#### 2021.10.11更新
### XTask类
Xtask类是所有任务都依赖的基类，定义了成员	:
* struct event_base* base = 0;
* int sock;
* int thread_id = 0;
* virtual bool Init() = 0;


*	当收到连接时，管道（socketpair）发送激活信息，由一个已经初始化好的线程处理任务，sock即接收连接的sock，base即该线程里的用于监听所有事件的eventbase，thread_id即该线程的线程id。
* 定义了bool类型的Init纯虚函数，要求所有派生类实现自己的初始化方法，每个派生类初始化的方法对应着其不同的业务需求，将多态大量地应用到项目里。

### XFtpFactory类
* 工厂类，采用单例模式。每收到一个连接，就调用CreateTask()方法，创建一个ftp消息处理对象，在该方法里注册有不同的请求头，对应不同的处理函数。
* 完成了USER,LIST...等请求的响应，分别对应不同的对象来进行实现，每个对象用父类XFtpTask的指针调用parse解析请求和完成处理。


### XFtpTask类
* 继承了XTask类，定义了成员:
* public:
* struct bufferevent* cmdbev = 0;
* virtual void Parse(std::string type, std::string msg){}
	//回复cmd消息
* void ResCMD(std::string msg);
* virtual void Read(struct bufferevent *bev){}
* virtual void Write(struct bufferevent* bev) {}
* virtual void Event(struct bufferevent* bev, short what) {}
* void SetCallback(struct bufferevent* bev);
* bool Init() { return true; }
* protected:
* static void ReadCB(bufferevent* bev, void* arg);
* static void WriteCB(bufferevent* bev, void* arg);
* static void EventCB(struct bufferevent* bev, short what, void* arg);
    cmdbev主要在派生类XFtpServerCMD中用到，服务器子线程收到消息时，在已经注册的calls字典里查找有无type类型，若查找到了，则调用相应对象中的方法解析(parse)请求（多态的应用），并在服务端完成响应。
    Parse方法，是虚函数，不同的请求头对应了不同的解析方法，因此需要各任务派生类重写。
    ResCMD方法，若calls里没有注册该请求，则回复一个OK 200
    Read、Write、Event和SetCallback相当于封装了libevent里的setcb函数，设置为虚函数，让不同功能的子类只需要继承父类，再实现不同的回调函数即可。
    
	
  

