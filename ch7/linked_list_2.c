#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

// list related functions
static LIST_HEAD(my_list);

struct my_entry {
  struct list_head clist;
  int num;
  char *name;
};

struct my_entry *entries[20];

static int node_count = 0;

static int walk_list(void)
{
  int j = 0;
  struct list_head *l;

  if (list_empty(&my_list)) {
    return 0;
  }

  list_for_each(l, &my_list) {
    struct my_entry *me = list_entry(l, struct my_entry, clist);
    printk(KERN_INFO "entry number: %d, name: %s\n", me->num, me->name);
    j++;
  }
  return j;
}



static void my_list_clear(void)
{
  struct list_head *list;
  struct list_head *tmp;

  if (list_empty(&my_list)) {
    printk(KERN_INFO "list empty. nothing to clear\n");
    return;
  }

  list_for_each_safe(list, tmp, &my_list) {
    struct my_entry *me = list_entry(list, struct my_entry, clist);
    list_del(&me->clist);
    kfree(me);
  }
}






// driver

dev_t first;
static int count = 1;
int maj_num;
int min_num;

static struct class *my_class;

static ssize_t my_write(struct file *filp, const char __user *buff, size_t size, loff_t *offset)
{
  struct my_entry *me;
  me = kmalloc(sizeof(struct my_entry), GFP_KERNEL);
  me->name = kmalloc(20 * sizeof(char), GFP_KERNEL);
  if (!me) {
    return 0;
  }

  int nbytes = size - copy_from_user(me->name, buff, size);
  printk(KERN_INFO "new name %s\n", me->name);

  node_count++;
  me->num = node_count;
  list_add(&me->clist, &my_list);

  entries[node_count - 1] = me;

  walk_list();
  return nbytes;
}


static ssize_t my_read(struct file *file, char __user *buff, size_t size, loff_t *ppos)
{
  if (list_empty(&my_list)) {
    return 0;
  }

  struct my_entry *me = entries[node_count - 1];
  node_count--;
  printk(KERN_INFO "name: %s\n", me->name);
  int nbytes = size - copy_to_user(buff, me->name, size);
  list_del(&me->clist);
  walk_list();
  return nbytes;
}



struct cdev *my_dev;
struct file_operations fops = {
  .owner = THIS_MODULE,
  .write = my_write,
  .read = my_read,
};




static int __init my_init(void)
{
  int res;
  res = alloc_chrdev_region(&first, 0, count, "my_driver");
  if (res) {
    printk(KERN_INFO "failed to allocate region");
    return -1;
  }

  maj_num = MAJOR(first);
  min_num = MINOR(first);
  printk(KERN_INFO "driver loaded with maj_num: %d\n", maj_num);


  my_dev = cdev_alloc();
  cdev_init(my_dev, &fops);
  printk(KERN_INFO "device initialized\n");

  cdev_add(my_dev, first, count);
  printk(KERN_INFO "device is ready\n");

  my_class = class_create(THIS_MODULE, "my_class");
  device_create(my_class, NULL, first, "%s", "list_driver");
  printk(KERN_INFO "created device nodes\n");

  return 0;
}


static void __exit my_exit(void)
{
  printk(KERN_INFO "clearing list\n");
  my_list_clear();
  printk(KERN_INFO "final list\n");
  walk_list();

  device_destroy(my_class, first);
  class_destroy(my_class);
  printk(KERN_INFO "removed device node\n");

  cdev_del(my_dev);
  printk(KERN_INFO "removed device\n");

  unregister_chrdev_region(first, count);
  printk(KERN_INFO "region unrigistered\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Maged Motawea");
