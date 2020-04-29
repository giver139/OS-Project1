#include<linux/linkage.h>
#include<linux/kernel.h>

asmlinkage int print_result(int pid,long start_s,long start_ns,long end_s,long end_ns){
	printk("[Project1] %d %ld.%09ld %ld.%09ld\n",pid,start_s,start_ns,end_s,end_ns);
	return 0;
}
