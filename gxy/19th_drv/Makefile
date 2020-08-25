KERN_DIR = /home/gxy/linux-2.6.22.6

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	cp led_dev.ko /home/gxy/nfs/jz2440_rootfs/home/root

clean:
	make -C $(KERN_DIR) M=`pwd` modules  clean
	rm -rf Module*
app:
	arm-linux-gcc -O0 -g -o led_dev_test led_dev_test.c
	cp led_dev_test /home/gxy/nfs/jz2440_rootfs/home/root

obj-m	+= led_dev.o
obj-m	+= led_drv.o
