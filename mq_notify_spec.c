#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <mqueue.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sched.h>
#include <signal.h>
#include <mqueue.h>

#define MSGSIZE 40
#define MAXMSGS 5
#define VALUE 1234

int val , code;

void handler(int signo,siginfo_t *info,void *ignored)
{
    psignal(signo,"Signal handler:\t\t");
    //读取信号附带的value值
    val = info->si_value.sival_int;
    //读取信号的代号
    code = info ->si_code;
    return;
}

int main(int argc,char **argv)
{
    struct sigaction act;
    struct sigevent notify;
    struct mq_attr attr;
    sigset_t set;
    char * mqname = "/iw3xv9";
    char rcv_buf[MSGSIZE];
    mqd_t mqdes1, mqdes2;
    
    int status;
    pid_t pid ,cpid;
    
    attr.mq_maxmsg = MAXMSGS;
    attr.mq_msgsize = MSGSIZE;
    
    //删除名为mqname的 message queue
    mq_unlink(mqname);
    //以 attr为属性 权限777（0表明后面的是权限的缩写） 以读写方式打开（O_EDWR）或者新建（O_CREAT）一个名为
    //mqname的 message queue
    mqdes1 = mq_open(mqname,O_CREAT|O_RDWR , 0777,&attr);

    sigemptyset(&set);
    act.sa_flags = SA_SIGINFO;
    //如果在handler函数执行过程中有set中的信号输入 那么阻塞（延迟）它 
    act.sa_mask = set;
    act.sa_sigaction = handler;

    //最后一个参数用来存储旧的&act信息
    sigaction(SIGUSR1,&act,0);
    //对sigevent的操作
    //https://blog.csdn.net/qq_35976351/article/details/87024570
    //SIGEV_SIGNAL 向进程中发送 sigev_signo 指定的信号
    notify.sigev_notify = SIGEV_SIGNAL;
    ///* Notification signal */
    notify.sigev_signo = SIGUSR1;
    ///* Data passed with notification */
    notify.sigev_value.sival_int = VALUE;

    //说明函数的作用：如果有新消息到达一个空的队列，允许调用线程注册一个异步通知，该通知会在消息到达时触发。
    //核心的思想在异步这两个字上，也就是说，我们希望从消息队列中获取消息，但是不想让当前进程阻塞在一个空的消息队列上，
    //那么通过该函数可以注册异步通知，来处理将来到达的事件。
    //参数1为消息队列指针 参数2为sigevent
    mq_notify(mqdes1,&notify);
    printf("\n Main program:\tTesting mq notification!\n\n");
    printf("Main program:\tsigev_value=%d\n",notify.sigev_value.sival_int);

    if((pid = fork())<0)
    {
        printf( "fork:Erro\n");
        printf("Test FAILED\n");
        exit(-1);
    }
    if(pid==0) //child
    {
        cpid=getpid();
        mqdes2 = mq_open(mqname, O_CREAT|O_RDWR,0777,&attr);
        printf("Child:\t\t\tSending message to empty queue\n");
        printf("Child:\t\t\tAt the same time, sending a notification to the parent!\n");
        mq_send(mqdes2,"First message !",MSGSIZE,30);
        sleep(2);
        mq_send(mqdes2,"Second message!",MSGSIZE,30);
        mq_close(mqdes2);
        printf("Child:\t\t\tFinished!\n");
        exit(0);

    }else
    {

        printf("Parent:\t\t\tWaiting for notification\n");
        sleep(1);
        mq_receive(mqdes1,rcv_buf,MSGSIZE,0);
        printf("Parent:\t\t\tQueue transition occured - received: %s\n",rcv_buf);
        mq_notify(mqdes1,&notify);
        sleep(3);
        mq_receive(mqdes1,rcv_buf,MSGSIZE,0);
        printf("Parent:\t\t\tQueue transition occured - received: %s\n",rcv_buf);
    }
    printf("Parent:\t\tsi_code=%d\n",code);
    printf("Parent:\t\tsi_value=%d\n",val);
    if(code != -3 || val!=VALUE)
    {
        printf("\nTest Failed\n\n");
    }
    mq_close(mqdes1);
    mq_unlink(mqname);
    printf("\nThe mq sample is finished!\n");
    return 0;
}