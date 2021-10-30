## 基于libevent实现的线程池以及高并发Ftp服务器
* 本项目基于之前的test_thread_pool,将线程池应用到Ftp服务器里，实现高并发。整套代码在linux和windows环境下均可运行，并部署到阿里云服务器测试。
* 大量应用了C++11的新特性，面向对象风格。
* 实现了"USER","PWD","STOR","PORT","LIST","RETR"等命令。
* 使用的filezilla作为ftp客户端测试，可以正常上传下载文件，并访问远程目录。
* 并发压力测试（待续）...


## 食用方法
* **依赖库：libevent2.1.8**

***
**使用linux**
* 我的linux用的ubuntu20.04,需要开放20、21端口，运行以下命令：
```
git clone https://github.com/BigCJL/libevent-Ftp_Server
sudo ufw allow 20
sudo ufw allow 21
sudo ufw reload
make
```
***
**使用window**
* 本地ide里添加工程，把文件添加到工程里直接运行即可。
* vs里会因为使用fopen报warning,需要在预编译头里添加忽略warning的宏。
***
看到thread_poll_init_success的提示后，使用ftp客户端连接即可，我使用的filezilla做测试，可以正常上传下载文件。
***
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
* 采用单例化的工厂模式，在一个Factory类中对FTP服务命令进行注册到一个map中：
* 	std::map<std::string, XFtpTask*>calls   //map的结构
* map的第一个元素是服务命令的字符串("USER","STOR"等)，第二个元素是XFtpTask对象的指针，这个对象通过重载XTask类里的write、read、event来实现不同的命令行为。
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
  *    cmdbev主要在派生类XFtpServerCMD中用到，服务器子线程收到消息时，在已经注册的calls字典里查找有无type类型，若查找到了，则调用相应对象中的方法解析(parse)请求（多态的应用），并在服务端完成响应。
  *   Parse方法，是虚函数，不同的请求头对应了不同的解析方法，因此需要各任务派生类重写。
  *   ResCMD方法，若calls里没有注册该请求，则回复一个OK 200
  *  Read、Write、Event和SetCallback相当于封装了libevent里的setcb函数，设置为虚函数，让不同功能的子类只需要继承父类，再实现不同的回调函数即可。
  *  ConnectPORT()方法，这里将PORT对象中获得的端口号和ip进行连接，建立数据传输通道，数据传输同样使用tcp/ip协议，在调用该方法时新建一个临时的bev,并设置回调,在RETR命令时，先调用ConnectorPort()方法建立连接,再在parse时就直接激发（trigger）写入事件，bev和fd用完即回收。(频繁传输时效率问题？？)
    
	
  
#### 2021.10.13更新
### ResCMD（string s）
* 用于回复客户端信息，基于libevent的bufferevent_write进行了封装，并作为父类XFtpTask的成员函数,使得功能子类调用更方便。

### XFtpPORT类
* 工厂模式下的功能对象，解析客户端的port，数据格式为PORT 127,0,0,1,70,96\r\n，加密方程为port = n5*256 + n6，格式错误的返回码为501？
* 解析获得的port和ip,不做连接和使用，在父类XFtpTask中增加port和ip成员存放，在其他任务需要用到时就可以使用了。
	
### XFtpLIST类
* 工厂模式下的功能对象，用于获取服务器的当前目录list，返回码257？
* 完成了"LIST"获取当前目录、“PWD”获取当前路径、"CWD"切换目录、"CDUP"回到上级目录等功能。
* 



#### 2021.10.16更新
### XFtpRETR类
对应"RETR"命令，客户端向服务器请求文件，从数据通道下载文件。

### XFtpUSER类
对应"USER"命令，服务器获取用户名。

### XFtpPORT类
对应"PORT"命令，获取服务端的端口号。

	
### XFtpSTOR类
对应"STOR"命令，客户端上传文件。

