# SSD1306DisplayDriver / SSD1316DisplayDriver

After module is loaded with `sudo insmod SSD1306Driver.ko` you can check driver messages with `sudo dmesg -c`
Remove driver with `sudo rmmod SSD1306Driver` 

To work with SSD1306Display on I2C Bus, you can use embedded driver's write command. Type `echo "0 data" >> /dev/SSD1306Driver` to write
data to to display, or `echo "1 command" >> /dev/SSD1306Driver` for command.

Datasheet of SSD1306 Display : [SSD1315 Datasheet Link](https://cursedhardware.github.io/epd-driver-ic/SSD1315.pdf)