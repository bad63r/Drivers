#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h> /*ovaj header nam daje moogucnost da koristimo parametre */

MODULE_LICENSE("GPL");

int param; /*definisemo parametar*/
int i=0;
module_param(param, int,S_IRUGO); /*marko koji sluzi za registrovanje parametra*/


static int function_init(void){
	    printk("<1> vrednost parametra je: %d \n",param);
	    if (param == 3){  
		while(i<3){  /*petlja koja ispisuje proba 3 puta */
	    	  printk("<1> proba\n");
	    	  i++;
	    	}
	    }
	
return 0;
}

static int function_exit(void){
	printk("<1> bye bye\n");
return 0;
}


module_init(function_init);
module_exit(function_exit);

/*• In this S_I is common header.
 * • R = read ,W =write ,X= Execute.
 * • USR =user ,GRP =Group
 * • Using OR ‘|’ (or operation) we can set
 * multiple perissions at a time.*/
