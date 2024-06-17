#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>

/* Meta information*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tomislav Pap");
MODULE_DESCRIPTION("LCD QAPASS 1602A driver");

/* Device number and class variables*/
static dev_t deviceNumber;
static struct class *driverClassPtr;
static struct cdev lcdDevice;

#define DRIVER_NAME "lcdDriver"
#define DRIVER_CLASS_NAME "lcdDriverClass"

/* LCD Pinout*/
static int lcd_pins[] = {
    
};


/*
** @brief This function is called when device file is opened.
*/
static int driverOpen(struct inode *device_file, struct file *instance)
{
        printk("LCD Module: Device file was opened. \n");
        return 0;
}

/*
** @brief This function is called when device file is closed.
*/
static int driverClose(struct inode *device_file, struct file *instance)
{
        printk("LCD Module: Device file was closed. \n");
        return 0;
}

/* LCD Driver file operations structure */
static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = driverOpen,
    .release = driverClose
};

/*
** @brief This function is called when we load driver into the kernel space.
*/
static int __init ModuleInit(void)
{
    /* Register device number */
    if ((alloc_chrdev_region(&deviceNumber, 0, 1, DRIVER_NAME)) < 0 ) 
    {
        printk("LCD Module: Error, can't allocate device driver number! \n");
        return -1;
    }

    printk("LCD Module: Registered module with major number: %d, and minor num: %d \n", deviceNumber >> 20, deviceNumber & 0xfffff);

    /* Create module class*/
    if ((driverClassPtr = class_create(DRIVER_CLASS_NAME)) == NULL)
    {
        printk("LCD Module: Error, can't create driver class! \n");
        goto ClassError;
    }

    /* Create device file */
    if ((device_create(driverClassPtr, NULL, deviceNumber, NULL, DRIVER_NAME)) == NULL)
    {
        printk("LCD Module: Error, can't create device file! \n");
        goto DeviceError;
    }

    /* Initiate device file */
    cdev_init(&lcdDevice, &fops);

    /* Register device into kernel space.*/
    if ((cdev_add(&lcdDevice, deviceNumber, 1)) < 0)
    {
        printk("LCD Module: Error, can't register device into kernel space! \n");
        goto CdevError;
    }

    printk("LCD Module: Module is loaded into kernel space! \n");
    return 0;

    CdevError:
        device_destroy(driverClassPtr, deviceNumber);
    DeviceError:
        class_destroy(driverClassPtr);
    ClassError:
        unregister_chrdev_region(deviceNumber, 1);
        return -1;
}

/*
** @brief This function is called when we remove driver from the kernel space.
*/
static void __exit ModuleExit(void)
{
    cdev_del(&lcdDevice);
    device_destroy(driverClassPtr, deviceNumber);
    class_destroy(driverClassPtr);
    unregister_chrdev_region(deviceNumber, 1);

    printk("LCD Module: Module is removed from kernel space :O \n");
}

module_init(ModuleInit);
module_exit(ModuleExit);