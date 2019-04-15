#include <linux/module.h>
#include <linux/init.h>


MODULE_AUTHOR("Maged Motawea");
MODULE_DESCRIPTION("Lab2");
MODULE_LICENSE("GPL v2");

int param = 0;
module_param(param, int, 0);



static int __init lab_init(void)
{
  printk(KERN_INFO "Lab2 Module Loaded with param: %d\n", param);
  return 0;
}

static void __exit lab_exit(void)
{
  printk(KERN_INFO "Lab2 Module Unloaded");
}

//module_init(lab_init);
module_exit(lab_exit);
