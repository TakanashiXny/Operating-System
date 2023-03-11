#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>


#define MAXSIZE 1024 // 指令最大长度
#define MAXARGC 128 // 指令参数最大数量
#define MAXPIPE 128


int flag_in, flag_out, flag_add, flag_pipe; // 分别代表"<"，">"，">>"，"|"
int flag[MAXPIPE][3]; // flag[][0]: 重定向'<' flag[][1]: 重定向'>' flag[][2]: 重定向'>>'
char *file[MAXPIPE][3]; //  存储重定向前后的指令名  file[][0]: 重定向'<' file[][1]: 重定向'>' file[][2]: 重定向'>>'
char *Argv[MAXPIPE][MAXARGC];
char *fShare = "temp.txt"; // 共享文件
pid_t pid;

void sigcat(){
    kill(pid,SIGINT);
}

int main()
{
    while (1) {
        signal(SIGINT, &sigcat);
        flag_in = 0;
        flag_out = 0;
        flag_add = 0;
        flag_pipe = 0;
        memset(flag, 0, sizeof(flag));
        memset(file, 0, sizeof(file));
        memset(Argv, 0, sizeof(Argv));
        char Instruction[MAXSIZE]; // 创建字符数组，用于存放输入的指令
        fgets(Instruction, MAXSIZE, stdin); // 从键盘输入指令
        Instruction[strlen(Instruction)-1] = '\0'; // 确定指令长度，并且将输入的回车设置为'\0'作为字符串的结尾

        char *argv[MAXARGC]; // 创建字符指针数组，每个元素为指向一个字符串的指针
        int ArgvIndex = 0;
        char *pInstruction = Instruction;

        // 将指令的每一个参数分离，并存入argv数组
        while (*pInstruction != '\0') { // 循环终止条件为遍历到指令的最后一个字符'\0'
            /**
             * 字符串中的空格起到分离参数的作用，在本段程序中用于判断是否遍历到有效字符
             * 空格后的第一个字符为参数的起始位置，将字符指针指向该位置时可存储整个参数
             * 在这一参数的最后设置一个'\0'可彻底将此参数与其余参数分离
             * 在遇到空格时，直接向后遍历即可
             */
            if (*pInstruction != ' ') {
                argv[ArgvIndex++] = pInstruction;

                while (*pInstruction != ' ' && *pInstruction != '\0') {
                    pInstruction++;
                }

                *pInstruction = '\0';
            }
            pInstruction++;
        }
        if (ArgvIndex == 0) {
            continue; // 无输入则直接准备接收下一条指令
        }
        argv[ArgvIndex] = NULL; // 设置字符指针数组的终点

        if (strcmp(argv[0], "history") != 0) {
            // 记录所有的指令
            char RealInstruction[MAXSIZE] = {'\0'};
            for (int i=0; argv[i] != NULL; i++) {
                strcat(RealInstruction, argv[i]);
                strcat(RealInstruction, " ");
            }
            RealInstruction[strlen(RealInstruction)-1] = '\n';

            // 将所有指令存入".bash_history"文件中
            FILE* f = fopen(".bash_history.txt", "a+");
            if (f == NULL) {
                exit(1);
            }
            fprintf(f, "%s", RealInstruction);
            fclose(f);
        }

        if (strcmp(argv[0], "cd") == 0) {
            chdir(argv[1]);
        } else if (strcmp(argv[0], "history") == 0) {
            if (argv[1] == NULL) {
                FILE *f1 = fopen(".bash_history.txt", "r");
                if (f1 == NULL) {
                    exit(1);
                }
                char ch;
                int id = 1;
                printf("%d ", id++);
                char ch1 = '\0';
                while ((ch = fgetc(f1)) != EOF) {
                    if (ch1 == '\n') {
                        printf("%d ", id++);
                    }
                    printf("%c", ch);
                    ch1 = ch;
                }
                fclose(f1);
            } else {
                int limit = atoi(argv[1]);
                FILE *f1 = fopen(".bash_history.txt", "r");
                if (f1 == NULL) {
                    exit(1);
                }
                char ch;
                int id = 1;
                printf("%d ", id++);
                char ch1 = '\0';
                while ((ch = fgetc(f1)) != EOF && id<=limit) {
                    if (ch1 == '\n') {
                        printf("%d ", id++);
                    }
                    printf("%c", ch);
                    ch1 = ch;
                }
                printf("\n");
                fclose(f1);
            }
        } else if (strcmp(argv[0], "exit") == 0) {
            break;
        } else if (strcmp(argv[0], "mytop") == 0){

        } else {
            int a = 0; // 当前命令参数序号

            // 遍历argv数组，将'<', '>', '>>', '|'提取出来
            for (int i=0; argv[i] != NULL; i++) {
                if (strcmp(argv[i], "|") == 0) {
                    Argv[flag_pipe++][a] = NULL;
                    a = 0;
                } else if (strcmp(argv[i], "<") == 0) {
                    flag[flag_pipe][0] = 1;
                    file[flag_pipe][0] = argv[i+1];
                    Argv[flag_pipe][a++] = NULL;
                } else if (strcmp(argv[i], ">") == 0) {
                    flag[flag_pipe][1] = 1;
                    file[flag_pipe][1] = argv[i+1];
                    Argv[flag_pipe][a++] = NULL;
                } else if (strcmp(argv[i], ">>") == 0) {
                    flag[flag_pipe][2] = 1;
                    file[flag_pipe][2] = argv[i+1];
                    Argv[flag_pipe][a++] = NULL;
                } else {
                    Argv[flag_pipe][a++] = argv[i];
                }
            }


            /**
             * 执行指令
             */
             pid = fork();
             if (pid < 0) {
                 perror("fork error\n");
                 exit(0);
             } else if (pid == 0) {
                 // 没有管道
                 if (flag_pipe == 0) {
                     if (flag[0][0] != 0) {
                         close(0);
                         int fd = open(file[0][0], O_RDONLY);
                     }
                     if (flag[0][1] != 0) {
                         close(1);
                         int fd2 = open(file[0][1], O_WRONLY|O_CREAT|O_TRUNC, 0666);
                     }
                     if (flag[0][2] != 0) {
                         close(1);
                         int fd3 = open(file[0][2], O_WRONLY|O_CREAT|O_APPEND, 0666);
                     }
                     execvp(Argv[0][0], Argv[0]);
                 } else {
                     // 有管道
                     int current = 0; // 记录当前命令序号
                     for (; current<=flag_pipe; current++) {
                         pid_t pid2 = fork();
                         if (pid2 < 0) {
                             perror("fork error\n");
                             exit(0);
                         } else if (pid2 == 0) {
                            if (current != 0) {
                                close(0);
                                int fd = open(fShare, O_RDONLY);
                            }
                            if (flag[current][0] != 0) {
                                close(0);
                                int fd = open(file[current][0], O_RDONLY);
                            }
                            if (flag[current][1] != 0) {
                                close(1);
                                int fd=open(file[current][1],O_WRONLY|O_CREAT|O_TRUNC,0666);
                            }
                            if (flag[current][2] != 0) {
                                close(1);
                                int fd=open(file[current][1],O_WRONLY|O_CREAT|O_APPEND,0666);
                            }
                            close(1);
                            remove(fShare);
                            int fd=open(fShare,O_WRONLY|O_CREAT|O_TRUNC,0666);
                            if(execvp(Argv[current][0],Argv[current])==-1) {
                                perror("execvp error!\n");
                                exit(0);
                            }
                         } else {
                             waitpid(pid2, NULL, 0);
                         }
                     }
                     close(0);
                     int fd=open(fShare,O_RDONLY);//输入重定向
                     if(flag[current][1]) {
                         close(1);
                         int fd=open(file[current][1],O_WRONLY|O_CREAT|O_TRUNC,0666);
                     }
                     execvp(Argv[current][0],Argv[current]);
                 }
             } else {
                 waitpid(pid, NULL, 0);
             }
        }
    }

    return 0;
}
