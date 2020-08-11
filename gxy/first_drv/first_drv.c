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

#define GPF_BASE_ADDR (0x56000050)
static int major;
static struct class* cls;

struct gpf_reg {
    unsigned long gpfcon;
    unsigned long gpfdat;
    unsigned long gpfup;
    unsigned long reserve;
};

static volatile struct gpf_reg* gpf_regs;

typedef enum {
    LEDS_OFF,
    LEDS_ON,
}cmd;



int first_drv_open(struct inode * inode, struct file * file)
{
    printk("open\n");
    return 0;
}

ssize_t first_drv_write(struct file *file, const char __user * buf, size_t count, loff_t *ppos)
{
    int cmd[1];
    unsigned long reg_val;
    reg_val = gpf_regs->gpfcon;
    reg_val |= (1<<8)|(1<<10)|(1<<12)|(1<<14);
    reg_val &= ~((1<<9)|(1<<11)|(1<<13)|(1<<15));
    gpf_regs->gpfcon = reg_val;
    printk("gpfcon: 0x%02x", gpf_regs->gpfcon);

	if(copy_from_user(cmd, buf, count)) {
        printk("copy_to_user failed\n");
		return -EFAULT;
    }
    switch (cmd[0]) {
        case LEDS_OFF:
            printk("LEDS_OFF\n");
            gpf_regs->gpfdat = 0;
            printk("gpfdat: 0x%02x", gpf_regs->gpfdat);
            break;
        case LEDS_ON:
            printk("LEDS_ON\n");
            gpf_regs->gpfdat = 0xf0;
            printk("gpfdat: 0x%02x", gpf_regs->gpfdat);
            break;
        default:
            break;
    }
    return count;
}

static struct file_operations first_drv_ops = 
{
    .owner = THIS_MODULE,
    .open = first_drv_open,
    .write = first_drv_write,
};


static int first_drv_init(void)
{
    int ret;
    gpf_regs = ioremap(GPF_BASE_ADDR, sizeof(struct gpf_reg));
	if (!gpf_regs) {
        printk("ioremap failed\n");
		ret = -ENOMEM;
	}
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
    iounmap((volatile void __iomem*)GPF_BASE_ADDR);
}

module_init(first_drv_init);
module_exit(first_drv_exit);
MODULE_LICENSE("GPL");
