

# Shell说明文档



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