#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <asm/system.h>  
#include <asm/uaccess.h> 

#define DEVICE_NAME "koxui_gpio_test"
enum led_color
{
	led_blue = 1,
	led_red,
	led_green
};

extern void set_led_colour_status(enum led_color led_type);
extern void turn_off_led(enum led_color led_type);

struct scull_qset{
	void **data;
	struct scull_qset *next; 
}; 
struct gpio_cdev{
	struct scull_qset *data;
	int quantum;
	int qset;  
	unsigned long size;  	
	struct semaphore sem;
	struct cdev 	my_cdev;
	struct class 	*my_class;
};

struct gpio_cdev *mt_gpio_cdev;
int device_node_nu=0;
	
static int gpio_test_open(struct inode *inode, struct file *filp)
{
	struct gpio_cdev *open_dev;
	open_dev = container_of(inode->i_cdev,struct gpio_cdev,my_cdev);
	filp->private_data = open_dev;

	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY){
		turn_off_led(led_green);
		turn_off_led(led_red);
		set_led_colour_status(led_blue);
	}
	printk(KERN_INFO"[koxui_gpio_test]:gpio_test_open ok\n");
	
	return 0;
}

ssize_t gpio_test_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	char *my_recive;
	char msg[10] = {'\0'};
	
	my_recive = msg;				//记得要给申请的指针初始化
	ssize_t retval = -ENOMEM; 
	if (copy_from_user(my_recive,buf,count)){
		retval = -EFAULT;
		goto out;
	}
	printk("[koxui_gpio_test]:info ouput %d\n",my_recive[0]);
	printk("[koxui_gpio_test]:koxui_gpio_test write ok\n");
	return 0;
out:
	printk("[koxui_gpio_test]:koxui_gpio_test error goto out \n");
	return retval;

}
static int gpio_test_release(struct inode *inode,struct file *file)
{
	printk("[koxui_gpio_test]:koxui_gpio_test release ok\n");
	return 0;
}
static struct file_operations my_ops = {
	.owner = THIS_MODULE,
	.open  = gpio_test_open,
	.write = gpio_test_write,
	.release = gpio_test_release,

};
static int __init koxui_gpio_test_init(void)
{
	int err = 0;
	int ret = 0;
	
	ret = alloc_chrdev_region(&device_node_nu,0,1,DEVICE_NAME);
	if (ret < 0)
	{
		printk("[koxui_gpio_test]:: alloc_chrdev_region failed\n");
		return ret;
	}
	mt_gpio_cdev = kmalloc(sizeof(struct gpio_cdev), GFP_KERNEL);
	if (!mt_gpio_cdev)
	{
		printk("[koxui_gpio_test]:: kmalloc failed\n");
		err = -ENOMEM;
		goto fail_malloc;
	}
	
	cdev_init(&mt_gpio_cdev->my_cdev, &my_ops);
	mt_gpio_cdev->my_cdev.owner = THIS_MODULE;
	mt_gpio_cdev->my_cdev.ops 	= &my_ops;
	cdev_add(&mt_gpio_cdev->my_cdev, device_node_nu, 1);

	mt_gpio_cdev->my_class = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(mt_gpio_cdev->my_class, NULL, device_node_nu, NULL, DEVICE_NAME);
	
	return 0;
fail_malloc:
	unregister_chrdev_region(device_node_nu, 1);
	return -1;
}
static void __exit koxui_gpio_test_exit(void)
{
	cdev_del(&mt_gpio_cdev->my_cdev);	
	unregister_chrdev_region(device_node_nu, 1);
	device_destroy(mt_gpio_cdev->my_class, device_node_nu);
	class_destroy(mt_gpio_cdev->my_class);
	kfree(&mt_gpio_cdev->my_cdev);
	kfree(mt_gpio_cdev->my_class);
}
module_init(koxui_gpio_test_init);
module_exit(koxui_gpio_test_exit);
MODULE_LICENSE("GPL");