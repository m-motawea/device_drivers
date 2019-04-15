#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

static LIST_HEAD(my_list);

struct my_entry {
  struct list_head clist;
  int num;
  char *name;
};


static int my_list_init (void)
{
  struct my_entry *me;
  int j;
  for (j = 0; j < 5; j++) {
    me = kmalloc(sizeof(struct my_entry), GFP_KERNEL);
    if (!me) {
      printk(KERN_INFO "failed to allocate me.\n");
      return -1;
    }
    me->num = j;
    //sprintf(me->name , "My_%d", j + 1);
    me->name = "dev";
    list_add(&me->clist, &my_list);
  }
  return 0;
}


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


static int __init my_init(void)
{
  my_list_init();
  printk(KERN_INFO "my_list initialized\n");
  walk_list();
  return 0;
}


static void __exit my_exit(void)
{
  printk(KERN_INFO "clearing list\n");
  my_list_clear();
  printk(KERN_INFO "final list\n");
  walk_list();
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Maged Motawea");
