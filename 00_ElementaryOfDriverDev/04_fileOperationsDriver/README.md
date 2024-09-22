# Driver with elementary file operations 

After module is loaded with `sudo insmod fileOperationsDriver.ko` you can check driver messages with `sudo dmesg -c`
To activate write function of the driver type `echo "test" > /dev/fileOperationsDriver` You might encounter permissions error so to solve that type `sudo chmod 777 /dev/fileOperationsDriver`
To activate read function of the driver type `cat /dev/fileOperationsDriver `
Remove driver with `sudo rmmod fileOperationsDriver` 
