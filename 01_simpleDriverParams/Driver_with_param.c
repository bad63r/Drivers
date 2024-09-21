#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h> /* This header enables parameters */

MODULE_LICENSE("GPL");

int param; /* here we will store parameter */
int i = 0;
module_param(param, int,S_IRUGO); /* macro which is used to define parameter */


static int __init function_init(void){
	printk(KERN_INFO "value of the parameter is: %d \n", param);
	return 0;
}

static void __exit function_exit(void){
	printk(KERN_INFO "bye bye \n");
}


module_init(function_init);
module_exit(function_exit);

/*• In this S_I is common header.
 * • R = read ,W =write ,X= Execute.
 * • USR =user ,GRP =Group
 * • Using OR ‘|’ (or operation) we can set
 * multiple permissions at a time.*/
