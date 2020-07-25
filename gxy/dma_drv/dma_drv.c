#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>

//#define DBG_PRINTK  printk
#define DBG_PRINTK(x...)
DECLARE_WAIT_QUEUE_HEAD(my_log_wait);

static  char mybuf[100]="123";
static int major;
static struct class *cls;

int dma_ioctl(struct inode * inode, struct file * file, unsigned int flag, unsigned long aa)
{
}

struct file_operations dma_ops={
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .ioctl  = dma_ioctl,
};


static ssize_t show_my_device(struct device *dev,
                  struct device_attribute *attr, char *buf)        //cat命令时,将会调用该函数
{
    return sprintf(buf, "%s\n", mybuf);
}


 
static ssize_t set_my_device(struct device *dev,
                 struct device_attribute *attr,
                 const char *buf, size_t len)        //echo命令时,将会调用该函数
{
    switch (buf[0]) {
        case '1':
            printk("case 1\n");
            //myprintk("%s, %s, %d\n", __FILE__, __func__, __LINE__);
            myprintk("123456\n");
            break;
        case '2':
            printk("case 2\n");
            //myprintk("%s, %s, %d\n", __FILE__, __func__, __LINE__);
            myprintk("\n");
            break;
        case '3':
            printk("case 3\n");
            print_my_log_buf_status();
            break;
        default:
            break;
    }
    return len;
}

//定义一个名字为my_device_test的设备属性文件
static DEVICE_ATTR(my_device_test, S_IWUSR|S_IRUSR, show_my_device, set_my_device);

static int dma_open(struct inode *inode, struct file *file)
{
    DBG_PRINTK(KERN_WARNING"%s, %s, %d\n", __FILE__, __func__, __LINE__);
	return 0;
}

static struct file_operations dma_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   dma_open,     
};

static int dma_init(void)
{
	struct device *mydev;  
    struct proc_dir_entry *entry;
    DBG_PRINTK(KERN_WARNING"%s, %s, %d\n", __FILE__, __func__, __LINE__);

	major=register_chrdev(0,"dma", &dma_ops);
	cls=class_create(THIS_MODULE, "dma_class");
	mydev = device_create(cls, NULL, MKDEV(major,0),"dma_device");    //创建dma_device设备   

	if(sysfs_create_file(&(mydev->kobj), &dev_attr_my_device_test.attr)){    //在dma_device设备目录下创建一个my_device_test属性文件
		return -1;}
	return 0;
}

static void dma_exit(void)
{
	DBG_PRINTK(KERN_WARNING"%s, %s, %d\n", __FILE__, __func__, __LINE__);
	device_destroy(cls, MKDEV(major,0));
	class_destroy(cls);
	unregister_chrdev(major, "dma");
}

module_init(dma_init);
module_exit(dma_exit);
MODULE_LICENSE("GPL");

