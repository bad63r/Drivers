#include <linux/module.h> /* #include dynamic_debug inserts printk() implicitly;
							 module_init(), module_exit()*/
#include <linux/init.h> /* __init, __exit */
#include <linux/kdev_t.h> /* MKDEV */
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple static driver.");
MODULE_AUTHOR("bad63r");
dev_t devID;
	/* for static allocation, you need to handle and guarantee that major number is available manually.
	   doing ls -l /dev/ , you can find all major and minor number for all present devices. */
#define MY_MAJOR_NUM 2
/* 
** @brief This function is executed when module is inserted into kernel.
*/
static int __init driverInit(void)
{
	int ret = 0;
	/* create major number */
	devID = MKDEV(MY_MAJOR_NUM, 0);
	/* Register device */
	ret = register_chrdev_region(devID, 1,  "staticDriver");
	if (ret)
	{
		printk(KERN_WARNING "Can't register device driver. Major number is not valid. \n");
		return -1;
	}
	printk(KERN_INFO "Successfully registered static driver. \n");
	return 0;
}
/* 
** @brief This function is executed when module is removed from the kernel.
*/
static void __exit driverExit(void)
{
	unregister_chrdev_region(devID, 1);
	printk(KERN_INFO "cya later, aligator! \n");
}
module_init(driverInit);
module_exit(driverExit);