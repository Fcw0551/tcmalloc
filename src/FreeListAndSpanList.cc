
#include "../include/FreeListAndSpanList.hpp"


MemoryPool<Span> SpanList::_spanPool;
//自由链表
    void FreeList::push(void* obj){
        //头插链表，表示释放空间
        assert(obj);
        nextObj(obj)=_freeList;
        _freeList=obj;
        _size++;
    }
    void FreeList::pushRange(void*start,void*end,size_t n){
        //头插一批

        assert(start);
        assert(end);
        //防止传进来一个导致错误
        if(start==end){
            push(start);
            return;
        }
        nextObj(end)=_freeList;
        _freeList=start;
        _size+=n;
    }
    void* FreeList::pop(){
        // 头删链表
        assert(_freeList);
        void *obj = _freeList;
        _freeList = nextObj(_freeList);
        _size--;
        return obj;
    }
    void FreeList::popRange(void*&start,void*&end,size_t n){
        //头删一批
        assert(n);
        assert(n<=_size);
        start=_freeList;
        end=start;
        for(size_t i=1;i<n;i++){
            end=nextObj(end);
        }
       
        _freeList=nextObj(end);
        nextObj(end)=nullptr;
        _size-=n;
        return ;
          
    }
    bool FreeList::empty(){
        return _freeList==nullptr;
    }
    size_t& FreeList::size(){
        return _size;
    }
    size_t& FreeList::maxSize(){
       
        return _maxSize;
    }
   




// 管理对齐和映射关系
//  整体控制在最多10%左右的内碎片浪费
//  [1,128] 8byte对齐       freelist[0,16)
//  [128+1,1024] 16byte对齐   freelist[16,72)
//  [1024+1,8*1024] 128byte对齐   freelist[72,128)
//  [8*1024+1,64*1024] 1024byte对齐     freelist[128,184)
//  [64*1024+1,256*1024] 8*1024byte对齐   freelist[184,208)

  

    // thread cache从central cache申请多少个内存块
    //运用慢启动：小对象多拿，大对象少拿，因为central cache是一个全局共享，需要加锁
    //小对象如int等类型申请的多
    size_t SizeMap::numMoveSize(size_t size){
        //策略：小内存块可以申请多个，大内存块申请少个
        if (size == 0) return 0;
    
        //计算需要多少个块
        int num = MAX_BYTES / size;
        if (num < 2)
            num = 2;
    
        //限制最多能够拿512个内存块
        if (num > 512)
            num = 512;
        return num;
    }

    //计算一次向系统/pageCache获取几个页
    size_t SizeMap::numMovePage(size_t size){
        size_t num=numMoveSize(size);
        size_t npage=num*size;//总字节数
        npage>>=PAGE_SHIFT;//算出需要多少页
        if(npage==0){
            npage=1;//不足一页算一页
        }
        return npage;
    }



    SpanList::SpanList(){
        //初始化哨兵位头节点
        _head=_spanPool.New();
        _head->_prev=_head;
        _head->_next=_head;
    }
    void SpanList::print(){
        std::cout<<"spanList开始打印:"<<std::endl;
        for(auto it=begin();it!=end();it=it->_next){
            std::cout<<"span:"<<it<<std::endl;
            std::cout<<"_n:"<<it->_n<<std::endl;
            std::cout<<"_pageID:"<<it->_pageID<<std::endl;
        }
    }
    //简单实现迭代器,左闭右开[begin,end)
    Span* SpanList::begin(){
        return _head->_next;
    }
    Span* SpanList::end(){
        //所以这里返回哨兵位
        return _head;
    }
    bool SpanList::empty(){
        return _head->_next==_head;
    }
    void SpanList::insert(Span* pos,Span*newSpan){
        assert(pos);
        assert(newSpan);
        Span* prev=pos->_prev;
        prev->_next=newSpan;
        newSpan->_prev=prev;
        newSpan->_next=pos;
        pos->_prev=newSpan;
    }
    void SpanList::pushFront(Span*span){
        // span->_next=_head->_next;
        // _head->_next->_prev=span;
        // _head->_next=span;
        // span->_prev=_head;

        //复用insert
        insert(begin(),span);
    }
    void SpanList::erase(Span*pos){
        //注意erase不需要调用free，因为它释放之后是需要交给pageCache的
        //这里的erase仅仅是改变链表结构

        assert(pos);
        assert(pos!=_head);
        Span* prev=pos->_prev;
        Span* next=pos->_next;

        prev->_next=next;
        next->_prev=prev;
        pos->_prev=nullptr;
        pos->_next=nullptr;
    }
    Span* SpanList::popFront()
    {
        if(empty())
        {
            return nullptr;
        }

        Span *front = _head->_next;
        erase(front);
        return front;
    }

