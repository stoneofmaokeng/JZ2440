#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>

enum cmd{
    R8,
    R16,
    R32,
    W8,
    W16,
    W32,
};

int main(int argc, char **argv)
{
    int fd;
    int i;
    unsigned long buf[2];
    unsigned long num;
    fd = open("/dev/regeditor_dev", O_RDWR);
    if (fd < 0) {
        printf("can't open!\n");
    }
    if (argc == 3 || argc ==4) {
    } else {
        printf("Usage :\n");
        printf("%s <R8|R16|R32> <phy_addr> [num]\n", argv[0]);
        printf("%s <W8|W16|W32> <phy_addr> <val>\n", argv[0]);
        return 0;
    }

    if (argv[3] != NULL) {
       num = strtoul(argv[3], NULL, 0); 
    } else {
        num = 1;
    }

    buf[0] = strtoul(argv[2], NULL, 0); 

    if (strcmp(argv[1], "R8") == 0) {
        printf("R8\n");
        for (i = 0; i < num; i++) {
            buf[0] += i*4;
            ioctl(fd, R8, &buf);
            printf("num=%d,phy_addr=0x%x,reg_val=0x%x\n", i, buf[0], buf[1]);
        }
    }
    else if(strcmp(argv[1], "R16") == 0) {
        printf("R16\n");
        for (i = 0; i < num; i++) {
            buf[0] += i*4;
            ioctl(fd, R16, &buf);
            printf("num=%d,phy_addr=0x%x,reg_val=0x%x\n", i, buf[0], buf[1]);
        }
    }
    else if(strcmp(argv[1], "R32") == 0) {
        printf("R32\n");
        for (i = 0; i < num; i++) {
            buf[0] += i*4;
            ioctl(fd, R32, &buf);
            printf("num=%d,phy_addr=0x%x,reg_val=0x%x\n", i, buf[0], buf[1]);
        }
    }
    else if(strcmp(argv[1], "W8") == 0) {
        printf("W8\n");
        buf[1] = num;
        printf("phy_addr=0x%x,reg_val=0x%x\n", buf[0], buf[1]);
        ioctl(fd, W8, &buf);
    }
    else if(strcmp(argv[1], "W16") == 0) {
        printf("W16\n");
        buf[1] = num;
        printf("phy_addr=0x%x,reg_val=0x%x\n", buf[0], buf[1]);
        ioctl(fd, W16, &buf);
    }
    else if(strcmp(argv[1], "W32") == 0) {
        printf("W32\n");
        buf[1] = num;
        printf("phy_addr=0x%x,reg_val=0x%x\n", buf[0], buf[1]);
        ioctl(fd, W32, &buf);
    }
    return 0;
}
