#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <signal.h>

static int fd;

void print_usage(char* file)
{
    printf("Usage :\n");
    printf("%s\n", file);
}

void func(int num)
{
    unsigned int key_val;
    read(fd, &key_val, sizeof(key_val));
    printf("key_val = 0x%x\n", key_val);
}

int main(int argc, char **argv)
{
    int ret;
    int oflag;
    signal(SIGIO, func);
    if (argc != 1) {
        print_usage(argv[0]);
        return 0;
    }
    fd = open("/dev/buttons", O_RDWR);
    if (fd < 0) {
        printf("can't open!\n");
        return -1;
    }

    fcntl(fd, F_SETOWN, getpid());
    oflag = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, oflag|FASYNC);

    while(1) {
        sleep(5);
    }
    close(fd);
    return 0;
}
