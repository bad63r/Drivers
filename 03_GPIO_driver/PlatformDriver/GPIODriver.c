#include <linux/module.h> /* #include dynamic_debug inserts printk() implicitly;
							 module_init(), module_exit()*/
#include <linux/init.h> /* __init, __exit */
#include <linux/kdev_t.h> /* MKDEV, implicitly dev_t type*/
#include <linux/device.h> /* device_create(), device_destroy() */
#include <linux/cdev.h> /* all functions related to cdev a.k.a. character device */
#include <linux/fs.h> /* struct file operations */
#include <linux/platform_device.h> /* struct platform_driver */
#include <linux/errno.h> /* errors */
#include <linux/io.h> /* iowrite ioread */
#include <linux/slab.h> /* kmalloc kfree */
#include <linux/ioport.h> /* ioremap */

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple file operations driver.");
MODULE_AUTHOR("bad63r");

dev_t devID;
static struct class *classPtr;
static struct device* devicePtr;
static struct cdev* cdevPtr;
static struct gpio_info *gp = NULL;

#define DRIVER_NAME "ledGPIO"
#define BUFF_SIZE 20
int endRead = 0;

struct gpio_info {
  unsigned long mem_start;
  unsigned long mem_end;
  void __iomem *base_addr;
};

static int gpioProbe(struct platform_device *pdev)
{
  struct resource *r_mem;
  int rc = 0;
  r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  if (!r_mem) {
    printk(KERN_ALERT "Failed to get resource\n");
    return -ENODEV;
  }
  gp = (struct gpio_info *) kmalloc(sizeof(struct gpio_info), GFP_KERNEL);
  if (!gp) {
    printk(KERN_ALERT "Could not allocate led device\n");
    return -ENOMEM;
  }

  gp->mem_start = r_mem->start;
  gp->mem_end = r_mem->end;
  //printk(KERN_INFO "Start address:%x \t end address:%x", r_mem->start, r_mem->end);

  if (!request_mem_region(gp->mem_start,gp->mem_end - gp->mem_start + 1,	DRIVER_NAME))
  {
    printk(KERN_ALERT "Could not lock memory region at %p\n",(void *)gp->mem_start);
    rc = -EBUSY;
    goto error1;
  }

  gp->base_addr = ioremap(gp->mem_start, gp->mem_end - gp->mem_start + 1);
  if (!gp->base_addr) {
    printk(KERN_ALERT "Could not allocate memory\n");
    rc = -EIO;
    goto error2;
  }

  printk(KERN_WARNING "led gpio platform driver registered\n");
  return 0;//ALL OK

error2:
  release_mem_region(gp->mem_start, gp->mem_end - gp->mem_start + 1);
error1:
  return rc;
}

static int gpioRemove(struct platform_device *pdev)
{
  printk(KERN_WARNING "led gpio platform driver removed\n");
  iowrite32(0, gp->base_addr);
  iounmap(gp->base_addr);
  release_mem_region(gp->mem_start, gp->mem_end - gp->mem_start + 1);
  kfree(gp);
  return 0;
}


static struct of_device_id gpio_of_match[] = {
  { .compatible = "led_gpio", },
  { /* end of list */ },
};

static struct platform_driver GPIODriverPlatform = {
	.driver = {
		.name = ledGPIO,
		.owner = THIS_MODULE,
		.of_match_table = gpio_of_match,
	},	
	.probe = gpioProbe,
	.remove = gpioRemove,
}

MODULE_DEVICE_TABLE(of, gpio_of_match);

