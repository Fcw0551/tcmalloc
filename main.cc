//#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <cstddef>
#include <chrono>
#include <cstdlib>
#include <algorithm>
#include <iomanip>   // 新增：用于 std::setprecision
#include <random>    // 新增：用于 std::shuffle 的随机数引擎
#include "../include/Alloc.hpp"
#include <atomic>
// // 测试配置（可调整）
// const int AllocCount = 10000;    // 每个线程申请次数（加大更能体现性能差异）
// const int ThreadNum = 6;        // 测试线程数
// const size_t TestSizes[] = {1, 8, 16, 32, 64, 128, 256, 512, 1024}; // 测试内存大小

// // ==================== 内存池 Alloc 测试函数 ====================
// void threadPoolAllocFunc(int threadId, std::chrono::high_resolution_clock::time_point& start, 
//                          std::chrono::high_resolution_clock::time_point& end) {
//     std::vector<void*> allocatedPtrs;
//     allocatedPtrs.reserve(AllocCount); // 预分配容量，避免vector扩容干扰计时

//     // 记录线程开始时间（所有线程统一以第一个线程开始为起点，最后一个线程结束为终点）
//     if (threadId == 0) {
//         start = std::chrono::high_resolution_clock::now();
//     }

//     // 循环申请内存
//     for (int i = 0; i < AllocCount; ++i) {
//         size_t size = TestSizes[i % (sizeof(TestSizes)/sizeof(TestSizes[0]))];
//         void* ptr = Alloc(size); // 内存池申请
//         allocatedPtrs.push_back(ptr);
//     }

//     // 记录线程结束时间（最后一个线程结束时记录终点）
//     if (threadId == ThreadNum - 1) {
//         end = std::chrono::high_resolution_clock::now();
//     }

//     // 不释放内存（专注测试分配性能）
//     std::cout << "内存池线程 " << threadId << " 执行完毕，共申请 " 
//               << allocatedPtrs.size() << " 块内存" << std::endl;
// }

// // ==================== 系统 malloc 测试函数 ====================
// void threadMallocFunc(int threadId, std::chrono::high_resolution_clock::time_point& start, 
//                       std::chrono::high_resolution_clock::time_point& end) {
//     std::vector<void*> allocatedPtrs;
//     allocatedPtrs.reserve(AllocCount); // 预分配容量，避免vector扩容干扰计时

//     // 记录线程开始时间
//     if (threadId == 0) {
//         start = std::chrono::high_resolution_clock::now();
//     }

//     // 循环申请内存
//     for (int i = 0; i < AllocCount; ++i) {
//         size_t size = TestSizes[i % (sizeof(TestSizes)/sizeof(TestSizes[0]))];
//         void* ptr = malloc(size); // 系统 malloc 申请
//         allocatedPtrs.push_back(ptr);
//     }

//     // 记录线程结束时间
//     if (threadId == ThreadNum - 1) {
//         end = std::chrono::high_resolution_clock::now();
//     }

//     // 不释放内存（专注测试分配性能）
//     std::cout << "系统malloc线程 " << threadId << " 执行完毕，共申请 " 
//               << allocatedPtrs.size() << " 块内存" << std::endl;
// }

// // ==================== 性能对比主函数 ====================
// int main() {
//     std::chrono::high_resolution_clock::time_point poolStart, poolEnd;
//     std::chrono::high_resolution_clock::time_point mallocStart, mallocEnd;

//     std::cout << "=== 内存池 vs 系统malloc 多线程性能测试 ===" << std::endl;
//     std::cout << "测试配置：" << ThreadNum << " 线程，每个线程申请 " << AllocCount << " 次" << std::endl;
//     std::cout << "测试内存大小：1B、8B、16B、32B、64B、128B、256B、512B、1024B" << std::endl;
//     std::cout << "===========================================" << std::endl;

//     // -------------------- 测试内存池 Alloc --------------------
//     std::cout << "\n【内存池 Alloc 测试开始】" << std::endl;
//     std::vector<std::thread> poolThreads;
//     for (int i = 0; i < ThreadNum; ++i) {
//         poolThreads.emplace_back(threadPoolAllocFunc, i, std::ref(poolStart), std::ref(poolEnd));
//     }
//     for (auto& t : poolThreads) {
//         t.join();
//     }

//     // 计算内存池总耗时（毫秒）
//     auto poolDuration = std::chrono::duration_cast<std::chrono::microseconds>(poolEnd - poolStart).count() / 1000.0;
//     size_t totalAlloc = ThreadNum * AllocCount; // 总申请次数
//     double poolAvgTime = poolDuration / totalAlloc; // 单次申请平均耗时（毫秒）

//     // -------------------- 测试系统 malloc --------------------
//     std::cout << "\n【系统 malloc 测试开始】" << std::endl;
//     std::vector<std::thread> mallocThreads;
//     for (int i = 0; i < ThreadNum; ++i) {
//         mallocThreads.emplace_back(threadMallocFunc, i, std::ref(mallocStart), std::ref(mallocEnd));
//     }
//     for (auto& t : mallocThreads) {
//         t.join();
//     }

//     // 计算系统 malloc 总耗时（毫秒）
//     auto mallocDuration = std::chrono::duration_cast<std::chrono::microseconds>(mallocEnd - mallocStart).count() / 1000.0;
//     double mallocAvgTime = mallocDuration / totalAlloc; // 单次申请平均耗时（毫秒）

