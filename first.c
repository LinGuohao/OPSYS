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

#define Ready SIGUSR1
#define MSGSIZE 5000
#define MAXMSGS 5
#define VALUE 1234
#define MEMSIZE 1024

int num = 0;
int number = 0;
int apipe[2];

FILE* fp=NULL;

sem_t* semaphore_create(char*name,int semaphore_value)
{
	//第二个参数:oflag参数可以是0、O_CREAT（创建一个信号灯）或O_CREAT|O_EXCL（如果没有指定的信号灯就创建），如果指定了O_CREAT，那么第三个和第四个参数是需要的
	//第四个参数：value 信号灯初始值
    sem_t *semid=sem_open(name,O_CREAT|O_EXCL,S_IRUSR|S_IWUSR,semaphore_value );
	if (semid==SEM_FAILED)
	perror("sem_open");
       
    return semid;
}

void semaphore_delete(char* name)
{
      sem_unlink(name);
}



char **explode(char sep, const char *str, int *size)
{
    int count = 0, i;
    for (i = 0; i < strlen(str); i++)
    {
        if (str[i] == sep)
        {
            count++;
        }
    }

    char **ret = calloc(++count, sizeof(char *));

    int lastindex = -1;
    int j = 0;

    for (i = 0; i < strlen(str); i++)
    {
        if (str[i] == sep)
        {
            ret[j] = calloc(i - lastindex, sizeof(char)); //分配子串长度+1的内存空间
            memcpy(ret[j], str + lastindex + 1, i - lastindex - 1);
            j++;
            lastindex = i;
        }
    }
    //处理最后一个子串
    if (lastindex <= strlen(str) - 1)
    {
        ret[j] = calloc(strlen(str) - lastindex, sizeof(char));
        memcpy(ret[j], str + lastindex + 1, strlen(str) - 1 - lastindex);
        j++;
    }

    *size = j;

    return ret;
}


void handler(int sig)
{
    num = num +1;
    char randomlist[1000] = "";
    //printf("%d",number);
    if(num==2)
    {
        //printf("%d",num);
        srand((unsigned)time(NULL));
        for(int i=0;i<number;i++)
        {
           
                
                int a = rand() %1000;
                char tmp[100]= "";
                sprintf(tmp,"%d",a);
                strcat(randomlist,tmp);
                
                if(i!=number-1)
                {
                    strcat(randomlist,",");
                    
                }
                //printf("%s",randomlist);
        }
        //printf("%s",randomlist);
        write(apipe[1], randomlist,1000);
        //
    }
}

int writetofile(char* c)
{
    time_t ts=0;
	int line=0;
	struct tm* tm=NULL;
	
	
		ts = time(NULL);//获取1970年1月1日00:00:00到现在的时间秒数
		if(ts==-1)
		{
			perror("get-time");
			return -1;
		}
//		printf("%ld \n",ts);
		
		tm = localtime(&ts);//把所获取的时间秒数格式化为日期和时间
		if(tm==NULL)
		{
			perror("local-time");
			return -1;
		}
		//printf("%dyear %02dmonth %02dday %02d: %02d: %02d;\n",
		//		tm->tm_year+1900,tm->tm_mon,tm->tm_mday,
		//		tm->tm_hour,tm->tm_min,tm->tm_sec);//再屏幕上标准输出所获取的日期和时间
	
		line++;//标记行号，每次+1行
		fprintf(fp,"LINE:%s: %dyear %02dmonth %02dday %02d: %02d: %02d;\n",c,
				tm->tm_year+1900,tm->tm_mon,tm->tm_mday,
				tm->tm_hour,tm->tm_min,tm->tm_sec);//在打开的time.txt文件中写入日期和时间

		fflush(fp);//刷新缓冲区，让存在缓冲区的数据写入文件中
		//sleep(1);//眼石1秒
		

	
	
	
		return 0;
}


key_t key;
sem_t* semid;
char* sem_name="/sem";

void timerhandler()
{
    
    semid= semaphore_create(sem_name,1);

}

