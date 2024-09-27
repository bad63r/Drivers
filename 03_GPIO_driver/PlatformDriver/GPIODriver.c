#include <linux/module.h> /* #include dynamic_debug inserts printk() implicitly;
							 module_init(), module_exit()*/
#include <linux/init.h> /* __init, __exit */
#include <linux/kdev_t.h> /* MKDEV, implicitly dev_t type*/
#include <linux/device.h> /* device_create(), device_destroy() */
#include <linux/cdev.h> /* all functions related to cdev a.k.a. character device */
#include <linux/fs.h> /* struct file operations */
#include <linux/platform_device.h> /* struct platform_driver */

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple file operations driver.");
MODULE_AUTHOR("bad63r");

dev_t devID;
static struct class *classPtr;
static struct device* devicePtr;
static struct cdev* cdevPtr;

#define DRIVER_NAME "ledGPIO"

static stru

static struct platform_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = led_of_match,
	},	
	.probe = gpioProbe,
	.remove = gpioRemove,
}


ssize_t GPIODriverRead (struct file *filePtr, char __user *buffer, size_t length, loff_t *offset)
{
	printk("Read function of the driver was called. \n");
	return 0;
}
ssize_t GPIODriverWrite (struct file *filePtr, const char __user *buffer, size_t length, loff_t *offset)
{
	printk("Write function of the driver was called. \n");
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
	.write = GPIODriverWrite
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

	return 0;

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
	cdev_del(cdevPtr);
	device_destroy(classPtr, devID);
	class_destroy(classPtr);
	unregister_chrdev_region(devID, 1);
	printk(KERN_INFO "GPIO driver removed successfully without errors! \n");
}


module_init(driverInit);
module_exit(driverExit);
