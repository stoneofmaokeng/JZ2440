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
#define MY_LOG_BUF_SIZE     (128)
static char my_log_buf[MY_LOG_BUF_SIZE];
static int buf_start = 0;
static int buf_start_for_read = 0;
static int buf_end = 0;
static DEFINE_SPINLOCK(my_logbuf_lock);
DECLARE_WAIT_QUEUE_HEAD(my_log_wait);
static void print_my_log_buf_status()
{
    printk("buf_start = %d\n", buf_start);
    printk("buf_end = %d\n", buf_end);
}

static int my_log_buf_full()
{
    if (((buf_end + 1) % MY_LOG_BUF_SIZE) == buf_start) {
        return 1;
    } else {
        return 0;
    }
}

static int my_log_buf_empty()
{
    if (buf_end == buf_start) {
        return 1;
    } else {
        return 0;
    }
}

static int my_log_buf_empty_for_read()
{
    if (buf_end == buf_start_for_read) {
        return 1;
    } else {
        return 0;
    }
}

static int my_put_c(char c)
{
    if (my_log_buf_full()) {
        buf_start = (buf_start + 1) % MY_LOG_BUF_SIZE;
        buf_start_for_read = buf_start;
    }
    my_log_buf[buf_end] = c;
    buf_end = (buf_end + 1) % MY_LOG_BUF_SIZE;
    return 1;
}

static int my_get_c(char* c)
{
    if (my_log_buf_empty()) {
        return 0;
    }
    *c = my_log_buf[buf_start];
    buf_start = (buf_start+ 1) % MY_LOG_BUF_SIZE;
    return 1;
}

static int my_get_c_for_read(char* c)
{
    if (my_log_buf_empty_for_read()) {
        return 0;
    }
    *c = my_log_buf[buf_start_for_read];
    buf_start_for_read = (buf_start_for_read+ 1) % MY_LOG_BUF_SIZE;
    return 1;
}

//int sprintf(char * buf, const char *fmt, ...)
int myprintk(const char *fmt, ...)
{
    static char printk_buf[MY_LOG_BUF_SIZE];
    int printed_len;
    va_list args;
    int i;

    va_start(args, fmt);
    printed_len=vsprintf(printk_buf,fmt,args);
    va_end(args);
    for (i = 0; i < printed_len; i++) {
        if (!my_put_c(printk_buf[i])) {
            break;
        }
    }
    wake_up_interruptible(&my_log_wait);
    return i;
}
EXPORT_SYMBOL(myprintk);

static  char mybuf[100]="123";
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


static int major;
static struct class *cls;

struct file_operations mytest_ops={
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
};


static DEVICE_ATTR(my_device_test, S_IWUSR|S_IRUSR, show_my_device, set_my_device);
                //定义一个名字为my_device_test的设备属性文件





static int my_msg_open(struct inode *inode, struct file *file)
{
    DBG_PRINTK(KERN_WARNING"%s, %s, %d\n", __FILE__, __func__, __LINE__);
    buf_start_for_read = buf_start;
    //myprintk("guoxiaoy\n");
    return 0;
}


static ssize_t my_msg_read(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
    int error = 0;
    unsigned long i, j;
    char c;
    DBG_PRINTK(KERN_WARNING"%s, %s, %d\n", __FILE__, __func__, __LINE__);
    if ((file->f_flags & O_NONBLOCK) && my_log_buf_empty_for_read())
        return -EAGAIN;

    wait_event_interruptible(my_log_wait,
                        !my_log_buf_empty_for_read());
    i = 0;
    spin_lock_irq(&my_logbuf_lock);
    while (my_get_c_for_read(&c) && (i < count)) {
        spin_unlock_irq(&my_logbuf_lock);
        __put_user(c,buf);
        buf++;
        i++;
        spin_lock_irq(&my_logbuf_lock);
    }
    spin_unlock_irq(&my_logbuf_lock);
    return i;
}

static struct file_operations my_msg_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   my_msg_open,     
    .read    =    my_msg_read,       
};


static int my_msg_init(void)
{
    struct device *mydev;  
    struct proc_dir_entry *entry;
    DBG_PRINTK(KERN_WARNING"%s, %s, %d\n", __FILE__, __func__, __LINE__);
    spin_lock_init(&my_logbuf_lock);
    entry = create_proc_entry("mymsg", S_IRUSR, &proc_root);
    if (entry)
        entry->proc_fops = &my_msg_fops;

    major=register_chrdev(0,"mytest", &mytest_ops);
    cls=class_create(THIS_MODULE, "mytest_class");
    mydev = device_create(cls, NULL, MKDEV(major,0),"mytest_device");    //创建mytest_device设备   

    if(sysfs_create_file(&(mydev->kobj), &dev_attr_my_device_test.attr)){    //在mytest_device设备目录下创建一个my_device_test属性文件
        return -1;}
    return 0;
}

static void my_msg_exit(void)
{
    DBG_PRINTK(KERN_WARNING"%s, %s, %d\n", __FILE__, __func__, __LINE__);
    device_destroy(cls, MKDEV(major,0));
    class_destroy(cls);
    unregister_chrdev(major, "mytest");
}

module_init(my_msg_init);
module_exit(my_msg_exit);
MODULE_LICENSE("GPL");