int main(int argc, char **argv)
{  
    number = atoi(argv[1]);
    int fd;
    //printf("%d",number);
    int fid=mkfifo("fifo.ftc", S_IRUSR|S_IWUSR ); 
    signal(Ready,handler);
    if(argc<2)
    {
        exit(EXIT_FAILURE);
    }
    pid_t checking, sealdown;
    if (fid==-1)
    {
	printf("Error number: %i",errno);
	exit(EXIT_FAILURE);
    }
    if (pipe(apipe) == -1)
    {
        perror("Opening error!");
        exit(EXIT_FAILURE);
    }
    //printf("here");
     char * mqname = "/iw3xv9";
    char rcv_buf[MSGSIZE];
    mqd_t mqdes;
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSGSIZE;
    mq_unlink(mqname);
    mqdes = mq_open(mqname,O_CREAT|O_RDWR , 0777,&attr);
    char *s;
  
    fp=fopen("./time.txt","w+");//打开（创建）time.txt文件
	if(fp==NULL)//判断是否打开成功
	{
		perror("fopen");
		return -1;
	}
	

    key=ftok(argv[0],1);
    int sh_mem_id;
    sh_mem_id=shmget(key,MEMSIZE,IPC_CREAT|S_IRUSR|S_IWUSR);
    s = shmat(sh_mem_id,NULL,0);
    
    
    semid = semaphore_create(sem_name,1);
    //定时器
    struct sigaction sa;
    struct itimerval timer;

    
    memset(&sa, 0, sizeof (sa));
    sa.sa_handler = &timerhandler;
    sigaction (SIGVTALRM, &sa, NULL);

   
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    
    timer.it_interval.tv_sec = 5;
    timer.it_interval.tv_usec = 0;
    

    setitimer (ITIMER_VIRTUAL, &timer, NULL);


    
    checking = fork();
    if(checking == 0)
    {   
        //child
        //printf("here\n");
        sleep(1);
        kill(getppid(),Ready);
        //printf("%d",num);
        sleep(2);
        char random[100] = "";
        read(apipe[0],random,1000); 
        //sleep(1);
        //printf("here");
        printf("%s\n",random);
        srand((unsigned)time(NULL));
        int size;
        char **ret = explode(',', random, &size);
        char numberwithchecked[5000] = "";
        for(int i=0;i<number;i++)
        {
             int a = rand() %99;
             
             if(a<20)
             {
                 strcat(ret[i]," cannot vote");
             }else
             {
                 strcat(ret[i]," can vote");
             }
             strcat(numberwithchecked,ret[i]);

             if(i!=number-1)
             {
                 strcat(numberwithchecked,",");
             }
        }
         
        fd=open("fifo.ftc",O_WRONLY);
        write(fd,numberwithchecked,5000);
        close(fd);
        
    

        //fflush(stdout); 
        //printf("here");
        sleep(2);
        sem_wait(semid);
        //printf("1111\n");
        sem_close(semid);
        writetofile("1");
        //sleep(3);
        abort();
    }
    sealdown = fork();
    if(sealdown == 0)
    {
        //child
        //printf("%d",num);
        sleep (6);
        kill(getppid(),Ready);
        //printf("%d",num);
        
        fd=open("fifo.ftc",O_RDONLY);
        char numberwithchecked[5000] = "";
        read(fd,numberwithchecked,sizeof(numberwithchecked));
	    printf("%s\n",numberwithchecked);
	    close(fd);
	    unlink("fifo.ftc");
        
        int size;
        char **ret = explode(',', numberwithchecked, &size);
        char numberwithvotes[5000];
        srand((unsigned)time(NULL));
        for(int i=0;i<size;i++)
        {   
            int size2;
            char** pickupbool = explode(' ', ret[i], &size2);
            if(strcmp("can",pickupbool[1])==0)
            {
                int a = rand()%5+1;
                char tmp[100]= "";
                sprintf(tmp,"%d",a);
                strcat(pickupbool[0]," ");
                strcat(pickupbool[0],tmp);
                strcat(numberwithvotes,pickupbool[0]);
                 if(i!=number-1)
             {
                 strcat(numberwithvotes,",");
             }
            }

            
        }
         mqdes = mq_open(mqname, O_CREAT|O_RDWR,0777,&attr);
         mq_send(mqdes,numberwithvotes,MSGSIZE,30);
        mq_close(mqdes);
        sem_wait(semid);
         //sleep(1);
         writetofile("2");
        sem_close(semid);
        
        //sleep(1);
        
        abort();
        
    }
    pause();
    pause();
    sleep(3);
    
    
     int prio;
    mq_receive(mqdes,rcv_buf,MSGSIZE,&prio);
    printf("%s\n",rcv_buf);
    int st1,st2;
    waitpid(checking, &st1, 0);
    waitpid(sealdown, &st2, 0);
    close(apipe[0]);
    close(apipe[1]);
    sem_wait(semid);
     //sleep(1);
    writetofile("3");
    sem_close(semid);
    //sleep(2);
    fclose(fp);//关闭打开的time.txt文件
    
	shmctl(sh_mem_id,IPC_RMID,NULL);
    return 0;
    


}