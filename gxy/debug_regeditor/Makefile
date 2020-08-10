KERN_DIR = /home/gxy/linux-2.6.22.6

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	cp debug_regeditor_drv.ko /home/gxy/nfs/jz2440_rootfs/home/root

clean:
	make -C $(KERN_DIR) M=`pwd` modules  clean
	rm -rf Module*
	rm debug_regeditor_test
app:
	arm-linux-gcc -O0 -g -o debug_regeditor_test debug_regeditor_test.c
	cp debug_regeditor_test /home/gxy/nfs/jz2440_rootfs/home/root

obj-m	+= debug_regeditor_drv.o