ssize_t GPIODriverRead (struct file *filePtr, char __user *buffer, size_t length, loff_t *offset)
{
	int ret;
	int len = 0;
	u32 gpio_val = 0;
	int i = 0;
	char buff[BUFF_SIZE];
	if (endRead){
		endRead = 0;
		return 0;
	}

	gpio_val = ioread32(gp->base_addr);

	//buffer: 0b????
	//index:  012345

	buff[0]= '0';
	buff[1]= 'b';
	for(i=0;i<4;i++)
	{
		if((gpio_val >> i) & 0x01)
			buff[5-i] = '1';
		else
			buff[5-i] = '0';
	}
	buff[6]= '\n';
	len=7;
	ret = copy_to_user(buffer, buff, len);
	if(ret)
		return -EFAULT;
	printk(KERN_INFO "Succesfully read\n");
	endRead = 1;

	return len;
}
ssize_t GPIODriverWrite (struct file *filePtr, const char __user *buffer, size_t length, loff_t *offset)
{
	char buff[BUFF_SIZE];
	int ret = 0;
	long int gpio_val=0;

	ret = copy_from_user(buff, buffer, length);
	if(ret)
		return -EFAULT;
	buff[length] = '\0';

	// HEX  INPUT
	if(buff[0] == '0' && (buff[1] == 'x' || buff[1] == 'X')) 
	{
		ret = kstrtol(buff+2,16,&gpio_val);
	}
	
	if (!ret)
	{
		iowrite32((u32)gpio_val, gp->base_addr);
		printk(KERN_INFO "Succesfully wrote value %#x",(u32)gpio_val); 
	}
	else
	{
		printk(KERN_INFO "Wrong command format\n"); 
	}

	return length;
}

/*
** @brief This function is used when file for this driver is opened.
*/
int GPIODriverOpen(struct inode *, struct file *)
{
	printk("Driver file was opened. \n");
	return 0;
}

/*
** @brief This function is used when file for this driver is closed.
*/
int GPIODriverClose(struct inode *, struct file *)
{
	printk("Driver file was closed. \n");
	return 0;
}

static struct file_operations myFileOperations = {
	.open = GPIODriverOpen,
	.release = GPIODriverClose,
	.read = GPIODriverRead,
	.write = GPIODriverWrite,
};

/* 
** @brief This function is executed when module is inserted into kernel.
*/
static int __init driverInit(void)
{
	int ret = 0;

	/* Register device */
	ret = alloc_chrdev_region(&devID, 0, 1, "GPIODriver");
	if (ret)
	{
		printk(KERN_WARNING "Can't register device driver. \n");
		return -1;
	}
	printk(KERN_INFO "Successfully allocated major number. \n");

	/* create class*/
	classPtr = class_create("GPIODriverClass");
	if (classPtr == NULL)
	{
		printk(KERN_WARNING "Can't create class. \n");
		goto fail_0;
	}
	printk(KERN_INFO "Successfully created the class. \n");

	/* create device */
	devicePtr = device_create(classPtr, NULL, devID, NULL, "GPIODriver");
	if (devicePtr == NULL)
	{
		printk(KERN_WARNING "Can't create struct device. \n");
		goto fail_1;
	}
	printk(KERN_INFO "Successfully created struct device. \n");
	
	/* allocate space for cdev structure */
	cdevPtr = cdev_alloc();
	/* initializing structure cdev */
	cdevPtr-> owner = THIS_MODULE;
	cdevPtr-> ops = &myFileOperations;
	/* add character device to the system */
	ret = cdev_add(cdevPtr, devID, 1);
	if (ret)
	{
		printk(KERN_WARNING "Can't add character device to the system. \n");
		goto fail_2;
	}
	printk(KERN_INFO "Successfully added GPIO driver to the system. \n");

	return platform_driver_register(&GPIODriverPlatform);

fail_2:
	device_destroy(classPtr, devID);
fail_1:
	class_destroy(classPtr);
fail_0:
	unregister_chrdev_region(devID, 1);
	return -1;
}

/* 
** @brief This function is executed when module is removed from the kernel.
*/
static void __exit driverExit(void)
{
    platform_driver_unregister(&GPIODriverPlatform);
	cdev_del(cdevPtr);
	device_destroy(classPtr, devID);
	class_destroy(classPtr);
	unregister_chrdev_region(devID, 1);
	printk(KERN_INFO "GPIO driver removed successfully without errors! \n");
}


module_init(driverInit);
module_exit(driverExit);
