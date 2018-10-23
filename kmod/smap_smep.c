#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <asm/irqflags.h>
#include <asm/tlbflush.h>
#include <uapi/asm/processor-flags.h>
#include <linux/delay.h>

#define DEVICE_NAME "smap-smep"
#define SMAP_READ 100
#define SMAP_WRITE 200
#define SMAP_EXEC 300

static int smap_read_open(struct inode *inode, struct file *filp)
{
    printk("smap and semp misc dev open!\n");
    return 0;
}

static int smap_read_close(struct inode *inode, struct file *filp)
{
    printk("smap and semp misc dev close!\n");
    return 0;
}

static void read_op(unsigned long addr)
{
    printk("addr=0x%lx\n", addr);
    printk("value=0x%lx\n", *(unsigned long *)addr);

    return;
}

static void write_op(unsigned long addr)
{
    printk("writing start\n");
    mdelay(500);
    *(unsigned long *)addr = 0x9012345678876543;
    printk("writing end\n");
    printk("value=0x%lx\n", *(unsigned long *)addr);

    return;
}

static void exec_op(unsigned long addr)
{
    void (*fun)(void) = (void (*)(void))addr;
    printk("executing start\n");
    printk("addr = 0x%lx \n", addr);
    fun();
    //asm("jmp %0"::"a"(addr));
    printk("executing end\n");
    return;    
}

static void judge_flag(void)
{
    unsigned long eflags = native_save_fl();
    unsigned long cr4 = cr4_read_shadow();

    if (eflags & X86_EFLAGS_AC)
        printk("X86_EFLAGS_AC=1\n");
    else
        printk("X86_EFLAGS_AC=0\n");

    if (cr4 & X86_CR4_SMAP)
        printk("X86_CR4_SMAP=1\n");
    else
        printk("X86_CR4_SMAP=0\n");

    if (cr4 & X86_CR4_SMEP)
        printk("X86_CR4_SMEP=1\n");
    else
        printk("X86_CR4_SMEP=0\n");

    return;
}

static long smap_read_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    judge_flag();

    switch(cmd){
        case SMAP_READ:
                read_op(arg);
                break;
        case SMAP_WRITE:
                write_op(arg);
                break;
        case SMAP_EXEC:
                exec_op(arg);
                break;
        default:
                printk("Invalid cmd for ioctl \n");
                return -EINVAL;
    }
    return 0; 
}

struct file_operations smap_read_ops = {
    .owner = THIS_MODULE,
    .open = smap_read_open,
    .release = smap_read_close,
    .unlocked_ioctl = smap_read_ioctl,
};

struct miscdevice smap_read_misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &smap_read_ops,
};

static int __init smap_read_init(void) 
{
    int ret;

    ret = misc_register(&smap_read_misc_dev);
    if(ret != 0){
        printk("smap and smep misc register failed! \n");
        goto failed;
    }
    printk("smap and smep misc init success! \n"); 

failed:
    return ret;
}

static void __exit smap_read_exit(void)
{
    misc_deregister(&smap_read_misc_dev);
    printk("smap read misc exit success \n");

    return;
}

module_init(smap_read_init);
module_exit(smap_read_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hongzhi");
