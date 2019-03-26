#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>


#define BUFFER_NUM 10	//缓冲区个数
#define BUFFER_SIZE 10000	//缓冲区大小
#define BUFF_KEY 2222		//申请共享缓冲区的键值
#define PIPE_KEY 3333		//管道所在共享内存键值
#define SEM_KEY 4444		//信号灯KEY值

void P(int semid,int index){
	struct sembuf sops = {index,-1,0};
	semop(semid,&sops,1);
}

void V(int semid,int index){
	struct sembuf sops = {index,+1,0};
	semop(semid,&sops,1);
}

union semun{
	int val;							/*value for SETVAL*/
	struct semid_ds *buf; 	/*buffer for IPC_STAT,IPC_SET*/
	unsigned short *arry;	/*array for GETALL,SETALL*/
	struct seminfo *__buf;	/*buffer for IPC_INFO*/
};

struct pipes{
		int writePipe[2],readPipe[2];
};

int shPipeID;
pipes *getPipes(){
  void *shm = NULL;
	shPipeID = shmget(PIPE_KEY,sizeof(pipes),0666|IPC_CREAT);
  if(shPipeID == -1){
		printf("shmget errno :%d %s\n",errno,strerror(errno));
    return (pipes*)-1;
  }
  shm = shmat(shPipeID,(void*)0,0);
  return (pipes*)shm;
}

void delPipes(){
	if (shmctl(shPipeID,IPC_RMID,NULL) == -1) {
		printf("pipeid: %d shmctl errno :%d %s\n",shPipeID,errno,strerror(errno));
		printf("删除管道共享内存失败\n");
		return;
	}
}

struct buffer{
	int len;//实际字符个数
	char content[BUFFER_SIZE];//缓冲区
};

struct node{
	buffer *buf;
	node *next;
};

int shBuffID[BUFFER_NUM];
node *getBuff(){
	void *shm = NULL;
	int i;
	node *head,*tail,*p_node;
	/**创建环形缓冲链表**/
	for(i=0;i<BUFFER_NUM;++i){
		shBuffID[i] = shmget(BUFF_KEY+i,sizeof(buffer),0666|IPC_CREAT);
		if(shBuffID[i] == -1){
			printf("shmget errno :%d %s\n",errno,strerror(errno));
			return (node*)-1;
		}
		shm = shmat(shBuffID[i],(void*)0,0);
		if(shm == (void*)-1) return (node*)shm;

		p_node = (node*)malloc(sizeof(node));

		p_node->buf = (buffer*)shm;
		p_node->next = head;
		head = p_node;
		if(i == 0) tail = head;
	}tail->next = head;
	return head;
}

void delBuff(node *head){
	for(int i=0;i<BUFFER_NUM;++i){
		if(shmctl(shBuffID[i],IPC_RMID,NULL) == -1){
			printf("bufid:%d shmctl errno :%d %s\n",shBuffID[i],errno,strerror(errno));
			printf("删除共享缓冲区失败\n");
			return;
		}
	}
}
