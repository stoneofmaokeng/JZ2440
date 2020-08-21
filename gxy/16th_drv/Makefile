KERN_DIR = /home/gxy/linux-2.6.22.6

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	cp vnet_drv.ko /home/gxy/nfs/jz2440_rootfs/home/root

clean:
	make -C $(KERN_DIR) M=`pwd` modules  clean
	rm -rf Module*
obj-m	+= vnet_drv.o
