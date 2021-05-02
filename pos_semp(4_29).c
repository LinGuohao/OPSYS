//*********************************
// Compile with -pthread 
//*****************************
https://blog.csdn.net/lamdoc/article/details/8787548?utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromMachineLearnPai2%7Edefault-11.control&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromMachineLearnPai2%7Edefault-11.control

#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#define MEMSIZE 1024

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

int main (int argc,char* argv[]) {

    pid_t child;
    key_t key;
    int sh_mem_id;
    sem_t* semid;
    char *s;
    char* sem_name="/apple";

    key=ftok(argv[0],1);
    sh_mem_id=shmget(key,MEMSIZE,IPC_CREAT|S_IRUSR|S_IWUSR);
    s = shmat(sh_mem_id,NULL,0);

    semid = semaphore_create(sem_name,0);

    child = fork();
    if(child>0)	// parent
	{    
       	char buffer[] = "I like Illes pop group!\n";
	printf("Parent starts and writes into the shared memory: %s\n",buffer);
       	sleep(4); // wait for a few seconds             
       	strcpy(s,buffer);	// copy data to shared memory
	printf("Parent:, semaphore up!\n");
       	sem_post(semid);	// semaphore up
	shmdt(s);	// release shared memory	
       	wait(NULL);       
       	semaphore_delete(sem_name);
	shmctl(sh_mem_id,IPC_RMID,NULL);
    	} 
	else 
	if ( child == 0 ) //child
		{	
       		/*  critical section   */
		printf("Child tries to close semaphore!\n");
		//函数sem_wait( sem_t *sem )被用来阻塞当前线程直到信号量sem的值大于0，解除阻塞后将sem的值减一
       		sem_wait(semid);	// semaphore down
       		printf("Child: the result is: %s",s);  
       		sem_post(semid);      
       		/*  end of critical section  */  	  
       		shmdt(s);
    		}

   return 0;
}


