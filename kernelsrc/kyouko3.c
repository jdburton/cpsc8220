/*
 ============================================================================
 Name        : kyouko3.c
 Author      : James Burton
 Version     :
 Copyright   : Copyright 2016
 Description : Hello World as a Linux kernel driver
 ============================================================================
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/kernel_stat.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/delay.h>
#include "kyouko3-registers.h"
#include "kyouko3-utils.h"

#define KYOUKO3_MAJOR 500
#define KYOUKO3_MINOR 127

#ifndef PCI_VENDOR_ID_CCOURSI
#define PCI_VENDOR_ID_CCOURSI 0x1234
#endif

#ifndef PCI_DEVICE_ID_KYOUKO3
#define PCI_DEVICE_ID_KYOUKO3 0x1113
#endif

#define KYOUKO3_CONTROL_SIZE 65536
#define DeviceRAM 0x0020

enum
{
	MMAP_CONTROL = 0,
	MMAP_FRAMEBUFFER = 0x8000
};


MODULE_LICENSE("GPL");
MODULE_AUTHOR("James Burton");

struct cdev kyouko3_cdev;

struct kyouko3_dev
{
	unsigned int  p_control_base;
	unsigned int  p_card_ram_base;
	unsigned int  *k_control_base;
	unsigned int  *k_card_ram_base;
	struct pci_dev * pci_dev;

} kyouko3;



unsigned int K_READ_REG(unsigned int reg)
{

	// reg == byte offset from control base.
	unsigned int value;

	// Why delay? Hardware can be flaky.
	// delay for 1 us to let hardware catch up.
	udelay(1);

	rmb();  //read memory barrier

	value = *((kyouko3.k_control_base + ( reg >> 2 )));

	// why divide by 4? Because it's a byte offset.  4 byte word = 32 bit.

	return(value);

}

int kyouko3_open(struct inode * inode, struct file * fp)
{


	printk(KERN_ALERT "Opening device kyouko3\n");

	//2.3.A Uses ioremap to obtain a kernel virtual base for the control region
	kyouko3.k_control_base = ioremap(kyouko3.p_control_base, KYOUKO3_CONTROL_SIZE);
	//2.3.B Uses ioremap to obtain a kernel virtual base for the ram base
	printk(KERN_ALERT "K_READ_REG(DeviceRAM) is %d\n",K_READ_REG(DeviceRAM));
//	printk(KERN_ALERT "K_READ_REG(KYOUKO3_CONTROL_SIZE) is %d\n",K_READ_REG(KYOUKO3_CONTROL_SIZE));

	kyouko3.k_card_ram_base = ioremap(kyouko3.p_card_ram_base, K_READ_REG(DeviceRAM) * 1024 * 1024);
	//2.3.C uses printk to print an opened device message
	printk(KERN_ALERT "Opened device kyouko3\n");

 	//2.3.D (optional) Allocate and initialize FIFO

	return 0;

}

int kyouko3_release(struct inode * inode, struct file * fp)
{

	printk(KERN_ALERT "Releasing device kyouko3\n");

	// 2.4.A - Call iounmap
	iounmap(kyouko3.k_control_base);
	iounmap(kyouko3.k_card_ram_base);
	// 2.4.B - say "BUUH BYE"
	printk(KERN_ALERT "BUUH BYE\n");
	// 2.4.C If you have allocated FIFO, call pci_free_consistent

	return 0;

}


static int kyouko3_mmap(struct file *filp, struct vm_area_struct *vma)
{

	if (vma->vm_pgoff << PAGE_SHIFT == MMAP_CONTROL)
	{
		// 2.5.A A kyouko3_mmap function that calls io_remap_pfn_range to provide user level access to the 64KB control region of the card
		if (io_remap_pfn_range(vma, vma->vm_start, kyouko3.p_control_base >> PAGE_SHIFT,
					vma->vm_end - vma->vm_start,
					vma->vm_page_prot))
		{

			return -EAGAIN;
		}
	}
	else if (vma->vm_pgoff << PAGE_SHIFT == MMAP_FRAMEBUFFER)
	{
		if (io_remap_pfn_range(vma, vma->vm_start, kyouko3.p_card_ram_base >> PAGE_SHIFT,
					vma->vm_end - vma->vm_start,
					vma->vm_page_prot))
		{

			return -EAGAIN;
		}
	}
	else
	{
		printk (KERN_ALERT "Invalid offset %x!\n", vma->vm_start);
		return -EAGAIN;
	}

    return 0;
}


int kyouko3_probe(struct pci_dev * pci_dev, const struct pci_device_id * pci_id)
{

	// Arguments point to device that is found when it is called

	// Need 3 pieces of information:

	//    1. The physical base address of control region

	kyouko3.p_control_base = pci_resource_start(pci_dev, 1);

	//     2. The physical base address of the on-board RAM (framebuffer)
	//          Framebuffer access is slow. Not usually used today.

	kyouko3.p_card_ram_base = pci_resource_start(pci_dev,2);

	//     3.  The interrupt line:
	//
	//           pci_dev -> irq;

	// but don't save IRQ (can be overwritten).

	// 2.1.C Save pci_dev pointer
	// pci_dev pointer saved at kyouko3.pci_dev in probe function.


	kyouko3.pci_dev = pci_dev;

	// 2.1.D. Enable bus mastering and memory access by card.

	// enable main memory access by the device

	pci_enable_device(pci_dev);

	// Finally, we need to allow the device to initiate DMA transfers

	pci_set_master(pci_dev);
	return 0;


}


int kyouko3_remove(void)
{
	pci_disable_device(kyouko3.pci_dev);
	return 0;
}

struct file_operations kyouko3_fops =
{
	.open = kyouko3_open,
	.release = kyouko3_release,
	.mmap = kyouko3_mmap,
	// .unlocked_ioctl = kyouko3_ioctl,
	.owner = THIS_MODULE
};

//This call will use its argument to search the list of device-vendor pairs to try to find a match on the PCIe bus.

struct pci_device_id kyouko3_dev_ids[] =
{
	{ PCI_DEVICE(PCI_VENDOR_ID_CCOURSI, PCI_DEVICE_ID_KYOUKO3) },
	{ 0 }
};



struct pci_driver kyouko3_pci_drv =
{
	.name = "kyouko3",
	.id_table = kyouko3_dev_ids, // type struct pci_dev_id[]
	.probe = kyouko3_probe,
	.remove = kyouko3_remove
};





static int kyouko3_init(void)
{
	printk(KERN_ALERT "Initializing device kyouko3\n");

	// 2.1.A register the character device

	cdev_init(&kyouko3_cdev, &kyouko3_fops);
	cdev_add(&kyouko3_cdev,MKDEV(KYOUKO3_MAJOR,KYOUKO3_MINOR),1);

	// 2.1.B Scan the PCI bus for the card
	pci_register_driver(&kyouko3_pci_drv);


	// 2.1.C Save pci_dev pointer
	// pci_dev pointer saved at kyouko3.pci_dev in probe function.

	// 2.1.D Enable bus mastering and memory access by card.
	// Done in probe function.

	return 0;

}

static void kyouko3_exit(void)
{

	printk(KERN_ALERT "Deleting device kyouko3\n");

	// 2.2.1 An exit function that unregisters the character device
	pci_unregister_driver(&kyouko3_pci_drv);
	cdev_del(&kyouko3_cdev);

}


module_init(kyouko3_init);
module_exit(kyouko3_exit);

