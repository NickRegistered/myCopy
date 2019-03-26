#include "myCopy.h"


void procBar(int *writePipe,int *readPipe){
	int i,writeSize = 0,totalSize;
	read(writePipe[0],&totalSize,sizeof(totalSize));
	close(writePipe[0]);

	double percent = 0.0,tempper = 0.0;
	int blk;
	char bar[21] = {0};
	while(writeSize < totalSize){
		read(readPipe[0],&writeSize,sizeof(writeSize));
		percent = writeSize/(double)totalSize;
		for(i = (percent-tempper)*20;i>=0;i--){
			blk = tempper*20;
			bar[blk] = '=';
			tempper = percent;
		}
		printf("[%-20s] %.2lf%% Total:%dBytes Finished:%dBytes\r",bar,percent*100,totalSize,writeSize);
	}
	close(readPipe[0]);
	putchar('\n');
	return;
}

int main(int argc,char *argv[]){
	if(argc != 3){
		printf("参数格式错误！\n");
		return -1;
	}
	const char *src = argv[1],*dest = argv[2];
	//申请保存管道描述符的共享内存
	pipes *p_pipes = getPipes();
	//创建管道
	int *writePipe,*readPipe;
	writePipe = p_pipes->writePipe;
	readPipe = p_pipes->readPipe;
	if(pipe(writePipe) == -1 || pipe(readPipe) == -1){
		printf("创建管道失败\n");
		return -1;
	}

	union semun arg;
	/**创建信号灯集**/
	int semid = semget(SEM_KEY,2,IPC_CREAT|0666);
	if(semid == -1){
		printf("信号灯集合创建失败\n");
		return -1;
	}

	arg.val = BUFFER_NUM;//信号量0的初值为BUFFER_NUM
	if(-1 == semctl(semid,0,SETVAL,arg)){
		printf("信号量赋值失败\n");
		return -1;
	}
	arg.val = 0;//信号量1的初值为0
	if(-1 == semctl(semid,1,SETVAL,arg)){
		printf("信号量赋值失败\n");
		return -1;
	}

	pid_t p1,p2;
	while((p1 = fork()) == -1);
	if(p1 == 0){//sub1
		execl("./writebuf",src,NULL);
	}
	else{//main
		while((p2 = fork()) == -1);
		if(p2 == 0){//sub2
			execl("./readbuf",dest,NULL);
		}
		else{//main
			procBar(writePipe,readPipe);
			/**等待子进程结束**/
			waitpid(p1,NULL,0);
			waitpid(p2,NULL,0);
			/*删除管道共享内存*/
			delPipes();
			/*删除信号量集合*/
			if(-1 == semctl(semid,0,IPC_RMID,arg)){
				printf("信号量集合删除失败\n");
				return -1;
			}
		}
	}
	return 0;
}
