//CentralCache的设计
//锁竞争？（只有桶锁，全部争一个桶的时候才需要锁，所以大概率是很少有竞争情况）
//依旧按照thread cache的管理进行管理内存
#pragma once 
#include "./Common.hpp"
#include "./PageCache.hpp"
class CentralCache{
    public:
    //单例模式

    static CentralCache*getInstance();
    //获取一个非空的span
    Span* getOneSpan(SpanList&list,size_t byteSize);
    //从中心缓存获取一定数量的对象给thread cache
    size_t getRangeObj(void*& start,void*& end,size_t batchNum,size_t size);
    //将一定的数量的对象释放到span跨度
    void delListToSpans(void* start,size_t byteSize);
    private:
    //成员变量
    SpanList _spanList[NFREELISTS];
    static CentralCache _sInst;
    private:
    //禁掉构造函数避免new等构建对象
    CentralCache(){}
    //禁掉拷贝构造函数
    CentralCache(const CentralCache& cc)=delete;
    CentralCache& operator=(const CentralCache&cc)=delete;
};

