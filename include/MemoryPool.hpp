#pragma once 
#include <iostream>
#include <vector>
#include "./Common.hpp"
using namespace std;

template <class T>
class MemoryPool
{
public:
    // new函数：动态申请空间
    T* New()
    {
        T* ptr = nullptr;
        // 1.先判断自由链表是否有空间
        if (_freeList)
        {
            ptr = (T *)_freeList;
            // 更新自由链表
            _freeList = *((void **)_freeList);
        }
        else
        {
            // 链表没有可分配的内存空间了
            // 需要判断内存块中剩余的是否足够
            if (_leftBytes < (size_t)sizeof(T))
            {
                // 不够需要额外申请
                //TODO(开辟页数这个对于span是否开多了，很多地方都用到了定长内存池，后期可以动态调整这个)
                //512是因为基数树所需
                int kpage=512;
                _memory = (char *)systemAlloc(kpage); // 系统申请空间
                if (_memory == nullptr)
                {
                    throw std::bad_alloc();
                }
                _leftBytes=kpage*(1<<12);
            }
            ptr = (T *)_memory;
            size_t ptrSize = sizeof(T) < sizeof(void *) ? sizeof(void *) : sizeof(T); // 这里是为了实现自由链表
            _memory += ptrSize;                                                       // 保证下一次获取
            _leftBytes -= ptrSize;
        }
        // 使用定位new调用T的构造函数初始化
        new (ptr) T;
        return ptr;
    }
    // delete:析构函数
    void Delete(T *obj)
    {
        // 显示调用T的析构函数进行清理
        obj->~T();
        // 把内存回收，头插到自由链表
        *((void **)obj) = _freeList;
        _freeList = obj;
    }

private:
    char *_memory = nullptr;   // 指向内存块的指针
    size_t _leftBytes = 0;        // 内存块中剩余的字节数
    void *_freeList = nullptr; // 管理换回来的内存对象的自由链表
};

// 测试

// struct TreeNode
// {
//     int _val;
//     TreeNode *_left;
//     TreeNode *_right;
//     TreeNode()
//         : _val(0), _left(nullptr), _right(nullptr)
//     {
//     }
// };
// void TestMemoryPool()
// {

//     // 使用ptmalloc
//     //  申请释放的轮次
//     const size_t Rounds = 3;
//     // 每轮申请释放多少次
//     const size_t N = 100000;
//     size_t begin1 = clock();
//     std::vector<TreeNode *> v1;
//     v1.reserve(N);
//     for (size_t j = 0; j < Rounds; ++j)
//     {
//         for (int i = 0; i < N; ++i)
//         {
//             v1.push_back(new TreeNode);
//         }
//         for (int i = 0; i < N; ++i)
//         {
//             delete v1[i];
//         }
//         v1.clear();
//     }
//     size_t end1 = clock();

//     // 使用定长内存池
//     MemoryPool<TreeNode> MP;
//     size_t begin2 = clock();
//     std::vector<TreeNode *> v2;
//     v2.reserve(N);
//     for (size_t j = 0; j < Rounds; ++j)
//     {
//         for (int i = 0; i < N; ++i)
//         {
//             v2.push_back(MP.New());
//         }
//         for (int i = 0; i < N; ++i)
//         {
//             MP.Delete(v2[i]);
//         }
//         v2.clear();
//     }
//     size_t end2 = clock();
//     cout << "ptmalloc time:" << end1 - begin1 << endl;
//     cout << "MemoryPool time:" << end2 - begin2 << endl;
// }
// int main(){
//     TestMemoryPool();
//     return 0;
// }