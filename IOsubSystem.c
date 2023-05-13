#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>

#define REPEAT 10000
#define MAXBLOCK (4 * 1024) // 最大块大小
#define FILESIZE (300 * 1024)
#define MAXLINE (50 * 1024)
#define CONCURRENCY 15
#define mod 1000000007

long long WriteText[MAXLINE];
long long ReadBuff[MAXLINE];
struct timeval starttime, endtime;

char *FilePathRam[15] = {
        "/root/myram/ram_test1", "/root/myram/ram_test2", "/root/myram/ram_test3", "/root/myram/ram_test4",
        "/root/myram/ram_test5", "/root/myram/ram_test6", "/root/myram/ram_test7", "/root/myram/ram_test8",
        "/root/myram/ram_test9", "/root/myram/ram_test10", "/root/myram/ram_test11", "/root/myram/ram_test12",
        "/root/myram/ram_test13", "/root/myram/ram_test14", "/root/myram/ram_test15",
};

char *FilePathDisk[15] = {
        "/usr/disk1.txt", "/usr/disk2.txt", "/usr/disk3.txt", "/usr/disk4.txt",
        "/usr/disk5.txt", "/usr/disk6.txt", "/usr/disk7.txt", "/usr/disk8.txt",
        "/usr/disk9.txt", "/usr/disk10.txt", "/usr/disk11.txt", "/usr/disk12.txt",
        "/usr/disk13.txt", "/usr/disk14.txt", "/usr/disk15.txt",
};

/*写文件：打开文件，判断返回值，如果正常打开文件就判断是否随机写，进行写操作*/
void write_file(int blocksize, bool isrand, char *filepath)
{
    int fd = open(filepath, O_RDWR | O_CREAT | O_SYNC, 0755);
    if (fd == -1) {
        printf("Fail to open!");
    } else {
        for (int i = 0; i < REPEAT; i++) {
            int x = write(fd, WriteText, blocksize);
            if (x == -1) {
                printf("Write Error!");
                break;
            }
            if (isrand) {
                lseek(fd, rand() % (FILESIZE - MAXBLOCK), SEEK_SET);
            }
        }
    }
    lseek(fd, 0, SEEK_SET);
}

/*读文件：打开文件，判断返回值，如果正常打开文件就判断是否随机读，进行读操作*/
void read_file(int blocksize, bool isrand, char *filepath)
{
    int fd = open(filepath, O_RDONLY | O_CREAT | O_SYNC, 0755);
    if (fd == -1) {
        printf("Fail to open!");
    } else {
        for (int i = 0; i < REPEAT; i++) {
            int x = read(fd, ReadBuff, blocksize);
            if (x == -1) {
                printf("Read Error!");
                break;
            }
            if (isrand) {
                lseek(fd, rand() % (FILESIZE - MAXBLOCK), SEEK_SET);
            }
        }
    }
    lseek(fd, 0, SEEK_SET);
}

/*计算时间差，在读或写操作前后分别取系统时间，然后计算差值即为时间差*/
long get_time_left(long start, long end)
{
    long spendtime = 1000 * (long)(end - start) + (end - start) / 1000;
    return spendtime;
}

void ram_seq_write()
{
    printf("Ram Sequential Write\n");
    printf("block\tprocess\tlatency\n");

    double BLOCK = 0;
    double LATENCY = 0;

    for (int block = 64; block <= MAXBLOCK; block *= 2) {
//        for (int process = 1; process <= CONCURRENCY; process++) {
//            gettimeofday(&starttime, NULL);
//
//            for (int i = 0; i < process; i++) {
//                if (fork() == 0) {
//                    write_file(block, false, FilePathRam[i]);
//                    exit(1);
//                }
//            }
//
//            while (wait(NULL) != -1) {
//
//            }
//
//            gettimeofday(&endtime, NULL);
//
//            long total_time = get_time_left(starttime.tv_sec, endtime.tv_sec);
//            double latency = total_time / (double) REPEAT / (double) process;
//
//            BLOCK += block;
//            LATENCY += latency;
//
//            printf("%d\t%d\t%.2f\n", block, process, latency);
//        }
//        double throughput = BLOCK / LATENCY;
//        printf("throughput: %.3f\n", throughput);
//        BLOCK = 0;
//        LATENCY = 0;

        gettimeofday(&starttime, NULL);

        for (int i = 0; i < CONCURRENCY; i++) {
            if (fork() == 0) {
                write_file(block, false, FilePathRam[i]);
                exit(1);
            }
        }

        while (wait(NULL) != -1) {

        }

        gettimeofday(&endtime, NULL);

        long spendtime = get_time_left(starttime.tv_sec, endtime.tv_sec) / 1000;

        int blocksize = block * CONCURRENCY * REPEAT;
        double speed = (double) blocksize / (double) spendtime / 1024.0 / 1024.0;
        printf("blocksize = %d, speed = %.2fMB/s", blocksize, speed);
    }
}

