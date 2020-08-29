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
#include <linux/poll.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>

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

int led_drv_open(struct inode * inode, struct file * file)
{
    printk("open\n");
    return 0;
}

ssize_t led_drv_write(struct file *file, const char __user * buf, size_t count, loff_t *ppos)
{
    int cmd[1];
    unsigned long reg_val;
    reg_val = gpf_regs->gpfcon;
    reg_val |= (1<<8)|(1<<10)|(1<<12);
    reg_val &= ~((1<<9)|(1<<11)|(1<<13));
    gpf_regs->gpfcon = reg_val;
    printk("gpfcon: 0x%02x\n", gpf_regs->gpfcon);

    if(copy_from_user(cmd, buf, count)) {
        printk("copy_to_user failed\n");
        return -EFAULT;
    }
    switch (cmd[0]) {
        case LEDS_OFF:
            printk("LEDS_OFF\n");
            gpf_regs->gpfdat |= (1<<4)|(1<<5)|(1<<6);
            printk("gpfdat: 0x%02x", gpf_regs->gpfdat);
            break;
        case LEDS_ON:
            printk("LEDS_ON\n");
            gpf_regs->gpfdat &= ~((1<<4)|(1<<5)|(1<<6)|(1<<7));
            printk("gpfdat: 0x%02x", gpf_regs->gpfdat);
            break;
        default:                                                                                                                                                                                 
            break;
    }
    return count;
}

static struct file_operations led_drv_ops = 
{
    .owner = THIS_MODULE,
    .open = led_drv_open,
    .write = led_drv_write,
};

static int led_probe(struct platform_device *led_dev) 
{
	int ret;
	struct resource *mem;
	printk("led_probe\n");
	mem = platform_get_resource(led_dev, IORESOURCE_MEM, 0);
	if (!mem)
		return -EINVAL;

    gpf_regs = ioremap(mem->start, mem->end - mem->start);
    if (!gpf_regs) {
        printk("ioremap failed\n");
        ret = -ENOMEM;
    }
    major=register_chrdev(0,"led_drv", &led_drv_ops);
    cls=class_create(THIS_MODULE, "led_drv_class");
    class_device_create(cls, NULL, MKDEV(major,0),NULL, "led_drv_device");    //创建dma_device设备   
    return 0;
}

static int led_remove(struct platform_device *led_dev)
{
    class_device_destroy(cls, MKDEV(major,0));
    class_destroy(cls);
    unregister_chrdev(major, "led_drv");
    iounmap((volatile void __iomem*)gpf_regs);
    return 0;
}

static struct platform_driver led_driver = {
	.probe		= led_probe,
	.remove		= led_remove,
	.driver		= {
		.name	= "my_led",
	},
};


static int led_drv_init(void)
{
    platform_driver_register(&led_driver);
    return 0;
}

static void led_drv_exit(void)
{
    platform_driver_unregister(&led_driver);
}

module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
