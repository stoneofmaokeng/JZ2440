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
static DECLARE_MUTEX(buttons_lock);


int buttons_drv_open(struct inode * inode, struct file * file)
{
    int minor = MINOR(inode->i_rdev);
    switch (minor) {
        case 0:
            s3c2410_gpio_cfgpin(S3C2410_GPF0, S3C2410_GPF0_INP);    //EINT0
            break;
        case 1:
            s3c2410_gpio_cfgpin(S3C2410_GPF2, S3C2410_GPF2_INP);    //EINT2
            break;
        case 2:
            s3c2410_gpio_cfgpin(S3C2410_GPG3, S3C2410_GPG3_INP);    //EINT11
            break;
        default:
            break;
    }
    printk("open\n");
    return 0;
}


ssize_t buttons_drv_read(struct file *filep, char __user *buf, size_t count, loff_t *oft)
{
    char val[4];
    unsigned int reg;
    reg = s3c2410_gpio_getpin(S3C2410_GPF0);
    if (reg & 1) {
        val[0] = 1;
    } else {
        val[0] = 0;
    }
    reg = s3c2410_gpio_getpin(S3C2410_GPF2);
    if (reg & (1<<2)) {
        val[1] = 1;
    } else {
        val[1] = 0;
    }
    reg = s3c2410_gpio_getpin(S3C2410_GPG3);
    if (reg & (1<<3)) {
        val[2] = 1;
    } else {
        val[2] = 0;
    }

    if (copy_to_user(buf, val, sizeof(val)));
        return -EFAULT;

    return count;
}

static struct file_operations buttons_drv_ops = 
{
    .owner = THIS_MODULE,
    .open = buttons_drv_open,
    .read = buttons_drv_read,
};


static int buttons_drv_init(void)
{
    major=register_chrdev(0,"buttons_drv", &buttons_drv_ops);
    cls=class_create(THIS_MODULE, "buttons_drv_class");
    class_device_create(cls, NULL, MKDEV(major,0),NULL, "buttons");    //创建dma_device设备   
    return 0;
}

static void buttons_drv_exit(void)
{
    class_device_destroy(cls, MKDEV(major,0));
    class_destroy(cls);
    unregister_chrdev(major, "buttons_drv");
}

module_init(buttons_drv_init);
module_exit(buttons_drv_exit);
MODULE_LICENSE("GPL");
