#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>


void print_usage(char* file)
{
    printf("Usage :\n");
    printf("%s\n", file);
}

int main(int argc, char **argv)
{
    int fd;
    int ret;
    unsigned int key_val;
    struct pollfd fds[1];
    if (argc != 1) {
        print_usage(argv[0]);
        return 0;
    }
    fd = open("/dev/buttons", O_RDWR);
    if (fd < 0) {
        printf("can't open!\n");
        return -1;
    }

    fds[0].fd = fd;
    fds[0].events = POLLIN;
    while(1) {
        ret = poll(fds, 1, 5000);
        if (ret) {
            read(fd, &key_val, sizeof(key_val));
            printf("key_val = 0x%x\n", key_val);
        } else {
            printf("over time\n");
        }
    }
    close(fd);
    return 0;
}
