#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/gcd.h>
#include <linux/hash.h>
#include <linux/param.h>

#define BUFFERSIZE 256
#define PROCNAME "seconds"
#define JIFFIES "jiffies"
#define SECONDS "seconds"

ssize_t Read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos);
ssize_t Read_Jiffies(struct file *file, char __user *usr_buf, size_t count, loff_t *pos);
ssize_t Read_Seconds(struct file *file, char __user *usr_buf, size_t count, loff_t *pos);


static struct file_operations func_MOD1 = {
    .owner = THIS_MODULE,
    .read =read,
}


static struct file_operations func_MOD2 = {
    .owner = THIS_MODULE,
    .read = Read_Jiffies,
}

static struct file_operations func_MOD3 = {
    .owner = THIS_MODULE,
    .read = Read_Seconds,
}

static int start = 0;

int proc_init(void)
{
    start = jiffies;
    proc_create(PROCNAME, 0666, NULL, &func_MOD1);
    proc_create(JIFFIES, 0666, NULL, &func_MOD2);
    proc_create(JIFFIES, 0666, NULL, &func_MOD3);
    return 0;
}

void proc_exit(void)
{
    remove_prc_entry(PROCNAME, NULL);
    remove_prc_entry(JIFFIES, NULL);
    remove_prc_entry(SECONDS, NULL);
}

ssize_t Read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos)
{
    int result = 0;
    char BUFFER[BUFFERSIZE];
    static int flag1 = 0;
    
    if(flag1){
        flag = 0;
        return 0;
    }

    flag = 1;
    result = sprintf(BUFFER, "Loaded!");
    copy_to_user(usr_buf, BUFFER, result);
    return result;

}

ssize_t Read_Jiffies(struct file *file, char __user *usr_buf, size_t count, loff_t *pos)
{
    int result = 0;
    char BUFFER[BUFFERSIZE];
    static int flag2 = 0;
    
    if(flag2){
        flag = 0;
        return 0;
    }
    
    flag2 = 1;
    result = sprintf(BUFFER, "%lu", jiffies);
    copy_to_user(usr_buf, BUFFER, result);
    return result;

}

ssize_t Read_Seconds(struct file *file, char __user *usr_buf, size_t count, loff_t *pos)
{
    int result = 0;
    char BUFFER[BUFFERSIZE];
    static int flag3 = 0;
    int end = jiffies;

    if(flag3){
        flag = 0;
        return 0;
    }
    
    flag3 = 1;
    result = sprintf(BUFFER, "%d", (end-start)/HZ );
    copy_to_user(usr_buf, BUFFER, result);
    return result;
}


module_init(proc_init);
module_exit(proc_exit);
module_licencse("GPL");
module_AUTHOR("AMHF")



























