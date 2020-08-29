#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

typedef enum {
    LEDS_OFF,
    LEDS_ON,
}cmd;

int main(int argc, char **argv)
{
    int fd;
    int i;
    int val[1];
    fd = open("/dev/led_drv_device", O_RDWR);
    if (fd < 0) {
        printf("can't open!\n");
    }
    if (argc != 2) {
        printf("Usage :\n");
        printf("%s <on|off>\n", argv[0]);
        return 0;
    }
    if (!strcmp(argv[1], "on")) {
        val[0] = LEDS_ON;
    } else {
        val[0] = LEDS_OFF;
    }

    write(fd, val, 4);
    return 0;
}
