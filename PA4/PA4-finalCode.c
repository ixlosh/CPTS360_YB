#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_NAME "sysprog_counter"
#define BUFFER_SIZE 128

static int counter = 0;
static struct proc_dir_entry *proc_entry;

/* ~TODO~ (DONE): Implement this function */
static ssize_t proc_read(struct file *file, char __user *user_buffer,
                         size_t count, loff_t *pos)
{
    char buffer[BUFFER_SIZE];
    int len;

    /* Are we already done? */
    if (*pos > 0) {
        return 0; 
    }

    /* We use snprintf to correctly format */
    len = snprintf(buffer, BUFFER_SIZE, "Counter value: %d\n", counter);

    /* Enough size in the buffer? */
    if (count < len) {
        return -EINVAL; // Is invalid if too small buffer
    }

    /* Safely copyinh */
    if (copy_to_user(user_buffer, buffer, len)) {
        return -EFAULT; // Bad address
    }

    /* New offset */
    *pos += len;

    /* Return read length */
    return len;
}

/* ~TODO~ (DONE): Implement this function */
static ssize_t proc_write(struct file *file, const char __user *user_buffer,
                          size_t count, loff_t *pos)
{
    char buffer[BUFFER_SIZE];
    int new_value;
    int ret;

    /* Must be less than our buffer (use < to ensure enough space for null terminator) */
    if (count >= BUFFER_SIZE) {
        return -EINVAL; // Invalid argument (input too large)
    }

    /* Safely copy data from user space to kernel space */
    if (copy_from_user(buffer, user_buffer, count)) {
        return -EFAULT; // Bad address
    }

    /* Null-termination (vital) */
    buffer[count] = '\0';

    /* PaRSING INTO integer */
    // -(kstrtoint is the safe kernel equivalent of atoi.)
    ret = kstrtoint(buffer, 10, &new_value);
    if (ret < 0) {
        return -EINVAL; // Send me an actual integer!!!
    }

    /* Updating the kernel-maintained counter */
    counter = new_value;

    /* Return number of bytes written */
    return count;
}

static const struct proc_ops proc_file_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

static int __init sysprog_init(void)
{
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_file_ops);
    if (!proc_entry) {
        return -ENOMEM;
    }

    printk(KERN_INFO "sysprog_counter module loaded\n");
    return 0;
}

static void __exit sysprog_exit(void)
{
    proc_remove(proc_entry);
    printk(KERN_INFO "sysprog_counter module unloaded\n");
}

module_init(sysprog_init);
module_exit(sysprog_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Systems Programming Course");
MODULE_DESCRIPTION("Basic kernel module with /proc interface");