//     // -------------------- 输出性能对比结果 --------------------
//     std::cout << "\n===========================================" << std::endl;
//     std::cout << "=== 性能对比结果 ===" << std::endl;
//     std::cout << "总申请次数：" << totalAlloc << " 次" << std::endl;
//     std::cout << "内存池总耗时：" << poolDuration << " 毫秒" << std::endl;
//     std::cout << "系统malloc总耗时：" << mallocDuration << " 毫秒" << std::endl;
//     std::cout << "-------------------------------------------" << std::endl;
//     std::cout << "内存池单次申请平均耗时：" << poolAvgTime << " 毫秒" << std::endl;
//     std::cout << "系统malloc单次申请平均耗时：" << mallocAvgTime << " 毫秒" << std::endl;
//     std::cout << "-------------------------------------------" << std::endl;
//     std::cout << "性能提升倍数：" << mallocAvgTime / poolAvgTime << " 倍" << std::endl;
//     std::cout << "===========================================" << std::endl;

//     return 0;
// }

// //单线程申请内存测试
// int main() {
//     Alloc(1);
//     Alloc(6);
//     Alloc(8);
//     Alloc(7);
// }



// //单线程申请内存释放内存测试
// int main(){
    
//     //申请5次，会造成maxsize
//     std::cout<<"正在申请内存"<<std::endl;
//     void* ptr=Alloc(6);//maxsize++
//     void* ptr1=Alloc(1);//maxsize++
//     void* ptr2=Alloc(7);
//     void* ptr3=Alloc(5);//maxsize++
//     void* ptr4=Alloc(4);
//     //maxsize变为4
//     std::cout<<"正在释放内存"<<std::endl;
//     //释放中必有size大于maxsize，因为总共申请了6块，当回收ptr3的时候就会出现size大于maxsize的
//     Dealloc(ptr);
//     Dealloc(ptr1);
//     Dealloc(ptr2);
//     Dealloc(ptr3);
//     Dealloc(ptr4);
// }


//单线程测试 释放到中心缓存之后还会不会释放到pagecache进行合并span成更大的页
// int main(){
//     //三次申请
//     void* ptr=Alloc(6);//maxsize++
//     void* ptr1=Alloc(1);//maxsize++
//     void* ptr2=Alloc(7);
//     //三次释放
//     Dealloc(ptr,6);
//     Dealloc(ptr1,1);
//     Dealloc(ptr2,7);
    
// }


//测试大内存直接向pageCache申请或者os申请
// int main(){
//     void* ptr=Alloc(1024*257);
//     Dealloc(ptr,1024*257);
// }
// int main(){
//     void* ptr=Alloc(1024*1024);
//     Dealloc(ptr,1024*1024);
// }












// // 测试配置(旧的Dealloc接口，需要传入size)
// const int AllocCount = 1000;    
// const int ThreadNum = 20;        
// const size_t TestSizes[] = {1, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
// const size_t TestSizeCount = sizeof(TestSizes) / sizeof(TestSizes[0]);

// // 内存池 申请+释放 测试函数
// void threadPoolAllocFreeFunc(int threadId, std::chrono::high_resolution_clock::time_point& start,
//                              std::chrono::high_resolution_clock::time_point& end) {
//     std::vector<void*> allocatedPtrs;
//     std::vector<size_t> allocatedSizes;
//     allocatedPtrs.reserve(AllocCount);
//     allocatedSizes.reserve(AllocCount);

//     if (threadId == 0) {
//         start = std::chrono::high_resolution_clock::now();
//     }

//     // 申请内存
//     for (int i = 0; i < AllocCount; ++i) {
//         size_t size = TestSizes[rand() % TestSizeCount];
//         void* ptr = Alloc(size);
//         allocatedPtrs.push_back(ptr);
//         allocatedSizes.push_back(size);
//     }

//     // 乱序释放（替换 random_shuffle 为 shuffle）
//     std::vector<int> freeIndices(AllocCount);
//     for (int i = 0; i < AllocCount; ++i) {
//         freeIndices[i] = i;
//     }
//     // 用 std::shuffle + 随机数引擎替代
//     std::random_device rd;
//     std::mt19937 g(rd());
//     std::shuffle(freeIndices.begin(), freeIndices.end(), g);

//     for (int idx : freeIndices) {
//         void* ptr = allocatedPtrs[idx];
//         size_t size = allocatedSizes[idx];
//         Dealloc(ptr, size);
//     }

//     if (threadId == ThreadNum - 1) {
//         end = std::chrono::high_resolution_clock::now();
//     }

//     std::cout << "内存池线程 " << threadId << " 执行完毕，共申请+释放 "
//               << allocatedPtrs.size() << " 块内存" << std::endl;
// }

// // 系统 malloc+free 测试函数
// void threadMallocFreeFunc(int threadId, std::chrono::high_resolution_clock::time_point& start,
//                           std::chrono::high_resolution_clock::time_point& end) {
//     std::vector<void*> allocatedPtrs;
//     std::vector<size_t> allocatedSizes;
//     allocatedPtrs.reserve(AllocCount);
//     allocatedSizes.reserve(AllocCount);

//     if (threadId == 0) {
//         start = std::chrono::high_resolution_clock::now();
//     }

//     // 申请内存
//     for (int i = 0; i < AllocCount; ++i) {
//         size_t size = TestSizes[rand() % TestSizeCount];
//         void* ptr = malloc(size);
//         allocatedPtrs.push_back(ptr);
//         allocatedSizes.push_back(size);
//     }

