#include "linux/kern_levels.h"
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>          /* printk(), module_init(), module_exit() */
#include <linux/init.h>            /* __init __exit */
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>          /* dev_t */
#include <linux/uaccess.h>         /* copy_to_user(), copy_from_user() */
#include <linux/errno.h>           /* error macros */
#include <linux/device.h>

#include <linux/io.h>              /* iowrite() ioread() */
#include <linux/slab.h>            /* kmalloc() kfree() */
#include <linux/platform_device.h> /* struct platform driver */
#include <linux/ioport.h>          /* ioremap(), request_mem_region() */
#include <linux/of_address.h>
#include "linux/interrupt.h"       /* request_irq() */

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Platform GPIO Driver");
MODULE_AUTHOR("bad63r");


#define BUFF_SIZE 20
#define DRIVER_NAME "axiTimerDriver"

struct axiTimer_info {
  unsigned long mem_start;
  unsigned long mem_end;
  int           irq_num;
  void __iomem *base_addr;
};

dev_t my_dev_id;
static struct class    *my_class;
static struct device   *my_device;
static struct cdev     *my_cdev;
static struct axiTimer_info *tp = NULL;

int endRead = 0;


static int axiTimer_probe(struct platform_device *pdev);
static int axiTimer_remove(struct platform_device *pdev);
int        axiTimer_open(struct inode *pinode, struct file *pfile);
int        axiTimer_close(struct inode *pinode, struct file *pfile);
ssize_t    axiTimer_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t    axiTimer_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

static int __init  axiTimer_init(void);
static void __exit axiTimer_exit(void);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = axiTimer_open,
	.read = axiTimer_read,
	.write = axiTimer_write,
	.release = axiTimer_close,
};

static struct of_device_id axiTimer_of_match[] = {
  { .compatible = "axiTimer", },
  { /* end of list */ },
};

static struct platform_driver axiTimer_driver = {
  .driver = {
    .name = DRIVER_NAME,
    .owner = THIS_MODULE,
    .of_match_table	= axiTimer_of_match,
  },
  .probe		= axiTimer_probe,
  .remove		= axiTimer_remove,
};


MODULE_DEVICE_TABLE(of, axiTimer_of_match);


irqreturn_t axiTimerHandler(int, void *)
{

}

static int axiTimer_probe(struct platform_device *pdev)
{
  struct resource *r_mem;
  int rc = 0;

  /* get physical address space from device tree */
  r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  if (!r_mem) {
    printk(KERN_ALERT "Failed to get resource\n");
    return -ENODEV;
  }

  /* get interrupt number from device tree */
  tp->irq_num = platform_get_irq(pdev, 0);
  if (!tp->irq_num)
  {
    printk(KERN_ERR "Can't get resource for IRQ of a axi timer. \n");
    return -ENODEV;
  }

  /* allocate memory for axiTimer_info structure */
  tp = (struct axiTimer_info *) kmalloc(sizeof(struct axiTimer_info), GFP_KERNEL);
  if (!tp) {
    printk(KERN_ALERT "Could not allocate axiTimer device\n");
    return -ENOMEM;
  }

  /* put physical addresses in timer axiTimer info structure */
  tp->mem_start = r_mem->start;
  tp->mem_end = r_mem->end;

  /* reserver memory in kernel space */
  if (!request_mem_region(tp->mem_start,tp->mem_end - tp->mem_start + 1,	DRIVER_NAME))
  {
    printk(KERN_ALERT "Could not lock memory region at %p\n",(void *)tp->mem_start);
    rc = -EBUSY;
    goto error1;
  }

  /* reserver IRQ num in kernel space */
  if (request_irq(tp->irq_num, axiTimerHandler, 0, DRIVER_NAME, NULL))
  {
    printk(KERN_ERR "Can't reserver IRQ line for axitimer! \n");
    goto error2;
  }

  /* maker virtual memory of a driver */
  tp->base_addr = ioremap(tp->mem_start, tp->mem_end - tp->mem_start + 1);
  if (!tp->base_addr) {
    printk(KERN_ALERT "Could not allocate memory\n");
    rc = -EIO;
    goto error3;
  }

  printk(KERN_WARNING "axi Timer platform driver registered\n");
  return 0;//ALL OK

error3:
  free_irq(tp->irq_num, NULL);
error2:
  release_mem_region(tp->mem_start, tp->mem_end - tp->mem_start + 1);
  kfree(tp);
error1:
  return rc;
}


