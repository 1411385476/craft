#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/syscall.h>

#include <linux/unistd.h>

#define NUMTHREADS 3

//获取线程的pid
pid_t gettid()
{
 return syscall(SYS_gettid);
}

void sighand(int signo);

//普通线程组的执行函数
void *threadfunc(void *parm)
{
 pthread_t  tid = pthread_self();

 int rc;

 printf("Thread %d entered\n", gettid());
	
 printf("^^^^^^^^^^^^^^^^^\n");
 
//at this time, the thread begin to catch the signal
 rc = sleep(10);

 printf("&&&&&&&&&&&&&&&&&\n");
 
 //需要仔细观察sleep函数的返回值
 printf("Thread %d did not get expected results! rc=%d\n", gettid(), rc);

 return NULL;
}

//屏蔽所有信号的线程组的执行函数
void *threadmasked(void *parm)
{
 pthread_t tid = pthread_self();

 sigset_t mask;

 int rc;

 printf("Masked thread %d entered\n", gettid());

 sigfillset(&mask);
	
 printf("*********************\n");

 //这组线程阻塞所有的信号
 rc = pthread_sigmask(SIG_BLOCK, &mask, NULL);
 
 printf("!!!!!!!!!!!!!!!!!!!!!\n");

 if (rc != 0)
 {
  printf("%d, %s\n", rc, strerror(rc));

  return NULL;
 }

 rc = sleep(5);

 //所以它们可以安心地睡到自然醒
 //如果rc不能0就不正常了～
 if (rc != 0)
 {
  printf("Masked thread %d did not get expected results! ""rc=%d \n", gettid(), rc);

  return NULL;
 }

 printf("Masked thread %d completed masked work\n", gettid());

 return NULL;
}

int main(int argc, char **argv)
{
 int rc;

 int i;

 struct sigaction actions;

 //两组线程
 pthread_t threads[NUMTHREADS];

 pthread_t maskedthreads[NUMTHREADS];

 printf("Enter Testcase - %s\n", argv[0]);

 printf("Set up the alarm handler for the process\n");

 memset(&actions, 0, sizeof(actions));

 sigemptyset(&actions.sa_mask);

 actions.sa_flags = 0;

 //信号处理函数
 actions.sa_handler = sighand;

 //给定时器事件注册处理器
 //如果不捕捉该信号，该信号会对整个进程起作用
 //Linux下收到SIGALRM信号不处理，默认会打印"提醒时钟"
 //然后进程就退出了
 rc = sigaction(SIGALRM, &actions, NULL);

 printf("Create masked and unmasked threads\n");

 //开启两组线程
 for(i=0; i < NUMTHREADS; ++i)
 {
  rc = pthread_create(&threads[i], NULL, threadfunc, NULL);

  if (rc != 0)
  {
   printf("%d, %s\n", rc, strerror(rc));

   return -1;
  }

  rc = pthread_create(&maskedthreads[i], NULL, threadmasked, NULL);

  if (rc != 0)
  {
   printf("%d, %s\n", rc, strerror(rc));

   return -1;
  }
 }

 //睡眠3秒再发信号
 sleep(3);

 printf("Send a signal to masked and unmasked threads\n");

 //向两组线程发送信号
 for(i=0; i < NUMTHREADS; ++i)
 {
  rc = pthread_kill(threads[i], SIGALRM);

  rc = pthread_kill(maskedthreads[i], SIGALRM);
 }

 printf("Wait for masked and unmasked threads to complete\n");

 //等待子进程退出
 sleep(15);

 printf("Main completed\n");

 return 0;
}

//SIGALRM信号处理函数
void sighand(int signo)
{
 printf("Thread %d in signal handler\n", gettid());

 return;
}
