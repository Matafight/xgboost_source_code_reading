class singleton
{
    private:
        //构造函数私有，只能在类内部创建该对象
        singleton(){};
        static singleton *instance;
    public:
        static singleton * getInstance(){
            if(instance==NULL)
            {
                instance = new singleton();
            }
            return instance;
        }
}
/*
上述代码是线程不安全的，可能会有多个线程同时第一次调用getInstance函数，并同时new 了一个各自的instance，很显然是错误的。
而且，当程序结束时，内存空间没有被释放，加一个析构函数不就行了吗？但是上述代码的析构函数不能按照正常的方式写，因为所有对象共享一个instance*/

//考虑使用一个私有内嵌类专门用来释放空间，最重要的是执行释放的时机，只有程序结束才会释放new的空间，考虑到static变量也是程序结束才会释放，有了下面的变种
class singleton2
{
    private:
        singletion2(){};
        static singleton2 *instance;

        class garb{
            ~garb(){
                if(singleton2::instance)
                {
                    delete singleton2::instance;
                }
                
            }
        }
        static garb gb;
    public:
        static singleton2 * getInstance(){
            if(instance==NULL)
            {
                instance = new singleton2();
            }
            return instance;
        }
}

//使用局部静态变量解决空间释放问题,代码很少很方便
class singleton3
{
    private:
        singleton3(){};
        
    public:
        static singleton3 &getInstance()
        {
            static singleton3 instance;
            return instance;

        }

}

//对于上述代码，singleton3 newint = singleton3::getInstance() 会发生类拷贝，这是因为c++为singleton3类构造了默认的拷贝构造函数，因此需要禁止类拷贝和类赋值
//通过显式声明来解决
class singleton4
{
    private:
        singleton4(){};
        //只声明不定义
        singleton4(const singleton4&);
        singleton4 & operator=(const singleton4&);
    public:
        static singleton4 &getInstance()
        {
            static singleton4 instance;
            return instance;
        }
}


//考虑线程安全的代码
class singleton5
{
    private:
        singleton5(){};
        singleton5(const singleton5 &);
        singleton5& operator=(const singleton5 &);
    public:
        static singleton5& getInstance()
        {
            //lock 和 unlock只是伪代码,在C++0X之后， 静态局部变量的声明已经是线程安全的。
            lock();
            static singleton5 instance;
            unlock();
            return instance;
        }
}

//双检测锁模式，因为我们只希望在第一次实例化的时候才加锁，其他都不用加锁，所以要判断是不是第一次调用实例化instance对象。
class singleton6
{
    private:
        singleton6();
        singleton6(const singleton6 &);
        singleton6 & operator=(const singleton6&);
        static singleton6 *instance;
    public:
        static singleton6* getInstance()
        {
            if(NULL==instance)
            {
                lock();
                if(NULL == instance)
                {
                    instance = new singleton6();
                }
                unlock();
            }
        }
}
参考：
http://www.zkt.name/dan-li-mo-shi-singleton-ji-c-shi-xian/
http://blog.csdn.net/hackbuteer1/article/details/7460019
http://www.cnblogs.com/ccdev/archive/2012/12/19/2825355.html