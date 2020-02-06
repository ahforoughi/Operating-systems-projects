#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/param.h>
#include <linux/gcd.h>
#include <linux/hash.h>

#define BUFFER_SIZE 128
#define PROC_NAME "jiffies"

ssize_t proc_read(struct file *file, char __user *user_buff, size_t count, loff_t *pos);

static struct file_operations proc_ops
{
    .owner = THIS_MODULE,
    .read = proc_read,
};

int proc_init(void)
{
    printk(KERN_INFO "loading kernel module\n")
    proc_create(PROC_NAME, 0666, NULL, &proc_ops);
    printk(KERN_INFO "jiffies created\n")
    return 0;
}

void proc_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "jiffies Deleted!\n");
}

ssize_t proc_read(struct file *file, char __user *user_buff, size_t count, loff_t *pos)
{
    int rv = 0;
    char buffer[BUFFER_SIZE];
    static int flag = 0;
    
    if(flag){
        flag = 0;
        return 0;
    }
        

    flag = 1;
    
    rv = sprintf(buffer, PROC_NAME);
    copt_to_user(user_buff, buffer, rv);
    printk(KERN_INFO "%s", buffer);
    return rv;
}

module_init(proc_init);
module_exit(proc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("creating jiffies");
MODULE_AUTHOR("AMHF");



