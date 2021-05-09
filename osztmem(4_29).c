//注意共享内存不会自动阻塞！
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

//https://blog.csdn.net/guoping16/article/details/6584058
int main (int argc,char* argv[]) {

    int pid;
    key_t key;
    int sh_mem_id;
    char *s;
	// key creation
    key=ftok(argv[0],1);
    // create shared memory for reading and writing (500 bytes )
    //第二个参数： 大于0的整数：新建的共享内存大小，以字节为单位
    //第三个参数:  
        //IPC_CREAT：当shmflg&IPC_CREAT为真时，如果内核中不存在键值与key相等的共享内存，则新建一个共享内存；如果存在这样的共享内存，返回此共享内存的标识符
        //IPC_CREAT|IPC_EXCL：如果内核中不存在键值与key相等的共享内存，则新建一个消息队列；如果存在这样的共享内存则报错
        //S_IRUSR S_IWUSR 为权限  此处等于0600
    sh_mem_id=shmget(key,500,IPC_CREAT|S_IRUSR|S_IWUSR);
    // to connect the shared memory, 
    //连接共享内存标识符为shmid的共享内存，连接成功后把共享内存区对象映射到调用进程的地址空间，随后可像本地空间一样访问
    //第二个参数： 指定共享内存出现在进程内存地址的什么位置，直接指定为NULL让内核自己决定一个合适的地址位置
    //第三个参数： SHM_RDONLY：为只读模式，其他为读写模式
    s = shmat(sh_mem_id,NULL,0);
    //
    pid = fork();
    if(pid>0){    
  
       char buffer[] = "Hurray Fradi! \n";                   
       // copy the value into the shared memory 
       strcpy(s,buffer);
       // release the memory
       
       //与shmat函数相反，是用来断开与共享内存附加点的地址，禁止本进程访问此片共享内存
       shmdt(s);	   
//	s[0]='B';  //this causes a segmentation fault 
       wait(NULL); 
	// IPC_RMID- to clear the shared memory
    //第二个参数： 
        //IPC_STAT：得到共享内存的状态，把共享内存的shmid_ds结构复制到buf中
        //IPC_SET：改变共享内存的状态，把buf所指的shmid_ds结构中的uid、gid、mode复制到共享内存的shmid_ds结构内
        //IPC_RMID：删除这片共享内存
       shmctl(sh_mem_id,IPC_RMID,NULL);
    } 
    else 
	if ( pid == 0 ) 
	{
	    sleep(1);	              
        printf("Child: %s",s);
	// it releases the shared memory 
        shmdt(s);
	}

   return 0;
}
