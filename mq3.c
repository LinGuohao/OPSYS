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

int main(int argc,char **argv)
{
    char * mqname = "/eq5or4";
    char rcv_buf[MSGSIZE];
    mqd_t mqdes1, mqdes2;
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSGSIZE;
    pid_t pid ,cpid;
    int status;
    //删除名为mqname的 message queue
    mq_unlink(mqname);
    //以 attr为属性 权限777（0表明后面的是权限的缩写） 以读写方式打开（O_EDWR）或者新建（O_CREAT）一个名为
    //mqname的 message queue
    mqdes1 = mq_open(mqname,O_CREAT|O_RDWR , 0777,&attr);
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
        //最后面的数字为优先级
        mq_send(mqdes2,"First message pr 30!",MSGSIZE,30);
        mq_send(mqdes2,"Second ! pr 20",MSGSIZE,20);
        mq_send(mqdes2,"Third pr 30!",MSGSIZE,30);
        mq_send(mqdes2,"Fourth pr 40!",MSGSIZE,40);
        mq_close(mqdes2);

    }else
    {
        int status;
        waitpid(cpid,&status,0);
        int i ,prio;
        sleep(1);
        struct mq_attr attrget;
        for (i =1;i<=4;i++)
        {
            //把mqdes1的属性放到attrget中
            mq_getattr(mqdes1,&attrget);
            //attrget对应的message queue里剩余的message数量
            printf("How many messages are in the mq %i\n",attrget.mq_curmsgs);
            //最后prio指针存放该条信息的优先级
            mq_receive(mqdes1,rcv_buf,MSGSIZE,&prio);
            printf("Parent:\t\t\tRechieved : %s prior : %i\n",rcv_buf,prio);
        }
        mq_close(mqdes1);
        mq_unlink(mqname);
        printf("\n The mq sample is finished!\n");
        return 0;
    }

}
//比mq2.c多加了判断剩余message queue里剩余的message数量
