#pragma once
#include "Common.hpp"
#include "ThreadCache.hpp"
#include <thread>
//这是一个TLS管理

class ThreadCacheManager{
    //实现一个ThreadCache实例的工具类，而非ThreadCacheManager的单例模式
    //内部是如何获取TLS的逻辑，后续可以进行修改，模块化
    public:
    static ThreadCache& getInstance(){
        //thread_local只会被初始化一次

        thread_local ThreadCache* tlsCachePtr=nullptr;//这是TLS指针

        //延迟初始化
        if(tlsCachePtr==nullptr){
            static std::mutex mtx;
            static MemoryPool<ThreadCache> tcPool;
            mtx.lock();
            tlsCachePtr=tcPool.New();//创建ThreadCache，绑定到tls中，私有
            mtx.unlock();
        }
        return *tlsCachePtr;
    }
    private:
    ThreadCacheManager();
    ThreadCacheManager& operator=(const ThreadCacheManager&)=delete;
    ThreadCacheManager(const ThreadCacheManager&) =delete;
   
};