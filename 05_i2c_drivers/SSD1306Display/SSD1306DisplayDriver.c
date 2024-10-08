#include <linux/module.h> /* #include dynamic_debug inserts printk() implicitly;
							 module_init(), module_exit()*/
#include <linux/init.h> /* __init, __exit */
#include <linux/kdev_t.h> /* MKDEV, implicitly dev_t type*/
#include <linux/device.h> /* device_create(), device_destroy() */
#include <linux/cdev.h> /* all functions related to cdev a.k.a. character device */
#include <linux/fs.h> /* struct file operations */

#include <linux/slab.h>//kmalloc kfree
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>


 
#define I2C_BUS_AVAILABLE   (          1 )              // I2C Bus available in our Raspberry Pi
#define SLAVE_DEVICE_NAME   ( "SSD1306_DISPLAY" )       // Device and Driver Name
#define SSD1306_SLAVE_ADDR  (       0x3C )              // SSD1306 Display Slave Address
 
static struct i2c_adapter *ssd1306_i2c_adapter = NULL;  // I2C Adapter Structure
static struct i2c_client  *ssd1306_i2c_client  = NULL;  // I2C Cient Structure (In our case it is SSD1306 Display)

static dev_t devID;
static struct class* classPtr;
static struct device* devicePtr;
static struct cdev* cdevPtr;

/*TODO */
ssize_t driverRead (struct file *filePtr, char __user *buffer, size_t length, loff_t *offset);
ssize_t driverWrite (struct file *filePtr, const char __user *buffer, size_t length, loff_t *offset);

int driverOpen (struct inode *, struct file *);
int driverClose (struct inode *, struct file *);
 
/*
** @brief This function writes the data into the I2C client
**
**  Arguments:
**      buff -> buffer to be sent
**      len  -> Length of the data
**   
*/
static int I2C_Write(unsigned char *buf, unsigned int len)
{
    /*
    ** Sending Start condition, Slave address with R/W bit, 
    ** ACK/NACK and Stop condtions will be handled internally.
    */ 
    int ret = i2c_master_send(ssd1306_i2c_client, buf, len);
    
    return ret;
}
 
/*
** This function reads one byte of the data from the I2C client
**
**  Arguments:
**      out_buff -> buffer wherer the data to be copied
**      len      -> Length of the data to be read
** 
*/
static int I2C_Read(unsigned char *out_buf, unsigned int len)
{
    /*
    ** Sending Start condition, Slave address with R/W bit, 
    ** ACK/NACK and Stop condtions will be handled internally.
    */ 
    int ret = i2c_master_recv(ssd1306_i2c_client, out_buf, len);
    
    return ret;
}
 
