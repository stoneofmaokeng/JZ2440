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

char b1;
short b2; 
long b4;

int major;
long * reg;
static struct class* cls;
static struct class_device * cls_dev;
enum cmd{
    R8,
    R16,
    R32,
    W8,
    W16,
    W32,
};


static int debug_regeditor(struct inode * inode, struct file * file, unsigned int cmd, unsigned long arg)
{
    long buf[2];
    if (copy_from_user(buf, (const void __user *)arg, 8)) {
        printk("copy_to_user failed\n");
        return -EFAULT;
    }
    reg = (long *)ioremap(buf[0], 4);
    if (reg == NULL) {
        printk("reg mmap fail\n");
        return -1;
    }


    switch (cmd) {
        case R8:
            *reg = *reg & 0xff;
            if (copy_to_user((void __user *)(arg + 4), (const void *)reg, 4)) {
                printk("copy_to_user failed\n");
                return -EFAULT;
            }
            break;
        case R16:
            *reg = *reg & 0xffff;
            if (copy_to_user((void __user *)(arg + 4), (const void *)reg, 4)) {
                printk("copy_to_user failed\n");
                return -EFAULT;
            }
            break;
        case R32:
            if (copy_to_user((void __user *)(arg + 4), (const void *)reg, 4)) {
                printk("copy_to_user failed\n");
                return -EFAULT;
            }
            break;
        case W8:
            *reg = *reg | buf[1];
            break;
        case W16:
            *reg = *reg | buf[1];
            break;
        case W32:
            *reg = buf[1];
            break;
        default:
            printk("unknown cmd\n");
            break;
    }
    return 0;
}

static struct file_operations debug_regeditor_ops = 
{
    .owner = THIS_MODULE,
    .ioctl = debug_regeditor,
};


static int debug_regeditor_init(void)
{
    major=register_chrdev(0,"debug_regeditor", &debug_regeditor_ops);
    cls=class_create(THIS_MODULE, "dma_class");
    cls_dev=class_device_create(cls, NULL, MKDEV(major,0),NULL, "debug_regeditor_device");    //创建dma_device设备   
    return 0;
}

static void debug_regeditor_exit(void)
{
    class_device_destroy(cls, MKDEV(major,0));
    class_destroy(cls);
    unregister_chrdev(major, "debug_regeditor");
}

module_init(debug_regeditor_init);
module_exit(debug_regeditor_exit);
MODULE_LICENSE("GPL");
