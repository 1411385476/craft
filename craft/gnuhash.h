#ifndef _GUN_HASH_LIST
#define _GNU_HASH_LIST
#include <string.h>
#define __USE_GNU
#include <search.h>


#define hash_create(size, tab) do {  \
        int len = sizeof(struct hsearch_data); \
        tab = malloc(len);  \
        memset(tab, 0, len); \
        hcreate_r(size,tab); \
} while (0)



#define hash_createa  hcreate_r
#define hash_destory  hdestroy_r



ENTRY  *hash_add(char *key, void *value, struct hsearch_data *tab)
{
        ENTRY item, *ret ;
        item.key = key ;
        item.data= value  ;
        int x=  hsearch_r(item, ENTER, &ret, tab);
        return ret ;
}

ENTRY *hash_find(char *key, struct hsearch_data *tab)
{
        ENTRY item, *ret ;
        item.key = key ;
        int x= hsearch_r(item, FIND, &ret, tab);
        return ret ;
}
ENTRY  *hash_addentry(ENTRY item, struct hsearch_data *tab)
{
        ENTRY *ret ;
        int x =  hsearch_r(item, ENTER, &ret, tab);
        return ret ;
}

ENTRY *hash_findentry(ENTRY item, struct hsearch_data *tab)
{
        ENTRY *ret ;
        int x= hsearch_r(item, FIND, &ret, tab);
        return ret ;
}


#endif
