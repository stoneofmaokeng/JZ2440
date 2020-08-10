KERN_DIR = /home/gxy/linux-2.6.22.6

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	cp first_drv.ko /home/gxy/nfs/jz2440_rootfs/home/root

clean:
	make -C $(KERN_DIR) M=`pwd` modules  clean
	rm -rf Module*
	rm first_drv_test
app:
	arm-linux-gcc -O0 -g -o first_drv_test first_drv_test.c
	cp first_drv_test /home/gxy/nfs/jz2440_rootfs/home/root

obj-m	+= first_drv.o