void ram_ran_write()
{
    printf("Ram Random Write\n");
    printf("block\tprocess\tlatency\n");

    double BLOCK = 0;
    double LATENCY = 0;

    for (int block = 64; block <= MAXBLOCK; block *= 2) {
        for (int process = 1; process <= CONCURRENCY; process++) {
            gettimeofday(&starttime, NULL);

            for (int i = 0; i < process; i++) {
                if (fork() == 0) {
                    write_file(block, true, FilePathRam[i]);
                    exit(1);
                }
            }

            while (wait(NULL) != -1) {

            }

            gettimeofday(&endtime, NULL);

            long total_time = get_time_left(starttime.tv_sec, endtime.tv_sec);
            double latency = total_time / (double) REPEAT / (double) process;

            BLOCK += block;
            LATENCY += latency;

            printf("%d\t%d\t%.2f\n", block, process, latency);
        }
        double throughput = BLOCK / LATENCY;
        printf("throughput: %.3f\n", throughput);
        BLOCK = 0;
        LATENCY = 0;
    }
}

void ram_seq_read()
{
    printf("Ram Sequential Write\n");
    printf("block\tprocess\tlatency\n");

    double BLOCK = 0;
    double LATENCY = 0;

    for (int block = 64; block <= MAXBLOCK; block *= 2) {
        for (int process = 1; process <= CONCURRENCY; process++) {
            gettimeofday(&starttime, NULL);

            for (int i = 0; i < process; i++) {
                if (fork() == 0) {
                    read_file(block, false, FilePathRam[i]);
                    exit(1);
                }
            }

            while (wait(NULL) != -1) {

            }

            gettimeofday(&endtime, NULL);

            long total_time = get_time_left(starttime.tv_sec, endtime.tv_sec);
            double latency = total_time / (double) REPEAT / (double) process;

            BLOCK += block;
            LATENCY += latency;

            printf("%d\t%d\t%.2f\n", block, process, latency);
        }
        double throughput = BLOCK / LATENCY;
        printf("throughput: %.3f\n", throughput);
        BLOCK = 0;
        LATENCY = 0;
    }
}

void ram_ran_read()
{
    printf("Ram Random Write\n");
    printf("block\tprocess\tlatency\n");

    double BLOCK = 0;
    double LATENCY = 0;

    for (int block = 64; block <= MAXBLOCK; block *= 2) {
        for (int process = 1; process <= CONCURRENCY; process++) {
            gettimeofday(&starttime, NULL);

            for (int i = 0; i < process; i++) {
                if (fork() == 0) {
                    read_file(block, true, FilePathRam[i]);
                    exit(1);
                }
            }

            while (wait(NULL) != -1) {

            }

            gettimeofday(&endtime, NULL);

            long total_time = get_time_left(starttime.tv_sec, endtime.tv_sec);
            double latency = total_time / (double) REPEAT / (double) process;

            BLOCK += block;
            LATENCY += latency;

            printf("%d\t%d\t%.2f\n", block, process, latency);
        }
        double throughput = BLOCK / LATENCY;
        printf("throughput: %.3f\n", throughput);
        BLOCK = 0;
        LATENCY = 0;
    }
}

void disk_seq_write()
{
    printf("Disk Sequential Write\n");
    printf("block\tprocess\tlatency\n");

    double BLOCK = 0;
    double LATENCY = 0;

    for (int block = 64; block <= MAXBLOCK; block *= 2) {
        for (int process = 1; process <= CONCURRENCY; process++) {
            gettimeofday(&starttime, NULL);

            for (int i = 0; i < process; i++) {
                if (fork() == 0) {
                    write_file(block, false, FilePathDisk[i]);
                    exit(1);
                }
            }

            while (wait(NULL) != -1) {

            }

            gettimeofday(&endtime, NULL);

            long total_time = get_time_left(starttime.tv_sec, endtime.tv_sec);
            double latency = total_time / (double) REPEAT / (double) process;

            BLOCK += block;
            LATENCY += latency;

            printf("%d\t%d\t%.2f\n", block, process, latency);
        }
        double throughput = BLOCK / LATENCY;
        printf("throughput: %.3f\n", throughput);
        BLOCK = 0;
        LATENCY = 0;
    }
}

