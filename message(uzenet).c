#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <sys/types.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <wait.h> 

struct messg { 
     long mtype;//message type
     char mtext [ 1024 ]; //this is the message itself
}; 

// sendig a message
int send( int mqueue ) 
{ 
     const struct messg m = { 5, "Hurray Fradi!" }; 
     int status; 
     //第一个参数 消息队列标识符
     //第二个参数 发送给队列的消息。msgp可以是任何类型的结构体，但第一个字段必须为long类型，即表明此发送消息的类型，msgrcv根据此接收消息
     //第三个参数 要发送消息的大小，不含mtype占用的4个字节,即mtext的长度
     //第四个参数 
               //0：当消息队列满时，msgsnd将会阻塞，直到消息能写进消息队列
               //IPC_NOWAIT：当消息队列已满的时候，msgsnd函数不等待立即返回
               //IPC_NOERROR：若发送的消息大于size字节，则把该消息截断，截断部分将被丢弃，且不通知发送进程。
     status = msgsnd( mqueue, &m, strlen ( m.mtext ) + 1 , 0 ); 
	// a 3rd parameter may be : sizeof(m.mtext)
     	// a 4th parameter may be  IPC_NOWAIT, equivalent with  0-val 
     if ( status < 0 ) 
          perror("msgsnd error"); 
     return 0; 
} 
     
// receiving a message. 
int receive( int mqueue ) 
{ 
     struct messg m; 
     int status; 
     // the last parameter is the number of the message
	// if it is 0, then we read the first one
	// if  >0 (e.g. 5), then message type 5 will be read
	 
      //第三个参数 
          //0：接收第一个消息
          //>0：接收类型等于msgtype的第一个消息
          //<0：接收类型等于或者小于msgtype绝对值的第一个消息
     //第四个参数
          //0: 阻塞式接收消息，没有该类型的消息msgrcv函数一直阻塞等待
          //IPC_NOWAIT：如果没有返回条件的消息调用立即返回，此时错误码为ENOMSG
          //IPC_EXCEPT：与msgtype配合使用返回队列中第一个类型不为msgtype的消息
          //IPC_NOERROR：如果队列中满足条件的消息内容大于所请求的size字节，则把该消息截断，截断部分将被丢弃
     //返回值 ：
          //成功：实际读取到的消息数据长度
          //出错：-1，错误原因存于error中
     status = msgrcv(mqueue, &m, 1024, 5, 0 ); 
     
     if ( status < 0 ) 
          perror("msgsnd error"); 
     else
          printf( "The code of the message is : %ld, text is:  %s\n", m.mtype, m.mtext ); 
     return 0; 
} 

int main (int argc, char* argv[]) { 
     pid_t child; 
     int messg, status; 
     key_t key; 
     
     // msgget needs a key, created by  ftok  
     //. 
     key = ftok(argv[0],1); 
     printf ("The key: %d\n",key);
     //创建或获得消息队列    key是用来判断是否已有消息队列用的 以全新600创建  IPC_CREAT已有则返回那个消息队列的标识符 没有则创建
     messg = msgget( key, 0600 | IPC_CREAT ); 
     if ( messg < 0 ) { 
          perror("msgget error"); 
          return 1; 
     } 
     
     child = fork(); 
     if ( child > 0 ) { 
          send( messg );  // Parent sends a message. 
          wait( NULL ); 
          // After terminating child process, the message queue is deleted. 
          
          //IPC_RMID：从内核中删除msgqid标识的消息队列
          //返回值:
               //成功：0
               //出错 ：-1
          status = msgctl( messg, IPC_RMID, NULL ); 
          if ( status < 0 ) 
               perror("msgctl error"); 
          return 0; 
     } else if ( child == 0 ) { 
          return receive( messg ); 
          // The child process receives a message. 
     } else { 
          perror("fork error"); 
          return 1; 
     } 
     
     return 0; 
} 
