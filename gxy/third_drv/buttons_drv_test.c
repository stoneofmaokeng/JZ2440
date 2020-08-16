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
    unsigned int key_val;
    if (argc != 1) {
        print_usage(argv[0]);
        return 0;
    }
    fd = open("/dev/buttons", O_RDWR);
    if (fd < 0) {
        printf("can't open!\n");
        return -1;
    }
    while(1) {
        read(fd, &key_val, sizeof(key_val));
        printf("key_val = 0x%x\n", key_val);
    }
    close(fd);
    return 0;
}
