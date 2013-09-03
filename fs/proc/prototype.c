/* You are requested not to blindly copy paste my codes and use them in your kernel without my permission. If you use any part of my code or learn something new from them, then the least you can do is give me some credit.*/

/*
PROTOTYPE ENGINE IS DIVIDED INTO 2 DIFFERENT IMPLEMENTATION LEVELS:
1.KERNEL 
2.RAMDISK

SO THIS IS JUST THE HALF OF THE ENGINE.
*/

/*
AIM:IT PROVIDES FLEXIBILITY TO THE DEVELOPER TO INCLUDE MANY TWEAKS,DEBUGGING SCRIPTS(KERNEL LEVEL),KERNEL&SYSTEM INFO AND OBVIOUSLY REALTIME KERNEL CHECKUP. IT ALSO CREATES A PORTAL TO ADD FEATURES TO THE KERNEL SUCH AS DUALBOOT ROM AND KERNEL.

AT THE MOMENT ITS REALLY AT ITS INITIAL STAGES AS I WAS NOT WORKING ON THE KERNEL IMPLEMENTATION TILL NOW. I WILL BE LEARNING NEW THINGS AND ADDING THEM HERE. SO IF YOU WANT TO CONTRIBUTE THEN JUST SEND ME A PULL REQUEST*/

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/syscore_ops.h>
#include<linux/printk.h>
#include <trace/events/power.h>

static int meminfo_proc_show(struct seq_file *m, void *v)
{ 
seq_printf(m,
	"Prototype Engine"
	);
return 0;
}
static int meminfo_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, meminfo_proc_show, NULL);
}

static const struct file_operations input = {
	.open		= meminfo_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_prototype_init(void)
{
	proc_create("prototype", 0, NULL, &input);
	return 0;
}
module_init(proc_prototype_init);
