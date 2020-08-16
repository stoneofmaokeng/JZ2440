KERN_DIR = /home/gxy/linux-2.6.22.6

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	cp leds_drv.ko /home/gxy/nfs/jz2440_rootfs/home/root

clean:
	make -C $(KERN_DIR) M=`pwd` modules  clean
	rm -rf Module*
	rm leds_drv_test
app:
	arm-linux-gcc -O0 -g -o leds_drv_test leds_drv_test.c
	cp leds_drv_test /home/gxy/nfs/jz2440_rootfs/home/root

obj-m	+= leds_drv.o