//     // 乱序释放
//     std::vector<int> freeIndices(AllocCount);
//     for (int i = 0; i < AllocCount; ++i) {
//         freeIndices[i] = i;
//     }
//     std::random_device rd;
//     std::mt19937 g(rd());
//     std::shuffle(freeIndices.begin(), freeIndices.end(), g);

//     for (int idx : freeIndices) {
//         void* ptr = allocatedPtrs[idx];
//         free(ptr);
//     }

//     if (threadId == ThreadNum - 1) {
//         end = std::chrono::high_resolution_clock::now();
//     }

//     std::cout << "系统malloc线程 " << threadId << " 执行完毕，共申请+释放 "
//               << allocatedPtrs.size() << " 块内存" << std::endl;
// }

// // 主函数
// int main() {
//     srand(42);

//     std::chrono::high_resolution_clock::time_point poolStart, poolEnd;
//     std::chrono::high_resolution_clock::time_point mallocStart, mallocEnd;

//     std::cout << "=== 内存池 vs 系统malloc 多线程申请+释放性能测试 ===" << std::endl;
//     std::cout << "测试配置：" << ThreadNum << " 线程，每个线程申请+释放 " << AllocCount << " 次" << std::endl;
//     std::cout << "测试内存大小：";
//     for (size_t i = 0; i < TestSizeCount; ++i) {
//         std::cout << TestSizes[i] << "B" << (i == TestSizeCount - 1 ? "" : "、");
//     }
//     std::cout << std::endl;
//     std::cout << "测试特点：随机申请大小、乱序释放（模拟真实业务场景）" << std::endl;
//     std::cout << "===========================================" << std::endl;

//     // 测试内存池
//     std::cout << "\n【内存池 申请+释放 测试开始】" << std::endl;
//     std::vector<std::thread> poolThreads;
//     for (int i = 0; i < ThreadNum; ++i) {
//         poolThreads.emplace_back(threadPoolAllocFreeFunc, i, std::ref(poolStart), std::ref(poolEnd));
//     }
//     for (auto& t : poolThreads) {
//         t.join();
//     }

//     // 计算耗时
//     auto poolDuration = std::chrono::duration_cast<std::chrono::microseconds>(poolEnd - poolStart).count() / 1000.0;
//     size_t totalOps = ThreadNum * AllocCount;
//     double poolAvgTime = poolDuration / totalOps;

//     // 测试系统malloc
//     std::cout << "\n【系统 malloc+free 测试开始】" << std::endl;
//     std::vector<std::thread> mallocThreads;
//     for (int i = 0; i < ThreadNum; ++i) {
//         mallocThreads.emplace_back(threadMallocFreeFunc, i, std::ref(mallocStart), std::ref(mallocEnd));
//     }
//     for (auto& t : mallocThreads) {
//         t.join();
//     }

//     auto mallocDuration = std::chrono::duration_cast<std::chrono::microseconds>(mallocEnd - mallocStart).count() / 1000.0;
//     double mallocAvgTime = mallocDuration / totalOps;

//     // 输出结果
//     std::cout << "\n===========================================" << std::endl;
//     std::cout << "=== 性能对比结果 ===" << std::endl;
//     std::cout << "总操作次数（申请+释放）：" << totalOps * 2 << " 次（各 " << totalOps << " 次）" << std::endl;
//     std::cout << "内存池总耗时：" << std::fixed << std::setprecision(3) << poolDuration << " 毫秒" << std::endl;
//     std::cout << "系统malloc总耗时：" << std::fixed << std::setprecision(3) << mallocDuration << " 毫秒" << std::endl;
//     std::cout << "-------------------------------------------" << std::endl;
//     std::cout << "内存池单次操作（申请/释放）平均耗时：" << std::fixed << std::setprecision(6) << poolAvgTime << " 毫秒" << std::endl;
//     std::cout << "系统malloc单次操作（申请/释放）平均耗时：" << std::fixed << std::setprecision(6) << mallocAvgTime << " 毫秒" << std::endl;
//     std::cout << "-------------------------------------------" << std::endl;
//     std::cout << "性能提升倍数：" << std::fixed << std::setprecision(3) << (mallocAvgTime / poolAvgTime) << " 倍" << std::endl;
//     std::cout << "===========================================" << std::endl;

//     return 0;
// }











// // 测试配置(新的Dealloc接口，不需要传入size)
// const int AllocCount = 1000;    
// const int ThreadNum = 20;        
// const size_t TestSizes[] = {1, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
// const size_t TestSizeCount = sizeof(TestSizes) / sizeof(TestSizes[0]);

// // 内存池 申请+释放 测试函数
// void threadPoolAllocFreeFunc(int threadId, std::chrono::high_resolution_clock::time_point& start,
//                              std::chrono::high_resolution_clock::time_point& end) {
//     std::vector<void*> allocatedPtrs;
//     std::vector<size_t> allocatedSizes;
//     allocatedPtrs.reserve(AllocCount);
//     //allocatedSizes.reserve(AllocCount);

//     if (threadId == 0) {
//         start = std::chrono::high_resolution_clock::now();
//     }

//     // 申请内存
//     for (int i = 0; i < AllocCount; ++i) {
//         size_t size = TestSizes[rand() % TestSizeCount];
//         void* ptr = Alloc(size);
//         allocatedPtrs.push_back(ptr);
//         //allocatedSizes.push_back(size);
//     }

//     // 乱序释放（替换 random_shuffle 为 shuffle）
//     std::vector<int> freeIndices(AllocCount);
//     for (int i = 0; i < AllocCount; ++i) {
//         freeIndices[i] = i;
//     }
//     // 用 std::shuffle + 随机数引擎替代
//     std::random_device rd;
//     std::mt19937 g(rd());
//     std::shuffle(freeIndices.begin(), freeIndices.end(), g);

