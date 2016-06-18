/*		Definitions		*/
#define DRV_NAME "RMem"
#define PFX "[" DRV_NAME "]: "
#define DEV_NAME "RMem_dev"
#define DEV_MEM_SIZE 1024*1024
#define DEV_BASE_ADDR 0xA3E00000
//==========================================================================================
/*		Headers			*/
#include <linux/types.h>
#include <asm/io.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
//==========================================================================================




/*		Variables		*/
struct cdev *rmem_dev;
struct class *rmem_class;

static dev_t rmem_dev_no;

static char *rmem_virt_addr;
//=========================================================================================

/*		Declearations		*/
int rmem_alloc(int);
static int rmem_mmap(struct file*,struct vm_area_struct*);
void rmem_free(void);
static int rmem_init(void);
static void rmem_exit(void);
/*--------------------------*/
static const struct file_operations rmem_fops = {
	.owner = THIS_MODULE,
	.mmap = rmem_mmap,
};

int rmem_alloc(int req_buf_size)
{
	//CCU board (prom.ko) request memory region 0xA3000000 ~ 0xA3FFFFFF, redundant memory base addr is 0xA3E00000
	//we can directly use ioremap
	/*
	if(request_mem_region(DEV_BASE_ADDR,DEV_MEM_SIZE,DEV_NAME) == NULL)
	{
		printk(KERN_ERR PFX"%s:unable to obtain I/O memory address 0x%08X\n",DEV_NAME,DEV_BASE_ADDR);
		return -EBUSY;
	}
	*/
	rmem_virt_addr = ioremap(DEV_BASE_ADDR,DEV_MEM_SIZE);
	printk(KERN_INFO PFX"RMem virtual address is ==> 0x%lx\n",(long unsigned int)rmem_virt_addr);
	return 0;
}


static int rmem_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long start = vma->vm_start;
	unsigned long size = PAGE_ALIGN(vma->vm_end - vma->vm_start);

	printk(KERN_INFO PFX"someone called rmem_mmap\n");
	if(size > DEV_MEM_SIZE || !rmem_virt_addr)
	{
		printk(KERN_ERR PFX"size overflow\n");
		return -EINVAL;
	}
	return remap_pfn_range(vma, start, (virt_to_phys(rmem_virt_addr) >> PAGE_SHIFT), size, PAGE_SHARED);
}

void rmem_free()
{
	
}

static int rmem_init(void)
{
	printk(KERN_INFO PFX"initializing...\n");
	if((rmem_dev = cdev_alloc()) == NULL)
	{
		printk(KERN_ERR PFX"cdev_alloc failed\n");
		return -ENOMEM; // in prototype of cdev_alloc ,linux use kzalloc to request memory for cdev
	}
	if(alloc_chrdev_region(&rmem_dev_no,0,10,"RMem_dev"))
	{
		printk(KERN_ERR PFX"register char device failed\n");
		return -1;
	}
	cdev_init(rmem_dev, &rmem_fops);
	if(cdev_add(rmem_dev,rmem_dev_no,1))
	{
		printk(KERN_ERR PFX"add char device failedn\n");
		return -1;
	}
	rmem_class = class_create(THIS_MODULE,"RMem_class");
	if(IS_ERR(rmem_class))
	{
		printk(KERN_ERR PFX"create class failed\n");
		return -1;
	}
	device_create(rmem_class, NULL, rmem_dev_no, NULL, "RMem_dev");

	//int res;
	//if((res = rmem_alloc((int)DEV_MEM_SIZE)) < 0)
	if(rmem_alloc((int)DEV_MEM_SIZE) < 0)
	{
		printk(KERN_ERR PFX"alloc redundant memory failed\n");
		return -1;
	}

	return 0;
}

static void rmem_exit(void)
{
	rmem_free();
	cdev_del(rmem_dev);
	device_destroy(rmem_class,rmem_dev_no);
	class_destroy(rmem_class);
	iounmap(rmem_virt_addr);
	printk(KERN_INFO PFX"module removed,bye QAQ\n");
}

/*		Module Settings		*/
MODULE_AUTHOR("mythares hqzzm@hotmail.com");
MODULE_LICENSE("Dual BSD/GPL");
module_init(rmem_init);
module_exit(rmem_exit);
