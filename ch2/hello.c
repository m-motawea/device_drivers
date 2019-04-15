#include <linux/module.h>
#include <linux/init.h>


static int __init my_init(void)
{
  printk(KERN_INFO "Hello: module loaded");
  return 0;
}

static void __exit my_exit(void)
{
  printk(KERN_INFO "Bye module unloaded");
}


module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("A GENIUS");
MODULE_LICENSE("GPL v2");
