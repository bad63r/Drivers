# LIFO(Last In First Out) Driver


This is simple "in memory" LIFO. It consists of 10 int elements. Module is loaded with `sudo insmod LIFODriver.ko` you can check driver messages with `sudo dmesg -c`

To write to LIFO you should `echo "value,position" > /dev/LIFODriver`
e.g. `echo "5,3" > /dev/LIFODriver` will write value 5 to position 3.
You might encounter permissions error so to solve that type `sudo chmod 777 /dev/LIFODriver`

To activate read function of the driver type `cat /dev/LIFODriver `

Remove driver with `sudo rmmod LIFODriver` 
