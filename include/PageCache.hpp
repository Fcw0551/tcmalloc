#pragma once 
//#include "TCMalloc_PageMap.hpp"
#include "./Common.hpp"
#include "./FreeListAndSpanList.hpp"
#include <unordered_map>    
#include "./MemoryPool.hpp"
#include "./RadixTree.hpp"

//不使用桶锁，因为进行span合并的时候如果使用桶锁，会导致大量的桶加锁解锁，性能下降
//并且基本不会到第三级缓存，使用统一的锁把这一层锁住反而效率会更高
class PageCache{
    public:
    //单例模式

    static PageCache* getInstance();


    //释放空闲的span回到pagecache，并且合并相邻的span
    void ReleaseSpanToPageCache(Span*span);
    

    //获取一个k页的span
    Span* newSpan(size_t k);

    //根据地址获取对象对应的span
    Span* mapObjectToSpan(void* obj);

    //锁实现线程安全
    std::mutex _pageMtx;

    private:
    //页号和span的映射关系
    //unordered_map<PAGE_ID,Span*> _idSpanMap;
    RadixTree<64-PAGE_SHIFT> _idSpanMap;

    private:
    static PageCache _sInst;
    private:
    SpanList _spanList[NPAGES]; //有多少个桶
      
    MemoryPool<Span> _spanPool;  //定长内存池申请span结点


    //实现单例模式
    PageCache(){}
    PageCache(const PageCache&)=delete;
    PageCache& operator=(const PageCache&)=delete;

};
