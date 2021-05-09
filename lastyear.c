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

#define nod SIGUSR1
#define MSGSIZE 500
#define MAXMSGS 5
#define VALUE 1234

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


void handler(int sig)
{
    printf("I am ready to start\n");
    numready = numready+1;
}

int main(int argc, char **argv)
{
    pid_t Police , Physician;
    signal(nod,handler);
    int pipefd[2],pipefd2[2]; // unnamed pipe file descriptor array
    if (pipe(pipefd) ==-1|| pipe(pipefd2)==-1) 
	{
        perror("Opening error!");
        exit(EXIT_FAILURE);
    }


    char * mqname = "/iw3xv9";
    mqd_t mqdes;
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSGSIZE;
    mq_unlink(mqname);
    mqdes = mq_open(mqname,O_CREAT|O_RDWR , 0777,&attr);


    key_t key;
    int sh_mem_id;
    char *s;
    key=ftok(argv[0],1);
    sh_mem_id=shmget(key,500,IPC_CREAT|S_IRUSR|S_IWUSR);
    s = shmat(sh_mem_id,NULL,0);


    sem_t* semid;
    char* sem_name="/apple";
    semid = semaphore_create(sem_name,0);


    Police = fork(); 
    if(Police == 0)
    {
        sleep(1);
        kill(getppid(),nod);
        char sz[500];
        sleep(2);
        //printf("here\n");
        read(pipefd[0],sz,sizeof(sz));
        //printf("here2\n");
        write(pipefd2[1],"Yes, it is compulsory to wear the mask when you leave your flat!",500);
        printf("%s\n",sz);
        abort();
    }
    Physician = fork();
    if(Physician ==0)
    {
        sleep(2);
        kill(getppid(),nod);
        sleep(3);
        mq_send(mqdes,"To wear a mask is really very important to save other people and ourself against the virus in the shops and on the roads",MSGSIZE,30);
        int a;
        srand((unsigned)time(NULL));
        a = rand()%1000;
        //printf("%d\n",a);
        char num[100] = "";
        sprintf(num, "%d", a);
        //printf("%s\n",num);
        char res[500]="The number of new infected people: ";
        strcat(res,num);
        strcpy(s,res);
        sem_post(semid);	
        shmdt(s);

        abort();
    }

    int st1,st2;

    while(numready !=2)
    {

    }
    
    write(pipefd[1], "Is it compulsory to wear a mask in the shops?",500); 
    char sz[500];
    read(pipefd2[0],sz,sizeof(sz));
    printf("%s\n",sz);
    //sleep(6);
    
    char rcv_buf[MSGSIZE];
    int prio;
    mq_receive(mqdes,rcv_buf,MSGSIZE,&prio);
    printf("%s\n",rcv_buf);


    sem_wait(semid);
    printf("%s\n",s);
    shmdt(s);
    



    waitpid(Police, &st1, 0);
    waitpid(Physician, &st2, 0);
    
    close(pipefd[0]);
    close(pipefd[1]);
    close(pipefd2[0]);
    close(pipefd2[1]);
    
    mq_close(mqdes);
    mq_unlink(mqname);

    shmctl(sh_mem_id,IPC_RMID,NULL);


    semaphore_delete(sem_name); 


}