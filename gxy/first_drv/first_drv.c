#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/proc_fs.h>
static int major;
static struct class* cls;


int first_drv_open(struct inode *, struct file *)
{
}

ssize_t first_drv_write(struct file *, const char __user *, size_t, loff_t *)
{
}

static struct file_operations first_drv_ops = 
{
    .owner = THIS_MODULE,
    .open = first_drv_open,
    .write = first_drv_write,
};


static int first_drv_init(void)
{
    major=register_chrdev(0,"first_drv", &first_drv_ops);
    cls=class_create(THIS_MODULE, "first_drv_class");
    class_device_create(cls, NULL, MKDEV(major,0),NULL, "first_drv_device");    //创建dma_device设备   
    return 0;
}

static void first_drv_exit(void)
{
    class_device_destroy(cls, MKDEV(major,0));
    class_destroy(cls);
    unregister_chrdev(major, "first_drv");
}

module_init(first_drv_init);
module_exit(first_drv_exit);
MODULE_LICENSE("GPL");
