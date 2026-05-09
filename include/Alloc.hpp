#pragma once 
#include "./ThreadCacheManager.hpp"
#include "./Common.hpp"
#include "./PageCache.hpp"
class SizeMap;
void* Alloc(size_t size);

void Dealloc(void* obj);