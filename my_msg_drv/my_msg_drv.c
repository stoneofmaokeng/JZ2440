#include <linux/module.h>
#include <linux/kernel.h>
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


#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/completion.h>
#include <linux/compat.h>
#include <linux/chio.h>			/* here are all the ioctls */
#include <linux/mutex.h>

#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_driver.h>
#include <scsi/scsi_ioctl.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_eh.h>
#include <scsi/scsi_dbg.h>


#define DBG_PRINTK  printk
//#define DBG_PRINTK(x...)
#define MY_LOG_BUF_SIZE     (128)
static char my_log_buf[MY_LOG_BUF_SIZE];
static int buf_start = 0;
static int buf_end = 0;
static DEFINE_SPINLOCK(my_logbuf_lock);
DECLARE_WAIT_QUEUE_HEAD(my_log_wait);

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

static int my_put_c(char c)
{
    if (my_log_buf_full()) {
        return 0;
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


static int my_msg_open(struct inode *inode, struct file *file)
{
    DBG_PRINTK(KERN_WARNING"%s, %s, %d\n", __FILE__, __func__, __LINE__);
	spin_lock_init(&my_logbuf_lock);
    buf_start = 0;
    buf_end = 0;
    myprintk("guoxiaoy\n");
	return 0;
}


static ssize_t my_msg_read(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int error = 0;
	unsigned long i, j;
	char c;
    DBG_PRINTK(KERN_WARNING"%s, %s, %d\n", __FILE__, __func__, __LINE__);
	if ((file->f_flags & O_NONBLOCK) && my_log_buf_empty())
		return -EAGAIN;

    wait_event_interruptible(my_log_wait,
                        (buf_start - buf_end));
    i = 0;
    spin_lock_irq(&my_logbuf_lock);
    while ((buf_start != buf_end) && i < count) {
        //c = LOG_BUF(buf_start);
        if (my_log_buf_empty()) {
            break;
        }
        c = my_log_buf[buf_start];
        buf_start++;
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
	.read	=	my_msg_read,	   
};


static int my_msg_init(void)
{
    struct proc_dir_entry *entry;
    DBG_PRINTK(KERN_WARNING"%s, %s, %d\n", __FILE__, __func__, __LINE__);
    entry = create_proc_entry("mymsg", S_IRUSR, &proc_root);
    if (entry)
        entry->proc_fops = &my_msg_fops;
	return 0;
}

static void my_msg_exit(void)
{
    DBG_PRINTK(KERN_WARNING"%s, %s, %d\n", __FILE__, __func__, __LINE__);
}

module_init(my_msg_init);
module_exit(my_msg_exit);
MODULE_LICENSE("GPL");

