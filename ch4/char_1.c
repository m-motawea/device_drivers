#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>


MODULE_AUTHOR("Maged Motawea");
MODULE_DESCRIPTION("Character 1");
MODULE_LICENSE("GPL v2");

int maj_num;
int min_num;
int count = 1;
dev_t my_drv;

struct cdev *my_dev;
struct file_operations fops = {};

static int __init dev_init(void)
{
  int res;

  //allocate device regions to get MAJOR & MINOR numbers
  res = alloc_chrdev_region(&my_drv, 0, count, "my_drv");
  if (res != 0)
  {
    printk(KERN_INFO "Failed to allocate driver region. ret_code=%d\n", res);
    return -1;
  }
  maj_num = MAJOR(my_drv);
  min_num = MINOR(my_drv);
  printk(KERN_INFO "driver loaded with maj_num: %d\n", maj_num);

  //registering the device
  my_dev = cdev_alloc();
  cdev_init(my_dev, &fops);
  printk(KERN_INFO "device initialized\n");

  //go live
  cdev_add(my_dev, my_drv, count);
  printk(KERN_INFO "device is ready\n");

  return 0;
}


static void __exit dev_exit(void)
{
  //removing the device
  cdev_del(my_dev);
  printk(KERN_INFO "device removed\n");

  //releasing device region (MAJOR & MINOR)
  unregister_chrdev_region(my_drv, count);
  printk(KERN_INFO "region unregistered\n");
}

module_init(dev_init);
module_exit(dev_exit);
