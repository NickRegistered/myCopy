#include "myCopy.h"

int main(int argc,char *argv[]){
  int fdSrc;
  fdSrc = open(argv[0],O_RDONLY);
	if(fdSrc == -1){
		printf("打开 %s 文件失败!\n",argv[0]);
		return -1;
	}

  struct pipes *p_pipes = getPipes();
  int *writePipe;
  writePipe = p_pipes->writePipe;
  //获取信号灯ID
  int semid = semget(SEM_KEY,2,IPC_CREAT|0666);
  struct stat sbuf;
  fstat(fdSrc,&sbuf);
  int totalSize = sbuf.st_size;
  write(writePipe[1],&totalSize,sizeof(totalSize));
  close(writePipe[1]);

	node *p_node = getBuff();
	int len;
	while(true){
		P(semid,0);
		len = read(fdSrc,p_node->buf->content,BUFFER_SIZE);
		p_node->buf->len = len;
		if(len == 0 || len < BUFFER_SIZE){
			V(semid,1);
      close(fdSrc);
      delBuff(p_node);
      delPipes();
			return 0;
		}
		V(semid,1);
		p_node = p_node->next;
	}
}
