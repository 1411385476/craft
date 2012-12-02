#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void *thread_a(void *arg)

{

        if(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL) != 0)

        {

                perror("setcancelstate\n");

        }

        if(pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL) != 0)

        {

                perror("setcancelsype\n");

        }

        while(1)

        {

                sleep(1);

                printf("thread_a\n");

                pthread_testcancel();

        }

}

void *thread_b(void *arg)

{

        if(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL) != 0)

        {

                perror("setcancelstate\n");

        }
	
	int i = 0;
        while(i<10)

        {

                sleep(1);

                printf("thread_b\n");

                pthread_testcancel();
		
		i++;

        }

}

void *thread_c(void *arg)

{

        if(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL) != 0)

        {

                perror("setcancelstate\n");

        }

        if(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL) != 0)

        {

              perror("setcancelsype\n");

        }

        while(1)

        {

                sleep(1);

                printf("thread_c\n");

                pthread_testcancel();

        }

}

int main(int argc, char **argv)

{
	
	printf("you are very good!\n");
	
	//exit(0);	

        pthread_t tid_a,tid_b,tid_c;

        int err;

        err = pthread_create(&tid_a,NULL,thread_a,(void *)&tid_a);

        if(err < 0)

        {

                perror("pthread_create thread_a\n");

        }

        printf("create tid_a = %u\n",tid_a);

        err = pthread_create(&tid_b,NULL,thread_b,(void *)&tid_b);

        if(err < 0)

        {

                perror("pthread_create thread_b\n");

        }

        printf("create tid_b = %u\n",tid_b);

        err = pthread_create(&tid_c,NULL,thread_c,(void *)&tid_c);

        if(err < 0)

        {

                perror("pthread_create thread_c\n");

        }

        printf("create tid_c = %u\n",tid_c);

              sleep(5);
	

        if(pthread_cancel(tid_a) != 0)

        {

                perror("pthread_cancel tid_a\n");

        }

              sleep(5);

        if(pthread_cancel(tid_b) != 0)

        {

                perror("pthread_cancel tid_b\n");

        }

              sleep(5);

        if(pthread_cancel(tid_c) != 0)

        {

                perror("pthread_cancel tid_c\n");

        }

        pthread_join(tid_a,NULL);
	pthread_join(tid_b,NULL);
	pthread_join(tid_c,NULL);

        printf("the main close\n");

        return 0;

}
