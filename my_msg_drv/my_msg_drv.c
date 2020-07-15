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

//static int vprintk(const char *fmt, va_list args)
static int myprintk(const char *fmt, va_list args)
{
    int i;
	static char printk_buf[MY_LOG_BUF_SIZE];
	int printed_len;
	printed_len = vscnprintf(printk_buf, sizeof(printk_buf), fmt, args);
    for (i = 0; i < printed_len; i++) {
        if (!my_put_c(printk_buf[i])) {
            break;
        }
    }
    wake_up_interruptible(&my_log_wait);
    return i;
}


static int my_msg_open(struct inode *inode, struct file *file)
{
    DBG_PRINTK(KERN_WARNING"%s, %s, %d\n", __FILE__, __func__, __LINE__);
	spin_lock_init(&my_logbuf_lock);
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
        //c = LOG_BUF(log_start);
        c = my_log_buf[log_start];
        log_start++;
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
    DBG_PRINTK(KERN_WARNING"%s, %s, %d\n", __FILE__, __func__, __LINE__);
    struct proc_dir_entry *entry;
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