/*
** This function is specific to the SSD_1306 OLED.
** This function sends the command/data to the OLED.
**
**  Arguments:
**      is_cmd -> true = command, flase = data
**      data   -> data to be written
** 
*/
static void SSD1306_Write(bool is_cmd, unsigned char data)
{
    unsigned char buf[2] = {0};
    int ret;
    
    /*
    ** First byte is always control byte. Data is followed after that.
    **
    ** There are two types of data in SSD_1306 OLED.
    ** 1. Command
    ** 2. Data
    **
    ** Control byte decides that the next byte is, command or data.
    **
    ** -------------------------------------------------------                        
    ** |              Control byte's | 6th bit  |   7th bit  |
    ** |-----------------------------|----------|------------|    
    ** |   Command                   |   0      |     0      |
    ** |-----------------------------|----------|------------|
    ** |   data                      |   1      |     0      |
    ** |-----------------------------|----------|------------|
    ** 
    ** Please refer the datasheet for more information. 
    **    
    */ 
    if( is_cmd == true )
    {
        buf[0] = 0x00;
    }
    else
    {
        buf[0] = 0x40;
    }
    
    buf[1] = data;
    
    ret = I2C_Write(buf, 2);
}
 
 
/*
** This function sends the commands that need to used to Initialize the OLED.
**
**  Arguments:
**      none
** 
*/
static int SSD1306_DisplayInit(void)
{
    msleep(100);               // delay
 
    /*
    ** Commands to initialize the SSD_1306 OLED Display
    */
    SSD1306_Write(true, 0xAE); // Entire Display OFF
    SSD1306_Write(true, 0xD5); // Set Display Clock Divide Ratio and Oscillator Frequency
    SSD1306_Write(true, 0x80); // Default Setting for Display Clock Divide Ratio and Oscillator Frequency that is recommended
    SSD1306_Write(true, 0xA8); // Set Multiplex Ratio
    SSD1306_Write(true, 0x3F); // 64 COM lines
    SSD1306_Write(true, 0xD3); // Set display offset
    SSD1306_Write(true, 0x00); // 0 offset
    SSD1306_Write(true, 0x40); // Set first line as the start line of the display
    SSD1306_Write(true, 0x8D); // Charge pump
    SSD1306_Write(true, 0x14); // Enable charge dump during display on
    SSD1306_Write(true, 0x20); // Set memory addressing mode
    SSD1306_Write(true, 0x00); // Horizontal addressing mode
    SSD1306_Write(true, 0xA1); // Set segment remap with column address 127 mapped to segment 0
    SSD1306_Write(true, 0xC8); // Set com output scan direction, scan from com63 to com 0
    SSD1306_Write(true, 0xDA); // Set com pins hardware configuration
    SSD1306_Write(true, 0x12); // Alternative com pin configuration, disable com left/right remap
    SSD1306_Write(true, 0x81); // Set contrast control
    SSD1306_Write(true, 0x80); // Set Contrast to 128
    SSD1306_Write(true, 0xD9); // Set pre-charge period
    SSD1306_Write(true, 0xF1); // Phase 1 period of 15 DCLK, Phase 2 period of 1 DCLK
    SSD1306_Write(true, 0xDB); // Set Vcomh deselect level
    SSD1306_Write(true, 0x20); // Vcomh deselect level ~ 0.77 Vcc
    SSD1306_Write(true, 0xA4); // Entire display ON, resume to RAM content display
    SSD1306_Write(true, 0xA6); // Set Display in Normal Mode, 1 = ON, 0 = OFF
    SSD1306_Write(true, 0x2E); // Deactivate scroll
    SSD1306_Write(true, 0xAF); // Display ON in normal mode
    
    return 0;
}
 
/*
** This function Fills the complete OLED with this data byte.
**
**  Arguments:
**      data  -> Data to be filled in the OLED
** 
*/
static void SSD1306_Fill(unsigned char data)
{
    unsigned int total  = 128 * 8;  // 8 pages x 128 segments x 8 bits of data
    unsigned int i      = 0;
    
    //Fill the Display
    for(i = 0; i < total; i++)
    {
        SSD1306_Write(false, data);
    }
}
 
/*
** @brief This function getting called when the slave has been found
** Note : This will be called only once when we load the driver.
*/
static int ssd1306_probe(struct i2c_client *client)
{
    SSD1306_DisplayInit();
    
    //fill the Display with this data
    SSD1306_Fill(0xFF);
 
    pr_info("SSD1306 Display is Probed/Initialized! \n");
    
    return 0;
}
 
/*
** @brief This function getting called when the slave has been removed
** Note : This will be called only once when we unload the driver.
*/
static void ssd1306_remove(struct i2c_client *client)
{   
    //fill the Display with this data
    SSD1306_Fill(0x00);
    
    pr_info("SSD1306 Display Removed! \n");
}
 
/*
** Structure that has slave device id
*/
static const struct i2c_device_id ssd1306_id[] = {
        { SLAVE_DEVICE_NAME, 0 },
        { }
};
MODULE_DEVICE_TABLE(i2c, ssd1306_id);
 
/*
** I2C driver Structure that has to be added to linux
*/
static struct i2c_driver ssd1306_driver = {
        .driver = {
            .name   = SLAVE_DEVICE_NAME,
            .owner  = THIS_MODULE,
        },
        .probe          = ssd1306_probe,
        .remove         = ssd1306_remove,
        .id_table       = ssd1306_id,
};
 
