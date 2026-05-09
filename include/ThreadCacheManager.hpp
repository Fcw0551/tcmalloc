#pragma once
#include "Common.hpp"
#include "ThreadCache.hpp"
#include <thread>
//这是一个TLS管理

class ThreadCacheManager{
    //实现一个ThreadCache实例的工具类，而非ThreadCacheManager的单例模式
    //内部是如何获取TLS的逻辑，后续可以进行修改，模块化
    public:
    static ThreadCache& getInstance();
    private:
    ThreadCacheManager();
    ThreadCacheManager& operator=(const ThreadCacheManager&)=delete;
    ThreadCacheManager(const ThreadCacheManager&) =delete;
   
};