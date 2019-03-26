# myCopy
Linux下通过共享缓存构建环形缓冲区以实现文件拷贝

![环形缓冲示意图](https://github.com/NickRegistered/myCopy/blob/master/pictures/%E7%8E%AF%E5%BD%A2%E7%BC%93%E5%86%B2%E7%A4%BA%E6%84%8F%E5%9B%BE.png)

采用三个进程实现，父进程进行复制进度的显示，writebuf进程读入文件并写进缓冲区，readbuf从缓冲区读出文件并写入文件。

使用到的进程之间通信方式有：

  &emsp;1.管道，子进程通过管道将文件读写的字节数通知父进程，便于显示复制进度。
  
  &emsp;2.信号灯，对writebuf与readbuf在读写缓冲区的行为上进行同步。
  
  &emsp;3.共享内存，构建环形缓冲区。