//     for (int idx : freeIndices) {
//         void* ptr = allocatedPtrs[idx];
//         //size_t size = allocatedSizes[idx];
//         Dealloc(ptr);
//     }

//     if (threadId == ThreadNum - 1) {
//         end = std::chrono::high_resolution_clock::now();
//     }

//     std::cout << "内存池线程 " << threadId << " 执行完毕，共申请+释放 "
//               << allocatedPtrs.size() << " 块内存" << std::endl;
// }

// // 系统 malloc+free 测试函数
// void threadMallocFreeFunc(int threadId, std::chrono::high_resolution_clock::time_point& start,
//                           std::chrono::high_resolution_clock::time_point& end) {
//     std::vector<void*> allocatedPtrs;
//     std::vector<size_t> allocatedSizes;
//     allocatedPtrs.reserve(AllocCount);
//    // allocatedSizes.reserve(AllocCount);

//     if (threadId == 0) {
//         start = std::chrono::high_resolution_clock::now();
//     }

//     // 申请内存
//     for (int i = 0; i < AllocCount; ++i) {
//         size_t size = TestSizes[rand() % TestSizeCount];
//         void* ptr = malloc(size);
//         allocatedPtrs.push_back(ptr);
//        // allocatedSizes.push_back(size);
//     }

//     // 乱序释放
//     std::vector<int> freeIndices(AllocCount);
//     for (int i = 0; i < AllocCount; ++i) {
//         freeIndices[i] = i;
//     }
//     std::random_device rd;
//     std::mt19937 g(rd());
//     std::shuffle(freeIndices.begin(), freeIndices.end(), g);

//     for (int idx : freeIndices) {
//         void* ptr = allocatedPtrs[idx];
//         free(ptr);
//     }

//     if (threadId == ThreadNum - 1) {
//         end = std::chrono::high_resolution_clock::now();
//     }

//     std::cout << "系统malloc线程 " << threadId << " 执行完毕，共申请+释放 "
//               << allocatedPtrs.size() << " 块内存" << std::endl;
// }

// // 主函数
// int main() {
//     srand(42);

//     std::chrono::high_resolution_clock::time_point poolStart, poolEnd;
//     std::chrono::high_resolution_clock::time_point mallocStart, mallocEnd;

//     std::cout << "=== 内存池 vs 系统malloc 多线程申请+释放性能测试 ===" << std::endl;
//     std::cout << "测试配置：" << ThreadNum << " 线程，每个线程申请+释放 " << AllocCount << " 次" << std::endl;
//     std::cout << "测试内存大小：";
//     for (size_t i = 0; i < TestSizeCount; ++i) {
//         std::cout << TestSizes[i] << "B" << (i == TestSizeCount - 1 ? "" : "、");
//     }
//     std::cout << std::endl;
//     std::cout << "测试特点：随机申请大小、乱序释放（模拟真实业务场景）" << std::endl;
//     std::cout << "===========================================" << std::endl;

//     // 测试内存池
//     std::cout << "\n【内存池 申请+释放 测试开始】" << std::endl;
//     std::vector<std::thread> poolThreads;
//     for (int i = 0; i < ThreadNum; ++i) {
//         poolThreads.emplace_back(threadPoolAllocFreeFunc, i, std::ref(poolStart), std::ref(poolEnd));
//     }
//     for (auto& t : poolThreads) {
//         t.join();
//     }

//     // 计算耗时
//     auto poolDuration = std::chrono::duration_cast<std::chrono::microseconds>(poolEnd - poolStart).count() / 1000.0;
//     size_t totalOps = ThreadNum * AllocCount;
//     double poolAvgTime = poolDuration / totalOps;

//     // 测试系统malloc
//     std::cout << "\n【系统 malloc+free 测试开始】" << std::endl;
//     std::vector<std::thread> mallocThreads;
//     for (int i = 0; i < ThreadNum; ++i) {
//         mallocThreads.emplace_back(threadMallocFreeFunc, i, std::ref(mallocStart), std::ref(mallocEnd));
//     }
//     for (auto& t : mallocThreads) {
//         t.join();
//     }

//     auto mallocDuration = std::chrono::duration_cast<std::chrono::microseconds>(mallocEnd - mallocStart).count() / 1000.0;
//     double mallocAvgTime = mallocDuration / totalOps;

//     // 输出结果
//     std::cout << "\n===========================================" << std::endl;
//     std::cout << "=== 性能对比结果 ===" << std::endl;
//     std::cout << "总操作次数（申请+释放）：" << totalOps * 2 << " 次（各 " << totalOps << " 次）" << std::endl;
//     std::cout << "内存池总耗时：" << std::fixed << std::setprecision(3) << poolDuration << " 毫秒" << std::endl;
//     std::cout << "系统malloc总耗时：" << std::fixed << std::setprecision(3) << mallocDuration << " 毫秒" << std::endl;
//     std::cout << "-------------------------------------------" << std::endl;
//     std::cout << "内存池单次操作（申请/释放）平均耗时：" << std::fixed << std::setprecision(6) << poolAvgTime << " 毫秒" << std::endl;
//     std::cout << "系统malloc单次操作（申请/释放）平均耗时：" << std::fixed << std::setprecision(6) << mallocAvgTime << " 毫秒" << std::endl;
//     std::cout << "-------------------------------------------" << std::endl;
//     std::cout << "性能提升倍数：" << std::fixed << std::setprecision(3) << (mallocAvgTime / poolAvgTime) << " 倍" << std::endl;
//     std::cout << "===========================================" << std::endl;

