#include <linux/module.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>
static int tx_cnt = 0;
static struct net_device *vnet_dev = NULL;
static int vnet_tx(struct sk_buff *skb, struct net_device *dev) 
{
    printk("vnet_tx cnt = %d\n", tx_cnt++);
    dev->stats.tx_packets++;
    dev->stats.tx_bytes+skb->len;
	return 0;
}

static void vnet_setup(struct net_device *dev)
{
	dev->hard_start_xmit	= vnet_tx;
    dev->dev_addr[0] = 0x08;
    dev->dev_addr[1] = 0x89;
    dev->dev_addr[2] = 0x89;
    dev->dev_addr[3] = 0x89;
    dev->dev_addr[4] = 0x89;
    dev->dev_addr[5] = 0x89;
}

static int vnet_init(void)
{
	vnet_dev = alloc_netdev(0, "vnet%d", vnet_setup);
	if (vnet_dev == NULL) {
        printk("alloc_netdev failed\n");
        return -1;
	}

	if (register_netdev(vnet_dev)) {
        printk("regitster_netdev failed\n");
        return -1;
	}
    return 0;
}

static void vnet_exit(void)
{
	unregister_netdev(vnet_dev);
	if (vnet_dev)
		free_netdev(vnet_dev);
}

module_init(vnet_init);
module_exit(vnet_exit);
MODULE_LICENSE("GPL");
