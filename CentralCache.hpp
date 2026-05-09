//CentralCache的设计
//锁竞争？（只有桶锁，全部争一个桶的时候才需要锁，所以大概率是很少有竞争情况）
//依旧按照thread cache的管理进行管理内存
#pragma once 
#include "Common.hpp"
#include "PageCache.hpp"
class CentralCache{
    public:
    //单例模式

    static CentralCache*getInstance(){
        return &_sInst;
    }
    //获取一个非空的span
    Span* getOneSpan(SpanList&list,size_t byteSize){
        //对于span是多页的，所以要明确一页一页的

        //1.在spanlist中寻找一个非空的span
        
        Span* it=list.begin();
        while(it!=list.end()){
            if(it->_freeList!=nullptr){
                //如果span当中的自由链表不为空则返回
                return it;
            }
            else{
                //如果自由链表为nullptr，则寻找下一个结点span
                it=it->_next;
            }
        }
        //先释放桶锁，再加大锁
        //因为此时算找不到了span了，但是可能会从ThreadCache释放到CentralCache
        list._spanMtx.unlock();

        PageCache::getInstance()->_pageMtx.lock();

        //2.如果找到最后没有非空的span，则需要向PageCache申请
        Span* span=PageCache::getInstance()->newSpan(SizeMap::numMovePage(byteSize));
        
        assert(span);
        span->_isuse=true;//标记span被使用
        span->_objSize=byteSize;//标记切成的每块小的内存块大小，后续释放的时候可以通过span结构体进行查找
        PageCache::getInstance()->_pageMtx.unlock(); //解大锁

        //此时不需要着急加桶锁，因为申请的这块span只有当前线程有地址
        //计算span的内存的起始地址和大小
        // 计算span的大块内存的起始地址和大块内存的大小（字节数）
        char *start = (char *)(span->_pageID << PAGE_SHIFT);
        size_t bytes = span->_n << PAGE_SHIFT;

        // 把大块内存切成size大小的对象链接起来
        char *end = start + bytes;
        // 先切一块下来去做尾，方便尾插
        span->_freeList = start;
        start += byteSize;
        void *tail = span->_freeList;
        // 尾插
        while (start < end)
        {
            nextObj(tail) = start;
            tail = nextObj(tail);
            start += byteSize;
        }
        nextObj(tail) = nullptr; // 尾的指向置空

        // 将切好的span头插到spanList
        list._spanMtx.lock(); // span切分完毕后，需要挂到桶里时再重新加桶锁
        list.pushFront(span);

        return span;
    }
    //从中心缓存获取一定数量的对象给thread cache
    size_t getRangeObj(void*& start,void*& end,size_t batchNum,size_t size){
        size_t index=SizeMap::index(size);//计算下标
        
        //注意此时需要加锁，因为你访问了临界资源
        _spanList[index]._spanMtx.lock();//加锁

        Span* span=getOneSpan(_spanList[index],size);


        assert(span);//span不为空
        assert(span->_freeList);//span中的自由链表不为空

        //从span中获取batchNum个块
        // size_t actualNum=1;

        // if(span->_freeList.size()>=batchNum){
        //     start=span->_freeList;
        //     end=span->_freeList;
        //     while(nextObj(end)&&batchNum-1){
        //         end=nextObj(end);
        //         actualNum++;
        //         batchNum--;
        //     }
        // }
        // else{
        //     //如果不够呢？向下一个span拿
        //     //如果没有下一个span，则向page申请
        // }

        //这里都是申请一个，所以我们至少返回一个就行
        //threadCache可能申请多个内存块，至少返回一个给线程用，多余的才挂在freeList中
        //有多少拿多少，上限是batchNum，但是不够至少返回一个也行
        start = span->_freeList;
        end = span->_freeList;
        size_t actualNum = 1;
        while (nextObj(end) && batchNum - 1)
        {
            end = nextObj(end);
            actualNum++;
            batchNum--;
        }

        //调整span
        span->_freeList=nextObj(end);
        nextObj(end)=nullptr;//取出来的最后一块内存块不要连接后面
        span->_useCount+=actualNum;//被拿走多少块freelist小内存
        CENTRALCACHE_LOG("从span中获取对象,实际获取数量为:"<<actualNum);
        CENTRALCACHE_LOG("从span中获取对象,_useCount为："<<span->_useCount);
        _spanList[index]._spanMtx.unlock();//解锁
        return actualNum;

    }
    //将一定的数量的对象释放到span跨度
    void delListToSpans(void* start,size_t byteSize){
        //这里的span内部的内存块顺序没关系，span掌管连续的内存空间即可
        
        //根据byteSize计算index
        size_t index = SizeMap::index(byteSize);
        _spanList[index]._spanMtx.lock(); // 加锁
        std::vector<Span*> spansToRelease;
        while (start)
        {
            void *next = nextObj(start);
            Span *span = PageCache::getInstance()->mapObjectToSpan(start);
            assert(span);
            nextObj(start) = span->_freeList;
            span->_freeList = start;
            span->_useCount--;
            CENTRALCACHE_LOG("释放对象到span,完成_useCount--后为:"<<span->_useCount);
            if (span->_useCount == 0)
            {
                _spanList[index].erase(span); // 移出链表
                span->_freeList = nullptr;
                span->_next = nullptr;
                span->_prev = nullptr;

               
                spansToRelease.push_back(span);
            }

            start = next;
        }

        // 所有对象处理完毕后，统一释放 span 给 PageCache
        for (auto &s : spansToRelease)
        {
            PageCache::getInstance()->_pageMtx.lock();
            PageCache::getInstance()->ReleaseSpanToPageCache(s);
            PageCache::getInstance()->_pageMtx.unlock();
        }

        _spanList[index]._spanMtx.unlock(); // 最后再解锁
    }
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
CentralCache CentralCache::_sInst;//类外进行初始化实现单例模式
