#include<linux/device.h>
#include<linux/fs.h>
#include<linux/module.h>
#include<linux/cdev.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
dev_t device_number;
struct cdev *c_op;
struct class *class_p;
struct device *dev_op;
ssize_t dev_write(struct file *f_ops,const char __user *buff,size_t count,loff_t *pos_of )
{

}
ssize_t dev_read(struct file *f_ops,char __user *buff,size_t count,loff_t *pos_of)
{

}
loff_t dev_lseek(struct file *f_ops,loff_t offset,int whwre)
{

}
int dev_open()
struct file_opreation f_ops=
{
	.owner=THIS_MODULE,
	.write=dev_write,
	.read=dev_read,
	.open=dev_open,
	.release=pcd_release
};
static int __init my_dev_init(void)
{
	// device number created 
alloc_chardev_region(&device_numbe,0,1,"my char");

// cdev file in init
cdev_init(&c_op,&f_ops);
//add cdev fsa
c_op.owner=THIS_MODUEL;
cdev_add(&c_op,device_number,1);
// class crted in sys/class
class_p=class_create(THIS_MODULE,"dev_op");
dev_op=(class_p,NULL,device_number,NULL,"my_de");

}
static void __exit my_dev_exit(void)
{

}

module_init(my_dev_init);
module_exit(my_dev_exit);

MODULE_AUTHOR("digambar jondhale");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("char driver");
