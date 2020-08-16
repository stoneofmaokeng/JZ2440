#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

typedef enum {
    OFF,
    ON,
}cmd;

void print_usage(char* file)
{
    printf("Usage :\n");
    printf("%s <dev> <on|off|read>\n", file);
    printf("eg. \n");
    printf("%s /dev/leds <on|off|read>\n", file);
    printf("%s /dev/led0 <on|off|read>\n", file);
    printf("%s /dev/led1 <on|off|read>\n", file);
    printf("%s /dev/led2 <on|off|read>\n", file);
}

int main(int argc, char **argv)
{
    int fd;
    int i;
    char val[4];
    char buf[4];
    char* dev;
    if (argc != 3) {
        print_usage(argv[0]);
        return 0;
    }
    dev = argv[1];
    fd = open(dev, O_RDWR);
    if (fd < 0) {
        printf("can't open!\n");
        return -1;
    }
    if (!strcmp(argv[2], "on")) {
        val[0] = ON;
        write(fd, val, 1);
    } else if (!strcmp(argv[2], "off")) {
        val[0] = OFF;
        write(fd, val, 1);
    } else if (!strcmp(argv[2], "read")) {
        if (!strcmp(argv[1], "/dev/leds")) {
            read(fd,buf,4);
            for (i = 0;i < 3;i++) {
                printf("led%d = 0x%x\n", i, buf[i+1]);
            }
        } else if (!strcmp(argv[1], "/dev/led0")) {
            read(fd,buf,4);
            printf("led0 = 0x%x\n", buf[1]);
        } else if (!strcmp(argv[1], "/dev/led1")) {
            read(fd,buf,4);
            printf("led1 = 0x%x\n", buf[1]);
        } else if (!strcmp(argv[1], "/dev/led2")) {
            read(fd,buf,4);
            printf("led2 = 0x%x\n", buf[1]);
        }
    } else {
        print_usage(argv[0]);
    }
    close(fd);
    return 0;
}
