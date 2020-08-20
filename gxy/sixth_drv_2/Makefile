KERN_DIR = /home/gxy/linux-2.6.22.6

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	cp buttons_drv.ko /home/gxy/nfs/jz2440_rootfs/home/root

clean:
	make -C $(KERN_DIR) M=`pwd` modules  clean
	rm -rf Module*
	rm buttons_drv_test
app:
	arm-linux-gcc -O0 -g -o buttons_drv_test buttons_drv_test.c
	cp buttons_drv_test /home/gxy/nfs/jz2440_rootfs/home/root

obj-m	+= buttons_drv.o
