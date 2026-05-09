#pragma once 
#include <iostream>
#include <assert.h>
#include "Common.hpp"
#include "CentralCache.hpp"

class ThreadCache{
    public:
    //申请内存-字节数
    void* alloc(size_t size);
    
    //释放内存
    void dealloc(void* obj,size_t size);
    //从CenterCache申请内存
    void* allocFromCentralCache(size_t index,size_t size);
    //释放内存到CeneterCache
    void dellocToCentralCache(FreeList& list,size_t byteSize);
    
    private:
    FreeList _freeLists[NFREELISTS];//哈希桶：桶的元素是一个
};

