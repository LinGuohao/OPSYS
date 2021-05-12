#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>

#define ready SIGUSR1
#define MSGSIZE 500

int numready = 0;

sem_t* semaphore_create(char*name,int semaphore_value)
{
	//第二个参数:oflag参数可以是0、O_CREAT（创建一个信号灯）或O_CREAT|O_EXCL（如果没有指定的信号灯就创建），如果指定了O_CREAT，那么第三个和第四个参数是需要的
	//第四个参数：value 信号灯初始值
    sem_t *semid=sem_open(name,O_CREAT,S_IRUSR|S_IWUSR,semaphore_value );
	if (semid==SEM_FAILED)
	perror("sem_open");
       
    return semid;
}

void semaphore_delete(char* name)
{
      sem_unlink(name);
}

void write_to_file(char * path, char*c)
{
    FILE *fp = NULL;
    fp = fopen(path, "a+");
    if (fp==NULL){perror("File opening error\n"); exit(1);}
    fprintf(fp,"%s\n",c);
    fclose(fp);
}


void handler(int sig)
{
    printf("I am ready to start\n");
    numready = numready+1;
}


int main(int argc, char **argv)
{
    pid_t Ursula , Sergeant;
    
    signal(ready,handler);
    int pipefd[2],pipefd2[2]; // unnamed pipe file descriptor array
    if (pipe(pipefd) ==-1|| pipe(pipefd2)==-1) 
	{
        perror("Opening error!");
        exit(EXIT_FAILURE);
    }
    if(argc<2)
    {
        perror("Dont have enough argc");
        exit(EXIT_FAILURE);
    }


    char * mqname = "/iw3xv9";
    mqd_t mqdes;
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSGSIZE;
    mq_unlink(mqname);
    mqdes = mq_open(mqname,O_CREAT|O_RDWR , 0777,&attr);

    sem_t* semid;
    char* sem_name="/apple";
    semid = semaphore_create(sem_name,1);

    Ursula = fork(); 
    if(Ursula == 0)
    {
        kill(getppid(),ready);
        char sz[500];
        read(pipefd[0],sz,sizeof(sz));
       

        int num = atoi(sz);
        int sick = 0;
        srand(getpid());
        for(int i=0;i<num ;i++)
        {
             int a = rand()%100+1;
             if(a<=20)
             {
                 sick = sick +1;
             }
        }

        int real = num - sick;
        char csick[100] = "";
        sprintf(csick, "%d", sick);
        char creal[100] = "";
        sprintf(creal, "%d", real);
        printf("I am Ursula.I will have %s patients\n",sz);
        char res[500]="";
        strcat(res,"I am Ursula.");
        strcat(res,csick);
        strcat(res," people sicked, and ");
        strcat(res,creal);
        strcat(res," people vaccinated ");
        mq_send(mqdes,res,MSGSIZE,30);
        abort();
    }
    Sergeant = fork();
    if(Sergeant ==0 )
    {
        sleep(2);
        kill(getppid(),ready);
        char sz[500];
        read(pipefd2[0],sz,sizeof(sz));
        
        int num = atoi(sz);
        int sick = 0;
        srand(getpid());
        for(int i=0;i<num ;i++)
        {
             int a = rand()%100+1;
             if(a<=20)
             {
                 sick = sick +1;
             }
        }

        int real = num - sick;
        char csick[100] = "";
        sprintf(csick, "%d", sick);
        char creal[100] = "";
        sprintf(creal, "%d", real);
        printf("I am Sergeant.I will have %s patients\n",sz);
        char res[500]="";
        strcat(res,"I am Sergeant.");
        strcat(res,csick);
        strcat(res," people sicked, and ");
        strcat(res,creal);
        strcat(res," people vaccinated ");
        mq_send(mqdes,res,MSGSIZE,30);
        abort();
    }

    int st1,st2;

    while(numready !=2)
    {

    }

    int a = atoi(argv[1]);
    char num[100] = "";
    sprintf(num, "%d", a/2);
    write(pipefd[1], num,500); 
    sprintf(num, "%d", a-a/2);
    write(pipefd2[1], num,500); 

    char rcv_buf[MSGSIZE];
    int prio;
    sem_wait(semid);
    for (int i =1;i<=2;i++)
        {
           
            mq_receive(mqdes,rcv_buf,MSGSIZE,&prio);
            printf("%s\n",rcv_buf,prio);

            write_to_file("data.txt",rcv_buf);
            sleep(1);
        }

    waitpid(Ursula, &st1, 0);
    waitpid(Sergeant, &st2, 0);
    
    close(pipefd[0]);
    close(pipefd[1]);
    close(pipefd2[0]);
    close(pipefd2[1]);
    
    mq_close(mqdes);
    mq_unlink(mqname);

    semaphore_delete(sem_name); 

    printf("Office closed\n");


}
