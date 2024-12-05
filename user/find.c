//
// Created by liulch on 24-10-14.
//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
find(char *path, char *name) {
  char buf[512], *p; // 定义一个缓冲区和一个字符指针
  int fd; // 文件描述符
  struct dirent de; // 目录项结构体
  struct stat st; // 文件状态结构体

  // 尝试以只读模式打开指定路径。如果打开失败，输出错误信息并返回
  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  // 使用 fstat 获取文件或目录的状态信息。如果获取失败，输出错误信息并关闭文件描述符
  if (fstat(fd, &st) < 0) {
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type) { // 根据文件类型进行处理
   case T_FILE:
      // 比较 path 和 name 是否相同
      if (strcmp(de.name,name) == 0) {
        printf("%s/%s\n", path,name); // 如果相同，输出找到的信息
      }
    break;
  case T_DIR: // 如果是目录类型
    // 检查路径长度是否超过缓冲区大小
      if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
        printf("find: path too long\n");
        break;
      }
    strcpy(buf, path); // 复制路径到缓冲区
    p = buf + strlen(buf); // 指针 p 指向缓冲区末尾
    *p++ = '/'; // 在路径末尾添加一个斜杠
    while (read(fd, &de, sizeof(de)) == sizeof(de)) { // 读取目录项
      if (de.inum == 0|| (strcmp(de.name, ".") == 0) || (strcmp(de.name, "..") == 0))
        continue; // 如果 inode 为 0，跳过
      memmove(p, de.name, DIRSIZ); // 将目录项的名字复制到缓冲区
      p[DIRSIZ] = 0; // 添加字符串结束符
      if (stat(buf, &st) < 0) { // 获取目录项状态
        printf("find: cannot stat %s\n", buf);
        continue;
      }
      if (st.type == T_FILE){
        if (strcmp(de.name, name) == 0){
          printf("%s\n", buf);
        }
      }else if (st.type == T_DIR){
        // 如果当前目录是文件夹, 则递归处理, buf为下一级目录
        find(buf, name);
      }
    }
    break;
  }
  close(fd); // 关闭文件描述符
}

int
main(int argc, char *argv[]){
  if (argc != 3){
    fprintf(2, "Please enter a dir and a filename!\n");
    exit(1);
  }else{
    char *path = argv[1];
    char *filename = argv[2];
    find(path, filename);
    exit(0);
  }

}