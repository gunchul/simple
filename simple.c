#include <linux/init.h>
#include <linux/module.h>

static int simple_init(void)
{
    printk(KERN_ALERT "%s\n", __func__);
    return 0;
}

static void simple_exit(void)
{
    printk(KERN_ALERT "%s\n", __func__);
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE("GPL");