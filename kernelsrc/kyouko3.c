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
#include <linux/delay.h>
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


MODULE_LICENSE("GPL");
MODULE_AUTHOR("James Burton");

struct cdev kyouko3_cdev;

struct fifo_entry

{
	u32 command;
	u32 value;

};


struct fifo
{
	// physical base address
	u64 p_base;
	// kernel base address array
	struct fifo_entry * k_base;
	// head of fifo
	u32 head;
	// tail of fifo
	u32 tail_cache;
};

struct kyouko3_dev
{
	unsigned int  p_control_base;
	unsigned int  p_card_ram_base;
	unsigned int  *k_control_base;
	unsigned int  *k_card_ram_base;
	struct pci_dev * pci_dev;
	unsigned int graphics_on;
	struct fifo fifo;

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

void K_WRITE_REG(unsigned int reg, unsigned int value)
{
	// hardware address to write to.
	unsigned int * hw_reg;

	// Why delay? Hardware can be flaky.
	// delay for 1 us to let hardware catch up.

	udelay(1);

	rmb();  //read memory barrier

	hw_reg = kyouko3.k_control_base + ( reg >> 2 );

	*hw_reg = value;



}

void FIFO_WRITE(unsigned int reg, unsigned int value)
{
	kyouko3.fifo.k_base[kyouko3.fifo.head].command = reg;
	kyouko3.fifo.k_base[kyouko3.fifo.head].value = value;
	kyouko3.fifo.head++;

	// TODO: read FIFO entries from card. Do this later.

	if (kyouko3.fifo.head >= FIFO_ENTRIES)
		kyouko3.fifo.head = 0;
}

static int kyouku3_init_fifo_intl(void)
{
	kyouko3.fifo.k_base = pci_alloc_consistent(
					kyouko3.pci_dev,
					8192u,
					&kyouko3.fifo.p_base);

	//Hardware has 1024 slots. Each has 8 bytes
	//(command, value)

	// load FifoStart with kyouko3.fifo.p_base
	K_WRITE_REG(FifoStart,kyouko3.fifo.p_base);


	// load FifoEnd with kyouko3.fifo.p_base + 8192
	K_WRITE_REG(FifoEnd,kyouko3.fifo.p_base + 8192);

	kyouko3.fifo.head = 0;
	kyouko3.fifo.tail_cache = 0;

	// pause

	while(K_READ_REG(FifoTail) != 0) schedule();

	// FifoTail is a registered. In the manual.

	return 1;

}


int kyouko3_open(struct inode * inode, struct file * fp)
{


	printk(KERN_ALERT "Opening device kyouko3\n");

	//2.3.A Uses ioremap to obtain a kernel virtual base for the control region
	kyouko3.k_control_base = ioremap(kyouko3.p_control_base, KYOUKO3_CONTROL_SIZE);
	//2.3.B Uses ioremap to obtain a kernel virtual base for the ram base
	printk(KERN_ALERT "K_READ_REG(Device_RAM) is %d\n",K_READ_REG(Device_RAM));
//	printk(KERN_ALERT "K_READ_REG(KYOUKO3_CONTROL_SIZE) is %d\n",K_READ_REG(KYOUKO3_CONTROL_SIZE));

	kyouko3.k_card_ram_base = ioremap(kyouko3.p_card_ram_base, K_READ_REG(Device_RAM) * 1024 * 1024);
	//2.3.C uses printk to print an opened device message
	printk(KERN_ALERT "Opened device kyouko3\n");

 	//2.3.D (optional) Allocate and initialize FIFO
	kyouku3_init_fifo_intl();

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
	pci_free_consistent(
						kyouko3.pci_dev,
						8192u,
						kyouko3.fifo.k_base,
						kyouko3.fifo.p_base
						);

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
	// Map the framebuffer
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

static void kyouku3_fifo_flush_intl(void)
{
	K_WRITE_REG(FifoHead,kyouko3.fifo.head);
	while(kyouko3.fifo.tail_cache != kyouko3.fifo.head)
	{
		kyouko3.fifo.tail_cache = K_READ_REG(FifoTail);
		schedule();
	}
}

long kyouko3_ioctl(struct file * fp, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	switch(cmd)
	{
		case VMODE:
					if ( arg == GRAPHICS_ON)
					{
						//setframe 0
						// set 5 registers at 0x8000
							// columns = 1024
							// rows = 768
							// pitch = 1024 * 4
							// format = 0xF888  // bits per channel
							// address = 0
						//set acceleration bitmask
							// 0x40000000
							// K_WRITE_REG it.
						//set dac 0  - Digital Analog Converter
							// width = 1024
							// height = 768
							// virtx = 0
							// virty = 0
							// frame = 0
						//modeset register
						msleep(10);
						// load clear color reg.  - FIFO_WRITE
						//
						// needs 4 floats: RGBA between [0.0,1.0] written as ints.
						// float one = 1.0
						// one_cs_int = *(unsigned int *)&one;
							// create REINTERPRET_CAST macro for this!
						// FIFO_WRITE  0x03 to ClearBuffer register
						// FIFO_WRITE 0x00 to Flush register
						kyouko3.graphics_on = 1;
					}
					else
					{
						// write 0 to acceleration bitmask.
						// write 0 to mode set.
					}
					break;
		/*
		case FIFO_QUEUE:
					// pull fifo entry from user space -> add to driver buffer
					// user will pass address of entry in arg

					ret = copy_from_user(
						&entry,  // entry is driver side variable.
						(struct fifo_entry *)arg,  // cast arg as struct fifo_entry *
						sizeof(struct_fifo_entry));

					// Might want to check to see if the queue is full.
					FIFO_WRITE(entry.command, entry.value);

					break;
		case FIFO_FLUSH:
					kyouku3_fifo_flush_intl();
					break;

		// see more in register list
		*/

	}
	return ret;

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
	.unlocked_ioctl = kyouko3_ioctl,
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

