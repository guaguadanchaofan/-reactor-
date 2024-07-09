#include "ChannelMap.h"

// 初始化
struct ChannelMap *initChannelMap(int size)
{
    struct ChannelMap *map = (struct ChannelMap *)malloc(sizeof(struct ChannelMap));
    if (map == NULL)
    {
        return NULL;
    }
    map->_size = size;
    map->_list = (struct Channel **)malloc(size * sizeof(struct Channel *));
    return map;
}

// 清空map
void ChannelMapClear(struct ChannelMap *map)
{
    if (map != NULL)
    {
        for (int i = 0; i < map->_size; i++)
        {
            if (map->_list[i] != NULL)
                free(map->_list[i]);
        }
        free(map->_list);
        map->_list = NULL;
        map->_size = 0;
    }
}

// 扩容
bool ChannelMapAddROM(struct ChannelMap *map, int newsize, int unitsize)
{
    if (map != NULL)
    {
        if (map->_size < newsize)
        {
            int cursize = map->_size;
            while (cursize < newsize)
            {
                cursize *= 2;
            }
            struct Channel **tmp = realloc(map->_list, cursize * unitsize);
            if (tmp == NULL)
            {
                return false;
            }
            map->_list = tmp;
            map->_size = cursize;
            memset(&map->_list[map->_size], 0, (cursize - map->_size) * unitsize);
            return true;
        }
    }
    return false;
}