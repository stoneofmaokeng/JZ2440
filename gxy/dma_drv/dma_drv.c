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
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>


#define CP_MEM_NO_DMA   0
#define CP_MEM_DMA   1

#define BUF_SIZE    (512*1024)

#define DMA3_BASE_ADDR (0x4B0000C0)

//#define DBG_PRINTK  printk
#define DBG_PRINTK(x...)
DECLARE_WAIT_QUEUE_HEAD(dma_wait);

static  char mybuf[100]="123";
static int major;
static struct class *cls;
static volatile dma_flag;

static void *src;
static void *dst;
static dma_addr_t src_phys;
static dma_addr_t dst_phys;

struct dma_regs {
    unsigned long disrc3; 
    unsigned long disrcc3;
    unsigned long didst3;
    unsigned long didstc3;
    unsigned long dcon3;
    unsigned long dstat3;
    unsigned long dcsrc3;
    unsigned long dcdst3;
    unsigned long dmasktrig3;
};

static volatile struct dma_regs* s3c_dma_regs;

int dma_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long arg)
{
    unsigned long i;
    unsigned long reg_value;
    int ret;
    switch (cmd) {
        case CP_MEM_NO_DMA:
            printk("cp no dma\n");
            i = 0;
            while (i != BUF_SIZE) {
                *((char*)dst + i) = *((char*)dst + i);
                i++;
            }
            ret =  memcmp(src, dst, BUF_SIZE);
            if (ret == 0) {
                printk("mem cp no dma ok\n");
            } else {
                printk("mem cp no dma fail\n");
            }
            break;
        case CP_MEM_DMA:
            printk("cp use dma\n");
            //设置DMA寄存器
            s3c_dma_regs->disrc3 = src_phys;
            s3c_dma_regs->disrcc3 = 0;
            s3c_dma_regs->didst3 = dst_phys;
            s3c_dma_regs->didstc3 = 0; 
            s3c_dma_regs->dcon3 = (1 << 30)|(1 << 29)|(0 << 28)|(1 << 27)|(0 << 23)|(1<<22)|(0<<20)|(BUF_SIZE<<0);
            //启动DMA
            dma_flag = 0;
            s3c_dma_regs->dmasktrig3 = (1 << 1) | (1 << 0);
            printk("wait\n");
            wait_event_interruptible(dma_wait, dma_flag);
            printk("wait over\n");
            ret =  memcmp(src, dst, BUF_SIZE);
            if (ret == 0) {
                printk("mem cp dma ok\n");
            } else {
                printk("mem cp dma fail\n");
            }
            break;
        default:
            break;
    }
    return 0;
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
            break;
        case '2':
            printk("case 2\n");
            //myprintk("%s, %s, %d\n", __FILE__, __func__, __LINE__);
            break;
        case '3':
            printk("case 3\n");
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

static irqreturn_t s3c_dma_irq(int irq, void *dev_id)
{
    printk("dma interrupt\n");
    dma_flag = 1;
    wake_up_interruptible(&dma_wait);
    return IRQ_HANDLED;
}

static int dma_init(void)
{
    int ret;
    struct device *mydev;  
    struct proc_dir_entry *entry;
    DBG_PRINTK(KERN_WARNING"%s, %s, %d\n", __FILE__, __func__, __LINE__);

    //申请DMA资源
    src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL); 
    if (src == NULL) {
        printk("src dma alloc fail\n");
        return -ENOMEM;
    }

    dst = dma_alloc_writecombine(NULL, BUF_SIZE, &dst_phys, GFP_KERNEL); 
    if (dst == NULL) {
        printk("dst dma alloc fail\n");
        goto dst_dma_error;
    }

    //映射DMA寄存器
    s3c_dma_regs = (struct dma_regs *)ioremap(DMA3_BASE_ADDR, sizeof(struct dma_regs));
    if (s3c_dma_regs  == NULL) {
        printk("dma_regs mmap fail\n");
        goto dma_regs_mmap_error;
    }


    //申请DMA中断
    ret = request_irq(IRQ_DMA3, s3c_dma_irq, 0, "s3c_dma", 1);
    if (ret < 0) {
        printk("irq fail\n");
        goto irq_error;
    }

    major=register_chrdev(0,"dma", &dma_ops);
    cls=class_create(THIS_MODULE, "dma_class");
    mydev = device_create(cls, NULL, MKDEV(major,0),"dma_device");    //创建dma_device设备   

    if(sysfs_create_file(&(mydev->kobj), &dev_attr_my_device_test.attr)){    //在dma_device设备目录下创建一个my_device_test属性文件
        return -1;}
    return 0;

irq_error:
dma_regs_mmap_error:
    dma_free_writecombine(NULL, BUF_SIZE, dst, dst_phys);
dst_dma_error:
    dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
    return -ENOMEM;
}

static void dma_exit(void)
{
    //free_irq(IRQ_DMA3, NULL);
    iounmap(s3c_dma_regs);
    device_destroy(cls, MKDEV(major,0));
    class_destroy(cls);
    unregister_chrdev(major, "dma");
    dma_free_writecombine(NULL, BUF_SIZE, dst, dst_phys);
    dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
    free_irq(IRQ_DMA3, 1);
}

module_init(dma_init);
module_exit(dma_exit);
MODULE_LICENSE("GPL");

