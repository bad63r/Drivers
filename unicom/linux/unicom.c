#include <linux/init.h>
#include <linux/module.h>


MODULE_LICENSE("GPL");

int unicom_init(void) {
	printk("<1>,Importing unicom driver to kernel...\n");
	return 0;
}

int unicom_exit(void) {
	printk("<1>,driver is disabled.\n");
	return 0;
}

module_init(unicom_init);
module_exit(unicom_exit);