//     return 0;
// }









// // // 测试配置(新的Dealloc接口，不需要传入size)
// const int AllocCount = 1000;    
// const int ThreadNum = 20;        
// //测试内存是1B到64KB
// const size_t TestSizes[] = {1, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192,16384, 32768, 65536,69999,262145};
// const size_t TestSizeCount = sizeof(TestSizes) / sizeof(TestSizes[0]);

// // 内存池 申请+释放 测试函数（仅去掉start/end参数，其余完全保留）
// void threadPoolAllocFreeFunc(int threadId) {
//     std::vector<void*> allocatedPtrs;
//     std::vector<size_t> allocatedSizes;
//     allocatedPtrs.reserve(AllocCount);
//     //allocatedSizes.reserve(AllocCount);

//     // 申请内存
//     for (int i = 0; i < AllocCount; ++i) {
//         size_t size = TestSizes[rand() % TestSizeCount];
//         void* ptr = Alloc(size);
//         allocatedPtrs.push_back(ptr);
//         //allocatedSizes.push_back(size);
//     }

//     // 乱序释放（替换 random_shuffle 为 shuffle）
//     std::vector<int> freeIndices(AllocCount);
//     for (int i = 0; i < AllocCount; ++i) {
//         freeIndices[i] = i;
//     }
//     // 用 std::shuffle + 随机数引擎替代
//     std::random_device rd;
//     std::mt19937 g(rd());
//     std::shuffle(freeIndices.begin(), freeIndices.end(), g);

//     for (int idx : freeIndices) {
//         void* ptr = allocatedPtrs[idx];
//         //size_t size = allocatedSizes[idx];
//         Dealloc(ptr);
//     }

//     std::cout << "内存池线程 " << threadId << " 执行完毕，共申请+释放 "
//               << allocatedPtrs.size() << " 块内存" << std::endl;
// }

// // 系统 malloc+free 测试函数（仅去掉start/end参数，其余完全保留）
// void threadMallocFreeFunc(int threadId) {
//     std::vector<void*> allocatedPtrs;
//     std::vector<size_t> allocatedSizes;
//     allocatedPtrs.reserve(AllocCount);
//    // allocatedSizes.reserve(AllocCount);

//     // 申请内存
//     for (int i = 0; i < AllocCount; ++i) {
//         size_t size = TestSizes[rand() % TestSizeCount];
//         void* ptr = malloc(size);
//         allocatedPtrs.push_back(ptr);
//        // allocatedSizes.push_back(size);
//     }

//     // 乱序释放
//     std::vector<int> freeIndices(AllocCount);
//     for (int i = 0; i < AllocCount; ++i) {
//         freeIndices[i] = i;
//     }
//     std::random_device rd;
//     std::mt19937 g(rd());
//     std::shuffle(freeIndices.begin(), freeIndices.end(), g);

//     for (int idx : freeIndices) {
//         void* ptr = allocatedPtrs[idx];
//         free(ptr);
//     }

//     std::cout << "系统malloc线程 " << threadId << " 执行完毕，共申请+释放 "
//               << allocatedPtrs.size() << " 块内存" << std::endl;
// }

// // 主函数（仅修改计时逻辑，其余完全保留）
// int main() {
//     srand(42);

//     std::chrono::high_resolution_clock::time_point poolStart, poolEnd;
//     std::chrono::high_resolution_clock::time_point mallocStart, mallocEnd;

//     std::cout << "=== 内存池 vs 系统malloc 多线程申请+释放性能测试 ===" << std::endl;
//     std::cout << "测试配置：" << ThreadNum << " 线程，每个线程申请+释放 " << AllocCount << " 次" << std::endl;
//     std::cout << "测试内存大小：";
//     for (size_t i = 0; i < TestSizeCount; ++i) {
//         std::cout << TestSizes[i] << "B" << (i == TestSizeCount - 1 ? "" : "、");
//     }
//     std::cout << std::endl;
//     std::cout << "测试特点：随机申请大小、乱序释放（模拟真实业务场景）" << std::endl;
//     std::cout << "===========================================" << std::endl;

//     // 测试内存池
//     std::cout << "\n【内存池 申请+释放 测试开始】" << std::endl;
//     std::vector<std::thread> poolThreads;
//     // 主线程统一记录开始时间（所有线程启动前）
//     poolStart = std::chrono::high_resolution_clock::now();
//     for (int i = 0; i < ThreadNum; ++i) {
//         poolThreads.emplace_back(threadPoolAllocFreeFunc, i); // 去掉start/end参数传递
//     }
//     for (auto& t : poolThreads) {
//         t.join();
//     }
//     // 主线程统一记录结束时间（所有线程执行完毕后）
//     poolEnd = std::chrono::high_resolution_clock::now();

//     // 计算耗时
//     auto poolDuration = std::chrono::duration_cast<std::chrono::microseconds>(poolEnd - poolStart).count() / 1000.0;
//     size_t totalOps = ThreadNum * AllocCount;
//     double poolAvgTime = poolDuration / totalOps;

