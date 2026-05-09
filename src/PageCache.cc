#include "../include/PageCache.hpp"
//不使用桶锁，因为进行span合并的时候如果使用桶锁，会导致大量的桶加锁解锁，性能下降
//并且基本不会到第三级缓存，使用统一的锁把这一层锁住反而效率会更高
//单例模式
PageCache PageCache::_sInst;
    PageCache* PageCache::getInstance(){
        return &_sInst;
    }


    //释放空闲的span回到pagecache，并且合并相邻的span
    void PageCache::ReleaseSpanToPageCache(Span*span){

        //再合并前先判断span的页数
        PAGE_ID npage=span->_n;
        PAGE_ID pageId=span->_pageID;
        if(npage>=NPAGES){
            //如果超过了释放给os
            PAGECACHE_LOG("释放给OS");
            //删除掉映射关系
            _idSpanMap.erase(pageId);
            _idSpanMap.erase(pageId+npage-1);
            systemFree((void*)(pageId<<PAGE_SHIFT),npage);
            //删除结点
            _spanPool.Delete(span);

            return;
        }

        //对于小页数可以进行合并
        // 向前合并
        while(1){
            if (span->_prev != nullptr || span->_next != nullptr){
                // span 已在某个桶中，先删除旧节点
                // 防止多轮合并时span会在多个桶间
                PAGECACHE_LOG("此时页号为："<<span->_pageID<<"先取下来");

                _spanList[span->_n].erase(span); // 先取下来
            }
            PAGE_ID pageId=span->_pageID;
            PAGECACHE_LOG("此时页号为："<<pageId<<" 向前合并");

            auto ret = _idSpanMap.get(pageId-1);
            if (ret == nullptr){
                // 前一个页号不存在（没有对应的 Span），无法向前合并
                PAGECACHE_LOG("前一页号不存在，停止向前合并");
                break;
            }
            //前一个页号存在，用preSpan管理
            Span* preSpan = (Span*)ret;

            if(preSpan->_isuse){
                PAGECACHE_LOG("前一个span正在使用，停止向前合并");
                break;
            }
            //合并出超过128页的span无法进行管理，停止向前合并
		    if (preSpan->_n + span->_n > NPAGES - 1){
                PAGECACHE_LOG("超过128页，无法进行合并");
			    break;
		    }
            if(!preSpan->_isuse){
                PAGECACHE_LOG("preSpan未在被使用，向前合并");
                //此时preSpan在page cache中

                // //先修改页和span的映射关系
                // for(size_t i=0;i<span->_n;i++){
                //     _idSpanMap.erase(span->_pageID+i);
                // }
                //这里仅需要删除首尾页即可，因为我们都是通过首尾页进行连接的，不会管到中间页

                _spanList[preSpan->_n].erase(preSpan);//取下来
                span->_pageID=preSpan->_pageID;
                span->_n+=preSpan->_n;
                _spanList[span->_n].pushFront(span);
                //删除原来的preSpan的映射
                _idSpanMap.erase(preSpan->_pageID);
                _idSpanMap.erase(preSpan->_pageID+preSpan->_n-1);
                //更新span的映射
                _idSpanMap.set(span->_pageID,span);
                _idSpanMap.set(span->_pageID+span->_n-1,span);
                PAGECACHE_LOG("合并后的页号为："<<span->_pageID<<" 页数为"<<span->_n);
                //preSpan只是管理内存的元数据                
                _spanPool.Delete(preSpan);
            }
        }
        //向后合并
        while(1){
            if (span->_prev != nullptr || span->_next != nullptr){
                PAGECACHE_LOG("此时页号为："<<span->_pageID<<"先取下来");
                _spanList[span->_n].erase(span);
            }
            PAGE_ID pageId=span->_pageID;
            PAGECACHE_LOG("此时页号为："<<pageId<<" 向后合并");

            PAGE_ID nextPageId=pageId+span->_n;//下一个span的页号
            auto ret = _idSpanMap.get(nextPageId);
            if (ret ==nullptr){
                // 后一个页号不存在（没有对应的 Span），无法向后合并
                PAGECACHE_LOG("后一个页号不存在,页号为"<<nextPageId<<" 停止向后合并");
                break;
            }
            Span* nextSpan = (Span*)ret;
            if(nextSpan->_isuse){
                PAGECACHE_LOG("后一个span正在使用，停止向后合并");
                break;
            }
            if(span->_n + nextSpan->_n > NPAGES - 1){
                PAGECACHE_LOG("超过128页，无法进行合并");
                break;
            }
            if(!nextSpan->_isuse){ 
                //此时nextSpan在page cache中
                PAGECACHE_LOG("nextSpan未在被使用，向后合并,后一个的页号为"<<nextPageId);
                
                // for(size_t i=0;i<nextSpan->_n;i++){
                //     _idSpanMap.erase(nextSpan->_pageID+i);
                // }
                //这里仅需要删除首尾页即可，因为我们都是通过首尾页进行连接的，不会管到中间页

                _spanList[nextSpan->_n].erase(nextSpan);//取下来
                span->_n+=nextSpan->_n;
                _spanList[span->_n].pushFront(span);
                //删除原来的nextSpan的映射
                _idSpanMap.erase(nextSpan->_pageID);
                _idSpanMap.erase(nextSpan->_pageID+nextSpan->_n-1);
                //更新span的映射
                _idSpanMap.set(span->_pageID,span);
                _idSpanMap.set(span->_pageID+span->_n-1,span);
                //nextSpan只是管理内存的元数据
                PAGECACHE_LOG("合并后的页号为："<<span->_pageID<<" 页数为"<<span->_n);
                _spanPool.Delete(nextSpan);
            }
        }
        if (span->_prev == nullptr && span->_next == nullptr){
                PAGECACHE_LOG("此时页号为："<<span->_pageID<<" 页数为"<<span->_n<<"未进行插入，现在插入");
                _spanList[span->_n].pushFront(span);
                _idSpanMap.set(span->_pageID,span);
                _idSpanMap.set(span->_pageID+span->_n-1,span);
            }
    }
    

    //获取一个k页的span
    Span* PageCache::newSpan(size_t k){
        assert(k>0);

        //对于超过128页的span，则直接从系统分配
        if(k>NPAGES-1){
            PAGECACHE_LOG("此时页数为："<<k<<" 获取span");
            void* start=systemAlloc(k);
            
            //管理
            Span* span= _spanPool.New();
            span->_pageID=(PAGE_ID)start>>PAGE_SHIFT;//计算出页号
            span->_n=k;
            span->_objSize=k<<PAGE_SHIFT;
            PAGECACHE_LOG("返回大小objSize："<<span->_objSize);
            span->_isuse=true;
           _idSpanMap.set(span->_pageID,span);
           _idSpanMap.set(span->_pageID+span->_n-1,span);
            PAGECACHE_LOG("返回页号为："<<span->_pageID<<" 页数为"<<span->_n);
            return span;
        }

        //所有小于的向pageCache申请
        //1.先检查桶中有没有span
        if(!_spanList[k].empty()){
            //不为空时，直接返回 
            //返回之前建立映射关系
            Span* obj=_spanList[k].popFront();
            for(PAGE_ID i=0;i<obj->_n;i++){
                _idSpanMap.set(obj->_pageID+i,obj);
            }
            obj->_prev=nullptr;
            obj->_next=nullptr;
            obj->_isuse=true;
            obj->_objSize=obj->_n<<PAGE_SHIFT;//这是必须的，因为可能直接分配给线程，跳过centralcache,如果没有这句会导致obj->_objSize为0
            return obj;
        }
        //2.如果为空则向下寻找大的span然后进行切分
        for(size_t i=k+1;i<NPAGES;i++){
            if(!_spanList[i].empty()){
                //如果找到了一个不为空则进行切割
                Span* bigSpan=_spanList[i].popFront();//先取下来
                _idSpanMap.erase(bigSpan->_pageID);
                _idSpanMap.erase(bigSpan->_pageID+bigSpan->_n-1);

                Span* kSpan= _spanPool.New();//new一个结点进行存储
                kSpan->_pageID=bigSpan->_pageID;//页的编号
                kSpan->_n=k;//页的数量
                //剩下的就是另一个了直接对bigSpan修改即可
                bigSpan->_pageID+=k;
                bigSpan->_n-=k;
                //将剩下的挂到对应的映射位置
                assert(bigSpan->_n>=0&&bigSpan->_n<NPAGES);
                _spanList[bigSpan->_n].pushFront(bigSpan);

                //建立bigspan的映射关系，后续进行合并的时候能找到
                //建立头尾即可
                _idSpanMap.set(bigSpan->_pageID,bigSpan);
                _idSpanMap.set(bigSpan->_pageID+bigSpan->_n-1,bigSpan);
                
                //返回之前建立kspan映射关系
                for (PAGE_ID i = 0; i < kSpan->_n; i++)
                {
                    _idSpanMap.set(kSpan->_pageID + i,kSpan);
                }
                kSpan->_prev=nullptr;
                kSpan->_next=nullptr;
                kSpan->_isuse=true;
                kSpan->_objSize=kSpan->_n<<PAGE_SHIFT;//必须的
                return kSpan;
            }
        }
        //3.没有找到，则向os申请内存，然后再进行划分

        //注意这里的地址不一样是因为bigSpan结构体的地址是new出来的，页编号计算出来的地址是os返回的，两者不是一个概念
        Span* bigSpan=_spanPool.New();
        PAGECACHE_LOG("此时pagecache中没有大页，准备向os申请"<<NPAGES-1<<"页");
        void* ptr=systemAlloc(NPAGES-1);
        bigSpan->_n=NPAGES-1;//页的数量为128
        bigSpan->_pageID=(PAGE_ID)ptr>>PAGE_SHIFT;//根据地址除页的偏移量计算出第几号
        bigSpan->_objSize=bigSpan->_n<<PAGE_SHIFT;
        _spanList[bigSpan->_n].pushFront(bigSpan);
        //递归调用
        // _spanList[bigSpan->_n].print();
        return newSpan(k);
    }

    //根据地址获取对象对应的span
    Span* PageCache::mapObjectToSpan(void* obj){
        PAGE_ID id = (PAGE_ID)obj >> PAGE_SHIFT; //页号
	    Span* ret = (Span*)_idSpanMap.get(id);
	    assert(ret != nullptr);
	    return ret;
    }