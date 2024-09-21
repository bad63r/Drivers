#include <linux/module.h> /* #include dynamic_debug inserts printk() implicitly;
							 module_init(), module_exit()*/
#include <linux/init.h> /* __init, __exit */

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple hello world module");
MODULE_AUTHOR("bad63r");

/* 
** @brief This function is executed when module is inserted into kernel.
*/
static int __init driverInit(void)
{
	printk(KERN_INFO "hello there \n");
	return 0;
}

/* 
** @brief This function is executed when module is removed from the kernel.
*/
static void __exit driverExit(void)
{
	printk(KERN_INFO "cya later, aligator! \n");
}



module_init(driverInit);
module_exit(driverExit);
