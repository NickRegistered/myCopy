#include "myCopy.h"

int main(int argc,char *argv[]){

  int fdDest;
  fdDest = open(argv[0],O_WRONLY|O_CREAT,0666);
  if(fdDest == -1){
    printf("打开 %s 文件失败！\n",argv[0]);
    return -1;
  }

	int writeSize = 0;
  node *p_node = getBuff();
  pipes *p_pipes = getPipes();
  int *readPipe;
  readPipe = p_pipes->readPipe;
  int semid = semget(SEM_KEY,2,IPC_CREAT|0666);
  if(semid == -1){
    printf("获取信号灯失败!\n");
    return -1;
  }

	while(true){
		P(semid,1);
		write(fdDest,p_node->buf->content,p_node->buf->len);
    if(p_node->buf->len < BUFFER_SIZE && p_node->buf->len != 0){
			writeSize += p_node->buf->len;
			V(semid,0);
      write(readPipe[1],&writeSize,sizeof(writeSize));
			close(readPipe[1]);
      close(fdDest);
      delBuff(p_node);
      return 0;
		}
		writeSize += p_node->buf->len;
		V(semid,0);
		write(readPipe[1],&writeSize,sizeof(writeSize));
    p_node = p_node->next;
	}
}
