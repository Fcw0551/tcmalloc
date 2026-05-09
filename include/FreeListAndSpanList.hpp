
#include <mutex>
#include <iostream>
#include <assert.h>
#include "./Common.hpp"
#include "./MemoryPool.hpp"
//自由链表
class FreeList{
    public:
    void push(void* obj);
    void pushRange(void*start,void*end,size_t n);
    void* pop();
    void popRange(void*&start,void*&end,size_t n);
    bool empty();
    size_t& size();
    size_t& maxSize();
    private:
    void* _freeList=nullptr;
    size_t _maxSize=1;//freeList最多能够缓存多少个空闲对象
    size_t _size=0;
};

// 管理对齐和映射关系
//  整体控制在最多10%左右的内碎片浪费
//  [1,128] 8byte对齐       freelist[0,16)
//  [128+1,1024] 16byte对齐   freelist[16,72)
//  [1024+1,8*1024] 128byte对齐   freelist[72,128)
//  [8*1024+1,64*1024] 1024byte对齐     freelist[128,184)
//  [64*1024+1,256*1024] 8*1024byte对齐   freelist[184,208)
class SizeMap{
    public:
      static inline size_t _roundUp(size_t bytes,size_t align){
        //根据字节数和对齐粒度算出对齐数
        //计算大于等于align的最小的align倍数
        //(align-1):align-1让高位全是0，低位全是1
        //按位取反，那高位全是1，低位全是0，相当于保留只能被align整除的部分
        //按位与就能够保留高位整除align的部分，省略掉低位
        PAGECACHE_LOG("对齐数："<<align<<"字节数："<<bytes);
        return (((bytes)+align-1)&~(align-1));
    }
    

    //256KB如果按照4kb一页来算最大能够申请64页的内存空间
    //对于64——128的直接向pageCache申请
    //>128的向os申请
    static inline size_t roundUp(size_t bytes){
        //对齐大小计算
        if(bytes<=128){
            //8对齐数
            return _roundUp(bytes,8);
        }
        else if(bytes<=1024){
            return _roundUp(bytes,16);
        }
        else if(bytes<=8*1024){
            return _roundUp(bytes,128);
        }
        else if(bytes<=64*1024){
            return _roundUp(bytes,1024);
        }
        else if(bytes<=256*1024){
            return _roundUp(bytes,8*1024);
        }
        else{
            //大于256KB的，4kb的对齐数
            CENTRALCACHE_LOG("256KB以上申请内存，请使用os申请,申请的字节数为："<<bytes);
            return _roundUp(bytes,1<<PAGE_SHIFT);
        }
        return -1;
    }
    
    static inline size_t _index(size_t bytes,size_t align_shift){
        //计算该bytes应该映射到目前级别下的第几个链表
        //1<<align_shift  1<<3=8  对齐8字节
        //向上取整
        return ((bytes+(1<<align_shift)-1)>>align_shift)-1;
    }
    //计算映射到哪个自由链表桶
    static inline size_t index(size_t bytes){
        assert(bytes<=MAX_BYTES);//必须小于256KB

        //每个区间有多少个链
        static int group_array[4]={16,56,56,56};
        if(bytes<=128){
            return _index(bytes,3);
        }
        else if(bytes<=1024){
            return _index(bytes-128,4)+group_array[0];
        }
        else if(bytes<=8*1024){
            return _index(bytes-1024,7)+group_array[0]+group_array[1];
        }
        else if(bytes<=64*1024){
            return _index(bytes-8*1024,10)+group_array[0]+group_array[1]+group_array[2];
        }
        else if(bytes<=256*1024){
            return _index(bytes-64*1024,13)+group_array[0]+group_array[1]+group_array[2]+group_array[3];
        }
        else{
           assert(false);//不会有这一步因为上一个assert已经判断了
        }
        return -1;
    }

    // thread cache从central cache申请多少个内存块
    //运用慢启动：小对象多拿，大对象少拿，因为central cache是一个全局共享，需要加锁
    //小对象如int等类型申请的多
    static size_t numMoveSize(size_t size);

    //计算一次向系统/pageCache获取几个页
    static size_t numMovePage(size_t size);
};


//以页为单位的大内存管理span的定义以及spanlist定义
struct Span{
    PAGE_ID _pageID=0;//大块内存起始页的页号
    size_t _n=0;      //页的数量
    
    Span* _next=nullptr;//双向链表的结构
    Span* _prev=nullptr;//

    size_t _objSize=0;      //切好的小对象的大小
    size_t _useCount=0;     //切好的小块内存，被分配给thread cache的数量
    void* _freeList=nullptr;//切好的小块内存的自由链表

    bool _isuse=false;  //是否在被使用(是否是刚刚central cache从page cache获取的，避免被合并)
};
class SpanList{
    public:
    SpanList();
    void print();
    //简单实现迭代器,左闭右开[begin,end)
    Span* begin();
    Span* end();
    bool empty();
    void insert(Span* pos,Span*newSpan);
    void pushFront(Span*span);
    void erase(Span*pos);
    Span* popFront();

    private:
    Span* _head;
    static MemoryPool<Span> _spanPool;
    public:
    std::mutex _spanMtx;//桶锁

};
