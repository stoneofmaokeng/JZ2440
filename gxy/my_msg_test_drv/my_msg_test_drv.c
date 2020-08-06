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


static int my_msg_test_init(void)
{
    myprintk("%s, %s, %d\n", __FILE__, __func__, __LINE__);
    return 0;
}

static void my_msg_test_exit(void)
{
    myprintk("%s, %s, %d\n", __FILE__, __func__, __LINE__);
}

module_init(my_msg_test_init);
module_exit(my_msg_test_exit);
MODULE_LICENSE("GPL");

