
#include "../include/ThreadCache.hpp"
    //申请内存-字节数
    void* ThreadCache::alloc(size_t size){
        assert(size<=MAX_BYTES);
        size_t alignSize=SizeMap::roundUp(size);//计算对应的那个freeList存储的是多少字节
        size_t index=SizeMap::index(size);//计算下标
        if(_freeLists[index].empty()){
            //ThreadCache为空，向CentralCache申请内存
            return allocFromCentralCache(index,alignSize); 
        }
        else{
            return _freeLists[index].pop();
        }
        
    }
    
    //释放内存
    void ThreadCache::dealloc(void* obj,size_t size){
        THREADCACHE_LOG("dealloc size："<<size);
        //释放内存是交给freeList
        assert(obj);
        assert(size<=MAX_BYTES);
        size_t index=SizeMap::index(size);
        _freeLists[index].push(obj);
        THREADCACHE_LOG("freeList size："<<_freeLists[index].size());

        //检查如果链表的长度太大，就需全部还给centralCache
        if(_freeLists[index].size()==_freeLists[index].maxSize()){
            THREADCACHE_LOG("freeList size："<<_freeLists[index].size());
            THREADCACHE_LOG("freeList maxsize:"<<_freeLists[index].maxSize());
            dellocToCentralCache(_freeLists[index],size);
        }
    }
    //从CenterCache申请内存
    void* ThreadCache::allocFromCentralCache(size_t index,size_t size){
        //计算需要的块数，取小的
        size_t allocNum=std::min(_freeLists[index].maxSize(),SizeMap::numMoveSize(size));
        if(allocNum==_freeLists[index].maxSize()){
            //慢启动策略
            _freeLists[index].maxSize()+=1; //下一次加1
            //_freeLists[index].maxSize()+=allocNum; //下一次加allocNum
           
        }
        
        void* start=nullptr;//标记第一块
        void* end=nullptr;//标记最后一块

        size_t actualNum=CentralCache::getInstance()->getRangeObj(start,end,allocNum,size);
        assert(actualNum>=1);//至少都会申请到一个
      
        if(actualNum==1){
            assert(start==end);
            return start;
        }
        else{
            //把剩下的插入到threadCache的自由链表当中   
            void *startNext = nextObj(start);
            _freeLists[index].pushRange(startNext, end, actualNum - 1);
            return start;
        }
    }
    //释放内存到CeneterCache
    void ThreadCache::dellocToCentralCache(FreeList& list,size_t byteSize){
       void* start=nullptr;
       void* end=nullptr;

       //方案先暂时把全部的释放掉---》TODO
       list.popRange(start,end,list.size());
       THREADCACHE_LOG("释放过后thread中list的size:"<<list.size());
       CentralCache::getInstance()->delListToSpans(start,byteSize);
    }



