#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAXSIZE 1024 // 指令最大长度
#define MAXARGC 32 // 指令参数最大数量
#define HISTSIZE 100

int get_Parameter(char* array[]) {
    int i = 0;
    for (; array[i]!=NULL; i++) {
        if (strcmp(array[i], ">") && strcmp(array[i], ">>") && strcmp(array[i], "<")) {
            continue;
        } else {
            return i;
        }
    }
    return 0;
}

int main()
{
    while (1) {
        char Instruction[MAXSIZE]; // 创建字符数组，用于存放输入的指令
        fgets(Instruction, MAXSIZE, stdin); // 从键盘输入指令
        Instruction[strlen(Instruction)-1] = '\0'; // 确定指令长度，并且将输入的回车设置为'\0'作为字符串的结尾

        char *argv[MAXARGC]; // 创建字符指针数组，每个元素为指向一个字符串的指针
        int ArgvIndex = 0;
        char *pInstruction = Instruction;

        // 将指令的每一个参数分离，并存入argv数组
        while (*pInstruction != '\0') { // 循环终止条件为遍历到指令的最后一个字符'\0'
            /*
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
            // FILE* f = fopen("E:\\OperatingSystem\\.bash_history.txt", "a+");
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
                // FILE* f1 = fopen("E:\\OperatingSystem\\.bash_history.txt", "r");
                FILE* f1 = fopen(".bash_history.txt", "r");
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
            }
        } else if (strcmp(argv[0], "exit") == 0) {
            break;
        } else if (strcmp(argv[0], "mytop") == 0){

        } else {
            if (get_Parameter(argv)) {
                int arrow = get_Parameter(argv);
                if (!strcmp(argv[arrow], ">")) {

                } else if (!strcmp(argv[arrow], ">>")) {

                } else if (!strcmp(argv[arrow], "<")) {
                    
                }
            } else {

            }
        }
    }
    return 0;
}
