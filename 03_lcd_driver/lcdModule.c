#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

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
 unsigned int lcd_pins[] = {
    2,     /* RS Pin */
    3,     /* Enable Pin */
    4,     /* D0 Data Pin */
    27,    /* D1 Data Pin */
    22,    /* D2 Data Pin*/
    5,     /* D3 Data Pin*/
    6,     /* D4 Data Pin */
    13,    /* D5 Data Pin */
    26,    /* D6 Data Pin*/
    23     /* D7 Data Pin*/
};

static char message[17];

static ssize_t driverWrite(struct file *instance, const char __user *user_buffer, size_t count, loff_t *offset)
{
    int to_copy;
    int not_copied;
    int delta;

    to_copy = min(count, sizeof(message));
    not_copied = copy_from_user(message, user_buffer, to_copy);

    delta = to_copy - not_copied;

    return delta;
}

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
    int i = 0;
    char *lcdPinNames[] = {"RS_Pin", "En_Pin", "D0_Pin", "D1_Pin", "D2_Pin", "D3_Pin", "D4_Pin", "D5_Pin", "D6_Pin", "D7_Pin"};

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

    /* Reserver GPIOs for LCD */
    for(i=0; i<10;i++)
    {
        if (gpio_request(lcd_pins[i], lcdPinNames[i]) < 0)
        {
            printk("LCD Module: Error, negative request for gpio pin %d \n", lcd_pins[i]);
            goto GpioReqError;
        }
    }

    /* Setting directions of the LCD pins */
    for(i=0; i<10;i++)
    {
        if (gpio_direction_output(lcd_pins[i], 0) < 0)
        {
            printk("LCD Module: Error, negative request for gpio pin %d \n", lcd_pins[i]);
            goto GpioDirError;
        }
    }




    printk("LCD Module: Module is loaded into kernel space! \n");
    return 0;

    GpioDirError:
        i=9;

        for(;i<=0;i--)
        {
            gpio_free(lcd_pins[i]);
        }

    GpioReqError:
        for(;i<=0;i--)
        {
            gpio_free(lcd_pins[i]);
        }

        cdev_del(&lcdDevice);
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
    int i=0;
    for(;i<10;i++)
    {
        gpio_free(lcd_pins[i]);
    }
    cdev_del(&lcdDevice);
    device_destroy(driverClassPtr, deviceNumber);
    class_destroy(driverClassPtr);
    unregister_chrdev_region(deviceNumber, 1);

    printk("LCD Module: Module is removed from kernel space :O \n");
}

module_init(ModuleInit);
module_exit(ModuleExit);