void disk_ran_write()
{
    printf("Disk Random Write\n");
    printf("block\tprocess\tlatency\n");

    double BLOCK = 0;
    double LATENCY = 0;

    for (int block = 64; block <= MAXBLOCK; block *= 2) {
        for (int process = 1; process <= CONCURRENCY; process++) {
            gettimeofday(&starttime, NULL);

            for (int i = 0; i < process; i++) {
                if (fork() == 0) {
                    write_file(block, true, FilePathDisk[i]);
                    exit(1);
                }
            }

            while (wait(NULL) != -1) {

            }

            gettimeofday(&endtime, NULL);

            long total_time = get_time_left(starttime.tv_sec, endtime.tv_sec);
            double latency = total_time / (double) REPEAT / (double) process;

            BLOCK += block;
            LATENCY += latency;

            printf("%d\t%d\t%.2f\n", block, process, latency);
        }
        double throughput = BLOCK / LATENCY;
        printf("throughput: %.3f\n", throughput);
        BLOCK = 0;
        LATENCY = 0;
    }
}

void disk_seq_read()
{
    printf("Disk Sequential Write\n");
    printf("block\tprocess\tlatency\n");

    double BLOCK = 0;
    double LATENCY = 0;

    for (int block = 64; block <= MAXBLOCK; block *= 2) {
        for (int process = 1; process <= CONCURRENCY; process++) {
            gettimeofday(&starttime, NULL);

            for (int i = 0; i < process; i++) {
                if (fork() == 0) {
                    read_file(block, false, FilePathDisk[i]);
                    exit(1);
                }
            }

            while (wait(NULL) != -1) {

            }

            gettimeofday(&endtime, NULL);

            long total_time = get_time_left(starttime.tv_sec, endtime.tv_sec);
            double latency = total_time / (double) REPEAT / (double) process;

            BLOCK += block;
            LATENCY += latency;

            printf("%d\t%d\t%.2f\n", block, process, latency);
        }
        double throughput = BLOCK / LATENCY;
        printf("throughput: %.3f\n", throughput);
        BLOCK = 0;
        LATENCY = 0;
    }
}

void disk_ran_read()
{
    printf("Disk Random Write\n");
    printf("block\tprocess\tlatency\n");

    double BLOCK = 0;
    double LATENCY = 0;

    for (int block = 64; block <= MAXBLOCK; block *= 2) {
        for (int process = 1; process <= CONCURRENCY; process++) {
            gettimeofday(&starttime, NULL);

            for (int i = 0; i < process; i++) {
                if (fork() == 0) {
                    read_file(block, true, FilePathDisk[i]);
                    exit(1);
                }
            }

            while (wait(NULL) != -1) {

            }

            gettimeofday(&endtime, NULL);

            long total_time = get_time_left(starttime.tv_sec, endtime.tv_sec);
            double latency = total_time / (double) REPEAT / (double) process;

            BLOCK += block;
            LATENCY += latency;

            printf("%d\t%d\t%.2f\n", block, process, latency);
        }
        double throughput = BLOCK / LATENCY;
        printf("throughput: %.3f\n", throughput);
        BLOCK = 0;
        LATENCY = 0;
    }
}

/**
 * 主函数：首先创建和命名文件，通过循环执行read_file和write_file函数测试读写差异。
 * 测试blocksize和concurrency对测试读写速度的影响，最后输出结果。
 * @return
 */
int main()
{
    srand((unsigned)time(NULL));
    int i = 0;

    for (int j = 0; j < MAXLINE; j++) {
        WriteText[j] = (j * j * j - 3 * MAXBLOCK * j) % mod;
    }

    ram_seq_write();
    printf("\n");
    ram_ran_write();
    printf("\n");
    ram_seq_read();
    printf("\n");
    ram_ran_read();
    printf("\n");
    disk_seq_write();
    printf("\n");
    disk_ran_write();
    printf("\n");
    disk_seq_read();
    printf("\n");
    disk_ran_read();
    printf("\n");
    return 0;
}
