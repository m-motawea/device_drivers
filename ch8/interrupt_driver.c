#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>


#define SHARED_IRQ 17

MODULE_AUTHOR("Maged Motawea");
MODULE_DESCRIPTION("Character 2");
MODULE_LICENSE("GPL v2");

int maj_num;
int min_num;

int count = 1;
dev_t my_drv;

static size_t s = (20 * PAGE_SIZE);
static char *kbuf;



//interrupt vars
static int irq = SHARED_IRQ, my_dev_id, irq_counter = 0;

static irqreturn_t my_handler(int irq, void *dev_id) {
  //interrupt handler
  irq_counter++;
  pr_info("In ISR: counter = %d\n", irq_counter);
  return IRQ_NONE;
}


static ssize_t my_read(struct file *file, char __user *buff, size_t lbuf, loff_t *ppos)
{
  int nbytes = lbuf - copy_to_user(buff, kbuf + *ppos, lbuf);
  *ppos += nbytes;
  printk(KERN_INFO "\n READING function, nbytes=%d, pos=%d\n", nbytes, (int) *ppos);
  return nbytes;
}


struct cdev *my_dev;
struct file_operations fops = {
  .owner = THIS_MODULE,
  .read = my_read,
};
static struct class *my_class;


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

  //creating device node
  my_class = class_create(THIS_MODULE, "my_class");
  device_create(my_class, NULL, my_drv, "%s", "mycdrv");
  printk(KERN_INFO "created device node");

  //allocating buffer
  kbuf = kmalloc(s, GFP_KERNEL);

  //registering interrupt handler
  if (request_irq(irq, my_handler, IRQF_SHARED, "my_interrupt", &my_dev_id)) {
    pr_info("Failed to reserve irq %d\n", irq);
    return -1;
  }
  pr_info("suceesfully loading ISR handler\n");

  return 0;
}


static void __exit dev_exit(void)
{
  //free kbuf
  kfree(kbuf);

  //removing device node
  device_destroy(my_class, my_drv);
  class_destroy(my_class);
  printk(KERN_INFO "removed device node");

  //removing the device
  cdev_del(my_dev);
  printk(KERN_INFO "device removed\n");

  //releasing device region (MAJOR & MINOR)
  unregister_chrdev_region(my_drv, count);
  printk(KERN_INFO "region unregistered\n");
}

module_init(dev_init);
module_exit(dev_exit);
