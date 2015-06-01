#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <asm/uaccess.h>

#define SIMPLE_DEVICE_NAME "simple"

#define SIMPLE_DEVICE_DATA_SIZE (4096)

struct simple_dev {
    char*       data;
    ssize_t     dlen;
    ssize_t     dsize;
    struct cdev cdev;
};

static dev_t              dev;
static struct cdev        cdev;
static struct simple_dev  this_dev;

static int simple_op_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &this_dev;

    printk(KERN_INFO "%s\n", __func__);

    return 0;
}

static int simple_op_release(struct inode *inode, struct file *filp)
{
    filp->private_data = NULL;

    printk(KERN_INFO "%s\n", __func__);

    return 0;
}

static ssize_t simple_op_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct simple_dev *ds = (struct simple_dev *)filp->private_data;
    ssize_t len;
    ssize_t left;
    ssize_t copied = 0;

    printk(KERN_INFO "read(len=%d, off=%lld)\n", count, *f_pos);

    if (*f_pos >= ds->dlen)
    {
        return 0; // nothing to read, beyond end-of-file
    }

    len = ds->dlen - *f_pos;
    if (len)
    {
        left = copy_to_user(buf, ds->data + *f_pos, len);
        copied = len - left;
    }
    *f_pos += len;

    return copied;
}

static ssize_t simple_op_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct simple_dev *ds = (struct simple_dev *)filp->private_data;
    ssize_t len;
    ssize_t left;
    ssize_t copied = 0;

    printk(KERN_INFO "write(len=%d, off=%lld)\n", count, *f_pos);

    if (*f_pos >= ds->dsize)
    {
        return 0; // nothing to write, beyond end-of-file
    }

    if (*f_pos + count >= ds->dsize)
    {
        len = ds->dsize - *f_pos;
    }
    else
    {
        len = count;
    }
    if (len)
    {
        left = copy_from_user(ds->data + *f_pos, buf, len);
        copied = len - left;
    }
    *f_pos += len;
    if (*f_pos > ds->dlen)
    {
        ds->dlen = *f_pos;
    }

    return copied;
}

static loff_t simple_op_llseek(struct file *filp, loff_t off, int whence)
{
    struct simple_dev* ds = (struct simple_dev*)filp->private_data;
    loff_t             newpos;

    switch (whence)
    {
        case 0: /* SEEK_SET */
            newpos = off;
            break;

        case 1: /* SEEK_CUR */
            newpos = filp->f_pos + off;
            break;

        case 2: /* SEEK_END */
            newpos = ds->dsize + off;
            break;

        default: /* can't happen */
            return -EINVAL;
    }

    if (newpos < 0 || newpos > ds->dsize)
    {
        return -EINVAL;
    }

    filp->f_pos = newpos;

    return newpos;
}

static struct file_operations simple_fops = {
    .owner   = THIS_MODULE,
    .open    = simple_op_open,
    .release = simple_op_release,
    .read    = simple_op_read,
    .write   = simple_op_write,
    .llseek  = simple_op_llseek,
};

void simple_exit(void)
{
    printk(KERN_INFO "%s\n", __func__);

    if ( this_dev.data )
    {
        kfree(this_dev.data);
    }
    this_dev.data  = NULL;
    this_dev.dlen  = 0;
    this_dev.dsize = 0;

    cdev_del(&cdev);
    unregister_chrdev_region(dev, 1);
}

int simple_init(void)
{
    if ( alloc_chrdev_region(&dev, 0, 1, SIMPLE_DEVICE_NAME) < 0 )
    {
        printk(KERN_ERR "alloc_chrdev_region(%s) failed\n", SIMPLE_DEVICE_NAME);
        return -1;
    }

    cdev_init(&cdev, &simple_fops);
    cdev.owner = THIS_MODULE;
    cdev.ops   = &simple_fops;
    if ( cdev_add (&cdev, dev, 1) < 0 )
    {
        printk(KERN_ERR "cdev_add failed\n");
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    printk(KERN_INFO "major:%d, minor:%d\n", MAJOR(dev), MINOR(dev));

    this_dev.data = kmalloc(SIMPLE_DEVICE_DATA_SIZE, GFP_KERNEL);
    if (NULL == this_dev.data)
    {
        simple_exit();
		return -ENOMEM;
	}
	memset(this_dev.data, 0, SIMPLE_DEVICE_DATA_SIZE);
	this_dev.dsize = SIMPLE_DEVICE_DATA_SIZE;
    this_dev.dlen  = 0;

    return 0;
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_AUTHOR("Nobody");
MODULE_DESCRIPTION("Minimal Driver Example");
MODULE_ALIAS("Does not do much");
MODULE_LICENSE("GPL");
