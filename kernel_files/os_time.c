#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/ktime.h>
#include<linux/timekeeping.h>

asmlinkage int getcurtime(long *sec,long *nsec){
	struct timespec t;
	getnstimeofday(&t);
	*nsec=t.tv_nsec;
	*sec=t.tv_sec;
	return 0;
}
