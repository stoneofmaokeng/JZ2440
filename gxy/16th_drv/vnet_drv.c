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
#include <linux/skbuff.h>

#include <linux/ip.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>



static int tx_cnt = 0;
static struct net_device *vnet_dev = NULL;

static void emulator_rx_packet(struct sk_buff *skb, struct net_device *dev)
{
    /* 参考LDD3 */
    unsigned char *type;
    struct iphdr *ih;
    __be32 *saddr, *daddr, tmp;
    unsigned char   tmp_dev_addr[ETH_ALEN];
    struct ethhdr *ethhdr;
    
    struct sk_buff *rx_skb;
        
    // 从硬件读出/保存数据
    /* 对调"源/目的"的mac地址 */
    ethhdr = (struct ethhdr *)skb->data;
    memcpy(tmp_dev_addr, ethhdr->h_dest, ETH_ALEN);
    memcpy(ethhdr->h_dest, ethhdr->h_source, ETH_ALEN);
    memcpy(ethhdr->h_source, tmp_dev_addr, ETH_ALEN);

    /* 对调"源/目的"的ip地址 */    
    ih = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
    saddr = &ih->saddr;
    daddr = &ih->daddr;

    tmp = *saddr;
    *saddr = *daddr;
    *daddr = tmp;
    
    //((u8 *)saddr)[2] ^= 1; /* change the third octet (class C) */
    //((u8 *)daddr)[2] ^= 1;
    type = skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr);
    //printk("tx package type = %02x\n", *type);
    // 修改类型, 原来0x8表示ping
    *type = 0; /* 0表示reply */
    
    ih->check = 0;         /* and rebuild the checksum (ip needs it) */
    ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);
    
    // 构造一个sk_buff
    rx_skb = dev_alloc_skb(skb->len + 2);
    skb_reserve(rx_skb, 2); /* align IP on 16B boundary */  
    memcpy(skb_put(rx_skb, skb->len), skb->data, skb->len);

    /* Write metadata, and then pass to the receive level */
    rx_skb->dev = dev;
    rx_skb->protocol = eth_type_trans(rx_skb, dev);
    rx_skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
    dev->stats.rx_packets++;
    dev->stats.rx_bytes += skb->len;

    // 提交sk_buff
    netif_rx(rx_skb);
}



static int vnet_tx(struct sk_buff *skb, struct net_device *dev) 
{
    printk("vnet_tx cnt = %d\n", tx_cnt++);
    netif_stop_queue(dev);
    emulator_rx_packet(skb, dev);
    dev_kfree_skb(skb);
    netif_wake_queue(dev);
    dev->stats.tx_packets++;
    dev->stats.tx_bytes+=skb->len;
	return 0;
}

static void vnet_setup(struct net_device *dev)
{
    printk("vnet_setup\n");
}

static int vnet_init(void)
{
	//vnet_dev = alloc_netdev(0, "vnet%d", vnet_setup);
	vnet_dev = alloc_netdev(0, "vnet%d", ether_setup);
	if (vnet_dev == NULL) {
        printk("alloc_netdev failed\n");
        return -1;
	}

	vnet_dev->hard_start_xmit	= vnet_tx;
    vnet_dev->dev_addr[0] = 0x08;
    vnet_dev->dev_addr[1] = 0x89;
    vnet_dev->dev_addr[2] = 0x89;
    vnet_dev->dev_addr[3] = 0x89;
    vnet_dev->dev_addr[4] = 0x89;
    vnet_dev->dev_addr[5] = 0x89;

	vnet_dev->flags           |= IFF_NOARP;
	vnet_dev->features        |= NETIF_F_NO_CSUM; 

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
