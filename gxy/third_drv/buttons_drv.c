#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>

static int major;
static volatile int buttons_flag = 0;
static volatile unsigned int key_v;
static struct class* cls;
static DECLARE_WAIT_QUEUE_HEAD(buttons_wait);

struct pin_desc
{
    unsigned int pin;
    unsigned int key_val;
};

static struct pin_desc pins_desc[3] = 
{
    {S3C2410_GPF0, 0x01},
    {S3C2410_GPF2, 0x02},
    {S3C2410_GPG3, 0x03},
};

#if 1
irqreturn_t buttons_irq(int irq, void *devid)
{
    unsigned int reg;
    struct pin_desc *ptr = devid;
    printk("irq func\n");

    reg = s3c2410_gpio_getpin(ptr->pin);
    printk("reg = 0x%x\n", reg);
    if (reg) {
        key_v = ptr->key_val;
    } else {
        key_v = ptr->key_val|0x80;
    }

    buttons_flag = 1;
    wake_up_interruptible(&buttons_wait);
	return IRQ_HANDLED;
}
#endif

static int buttons_drv_open(struct inode * inode, struct file * file)
{
    int err;
    err = request_irq(IRQ_EINT0, buttons_irq, IRQF_DISABLED|IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "EINT0", &(pins_desc[0]));
    if (err) {
        printk("request_irq IRQ_EINT0 error\n");
        return err;
    }
    err = request_irq(IRQ_EINT2, buttons_irq, IRQF_DISABLED|IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "EINT2", &(pins_desc[1]));
    if (err) {
        printk("request_irq IRQ_EINT2 error\n");
        return err;
    }
    err = request_irq(IRQ_EINT11, buttons_irq, IRQF_DISABLED|IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "EINT11", &(pins_desc[2]));
    if (err) {
        printk("request_irq IRQ_EINT11 error\n");
        return err;
    }

    printk("open\n");
    return 0;
}


static ssize_t buttons_drv_read(struct file *filep, char __user *buf, size_t count, loff_t *oft)
{
    wait_event_interruptible(buttons_wait, buttons_flag);
    buttons_flag = 0;
    if (copy_to_user(buf, &key_v, 4)); 
        return -EFAULT;

    return count;
}

static int buttons_drv_release(struct inode *inode, struct file *filep)
{
    printk("release\n");
    free_irq(IRQ_EINT0,  &(pins_desc[0]));
    free_irq(IRQ_EINT2,  &(pins_desc[1]));
    free_irq(IRQ_EINT11, &(pins_desc[2]));
    return 0;
}

static struct file_operations buttons_drv_ops = 
{
    .owner   = THIS_MODULE,
    .open    = buttons_drv_open,
    .release = buttons_drv_release,
    .read    = buttons_drv_read,
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
