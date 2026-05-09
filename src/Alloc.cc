
#include "../include/Alloc.hpp"


void* Alloc(size_t size){
    if(size>MAX_BYTES){
        PAGECACHE_LOG("Alloc size"<<size <<" is too large");
        
        size_t alignSize=SizeMap::roundUp(size);//向上对齐
        PAGECACHE_LOG("alignSize"<<alignSize);
        size_t kpage=alignSize>>PAGE_SHIFT;//计算页数
        PAGECACHE_LOG("kpage"<<kpage);
        //加锁
        PageCache::getInstance()->_pageMtx.lock();
        Span* span=PageCache::getInstance()->newSpan(kpage);
        PageCache::getInstance()->_pageMtx.unlock();

        void*start=(void*)(span->_pageID<<PAGE_SHIFT);
        return start;   
    
    }
    else{
        //通过线程缓存管理，TLS获取线程缓存
        PAGECACHE_LOG("Alloc size"<<size <<" is smallor than MAX_BYTES");

        ThreadCache& tc=ThreadCacheManager::getInstance();
        return tc.alloc(size);
    }
}

void Dealloc(void* obj){

    //对于释放
    //小页(0,64]-->threadcache
    //中页(65,]-->pagecache(由pagecache来做决策是不是交给os还是)
    Span* span=PageCache::getInstance()->mapObjectToSpan(obj);
    PAGECACHE_LOG("Dealloc span"<<span);
    size_t size=span->_objSize;
    PAGECACHE_LOG("Dealloc size"<<size);
    if(size>MAX_BYTES){
        PAGECACHE_LOG("Dealloc size"<<size <<" is too large");
        PageCache::getInstance()->_pageMtx.lock();
        //根据地址计算出对应的span，然后交给函数release释放掉
        span->_isuse=false;
        PageCache::getInstance()->ReleaseSpanToPageCache(span);
        PageCache::getInstance()->_pageMtx.unlock();
    }
    else{
        //通过线程缓存管理，TLS获取线程缓存
        ThreadCache& tc=ThreadCacheManager::getInstance();
        tc.dealloc(obj,size);
    }
}