# GPIO Driver as platfrom  driver 

After module is loaded with `sudo insmod GPIODriver.ko` you can check driver messages with `sudo dmesg -c`
Remove driver with `sudo rmmod GPIODriver` 

# Development Config

To develop this Linux kernel driver there are two config files/setups for LSP. One for **Visual Studio Code** and other for **Kate** text editor.

* For Visual Studio Code, you might have to edit paths in c_cpp_properties.json in order for VSC to pick up config
* For Kate, there is generate compile_commands.json file which Kate picks up automatically for LSP. 

# How To Generate compile_commands.json LSP config from Linux Kernel source code

Download Linux Kernel source code of your choice. Compile it with:
```
make CC=clang defconfig
make CC=clang
```
Once the code is compiled simply run `python scripts/clang-tools/gen_compile_commands.py` from root of the kernel source directory
which will create a *compile_commands.json* which LSP mode will automatically find. Then you're good to go. 

