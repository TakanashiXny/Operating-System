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
char *file[MAXPIPE][3]; // 存储重定向前后的指令名  file[][0]: 重定向'<' file[][1]: 重定向'>' file[][2]: 重定向'>>'
char *Argv[MAXPIPE][MAXARGC];
char *argv[MAXARGC]; // 创建字符指针数组，每个元素为指向一个字符串的指针
int argvNum;
char *fShare = "temp.txt"; // 共享文件
pid_t pid;


void sigcat() {
    kill(pid,SIGINT);
}

void commandNoPipe(int left, int right) {
    // 判断重定向
    int in = 0;
    int out = 0;
    int add = 0;
    char *inFile = NULL;
    char *outFile = NULL;
    char *addFile = NULL;
    int endIdx = right;

    for (int i=left; i<right; i++) {
        if (strcmp(argv[i], "<") == 0) {
            in++;
            if (i+1 < right) {
                inFile = argv[i+1];
            } else {
                perror("missing parameter");
                exit(0);
            }
            if (endIdx == right) {
                endIdx = i;
            }
        } else if (strcmp(argv[i], ">") == 0) {
            out++;
            if (i+1 < right) {
                outFile = argv[i + 1];
            } else {
                perror("missing parameter");
                exit(0);
            }
            if (endIdx == right) {
                endIdx = i;
            }
        } else if (strcmp(argv[i], ">>") == 0) {
            add++;
            if (i+1 < right) {
                addFile = argv[i + 1];
            } else {
                perror("missing parameter");
                exit(0);
            }
            if (endIdx == right) {
                endIdx = i;
            }
        }
    }

    // 重定向
    if (in > 1) {
        perror("too much in");
        exit(0);
    }
    if (out > 1) {
        perror("too much out");
        exit(0);
    }
    if (add > 1) {
        perror("too much add");
        exit(0);
    }
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork error");
        exit(0);
    } else if (pid1 == 0) {
        int fd;
        if (in == 1) {
            close(0);
            fd = open(inFile, O_RDONLY);
        }
        if (out == 1) {
            close(1);
            fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, 0666);
        }
        if (add == 1) {
            close(1);
            fd = open(addFile, O_WRONLY|O_CREAT|O_APPEND, 0666);
        }
        char *com[MAXSIZE];
        for (int i=left; i<endIdx; i++) {
            com[i] = argv[i];
        }
        com[endIdx] = NULL;
        if (execvp(com[left], com+left) == -1) {
            perror("execvp error!\n");
            exit(0);
        }

    } else {
        int status;
        waitpid(pid1, &status, 0);
    }
    return;
}

void commandPipe(int left, int right) {
    if (left >= right) {
        return;
    }
    // 判断有无管道
    int pipeIdx = -1;
    for (int i=left; i<right; i++) {
        if (strcmp(argv[i], "|") == 0) {
            pipeIdx = i;
            break;
        }
    }
    if (pipeIdx == -1) {
        commandNoPipe(left, right);
    } else if (pipeIdx+1 == right) {
        perror("missing parameter");
        exit(0);
    } else {
        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe error");
            exit(0);
        }
        pid_t pid1 = fork();
        if (pid1 == -1) {
            perror("fork error");
            exit(0);
        } else if (pid1 == 0) {
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            commandNoPipe(left, pipeIdx);
            exit(0);
        } else {
            int status;
            waitpid(pid1, &status, 0);
            if (pipeIdx+1 < right) {
                close(fd[1]);
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);
                commandPipe(pipeIdx+1, right);
            }
        }
    }
}



int main()
{
    while (1) {
        printf("$ ");
        signal(SIGINT, &sigcat);
        flag_in = 0;
        flag_out = 0;
        flag_add = 0;
        flag_pipe = 0;
        memset(flag, 0, sizeof(flag));
        memset(file, 0, sizeof(file));
        memset(Argv, 0, sizeof(Argv));
        memset(argv, 0, sizeof(argv));
        argvNum = 0;
        char Instruction[MAXSIZE]; // 创建字符数组，用于存放输入的指令
        fgets(Instruction, MAXSIZE, stdin); // 从键盘输入指令
        Instruction[strlen(Instruction)-1] = '\0'; // 确定指令长度，并且将输入的回车设置为'\0'作为字符串的结尾

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

        int fore = 0;
        if (strcmp(argv[ArgvIndex-1], "&") == 0) {
            fore = 1;
        }

        // 记录所有的指令
        char RealInstruction[MAXSIZE] = {'\0'};
        for (int i=0; argv[i] != NULL; i++) {
            strcat(RealInstruction, argv[i]);
            strcat(RealInstruction, " ");
        }
        RealInstruction[strlen(RealInstruction)-1] = '\n';


        if (strcmp(argv[0], "history") != 0) {
            // 记录所有的指令
            char RealInstruction[MAXSIZE] = {'\0'};
            for (int i=0; argv[i] != NULL; i++) {
                strcat(RealInstruction, argv[i]);
                strcat(RealInstruction, " ");
            }
            RealInstruction[strlen(RealInstruction)-1] = '\n';

            char buf[MAXSIZE];
            int i = 0;
            char ch;
            FILE *f = fopen(".bash_history.txt", "r");
            if (f == NULL) {
                exit(1);
            }
            while ((ch = fgetc(f)) != EOF) {
                buf[i++] = ch;
            }
            buf[i] = '\0';
            fclose(f);

            // 将所有指令存入".bash_history"文件中
            f = fopen(".bash_history.txt", "w");
            if (f == NULL) {
                exit(1);
            }
            fprintf(f, "%s", RealInstruction);
            fprintf(f, "%s", buf);
            fclose(f);

        }

        if (strcmp(argv[0], "cd") == 0) {
            chdir(argv[1]);
        } else if (strcmp(argv[0], "history") == 0) {
            /**
             * history函数
             */
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
                printf("%d ", id);
                char ch1 = '\0';
                while ((ch = fgetc(f1)) != EOF && id<=limit) {
                    if (ch1 == '\n') {
                        id++;
                        if (id > limit) {
                            break;
                        }
                        printf("%d ", id);
                    }
                    
                    printf("%c", ch);
                    ch1 = ch;
                }
                printf("\n");
                fclose(f1);
            }

        } else if (strcmp(argv[0], "exit") == 0) {
            break;
        } else if (strcmp(argv[0], "mytop") == 0) {

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
                    if (execvp(Argv[0][0], Argv[0]) == -1) {
                        perror("execvp error!\n");
                        exit(0);
                    }
                } else {
                    int inFd = dup(STDIN_FILENO);
                    int outFd = dup(STDOUT_FILENO);
                    commandPipe(0, ArgvIndex);
                    dup2(inFd, STDIN_FILENO);
                    dup2(outFd, STDOUT_FILENO);
                    exit(0);

                }
            } else {
                int status;
                waitpid(pid, &status, 0);
            }
        }
    }
    return 0;
}