/*
** I2C Board Info strucutre
*/
static struct i2c_board_info ssd1306_i2c_board_info = 
{
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, SSD1306_SLAVE_ADDR)
};


/*
** @brief This function is used when file for this driver is opened.
*/
int driverOpen (struct inode *, struct file *)
{
    printk(KERN_INFO "SSD1306Driver file was opened. \n");
    return 0;
}

/*
** @brief This function is used when file for this driver is closed.
*/
int driverClose (struct inode *, struct file *)
{
    printk(KERN_INFO "SSD1306Driver file was closed. \n");
    return 0;
}

/* file operations structure */
static struct file_operations myFileOperations = 
{
    .open = driverOpen,
    .release = driverClose,
    /* TODO */
    // .read = driverRead,
    // .write = driverWrite,
};

 
/*
** Module Init function
*/
static int __init driverInit(void)
{
    int ret = 0;

    /* allocated major number */
    ret = alloc_chrdev_region(&devID, 0, 1, SLAVE_DEVICE_NAME);
    if (ret)
    {
        printk(KERN_ALERT "Can't allocate major number for SSD1306Driver \n");
        return -1;
    }
    printk(KERN_INFO "Successfully allocated majon number for SSD1306Driver \n");

    /* create class of the device */
    classPtr = class_create("SSD1306DriverClass");
    if (classPtr == NULL)
    {
        printk(KERN_ALERT "Can't create class of SSD1306Driver. \n");
        goto fail_0;
    }
    printk(KERN_INFO "Successfully created class for SSD1306Driver \n");

    /* create device */
    devicePtr = device_create(classPtr, NULL, devID, NULL, "SSD1306Driver");
    if (devicePtr == NULL)
    {
        printk(KERN_ALERT "Can't create device of SSD1306Driver. \n");
        goto fail_1;
    }
    printk(KERN_INFO "Successfully created device for SSD1306Driver \n");

    /* cdev allocation */
    cdevPtr = cdev_alloc();
    /* initialize cdev structure*/
    cdevPtr->owner = THIS_MODULE;
    cdevPtr->ops = &myFileOperations;
	/* add character device to the system */
	ret = cdev_add(cdevPtr, devID, 1);
	if (ret)
	{
		printk(KERN_WARNING "Can't add character device to the system. \n");
		goto fail_2;
	}
	printk(KERN_INFO "Successfully added character device to the system. \n");

    /* get i2c adapter */
    ssd1306_i2c_adapter     = i2c_get_adapter(I2C_BUS_AVAILABLE);
    
    if( ssd1306_i2c_adapter != NULL )
    {
        /* if adapter is available, create new i2c client device*/
        ssd1306_i2c_client = i2c_new_client_device(ssd1306_i2c_adapter, &ssd1306_i2c_board_info);
        
        if( ssd1306_i2c_client != NULL )
        {
            i2c_add_driver(&ssd1306_driver);
            ret = 0;
        }
        
        i2c_put_adapter(ssd1306_i2c_adapter);
    }
    
    printk(KERN_INFO "SSD1306Driver Added! \n");
    return ret;


    fail_2:
        device_destroy(classPtr, devID);
    fail_1:
        class_destroy(classPtr);
    fail_0:
        unregister_chrdev_region(devID, 1);
        return -1;
}
 
/*
** Module Exit function
*/
static void __exit driverExit(void)
{
    i2c_unregister_device(ssd1306_i2c_client);
    i2c_del_driver(&ssd1306_driver);
    cdev_del(cdevPtr);
    device_destroy(classPtr, devID);
    class_destroy(classPtr);
    unregister_chrdev_region(devID, 1);
    printk(KERN_INFO "SSD1306Driver successfully removed! \n");
}
 
module_init(driverInit);
module_exit(driverExit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("bad63r");
MODULE_DESCRIPTION("Simple I2C driver(SSD_1306 Display)");