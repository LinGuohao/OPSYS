//gcc -lrt 
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

void handler(int signumber){
  printf("Signal with number %i has arrived\n",signumber);
  printf("Signal handler ends \n");
}

int main(){

  struct sigaction sigact;
  sigact.sa_handler=handler; //SIG_DFL,SIG_IGN
  sigemptyset(&sigact.sa_mask); //during execution of handler these signals will be blocked plus the signal    
  //now only the arriving signal, SIGTERM will be blocked
  //sigact.sa_flags=0; //the handler execution interrupts the process - real time signal
  //sleep is not a real time safe function
  //只有中断信号的SA_RESTART标志有效时，系统调用才会自动重启 防止慢调用异常退出
  sigact.sa_flags=SA_RESTART; //the interrupted signal will restart!!! 
  sigaction(SIGUSR1,&sigact,NULL);
  //SIGUSR1 normal signal *** ONLY ONE is delivered
  //1. parameter the signal number
  //2. parameter the new sigaction with handler and blocked signals during the execution of handler (sa_mask) and a 
  //special sa_flags - it change the behavior of signal, 
  //e.g. SIGNOCLDSTOP - after the child process ended it won't send a signal to the parent 
  //3. parameter - &old sigset or NULL. 
  //If there is a variable, the function will fill with the value of formerly set sigset
 //*************************
 //real time signals between SIGRTMIN - SIGRTMAX 32-64
 //Several delivery
 //实时信号 无固定含义  小的实时信号会优先执行
 //************************************************  
  sigaction(SIGRTMIN,&sigact,NULL);
 
  pid_t child=fork();
  if (child>0)
  {
    printf("The program comes back from suspending\n");
    int status;
    wait(&status);
    printf("Parent process ended\n");
  }
  else 
  {
    printf("Waits 1 seconds, then send 5 SIGUSR %i signals and 5 SIGRTMIN %i signals \n", SIGUSR1,SIGRTMIN);
    //real time signals are between SIGRTMIN and SIGRTMAX
    //the smalest real time signal will be executed first
    //the execution order of normal signals is not predefined
    
    printf("If multiple copies of a normal signal is pending, only once of them will be executed\n ");
    printf("Each of real time signals will be executed - the order of execution is the same as the calling order\n ");
    sleep(1);
    int i;
     
    //多个normal信号只会处理一次 而实时信号会处理多次 所以下面只会打出一个10 而打出5个 34
    for (i=0;i<5;i++){
      kill(getppid(),SIGUSR1);
      kill(getppid(),SIGRTMIN);
//      sleep(2);  //if sleep is working, then sigusr1 has done before the next call performs
    }
    printf("Child process ended\n");  
  }
  return 0;
}

/*

The program comes back from suspending
Waits 1 seconds, then send 5 SIGUSR 10 signals and 5 SIGRTMIN 34 signals 
If multiple copies of a normal signal is pending, only once of them will be executed
 Each of real time signals will be executed - the order of execution is the same as the calling order
 Child process ended
Signal with number 34 has arrived
Signal handler ends 
Signal with number 34 has arrived
Signal handler ends 
Signal with number 34 has arrived
Signal handler ends 
Signal with number 34 has arrived
Signal handler ends 
Signal with number 34 has arrived
Signal handler ends 
Signal with number 10 has arrived
Signal handler ends 
Parent process ended
*/