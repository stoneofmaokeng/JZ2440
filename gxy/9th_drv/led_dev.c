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

static struct resource led_dev_resource[] = {
	[0] = {
		.start = 0x56000050,
		.end   = 0x56000050 + 8 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = 4,
		.end   = 4,
		.flags = IORESOURCE_IRQ,
	},
};


struct platform_device my_led_dev = {
	.name		  = "my_led",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(led_dev_resource),
	.resource	  = led_dev_resource,
	.dev              = {
	}
};

static int led_dev_init(void)
{
    platform_device_register(&my_led_dev);
    return 0;
}

static void led_dev_exit(void)
{
    platform_device_unregister(&my_led_dev);
}

module_init(led_dev_init);
module_exit(led_dev_exit);
MODULE_LICENSE("GPL");
