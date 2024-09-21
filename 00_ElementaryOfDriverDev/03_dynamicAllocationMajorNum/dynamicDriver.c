#include <linux/module.h> /* #include dynamic_debug inserts printk() implicitly;
							 module_init(), module_exit()*/
#include <linux/init.h> /* __init, __exit */
#include <linux/kdev_t.h> /* MKDEV, implicitly dev_t type*/
#include <linux/device.h> /* device_create(), device_destroy() */

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple dynamic driver.");
MODULE_AUTHOR("bad63r");

dev_t devID;
static struct class *classPtr;
static struct device* devicePtr;

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
	classPtr = class_create("dynamicDriverClass");
	if (classPtr == NULL)
	{
		printk(KERN_WARNING "Can't create class. \n");
		goto fail_0;
	}
	printk(KERN_INFO "Successfully created the class. \n");

	/* create device */
	devicePtr = device_create(classPtr, NULL, devID, NULL, "dynamicDriver");
	if (devicePtr == NULL)
	{
		printk(KERN_WARNING "Can't create struct device. \n");
		goto fail_1;
	}
	printk(KERN_INFO "Successfully created struct device. \n");


	return 0;

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
	device_destroy(classPtr, devID);
	class_destroy(classPtr);
	unregister_chrdev_region(devID, 1);
	printk(KERN_INFO "dynamicDriver removed without errors! \n");
}



module_init(driverInit);
module_exit(driverExit);
