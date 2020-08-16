#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

void print_usage(char* file)
{
    printf("Usage :\n");
    printf("%s\n", file);
}

int main(int argc, char **argv)
{
    int fd;
    char buf[4];
    if (argc != 1) {
        print_usage(argv[0]);
        return 0;
    }
    fd = open("/dev/buttons", O_RDWR);
    if (fd < 0) {
        printf("can't open!\n");
        return -1;
    }
    while (1) {
        read(fd,buf,4);
        if (buf[0] == 0) {
            printf("ENT0 down\n");
        }
        if (buf[1] == 0) {
            printf("ENT2 down\n");
        }
        if (buf[2] == 0) {
            printf("ENT11 down\n");
        }
    }
    close(fd);
    return 0;
}
