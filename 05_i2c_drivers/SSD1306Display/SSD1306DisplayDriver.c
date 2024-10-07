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
static int ssd1306_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
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
static int ssd1306_remove(struct i2c_client *client)
{   
    //fill the Display with this data
    SSD1306_Fill(0x00);
    
    pr_info("SSD1306 Display Removed! \n");
    return 0;
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
static struct i2c_board_info ssd1306_i2c_board_info = {
        I2C_BOARD_INFO(SLAVE_DEVICE_NAME, SSD1306_SLAVE_ADDR)
    };
 
/*
** Module Init function
*/
static int __init driverInit(void)
{
    int ret = -1;
    i2c_adapter     = i2c_get_adapter(I2C_BUS_AVAILABLE);
    
    if( i2c_adapter != NULL )
    {
        i2c_client = i2c_new_device(i2c_adapter, &ssd1306_i2c_board_info);
        
        if( i2c_client != NULL )
        {
            i2c_add_driver(&ssd1306_driver);
            ret = 0;
        }
        
        i2c_put_adapter(i2c_adapter);
    }
    
    pr_info("Driver Added! \n");
    return ret;
}
 
/*
** Module Exit function
*/
static void __exit driverExit(void)
{
    i2c_unregister_device(i2c_client);
    i2c_del_driver(&ssd1306_driver);
    pr_info("Driver Removed!!!\n");
}
 
module_init(driverInit);
module_exit(driverExit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("bad63r");
MODULE_DESCRIPTION("Simple I2C driver(SSD_1306 Display)");