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
static DECLARE_MUTEX(leds_lock);

typedef enum {
    OFF,
    ON,
}cmd;



int leds_drv_open(struct inode * inode, struct file * file)
{
    int minor = MINOR(inode->i_rdev);
    switch (minor) {
        case 0:
            s3c2410_gpio_cfgpin(S3C2410_GPF4, S3C2410_GPF4_OUTP);
            s3c2410_gpio_cfgpin(S3C2410_GPF5, S3C2410_GPF5_OUTP);
            s3c2410_gpio_cfgpin(S3C2410_GPF6, S3C2410_GPF6_OUTP);
            break;
        case 1:
            s3c2410_gpio_cfgpin(S3C2410_GPF4, S3C2410_GPF4_OUTP);
            break;
        case 2:
            s3c2410_gpio_cfgpin(S3C2410_GPF5, S3C2410_GPF5_OUTP);
            break;
        case 3:
            s3c2410_gpio_cfgpin(S3C2410_GPF6, S3C2410_GPF6_OUTP);
            break;
    }
    printk("open\n");
    return 0;
}

ssize_t leds_drv_write(struct file *file, const char __user * buf, size_t count, loff_t *ppos)
{
    int cmd[1];
    int minor;

	if(copy_from_user(cmd, buf, count)) {
        printk("copy_to_user failed\n");
		return -EFAULT;
    }
    
    minor = MINOR(file->f_dentry->d_inode->i_rdev);


    switch (minor){
        case 0:
            if (buf[0] == OFF) {
                printk("leds off\n");
                s3c2410_gpio_setpin(S3C2410_GPF4, 1);
                s3c2410_gpio_setpin(S3C2410_GPF5, 1);
                s3c2410_gpio_setpin(S3C2410_GPF6, 1);

            } else {
                printk("leds on\n");
                s3c2410_gpio_setpin(S3C2410_GPF4, 0);
                s3c2410_gpio_setpin(S3C2410_GPF5, 0);
                s3c2410_gpio_setpin(S3C2410_GPF6, 0);
            }
            break;
        case 1:
            if (buf[0] == OFF) {
                printk("led0 off\n");
                s3c2410_gpio_setpin(S3C2410_GPF4, 1);
            } else {
                printk("led0 on\n");
                s3c2410_gpio_setpin(S3C2410_GPF4, 0);
            }
            break;
        case 2:
            if (buf[0] == OFF) {
                printk("led1 off\n");
                s3c2410_gpio_setpin(S3C2410_GPF5, 1);
            } else {
                printk("led1 on\n");
                s3c2410_gpio_setpin(S3C2410_GPF5, 0);
            }
            break;
        case 3:
            if (buf[0] == OFF) {
                printk("led2 off\n");
                s3c2410_gpio_setpin(S3C2410_GPF6, 1);
            } else {
                printk("led2 on\n");
                s3c2410_gpio_setpin(S3C2410_GPF6, 0);
            }
            break;
        default:
            break;
    }
    return count;
}

ssize_t leds_drv_read(struct file *filep, char __user *buf, size_t count, loff_t *oft)
{
    int cmd[1];
    char val[4];
    int minor;

	if(copy_from_user(cmd, buf, count)) {
        printk("copy_to_user failed\n");
		return -EFAULT;
    }
    
    minor = MINOR(filep->f_dentry->d_inode->i_rdev);


    switch (minor){
        case 0:
            printk("read leds\n");
            down(&leds_lock);
            val[1] = s3c2410_gpio_getpin(S3C2410_GPF4);
            printk("val[1]=0x%x\n",val[1]);
            val[2] = s3c2410_gpio_getpin(S3C2410_GPF5);
            printk("val[2]=0x%x\n",val[2]);
            val[3] = s3c2410_gpio_getpin(S3C2410_GPF6);
            printk("val[3]=0x%x\n",val[3]);
            up(&leds_lock);
            if (copy_to_user(buf, val, sizeof(val)));
                return -EFAULT;
            break;
        case 1:
            printk("read led0\n");
            down(&leds_lock);
            val[1] = s3c2410_gpio_getpin(S3C2410_GPF4);
            up(&leds_lock);
            if (copy_to_user(buf, val, sizeof(val)));
                return -EFAULT;
            break;
        case 2:
            printk("read led1\n");
            down(&leds_lock);
            val[1] = s3c2410_gpio_getpin(S3C2410_GPF5);
            up(&leds_lock);
            if (copy_to_user(buf, val, sizeof(val)));
                return -EFAULT;
            break;
        case 3:
            printk("read led2\n");
            down(&leds_lock);
            val[1] = s3c2410_gpio_getpin(S3C2410_GPF6);
            up(&leds_lock);
            if (copy_to_user(buf, val, sizeof(val)));
                return -EFAULT;
            break;
        default:
            break;
    }
    return count;
}

static struct file_operations leds_drv_ops = 
{
    .owner = THIS_MODULE,
    .open = leds_drv_open,
    .write = leds_drv_write,
    .read = leds_drv_read,
};


static int leds_drv_init(void)
{
    int i = 0;

    s3c2410_gpio_cfgpin(S3C2410_GPF4, S3C2410_GPF4_OUTP);
    s3c2410_gpio_cfgpin(S3C2410_GPF5, S3C2410_GPF5_OUTP);
    s3c2410_gpio_cfgpin(S3C2410_GPF6, S3C2410_GPF6_OUTP);

    major=register_chrdev(0,"leds_drv", &leds_drv_ops);
    cls=class_create(THIS_MODULE, "leds_drv_class");
    class_device_create(cls, NULL, MKDEV(major,0),NULL, "leds");    //创建dma_device设备   
    for (i = 0; i < 3; i++) {
        class_device_create(cls, NULL, MKDEV(major,i+1),NULL, "led%d", i);    //创建dma_device设备   
    }
    return 0;
}

static void leds_drv_exit(void)
{
    int i = 0;

    for (i = 0; i < 4; i++) {
        class_device_destroy(cls, MKDEV(major,i));
    }
    class_destroy(cls);
    unregister_chrdev(major, "leds_drv");
}

module_init(leds_drv_init);
module_exit(leds_drv_exit);
MODULE_LICENSE("GPL");
