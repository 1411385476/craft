#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
//#include "gnuhash.h"

char *data[] = { "alpha", "bravo", "charlie", "delta",
     "echo", "foxtrot", "golf", "hotel", "india", "juliet",
     "kilo", "lima", "mike", "november", "oscar", "papa",
     "quebec", "romeo", "sierra", "tango", "uniform",
     "victor", "whisky", "x-ray", "yankee", "zulu"
};

int main(void)
{
    int i,size=5;
    struct hsearch_data *htab;
    ENTRY e, *ep;
	int len = sizeof(struct hsearch_data);
	htab = malloc(len); 
	memset(htab, 0, len); 
	hcreate_r(size,htab); 
    //hash_create(30,htab);
	//printf("%u,%u\n",&ep,sizeof(ep));
   for (i = 0; i < 24; i++) {
        e.key = data[i];
        /* data is just an integer, instead of a
           pointer to something */
        e.data = (void *) i;
        //hsearch_r(e, ENTER,&ep,htab);
	ep = hsearch_r(e.key,e.data,htab);
        /* there should be no failures */
        /*if (ep == NULL) {
            fprintf(stderr, "entry failed\n");
            exit(EXIT_FAILURE);
        }*/
    }

   for (i = 22; i < 26; i++) {
        /* print two entries from the table, and
           show that two are not in the table */
        e.key = data[i];
        hsearch_r(e, FIND,&ep,htab);
        printf("%9.9s -> %9.9s:%d\n", e.key,
               ep ? ep->key : "NULL", ep ? (int)(ep->data) : 0);
    }
    hdestroy_r(htab);
    exit(EXIT_SUCCESS);
}

