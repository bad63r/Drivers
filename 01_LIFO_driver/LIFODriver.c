#include <linux/module.h> /* #include dynamic_debug inserts printk() implicitly;
							 module_init(), module_exit()*/
#include <linux/init.h> /* __init, __exit */
#include <linux/kdev_t.h> /* MKDEV, implicitly dev_t type*/
#include <linux/device.h> /* device_create(), device_destroy() */
#include <linux/cdev.h> /* all functions related to cdev a.k.a. character device */
#include <linux/fs.h> /* struct file operations */
#include <linux/uaccess.h> /* copy_to_user(), copy_from_user() */
#include <linux/kernel.h> /* sscanf() */

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Last In First Out (LIFO) Driver.");
MODULE_AUTHOR("bad63r");

dev_t devID;
static struct class *classPtr;
static struct device* devicePtr;
static struct cdev* cdevPtr;

int storage[10];
int pos = 0;
int endRead = 0;
#define BUFF_SIZE 32



ssize_t driverRead (struct file *filePtr, char __user *buffer, size_t length, loff_t *offset)
{
	int ret = 0;
	int len = 0;
	char buff[BUFF_SIZE];

	if (endRead)
	{
		endRead = 0;
		return 0;
	}

	if (pos > 0)
	{
		pos--;
		len = scnprintf(buff, BUFF_SIZE, "%d ", storage[pos]);
		ret = copy_to_user(buffer, buff, len);
		if (ret)
		{
			return -EFAULT;
		}

		printk("Successfully read from the LIFO. pos = %d. \n", pos);
		endRead = 1;
	}
	else
	{
		printk("LIFO is empty. \n");
	}

	return len;
}
ssize_t driverWrite (struct file *filePtr, const char __user *buffer, size_t length, loff_t *offset)
{
	int ret = 0;
	int len = 0;
	char buff[BUFF_SIZE];
	int val = 0;

	ret = copy_from_user(buff, buffer, length);
	if (ret)
	{
		return -EFAULT;
	}
	buff[length -1] = '\0';

	len = sscanf(buff, "%d", &val);
	
	if (len == 1)
	{
		if (pos >=0 && pos <= 9)
		{
			storage[pos] = val;
			pos++;
			printk(KERN_INFO "Successfully written to storage of LIFO. \n");
		}
		else
		{
			printk(KERN_WARNING "LIFO is full! \n");
		}
	}
	else
	{
		printk(KERN_WARNING "Wrong number of arguments. Can't write to storage of LIFO. \n" );
	}

	return length;
}

/*
** @brief This function is used when file for this driver is opened.
*/
int driverOpen(struct inode *, struct file *)
{
	printk(KERN_INFO "Driver file was opened. \n");
	return 0;
}

/*
** @brief This function is used when file for this driver is closed.
*/
int driverClose(struct inode *, struct file *)
{
	printk(KERN_INFO "Driver file was closed. \n");
	return 0;
}

static struct file_operations myFileOperations = {
	.open = driverOpen,
	.release = driverClose,
	.read = driverRead,
	.write = driverWrite
};

/* 
** @brief This function is executed when module is inserted into kernel.
*/
static int __init driverInit(void)
{
	int ret = 0;

	/* Register device */
	ret = alloc_chrdev_region(&devID, 0, 1, "dynamicDriver");
	if (ret)
	{
		printk(KERN_WARNING "Can't register device driver. \n");
		return -1;
	}
	printk(KERN_INFO "Successfully allocated major number. \n");

	/* create class*/
	classPtr = class_create("LIFODriverClass");
	if (classPtr == NULL)
	{
		printk(KERN_WARNING "Can't create class. \n");
		goto fail_0;
	}
	printk(KERN_INFO "Successfully created the class. \n");

	/* create device */
	devicePtr = device_create(classPtr, NULL, devID, NULL, "LIFODriver");
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
	printk(KERN_INFO "Successfully added character device to the system. \n");

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
	printk(KERN_INFO "driver removed without errors! \n");
}


module_init(driverInit);
module_exit(driverExit);
