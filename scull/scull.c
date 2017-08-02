#include <linux/init.h>

#include <linux/module.h>

#include <linux/kernel.h>

#include <linux/slab.h> /*kmalloc*/

#include <linux/fs.h> /*file system-write,read functions etc.*/

#include <linux/errno.h> /*error functions*/

#include <linux/types.h> /*size_t*/

#include <linux/proc.h>

#include <linux/fcntl.h> /*0_ACCMODE*/

#include <asm/system.h> /*(cli,*_flags */

#include <ams/uaccess.h> /*copy_from/to_user*/



MODULE_LICENSE("GPL");

int memory_open(struct inode *inode, struct file *filp);
int memory_release(struct inode *inode, struct file *filp);
ssize_t memory_read(struct inode *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t memory_write(struct inode *filp, char *buf, size_t count, loff_t *f_pos);
int memory_init(void);
int memory_exit(void);

struct file_operations memory_fops = {
	read: memory_read,
	write: memory_write,
	open: memory_open,
	release: memory_release
};


int memory_major = 60;

char *memory_buffer;


int memory_init(void){
  int result;

 	 result = register_chrdev(memory_major, "memory", &memory_fops);
	 if (result < 0) {
       	 printk("<1>,can't get major number,%d",memory_major);
  	 return result;

  	 }

  	 memory_buffer = kmalloc(1, GFP_KERNEL);
  
	 if (!memory_buffer){
	  result = -ENOMEM;
	  goto fail;
  	 }

  	memset(memory_buffer, 0, 1);

  	printk("<1>,inserting memory module " );
  return 0;

fail:
  memory_exit();
  return result;
}


int memory_exit(void){

	unregister_chrdev(memory_major, "memory");
	
	if (memory_buffer){
	kfree(memory_buffer);
	}

	printk("<1>, Removing memory module");

}


int memory_open(struct inode *inode, struct file *filp){

	return 0;
}

int memory_release(struct inode *inode, struct file *filp){

	return 0;
}

ssize_t memory_read(struct *filp, char *buf, size_t count, loff_t *f_ops){

	copy_to_user(buf, memory_buffer,1);

 	if(*f_ops == 0){
		f_ops+=1;
		return 1;
	}else{
		return 0;
	}

}


ssize_t memory_write(struct *filp, char *buf, size_t count, loff_t *f_ops){

	char *tmp;

	tmp = buf+count-1;

	copy_from_user(memory_buffer,tmp,1);

	return 1;

}
