#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>

#define CP_MEM_NO_DMA   0
#define CP_MEM_DMA   1

int main(int argc, char **argv)
{
    int fd;
    int val = 1;
    unsigned long temp;
    fd = open("/dev/dma", O_RDWR);
    if (fd < 0) {
        printf("can't open!\n");
    }
    if (argc != 2) {
        printf("Usage :\n");
        printf("%s <0|1>\n", argv[0]);
        return 0;
    }

    if ((*argv[1] - '0') == 0) {
        val  = CP_MEM_NO_DMA;
    }
    else {
        val = CP_MEM_DMA;
    }
    
    ioctl(fd, val, &temp);
    return 0;
}
