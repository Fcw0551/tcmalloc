# TCMalloc-Allocator

基于 C++ 实现的高并发内存池，参考 Google TCMalloc 设计思想，实现 Thread Cache、Central Cache 与 Page Cache 多级缓存结构，优化多线程场景下的小对象内存分配性能，减少系统调用与锁竞争开销。

---

# 项目特点

- 基于 Thread Cache 实现线程私有缓存
- 基于 Central Cache 实现线程共享内存管理
- 基于 Page Cache 管理大块内存页
- 支持小对象内存高效分配
- 降低频繁 malloc/free 带来的系统调用开销
- 减少多线程环境下锁竞争问题
- 基于 FreeList 管理小对象内存块
- 支持对象回收与内存复用

---

# 系统架构

```text
                ┌────────────────────┐
                │      Thread         │
                └─────────┬──────────┘
                          │
                          ▼
                ┌────────────────────┐
                │    Thread Cache     │
                │  线程私有缓存       │
                └─────────┬──────────┘
                          │ 内存不足
                          ▼
                ┌────────────────────┐
                │   Central Cache     │
                │  线程共享缓存       │
                └─────────┬──────────┘
                          │ Span 不足
                          ▼
                ┌────────────────────┐
                │     Page Cache      │
                │   页级内存管理      │
                └─────────┬──────────┘
                          │
                          ▼
                ┌────────────────────┐
                │    System Alloc     │
                │ mmap / brk 系统调用 │
                └────────────────────┘
```

---

# 项目结构

```text
TCMalloc-Allocator/
├── build/                         # 编译生成目录
│
├── include/                       # 核心头文件
│
│ ├── Alloc.hpp                    # 内存分配接口
│ ├── ThreadCache.hpp              # Thread Cache
│ ├── ThreadCacheManager.hpp       # Thread Cache 管理器
│ ├── CentralCache.hpp             # Central Cache
│ ├── PageCache.hpp                # Page Cache
│ ├── FreeListAndSpanList.hpp      # FreeList 与 SpanList
│ ├── MemoryPool.hpp               # 内存池封装
│ ├── SystemAlloc.hpp              # 系统内存申请
│ ├── RadixTree.hpp                # 基数树结构
│ └── Common.hpp                   # 公共配置与工具
│
├── src/                           # 源文件实现
│
│ ├── Alloc.cc                     # 内存分配实现
│ ├── ThreadCache.cc               # Thread Cache 实现
│ ├── ThreadCacheManager.cc        # Thread Cache 管理
│ ├── CentralCache.cc              # Central Cache 实现
│ ├── PageCache.cc                 # Page Cache 实现
│ └── FreeListAndSpanList.cc       # FreeList / SpanList 实现
│
├── main.cc                        # 测试入口
├── makefile                       # Makefile 构建
└── mypool                         # 生成可执行文件
```

---

# 核心模块说明

## Thread Cache

为每个线程维护独立缓存：

```text
Thread
   ↓
Thread Cache
```

特点：

- 无锁分配
- 减少线程竞争
- 提高小对象分配效率

线程优先从本地缓存申请内存。

---

## Central Cache

线程共享缓存层：

```text
Thread Cache
      ↓
Central Cache
```

作用：

- 管理多个 Span
- 为 Thread Cache 提供内存
- 回收线程归还的对象

通过批量分配减少锁竞争频率。

---

## Page Cache

页级内存管理模块：

```text
Central Cache
      ↓
Page Cache
```

作用：

- 管理大块页内存
- 支持 Span 切分与合并
- 向系统申请内存页

---

## FreeList

基于单链表管理小对象：

- 快速插入
- 快速删除
- 支持对象复用

减少频繁 malloc/free 带来的性能损耗。

---

## Span 管理

实现 SpanList：

- 管理连续页
- 支持页拆分
- 支持页合并

用于：

- 小对象切分
- 大对象管理

---

# 内存分配流程

```text
申请内存
    ↓
Thread Cache 是否存在可用对象
    ↓
存在 → 直接返回
    ↓
不存在
    ↓
Central Cache 获取 Span
    ↓
Central Cache 不足
    ↓
Page Cache 申请页内存
    ↓
系统 mmap/brk 申请内存
```

---

# 技术栈

- C++17
- Linux
- 多线程
- 内存池
- FreeList
- Span
- RadixTree

---

# 构建方式

```bash
make
```

---

# 运行

```bash
./mypool
```

---

# 项目收获

- 理解 TCMalloc 内存分配思想
- 理解 Thread Cache 多级缓存结构
- 理解高并发场景下的锁竞争问题
- 学习页级内存管理机制
- 熟悉小对象内存池设计
- 理解高性能内存分配器实现原理
