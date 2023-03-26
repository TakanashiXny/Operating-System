

# Shell说明文档

- [Shell说明文档](#shell说明文档)
  - [实现功能](#实现功能)
  - [实现思路](#实现思路)
  - [实现过程](#实现过程)
    - [1. 解析指令](#1-解析指令)
      - [全局变量申明](#全局变量申明)
      - [实现代码](#实现代码)
      - [代码说明](#代码说明)
    - [2. 记录指令](#2-记录指令)
      - [全局变量声明](#全局变量声明)
      - [实现代码](#实现代码-1)
      - [代码说明](#代码说明-1)
    - [3. 处理内置命令](#3-处理内置命令)
      - [实现代码](#实现代码-2)
      - [代码说明](#代码说明-2)
    - [4. 处理没有管道的命令](#4-处理没有管道的命令)
      - [全局变量声明](#全局变量声明-1)
      - [实现代码](#实现代码-3)
      - [代码说明](#代码说明-3)
    - [5. 处理有管道的命令](#5-处理有管道的命令)
      - [实现代码1](#实现代码1)
      - [代码说明1](#代码说明1)
      - [实现代码2](#实现代码2)
      - [代码说明2](#代码说明2)
    - [6. 处理后台运行](#6-处理后台运行)
      - [实现代码](#实现代码-4)
      - [代码说明](#代码说明-4)


## 实现功能

1. 执行内置命令
2. 执行program命令
3. 重定向
4. 管道（多重管道）

## 实现思路

1. 整个程序为无限循环体
2. 解析输入的指令，将它们按照空格进行分割
3. 区分内置命令与program命令
4. 如果是program命令则考虑重定向与管道

## 实现过程

### 1. 解析指令

#### 全局变量申明

```c
#define MAXSIZE 1024 // 指令最大长度
char *argv[MAXARGC]; // 创建字符指针数组，每个元素为指向一个字符串的指针
```

#### 实现代码

```C
char Instruction[MAXSIZE]; // 创建字符数组，用于存放输入的指令
fgets(Instruction, MAXSIZE, stdin); // 从键盘输入指令
Instruction[strlen(Instruction)-1] = '\0'; // 确定指令长度，并且将输入的回车设置为'\0'作为字符串的结尾

int ArgvIndex = 0; // 指令块数
char *pInstruction = Instruction;

// 将指令的每一个参数分离，并存入argv数组
while (*pInstruction != '\0') { // 循环终止条件为遍历到指令的最后一个字符'\0'
    /**
             * 字符串中的空格起到分离参数的作用，在本段程序中用于判断是否遍历到有效字符6t
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
```

#### 代码说明

1. 使用`Instruction`数组用于存放输入的字符串，需要注意将最后一位的`\n`置为`'\0'`.
2. 根据空格来对于输入的字符串进行分割，以此来区分指令以及参数，使用`argv`数组进行存储。
3. 如果输入为空，则无需执行，直接进行下一轮循环。
4. 最终需要将字符串数组的最后一位置为`NULL`，以便于后续的循环遍历的终止。



### 2. 记录指令

这一过程是针对于`history n`指令而执行的。这一指令需要将最近的若干条指令输出，所以需要将所有的输入存储在一个数组中。

#### 全局变量声明

```C
char allInstruction[MAXSIZE][MAXSIZE]; // 二维字符数组，用于存储所有的字符串
int NumInstruction; // 已经处理的指令数量
```

#### 实现代码

```C
char RealInstruction[MAXSIZE] = {'\0'};
for (int i=0; argv[i] != NULL; i++) {
    strcat(RealInstruction, argv[i]);
    strcat(RealInstruction, " ");
}
RealInstruction[strlen(RealInstruction)-1] = '\0';

for (int i=0; i<MAXSIZE; i++) {
    allInstruction[NumInstruction][i] = RealInstruction[i];
}
NumInstruction++;
```

#### 代码说明

1. 设置局部变量`RealInstruction[MAXSIZE]`来存储修正后（去除多余的空格）的当前指令。
2. 将`argv`数组中的每一项连接在`RealInstruction`后来拼接字符串，并将最后一个空格设置为`'\0'`.
3. 通过循环遍历，将`RealInstruction`中的每个字符复制到`allInstruction`数组中的相应行
4. 最后将`NumInstruction`计数器加1.



### 3. 处理内置命令

内置命令包括`cd`，`history n`，`exit`，`mytop`. 此处实现了前三个内置命令。

#### 实现代码

```C
if (strcmp(argv[0], "cd") == 0) {
    chdir(argv[1]);
} else if (strcmp(argv[0], "history") == 0) {
    /**
                 * history函数
                 */
    int limit = atoi(argv[1]);
    int k=1;
    while (k<=limit && NumInstruction-k>=0) {
        printf("%d %s\n", k, allInstruction[NumInstruction-k]);
        k++;
    }
} else if (strcmp(argv[0], "exit") == 0) {
    exit(0);
}
```

#### 代码说明

1. 执行`cd`指令时，只需要调用内置命令`chdir`系统调用即可。
2. 执行`history n`命令时，需要使用`atoi()`函数得到需要输出的指令条数，此处使用`k`进行存储。随后需要对`allInstruction`数组进行倒序遍历，按照从近及远的顺序打印`k`条指令。
3. 执行`exit`命令时，只需要调用`exit()`函数即可。



### 4. 处理没有管道的命令

此处使用最简单的方式来处理重定向。即判定是输入，输出还是追加，同时在打开文件时赋予相应的权限。

#### 全局变量声明

```C
int flag[MAXPIPE][3]; // flag[][0]: 重定向'<' flag[][1]: 重定向'>' flag[][2]: 重定向'>>'
char *file[MAXPIPE][3]; // 存储重定向前后的指令名  file[][0]: 重定向'<' file[][1]: 重定向'>' file[][2]: 重定向'>>'
```

#### 实现代码

```C
int a = 0; // 当前命令参数序号

// 遍历argv数组，将'<', '>', '>>'提取出来
for (int i=0; argv[i] != NULL; i++) {
  	if (strcmp(argv[i], "<") == 0) {
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
```

```C
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
    }   
} else {
    int status;
    waitpid(pid, &status, 0);
}
```

#### 代码说明

1. `flag`数组用于存储当前指令是否有重定向，每一列分别代表输入，输出与追加。
2. `file`数组用于存储当前指令重定向所涉及到的文件。
3. 在子进程中进行处理。如果有文件输入，则关闭默认输入；如果有文件输出或追加，则关闭默认输出。
4. 对于输入文件，打开只读权限；对于输出文件，打开覆盖写权限；对于追加文件，打开追加写功能。
5. 处理完文件权限之后使用`execvp()`函数执行命令。第一个参数为需要执行的文件，如`ls`，`vi`。第二个参数为这一指令所需要的所有参数。



### 5. 处理有管道的命令

#### 实现代码1

```C
void commandPipe(int left, int right) {
    if (left >= right) {
        return;
    }
    // 判断有无管道
    // 找到第一个管道的位置
    int pipeId = -1;
    for (int i=left; i<right; i++) {
        if (strcmp(argv[i], "|") == 0) {
            pipeId = i;
            break;
        }
    }
    if (pipeId == -1) { // 无管道
        commandNoPipe(left, right);
    } else if (pipeId+1 == right) { // 无参数
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
            commandNoPipe(left, pipeId);
            exit(0);
        } else {
            int status;
            waitpid(pid1, &status, 0);
            if (pipeId+1 < right) {
                close(fd[1]);
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);
                commandPipe(pipeId+1, right);
            }
        }
    }
}
```

#### 代码说明1

1. 为了能够处理多重管道，此处使用递归的思想。在指令中找到第一个管道位置，以该管道为界，分别执行左侧与右侧的指令。管道左侧的指令无管道，右侧的指令可能仍然存在管道，此时则需要递归使用此代码来处理右侧指令。
2. 此处需要使用全局变量`argv`数组。`left`为指令最左侧下标，`right`为指令最右侧下标。当$left \geqslant right$时，已经不满足最基本的要求了，所以直接返回。
3. 首先通过遍历得到第一个管道所在的位置。如果发现无管道，则按照重新写的无管道的处理方式（见下文）。如果发现管道后无指令，则说明指令输入有误，则直接返回。一切正常则开始执行。
4. 使用管道时，需要由`fd`返回两个文件描述符，其中`fd[0]`为读而打开，`fd[1]`为写而打开。`fd[1]`的输出是`fd[0]`的输入。
5. 通常，先调用`pipe()`，再调用`fork()`，从而创建从父进程到子进程的$IPC$通道。
6. 此处选择使用从子进程到父进程的管道。子进程关闭管道的读段`fd[0]`，父进程关闭写端`fd[1]`. 子进程中执行管道左侧的指令，且左侧无管道；父进程需要在子进程执行完成后执行右侧的指令，且右侧可能有管道，所以需要递归调用此函数。



#### 实现代码2

此代码为处理无管道中的重定向。

```C
void commandNoPipe(int left, int right) {
    // 判断重定向
    int in = 0; // 输入文件数量
    int out = 0; // 输出文件数量
    int add = 0; // 追加文件数量
    char *inFile = NULL; // 用作输入的文件
    char *outFile = NULL; // 用作输出的文件
    char *addFile = NULL; // 用作追加的文件
    int endIdx = right; // 考虑输入命令的最右端

    /**
     * 指令中每一块内容存储在argv字符串数组中
     * 从第一项向后遍历，寻找重定向
     * 对重定向进行计数
     * 同时判断重定向参数设置是否正确
     */
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
            close(0); // 关闭默认输入
            fd = open(inFile, O_RDONLY);
        }
        if (out == 1) {
            close(1); // 关闭默认输出
            fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, 0666);
        }
        if (add == 1) {
            close(1); // 关闭默认输出
            fd = open(addFile, O_WRONLY|O_CREAT|O_APPEND, 0666);
        }
        char *com[MAXSIZE];
        // 将重定向前内容提出，作为执行参数
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
```

#### 代码说明2

1. 重定向包括输入，输出，追加三种情况。`in`，`out`，`add`为了记录是哪一种重定向，且它们只能出现一次。`inFile`，`outFile`，`addFile`分别存储输入，输出和追加的文件。
2. 紧接着的循环用于得到重定向符号的位置，从而定位重定向右侧的文件。
3. 如果重定向个数超过了一个，则直接以错误返回。
4. 进行`fork()`，在子进程中执行指令。根据重定向类型，确定打开文件的权限。与第4块代码相类似。
5. 第二块循环用于确定指令和指令的参数。需要将重定向符号前的指令提取出来，第一个对象为指令，后面的都是执行的参数。
6. 重新编写是为了利用指令的前后下标，从而简化代码。



### 6. 处理后台运行

#### 实现代码

```C
close(0);
int fd1 = open("/dev/null", O_RDONLY);
dup2(fd1, 0);
close(1);
int fd2 = open("/dev/null", O_WRONLY);
dup2(fd2, 0);
dup2(fd1, 2);
signal(SIGCHLD, SIG_IGN);
execvp(argv[0], argv);
exit(0);
```

#### 代码说明

1. 为了屏蔽键盘和控制台，子进程的标准输入和输出需要映射成`/dev/null`. 
2. 子进程需要调用`signal(SIGCHLD, SIG_IGN)`，使得minix接管此进程。
3. 使用`waitpid()`函数同样也能达到这样的目的。