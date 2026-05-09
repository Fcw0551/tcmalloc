#pragma once 
#include <iostream>
#include <mutex>

//以页为单位的大内存管理span的定义以及spanlist定义
struct Span{
    PAGE_ID _pageID=0;//大块内存起始页的页号
    size_t _n=0;      //页的数量
    
    Span* _next=nullptr;//双向链表的结构
    Span* _prev=nullptr;//

   // size_t _objSize=0;      //切好的小对象的大小
    size_t _useCount=0;     //切好的小块内存，被分配给thread cache的数量
    void* _freeList=nullptr;//切好的小块内存的自由链表

  //  bool _isuse=false;  //是否在被使用
};
class SpanList{
    public:
    SpanList(){
        //初始化哨兵位头节点
        _head=new Span;
        _head->_prev=_head;
        _head->_next=_head;
    }
    //简单实现迭代器,左闭右开[begin,end)
    Span* begin(){
        return _head->_next;
    }
    Span* end(){
        //所以这里返回哨兵位
        return _head;
    }
    bool empty(){
        return _head->_next==_head;
    }
    void insert(Span* pos,Span*newSpan){
        assert(pos);
        assert(newSpan);
        Span* prev=pos->_prev;
        prev->_next=newSpan;
        newSpan->_prev=prev;
        newSpan->_next=pos;
        pos->_prev=newSpan;
    }
    void pushFront(Span*span){
        // span->_next=_head->_next;
        // _head->_next->_prev=span;
        // _head->_next=span;
        // span->_prev=_head;

        //复用insert
        insert(begin(),span);
    }
    void erase(Span*pos){
        assert(pos);
        assert(pos!=_head);
        Span* prev=pos->_perv;
        Span* next=pos->_next;

        prev->_next=next;
        next->_prev=prev;
        //注意erase不需要调用free，因为它释放之后是需要交给pageCache的
    }
    Span* popFront(){ 
        erase(_head->_next);
        return _head->_next; 
    } 
    private:
    Span* _head;
    public:
    std::mutex _SpanMtx;//桶锁

};