#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gnuhash.h"

void prtht(ENTRY *ret)
{
        printf("%9.9s->%s\n",
                        ret ? ret->key : "NULL",
                        ret ? (char *)(ret->data) : "");
}

int main(int argc ,char **argv) {
        ENTRY e, *ret;
        int i ;

        struct hsearch_data *ht ;
        hash_create(5, ht);
        printf ("    key->value\n") ;
        ret = hash_add("test1", "aaa", ht) ;
        prtht(ret) ;

        ret = hash_add("test2", "bbb", ht) ;
        prtht(ret) ;

        ret = hash_add("test3", "ccc", ht) ;
        prtht(ret) ;

        ret = hash_add("test1", "ddd", ht) ;
        prtht(ret) ;

        printf("\nfind\n") ;
        ret = hash_find("test1", ht) ;
        prtht(ret) ;
        hash_destory(ht) ;


        return (EXIT_SUCCESS);
}
