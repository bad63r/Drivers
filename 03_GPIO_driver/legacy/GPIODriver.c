#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bad63r");
MODULE_DESCRIPTION("Simple legacy GPIO driver");

/* Device file and class variables*/
static dev_t myDeviceNumber;
static struct class *myClassPtr;
static struct cdev myDevice;

#define DRIVER_NAME "GPIODriver"
#define DRIVER_CLASS_NAME "GPIODriverClass"
/* Pin Number must be BCM pinout, not PiWire! */
#define GPIO_PIN_NUM 22

/*
** @brief This function is called when you write to device file.
*/
static ssize_t driverWrite(struct file *instance, const char __user *user_buffer, size_t count, loff_t *offset)
{
    int to_copy;
    int not_copied;
    char value;
    int delta;

    to_copy = min(count, sizeof(value));
    not_copied = copy_from_user(&value, user_buffer, to_copy);

    switch(value)
    {
        case '0':
            gpio_set_value(GPIO_PIN_NUM, 0);
            printk("GPIO Module: Write was ok set to 0!\n");
            break;
        case '1':
            gpio_set_value(GPIO_PIN_NUM, 1);
            printk("GPIO Module: Write was ok set to 1!\n");
            break;
        default:
            printk("GPIO Module: Invalid Input n: %d ! \n", value);
            printk("GPIO Module: Invalid Input c: %c ! \n", value);
            break;
    }

    delta = to_copy - not_copied;
    
    return delta;
}

/*
** @brief This function is called when device file is opened.
*/
static int driverOpen(struct inode *device_file, struct file *instance)
{
    printk("GPIO Module: Device file was opened! \n");
    return 0;
}

/*
** @brief This function is called when device file is closed.
*/
static int driverClose(struct inode *device_file, struct file *instance)
{
    printk("GPIO Module: Device file was closed! \n");
    return 0;
}

/* File operations structure*/
static struct file_operations fops = 
{
    .owner = THIS_MODULE,
    .open = driverOpen,
    .release = driverClose,
    .write = driverWrite
};

/*
** @brief This function is called when module is loaded into kernel space.
*/
static int __init moduleInit(void)
{

    char message[100];
    char gpio_pin_name[20];

    /* Allocating device module number */
    if (alloc_chrdev_region(&myDeviceNumber, 0, 1, DRIVER_NAME) < 0)
    {
        printk("GPIO Module: Error, can't register driver major number!. \n");
        return -1;
    }
    
    printk("GPIO Module: Module initialized with major number: %d, and minor number: %d \n", myDeviceNumber >> 20, myDeviceNumber & 0xfffff);

    /* Create module class */
    if ((myClassPtr = class_create(DRIVER_CLASS_NAME)) == NULL)
    {
        printk("GPIO Module: Error, can't create class for the module!. \n");
        goto ClassError;
    }

    /* Create device file for the module */
    if ((device_create(myClassPtr, NULL, myDeviceNumber, NULL, DRIVER_NAME)) == NULL) 
    {
        printk("GPIO Module: Error, can't create device file for this module!. \n");
        goto DeviceCreateError;
    }

    /* Initialize Device file */
    cdev_init(&myDevice, &fops);

    /* Add Device to kernel space. */
    if ((cdev_add(&myDevice, myDeviceNumber, 1)) < 0)
    {
        printk("GPIO Module: Error, can't add device to kernel space! \n");
        goto CdevError;
    }

    /* Requesting GPIO pin for usage*/
    sprintf(gpio_pin_name, "rpi-gpio-%d", GPIO_PIN_NUM);

    if((gpio_request(GPIO_PIN_NUM, gpio_pin_name)) )
    {
        sprintf(message, "GPIO Module: Error, can't allocate GPIO %d for the device. \n", GPIO_PIN_NUM);
        printk("%s\n", message);
        goto GpioError;
    }

    /* Setting GPIO pin direction*/
    if (gpio_direction_output(GPIO_PIN_NUM, 0))
    {
        printk("GPIO Module: Error, can't set gpio 15 to output!. \n");
        goto  GpioError;
    }


    printk("GPIO Module: initialized kernel module! \n");
    return 0;

    /* Releasing resources if something went wrong from the point of failiure*/
    GpioError:
    	gpio_free(GPIO_PIN_NUM);
        cdev_del(&myDevice);
    CdevError:
        device_destroy(myClassPtr, myDeviceNumber);
    DeviceCreateError:
        class_destroy(myClassPtr);
    ClassError: 
        unregister_chrdev_region(myDeviceNumber, 1);
        return -1;
}

/*
** @brief This function is called when module is removed from kernel space.
*/
static void __exit moduleExit(void)
{
    gpio_free(GPIO_PIN_NUM);
    cdev_del(&myDevice);
    device_destroy(myClassPtr, myDeviceNumber);
    class_destroy(myClassPtr);
    unregister_chrdev_region(myDeviceNumber, 1);

    printk("GPIO Module: module is removed from kernel space! \n");
}

module_init(moduleInit);
module_exit(moduleExit);
