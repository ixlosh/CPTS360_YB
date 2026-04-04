YUSUF BILGICH
11848141

-> How to compile and run your program
In a Linux Kernel environment, please have all the necesseary headers installed. Then run the Makefile by invoking:
make
sudo insmod PA4-finalCode.ko
You can also verify the module load by:
sudo dmesg | tail
and as a result you should see:
sysprog_counter module loaded

-> Assumptions made, if any
We are in Linux kernel

-> Example input and output

$ cat /proc/sysprog_counter
  Counter value: 0

(where x is an int)
$ echo x | sudo tee /proc/sysprog_counter
$ cat /proc/sysprog_counter
  Counter value: x