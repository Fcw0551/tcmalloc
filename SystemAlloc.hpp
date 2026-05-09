#pragma once
#include <sys/mman.h>
#include "Common.hpp"
//向堆申请空间
inline static void* systemAlloc(size_t kpage){
#ifdef _WIN32
    //win
    void *ptr = VirtualAlloc(0, kpage * (1 << PAGE_SHIFT), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    // linux
    void *ptr = mmap(0, kpage * (1 << PAGE_SHIFT), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
#endif
    if (ptr == nullptr)
    {
        throw std::bad_alloc();
    }
    return ptr;
}

//释放空间给堆
inline static void systemFree(void* ptr,size_t kpage)
{
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	//linux
    munmap(ptr, kpage*(1 << PAGE_SHIFT));
#endif
}