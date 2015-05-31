#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>

#define SIMPLE_DEVICE_NAME "simple"

static dev_t       simple_dev;
static struct cdev simple_cdev;

static int simple_op_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "%s\n", __func__);
    return 0;
}

static int simple_op_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "%s\n", __func__);
    return 0;
}

static ssize_t simple_op_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "%s\n", __func__);
    return count;
}

static ssize_t simple_op_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "%s\n", __func__);
    return count;
}

static struct file_operations simple_fops = {
    .owner   = THIS_MODULE,
    .open    = simple_op_open,
    .release = simple_op_release,
    .read    = simple_op_read,
    .write   = simple_op_write,
};

int simple_init(void)
{
    if ( alloc_chrdev_region(&simple_dev, 0, 1, SIMPLE_DEVICE_NAME) < 0 )
    {
        printk(KERN_ERR "alloc_chrdev_region(%s) failed\n", SIMPLE_DEVICE_NAME);
        return -1;
    }

    cdev_init(&simple_cdev, &simple_fops);
    simple_cdev.owner = THIS_MODULE;
    simple_cdev.ops   = &simple_fops;
    if ( cdev_add (&simple_cdev, simple_dev, 1) < 0 )
    {
        printk(KERN_ERR "cdev_add failed\n");
        return -1;
    }

    printk(KERN_INFO "major:%d, minor:%d\n", MAJOR(simple_dev), MINOR(simple_dev));

    return 0;
}

void simple_exit(void)
{
    printk(KERN_INFO "%s\n", __func__);
    unregister_chrdev_region(simple_dev, 1);
    cdev_del(&simple_cdev);
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_AUTHOR("Gunchul, Song");
MODULE_LICENSE("Dual BSD/GPL");