static int axiTimer_remove(struct platform_device *pdev)
{
  // Disable timer
  unsigned int data=0;
  data = ioread32(tp->base_addr + AXI_TIMER_TCSR_OFFSET);
  iowrite32(data & ~(TIMER_CSR_ENABLE_TMR_MASK),
          tp->base_addr + TIMER_TCSR_OFFSET);
  /* free resources taken in probe */
  free_irq(tp->irq_num, NULL);
  iowrite32(0, tp->base_addr);
  iounmap(tp->base_addr);
  release_mem_region(tp->mem_start, tp->mem_end - tp->mem_start + 1);
  kfree(tp);

  printk(KERN_WARNING "axi Timer platform driver removed\n");
  return 0;
}


int axiTimer_open(struct inode *pinode, struct file *pfile)
{
		//printk(KERN_INFO "Succesfully opened axiTimer\n");
		return 0;
}


int axiTimer_close(struct inode *pinode, struct file *pfile)
{
		//printk(KERN_INFO "Succesfully closed axiTimer\n");
		return 0;
}


ssize_t axiTimer_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	int ret;
	int len     = 0;
	u32 led_val = 0;
	int i = 0;
	char buff[BUFF_SIZE];

	if (endRead){
		endRead = 0;
		return 0;
	}

	led_val = ioread32(tp->base_addr);

	//buffer: 0b????
	//index:  012345

	buff[0]= '0';
	buff[1]= 'b';
	for(i=0;i<4;i++)
	{
		if((led_val >> i) & 0x01)
			buff[5-i] = '1';
		else
			buff[5-i] = '0';
	}
	buff[6]= '\n';
	len=7;
	ret = copy_to_user(buffer, buff, len);
	if(ret)
		return -EFAULT;
	//printk(KERN_INFO "Succesfully read\n");
	endRead = 1;

	return len;
}


ssize_t axiTimer_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
	char buff[BUFF_SIZE];
	int ret = 0;
	long int led_val=0;

	ret = copy_from_user(buff, buffer, length);
	if(ret)
		return -EFAULT;
	buff[length] = '\0';

	// HEX  INPUT
	if(buff[0] == '0' && (buff[1] == 'x' || buff[1] == 'X')) 
	{
		ret = kstrtol(buff+2,16,&led_val);
	}
	// BINARY INPUT
	else if(buff[0] == '0'  && (buff[1] == 'b' || buff[1] == 'B')) 
	{
		ret = kstrtol(buff+2,2,&led_val);
	}
	// DECIMAL INPUT
	else 
	{
		ret = kstrtol(buff,10,&led_val);
	}

	if (!ret)
	{
		iowrite32((u32)led_val, tp->base_addr);
		//printk(KERN_INFO "Succesfully wrote value %#x",(u32)led_val); 
	}
	else
	{
		printk(KERN_INFO "Wrong command format\n"); 
	}

	return length;
}


static int __init axiTimer_init(void)
{
   int ret = 0;

	//Initialize array

   ret = alloc_chrdev_region(&my_dev_id, 0, 1, DRIVER_NAME);
   if (ret){
      printk(KERN_ERR "failed to register char device\n");
      return ret;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create("axiTimer_class");
   if (my_class == NULL){
      printk(KERN_ERR "failed to create class\n");
      goto fail_0;
   }
   printk(KERN_INFO "class created\n");
   
   my_device = device_create(my_class, NULL, my_dev_id, NULL, DRIVER_NAME);
   if (my_device == NULL){
      printk(KERN_ERR "failed to create device\n");
      goto fail_1;
   }
   printk(KERN_INFO "device created\n");

	my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1);
	if (ret)
	{
      printk(KERN_ERR "failed to add cdev\n");
		goto fail_2;
	}
   printk(KERN_INFO "cdev added\n");
   printk(KERN_INFO "GPIO driver added to the kernel\n");

  return platform_driver_register(&axiTimer_driver);

   fail_2:
      device_destroy(my_class, my_dev_id);
   fail_1:
      class_destroy(my_class);
   fail_0:
      unregister_chrdev_region(my_dev_id, 1);
   return -1;
}

static void __exit axiTimer_exit(void)
{
   platform_driver_unregister(&axiTimer_driver);
   cdev_del(my_cdev);
   device_destroy(my_class, my_dev_id);
   class_destroy(my_class);
   unregister_chrdev_region(my_dev_id,1);
   printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(axiTimer_init);
module_exit(axiTimer_exit);
