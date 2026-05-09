#pragma once
// 全局作用域（函数外）添加头文件包含（仅 Linux）
#ifdef __linux__
#include <sys/mman.h>
#include <unistd.h>
#endif
//自由链表的哈希桶跟对象大小的映射关系
static const size_t MAX_BYTES=256*1024;//大于向系统或者PageCache申请，小于向thread cache
static const size_t NFREELISTS=208;//thread cache和central cache的自由链表哈希桶的表大小
static const size_t NPAGES=129;//page cache管理的span list哈希表大小  129是让第1个对应1 0~128=129个
static const size_t PAGE_SHIFT=12;//页大小转换偏移，即一页定义为2^12 4KB


//调试宏
#define ENABLE_THREADCACHE_LOG 0    // ThreadCache 日志开关
#define ENABLE_CENTRALCACHE_LOG 0   // CentralCache 日志开关
#define ENABLE_PAGECACHE_LOG 0      // PageCache 日志开关
// 各模块日志宏（前缀区分模块）
#define THREADCACHE_LOG(...) \
    do { \
        if (ENABLE_THREADCACHE_LOG) { \
            std::cout << "[ThreadCache][" << __FILE__ << ":" << __LINE__ << "] " << __VA_ARGS__ << std::endl; \
        } \
    } while (0)

#define CENTRALCACHE_LOG(...) \
    do { \
        if (ENABLE_CENTRALCACHE_LOG) { \
            std::cout << "[CentralCache][" << __FILE__ << ":" << __LINE__ << "] " << __VA_ARGS__ << std::endl; \
        } \
    } while (0)

#define PAGECACHE_LOG(...) \
    do { \
        if (ENABLE_PAGECACHE_LOG) { \
            std::cout << "[PageCache][" << __FILE__ << ":" << __LINE__ << "] " << __VA_ARGS__ << std::endl; \
        } \
    } while (0)


//注意先判断64位，否则可能会导致64位系统进入32位
// 地址类型大小，32位下是4bytes 64位是8bytes
#if  defined(_WIN64) ||  defined(__x86_64__)
typedef unsigned long long ADDRESS_INT;
#else  
typedef size_t ADDRESS_INT;
#endif


// 页编号类型，32位下是4bytes，64位是8bytes
#if  defined(_WIN64) ||  defined(__x86_64__)

typedef unsigned long long PAGE_ID;
#else
typedef size_t PAGE_ID;

#endif


//向堆申请内存
inline static void* systemAlloc(size_t kpage) {
    size_t totalSize = kpage << PAGE_SHIFT; // 总分配大小 = 页数 × 页大小
    THREADCACHE_LOG("systemAlloc: " << kpage<< "pages");
    THREADCACHE_LOG("systemAlloc: " << totalSize << " bytes");
    void* ptr = nullptr;

#ifdef _WIN32
    // Windows 平台：使用 VirtualAlloc
    ptr = VirtualAlloc(
        0,                  // 让系统自动选择分配地址
        totalSize,          // 要分配的总字节数
        MEM_COMMIT | MEM_RESERVE, // 提交+保留内存（必须组合）
        PAGE_READWRITE      // 内存权限：可读可写
    );

    // Windows 特有：VirtualAlloc 失败返回 NULL，单独判断
    if (ptr == nullptr) {
        perror("VirtualAlloc failed"); // 打印 Windows 系统错误（可选）
        throw std::bad_alloc();
    }
#else
    // Linux/macOS 平台：使用 mmap，需包含对应头文件
    ptr = mmap(
        0,                  // 让内核自动选择地址
        totalSize,          // 总分配大小
        PROT_READ | PROT_WRITE, // 内存权限：可读可写
        MAP_ANONYMOUS | MAP_PRIVATE, // 匿名映射（无文件）+ 私有映射
        -1,                 // 无文件描述符（匿名映射专用）
        0                   // 文件偏移量（匿名映射无效，填 0）
    );

    // Linux 特有：mmap 失败返回 MAP_FAILED，单独判断
    if (ptr == MAP_FAILED) {
        perror("mmap failed"); // 打印 Linux 系统错误（如内存不足，可选）
        throw std::bad_alloc();
    }
#endif
    THREADCACHE_LOG("systemAlloc: " << ptr << " bytes");
    return ptr;
}

//释放空间给堆
inline static void systemFree(void* ptr,size_t kpage)
{
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	//linux
    PAGECACHE_LOG("systemFree: " << ptr << " bytes");

    munmap(ptr, kpage*(1 << PAGE_SHIFT));
#endif
}


//获取内存对象中头4或8字节
inline static void*& nextObj(void*obj){
    return *((void**)obj);
}