//     // 测试系统malloc
//     std::cout << "\n【系统 malloc+free 测试开始】" << std::endl;
//     std::vector<std::thread> mallocThreads;
//     // 主线程统一记录开始时间（所有线程启动前）
//     mallocStart = std::chrono::high_resolution_clock::now();
//     for (int i = 0; i < ThreadNum; ++i) {
//         mallocThreads.emplace_back(threadMallocFreeFunc, i); // 去掉start/end参数传递
//     }
//     for (auto& t : mallocThreads) {
//         t.join();
//     }
//     // 主线程统一记录结束时间（所有线程执行完毕后）
//     mallocEnd = std::chrono::high_resolution_clock::now();

//     auto mallocDuration = std::chrono::duration_cast<std::chrono::microseconds>(mallocEnd - mallocStart).count() / 1000.0;
//     double mallocAvgTime = mallocDuration / totalOps;

//     // 输出结果
//     std::cout << "\n===========================================" << std::endl;
//     std::cout << "=== 性能对比结果 ===" << std::endl;
//     std::cout << "总操作次数（申请+释放）：" << totalOps * 2 << " 次（各 " << totalOps << " 次）" << std::endl;
//     std::cout << "内存池总耗时：" << std::fixed << std::setprecision(3) << poolDuration << " 毫秒" << std::endl;
//     std::cout << "系统malloc总耗时：" << std::fixed << std::setprecision(3) << mallocDuration << " 毫秒" << std::endl;
//     std::cout << "-------------------------------------------" << std::endl;
//     std::cout << "内存池单次操作（申请/释放）平均耗时：" << std::fixed << std::setprecision(6) << poolAvgTime << " 毫秒" << std::endl;
//     std::cout << "系统malloc单次操作（申请/释放）平均耗时：" << std::fixed << std::setprecision(6) << mallocAvgTime << " 毫秒" << std::endl;
//     std::cout << "-------------------------------------------" << std::endl;
//     std::cout << "性能提升倍数：" << std::fixed << std::setprecision(3) << (mallocAvgTime / poolAvgTime) << " 倍" << std::endl;
//     std::cout << "===========================================" << std::endl;

//     return 0;
// }












// // // 测试配置(新的Dealloc接口，不需要传入size)
// const int AllocCount = 1000;    
// const int ThreadNum = 10;        
// //测试内存是1B到256KB
// const size_t TestSizes[] = {1, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192,16384, 32768, 65536,131072,196608, 262144,273498,325489,524288,634244};
// const size_t TestSizeCount = sizeof(TestSizes) / sizeof(TestSizes[0]);

// // 内存池 申请+释放 测试函数（仅去掉start/end参数，其余完全保留）
// void threadPoolAllocFreeFunc(int threadId) {
//     std::vector<void*> allocatedPtrs;
//     std::vector<size_t> allocatedSizes;
//     allocatedPtrs.reserve(AllocCount);
//     //allocatedSizes.reserve(AllocCount);

//     // 申请内存
//     for (int i = 0; i < AllocCount; ++i) {
//         size_t size = TestSizes[rand() % TestSizeCount];
//         void* ptr = Alloc(size);
//         allocatedPtrs.push_back(ptr);
//         //allocatedSizes.push_back(size);
//     }

//     // 乱序释放（替换 random_shuffle 为 shuffle）
//     std::vector<int> freeIndices(AllocCount);
//     for (int i = 0; i < AllocCount; ++i) {
//         freeIndices[i] = i;
//     }
//     // 用 std::shuffle + 随机数引擎替代
//      std::mt19937 g(42 + threadId);
//    // std::mt19937 g(rd());
//     std::shuffle(freeIndices.begin(), freeIndices.end(), g);

//     for (int idx : freeIndices) {
//         void* ptr = allocatedPtrs[idx];
//         //size_t size = allocatedSizes[idx];
//         Dealloc(ptr);
//     }

//     std::cout << "内存池线程 " << threadId << " 执行完毕，共申请+释放 "
//               << allocatedPtrs.size() << " 块内存" << std::endl;
// }

// // 系统 malloc+free 测试函数（仅去掉start/end参数，其余完全保留）
// void threadMallocFreeFunc(int threadId) {
//     std::vector<void*> allocatedPtrs;
//     std::vector<size_t> allocatedSizes;
//     allocatedPtrs.reserve(AllocCount);
//    // allocatedSizes.reserve(AllocCount);

//     // 申请内存
//     for (int i = 0; i < AllocCount; ++i) {
//         size_t size = TestSizes[rand() % TestSizeCount];
//         void* ptr = malloc(size);
//         allocatedPtrs.push_back(ptr);
//        // allocatedSizes.push_back(size);
//     }

//     // 乱序释放
//     std::vector<int> freeIndices(AllocCount);
//     for (int i = 0; i < AllocCount; ++i) {
//         freeIndices[i] = i;
//     }
//     std::mt19937 g(42 + threadId);
//    // std::mt19937 g(rd());
//     std::shuffle(freeIndices.begin(), freeIndices.end(), g);

//     for (int idx : freeIndices) {
//         void* ptr = allocatedPtrs[idx];
//         free(ptr);
//     }

//     std::cout << "系统malloc线程 " << threadId << " 执行完毕，共申请+释放 "
//               << allocatedPtrs.size() << " 块内存" << std::endl;
// }

// // 主函数（仅修改计时逻辑，其余完全保留）
// int main() {
//     srand(42);

//     std::chrono::high_resolution_clock::time_point poolStart, poolEnd;
//     std::chrono::high_resolution_clock::time_point mallocStart, mallocEnd;

