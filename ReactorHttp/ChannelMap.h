#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct ChannelMap
{
    // 指针指向数组元素的个数
    int _size;
    // struct Channel* _list[]
    struct Channel **_list;
};

// 初始化
struct ChannelMap *initChannelMap(int size);

// 清空map
void ChannelMapClear(struct ChannelMap *map);

// 扩容
bool ChannelMapAddROM(struct ChannelMap *map, int newsize, int unitsize);