//     std::cout << "=== 内存池 vs 系统malloc 多线程申请+释放性能测试 ===" << std::endl;
//     std::cout << "测试配置：" << ThreadNum << " 线程，每个线程申请+释放 " << AllocCount << " 次" << std::endl;
//     std::cout << "测试内存大小：";
//     for (size_t i = 0; i < TestSizeCount; ++i) {
//         std::cout << TestSizes[i] << "B" << (i == TestSizeCount - 1 ? "" : "、");
//     }
//     std::cout << std::endl;
//     std::cout << "测试特点：随机申请大小、乱序释放（模拟真实业务场景）" << std::endl;
//     std::cout << "===========================================" << std::endl;

//     // 测试内存池
//     std::cout << "\n【内存池 申请+释放 测试开始】" << std::endl;
//     std::vector<std::thread> poolThreads;
//     // 主线程统一记录开始时间（所有线程启动前）
//     poolStart = std::chrono::high_resolution_clock::now();
//     for (int i = 0; i < ThreadNum; ++i) {
//         poolThreads.emplace_back(threadPoolAllocFreeFunc, i); // 去掉start/end参数传递
//     }
//     for (auto& t : poolThreads) {
//         t.join();
//     }
//     // 主线程统一记录结束时间（所有线程执行完毕后）
//     poolEnd = std::chrono::high_resolution_clock::now();

//     // 计算耗时
//     auto poolDuration = std::chrono::duration_cast<std::chrono::microseconds>(poolEnd - poolStart).count() / 1000.0;
//     size_t totalOps = ThreadNum * AllocCount;
//     double poolAvgTime = poolDuration / totalOps;

//     // 测试系统malloc
//     std::cout << "\n【系统 malloc+free 测试开始】" << std::endl;
//     std::vector<std::thread> mallocThreads;
//     // 主线程统一记录开始时间（所有线程启动前）
//     mallocStart = std::chrono::high_resolution_clock::now();
//     for (int i = 0; i < ThreadNum; ++i) {
//         mallocThreads.emplace_back(threadMallocFreeFunc, i); // 去掉start/end参数传递
//     }
//     for (auto& t : mallocThreads) {
//         t.join();
//     }
//     // 主线程统一记录结束时间（所有线程执行完毕后）
//     mallocEnd = std::chrono::high_resolution_clock::now();

//     auto mallocDuration = std::chrono::duration_cast<std::chrono::microseconds>(mallocEnd - mallocStart).count() / 1000.0;
//     double mallocAvgTime = mallocDuration / totalOps;

//     // 输出结果
//     std::cout << "\n===========================================" << std::endl;
//     std::cout << "=== 性能对比结果 ===" << std::endl;
//     std::cout << "总操作次数（申请+释放）：" << totalOps * 2 << " 次（各 " << totalOps << " 次）" << std::endl;
//     std::cout << "内存池总耗时：" << std::fixed << std::setprecision(3) << poolDuration << " 毫秒" << std::endl;
//     std::cout << "系统malloc总耗时：" << std::fixed << std::setprecision(3) << mallocDuration << " 毫秒" << std::endl;
//     std::cout << "-------------------------------------------" << std::endl;
//     std::cout << "内存池单次操作（申请/释放）平均耗时：" << std::fixed << std::setprecision(6) << poolAvgTime << " 毫秒" << std::endl;
//     std::cout << "系统malloc单次操作（申请/释放）平均耗时：" << std::fixed << std::setprecision(6) << mallocAvgTime << " 毫秒" << std::endl;
//     std::cout << "-------------------------------------------" << std::endl;
//     std::cout << "性能提升倍数：" << std::fixed << std::setprecision(3) << (mallocAvgTime / poolAvgTime) << " 倍" << std::endl;
//     std::cout << "===========================================" << std::endl;

//     return 0;
// }







// int main(){
//     void* ptr = Alloc(524288);//申请大内存
//     Dealloc(ptr);
// }


// int main(){
//     void* ptr = Alloc(524289);//申请大内存
//     Dealloc(ptr);
// }



// int main(){
//     void* ptr = Alloc(262146);//申请大内存
//     void* ptr2 = Alloc(297543);
//     void* ptr3 = Alloc(524289);
//     void* ptr4 = Alloc(524288);
//     Dealloc(ptr);
//     Dealloc(ptr2);
//     Dealloc(ptr3);
//     Dealloc(ptr4);
// }





// // 测试配置(新的Dealloc接口，不需要传入size)
const int AllocCount = 1000;    
const int ThreadNum = 32;        
//测试内存是1B到256KB
const size_t TestSizes[] = {1,8,16,32,64, 128, 256, 512, 1024, 2048, 4096, 8192,16384, 32768, 65536};
const size_t TestSizeCount = sizeof(TestSizes) / sizeof(TestSizes[0]);

// 内存池 申请+释放 测试函数（仅去掉start/end参数，其余完全保留）
void threadPoolAllocFreeFunc(int threadId) {
    std::vector<void*> allocatedPtrs;
    std::vector<size_t> allocatedSizes;
    allocatedPtrs.reserve(AllocCount);
    //allocatedSizes.reserve(AllocCount);

    // 申请内存
    for (int i = 0; i < AllocCount; ++i) {
        size_t size = TestSizes[rand() % TestSizeCount];
        void* ptr = Alloc(size);
        allocatedPtrs.push_back(ptr);
        //allocatedSizes.push_back(size);
    }

    // 乱序释放（替换 random_shuffle 为 shuffle）
    std::vector<int> freeIndices(AllocCount);
    for (int i = 0; i < AllocCount; ++i) {
        freeIndices[i] = i;
    }
    // 用 std::shuffle + 随机数引擎替代
     std::mt19937 g(42 + threadId);
   // std::mt19937 g(rd());
    std::shuffle(freeIndices.begin(), freeIndices.end(), g);

    for (int idx : freeIndices) {
        void* ptr = allocatedPtrs[idx];
        //size_t size = allocatedSizes[idx];
        Dealloc(ptr);
    }

    std::cout << "内存池线程 " << threadId << " 执行完毕，共申请+释放 "
              << allocatedPtrs.size() << " 块内存" << std::endl;
}

// 系统 malloc+free 测试函数（仅去掉start/end参数，其余完全保留）
void threadMallocFreeFunc(int threadId) {
    std::vector<void*> allocatedPtrs;
    std::vector<size_t> allocatedSizes;
    allocatedPtrs.reserve(AllocCount);
   // allocatedSizes.reserve(AllocCount);

    // 申请内存
    for (int i = 0; i < AllocCount; ++i) {
        size_t size = TestSizes[rand() % TestSizeCount];
        void* ptr = malloc(size);
        allocatedPtrs.push_back(ptr);
       // allocatedSizes.push_back(size);
    }

    // 乱序释放
    std::vector<int> freeIndices(AllocCount);
    for (int i = 0; i < AllocCount; ++i) {
        freeIndices[i] = i;
    }
    std::mt19937 g(42 + threadId);
   // std::mt19937 g(rd());
    std::shuffle(freeIndices.begin(), freeIndices.end(), g);

    for (int idx : freeIndices) {
        void* ptr = allocatedPtrs[idx];
        free(ptr);
    }

    std::cout << "系统malloc线程 " << threadId << " 执行完毕，共申请+释放 "
              << allocatedPtrs.size() << " 块内存" << std::endl;
}

// 主函数（仅修改计时逻辑，其余完全保留）
int main() {
    srand(42);

    std::chrono::high_resolution_clock::time_point poolStart, poolEnd;
    std::chrono::high_resolution_clock::time_point mallocStart, mallocEnd;

    std::cout << "=== 内存池 vs 系统malloc 多线程申请+释放性能测试 ===" << std::endl;
    std::cout << "测试配置：" << ThreadNum << " 线程，每个线程申请+释放 " << AllocCount << " 次" << std::endl;
    std::cout << "测试内存大小：";
    for (size_t i = 0; i < TestSizeCount; ++i) {
        std::cout << TestSizes[i] << "B" << (i == TestSizeCount - 1 ? "" : "、");
    }
    std::cout << std::endl;
    std::cout << "测试特点：随机申请大小、乱序释放（模拟真实业务场景）" << std::endl;
    std::cout << "===========================================" << std::endl;

    // 测试内存池
    std::cout << "\n【内存池 申请+释放 测试开始】" << std::endl;
    std::vector<std::thread> poolThreads;
    // 主线程统一记录开始时间（所有线程启动前）
    poolStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ThreadNum; ++i) {
        poolThreads.emplace_back(threadPoolAllocFreeFunc, i); // 去掉start/end参数传递
    }
    for (auto& t : poolThreads) {
        t.join();
    }
    // 主线程统一记录结束时间（所有线程执行完毕后）
    poolEnd = std::chrono::high_resolution_clock::now();

    // 计算耗时
    auto poolDuration = std::chrono::duration_cast<std::chrono::microseconds>(poolEnd - poolStart).count() / 1000.0;
    size_t totalOps = ThreadNum * AllocCount;
    double poolAvgTime = poolDuration / totalOps;

    // 测试系统malloc
    std::cout << "\n【系统 malloc+free 测试开始】" << std::endl;
    std::vector<std::thread> mallocThreads;
    // 主线程统一记录开始时间（所有线程启动前）
    mallocStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ThreadNum; ++i) {
        mallocThreads.emplace_back(threadMallocFreeFunc, i); // 去掉start/end参数传递
    }
    for (auto& t : mallocThreads) {
        t.join();
    }
    // 主线程统一记录结束时间（所有线程执行完毕后）
    mallocEnd = std::chrono::high_resolution_clock::now();

    auto mallocDuration = std::chrono::duration_cast<std::chrono::microseconds>(mallocEnd - mallocStart).count() / 1000.0;
    double mallocAvgTime = mallocDuration / totalOps;

    // 输出结果
    std::cout << "\n===========================================" << std::endl;
    std::cout << "=== 性能对比结果 ===" << std::endl;
    std::cout << "总操作次数（申请+释放）：" << totalOps * 2 << " 次（各 " << totalOps << " 次）" << std::endl;
    std::cout << "内存池总耗时：" << std::fixed << std::setprecision(3) << poolDuration << " 毫秒" << std::endl;
    std::cout << "系统malloc总耗时：" << std::fixed << std::setprecision(3) << mallocDuration << " 毫秒" << std::endl;
    std::cout << "-------------------------------------------" << std::endl;
    std::cout << "内存池单次操作（申请/释放）平均耗时：" << std::fixed << std::setprecision(6) << poolAvgTime << " 毫秒" << std::endl;
    std::cout << "系统malloc单次操作（申请/释放）平均耗时：" << std::fixed << std::setprecision(6) << mallocAvgTime << " 毫秒" << std::endl;
    std::cout << "-------------------------------------------" << std::endl;
    std::cout << "性能提升倍数：" << std::fixed << std::setprecision(3) << (mallocAvgTime / poolAvgTime) << " 倍" << std::endl;
    std::cout << "===========================================" << std::endl;

    return 0;
}
