#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <asm/system.h>  
#include <asm/uaccess.h> 
#include<asm/semaphore.h>

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
	struct delayed_work my_delay_work_list;
	
};

struct gpio_cdev *mt_gpio_cdev;
int device_node_nu=0;


static void delay_work_func(struct work_struct *work)
{
	schedule_delayed_work(&mt_gpio_cdev->my_delay_work_list, msecs_to_jiffies(500));
	printk(KERN_INFO"[koxui_gpio_test]:delay_work_func ok\n");	
}
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
	schedule_delayed_work(&mt_gpio_cdev->my_delay_work_list, msecs_to_jiffies(500));
	return 0;
}

ssize_t gpio_test_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	char *my_recive;
	char msg[10] = {0};
	
	my_recive = msg;				
	ssize_t retval = -ENOMEM; 
	if (copy_from_user(msg,buf,count)){
		retval = -EFAULT;
		goto out;
	}
	printk("[koxui_gpio_test]:info ouput %d\n",msg[0]);
	printk("[koxui_gpio_test]:koxui_gpio_test write ok\n");
	return 0;
out:
	printk("[koxui_gpio_test]:koxui_gpio_test error goto out \n");
	return retval;

}
static long gpio_test_unlocked_ioctl(struct file *file, unsigned int cmd,unsigned long arg)       
{
	printk("[koxui_gpio_test]:gpio_test_unlocked_ioctl cmd is %d \n",cmd);
	switch(cmd){
	case 1:
		printk("[koxui_gpio_test]:gpio_test_unlocked_ioctl cmd is %d \n",cmd);
		break;
	case 2:
		printk("[koxui_gpio_test]:gpio_test_unlocked_ioctl cmd is %d \n",cmd);
		break;	
	default:
		break;
	}
	return 0;
}
static int gpio_test_release(struct inode *inode,struct file *file)
{
	printk("[koxui_gpio_test]:koxui_gpio_test release ok\n");
	return 0;
}
static struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open  = gpio_test_open,
	.write = gpio_test_write,
	.release = gpio_test_release,
	.unlocked_ioctl = gpio_test_unlocked_ioctl,

};
static struct miscdevice my_gpio_device = {
	.minor = MISC_DYNAMIC_MINOR,//设备的副设备号，以此来区分设备
	.name = DEVICE_NAME,
	.fops = &my_fops,
};
static int __init koxui_gpio_test_init(void)
{
	int err = 0;
	int ret = 0;
	
	//register misc device
	mt_gpio_cdev = kmalloc(sizeof(struct gpio_cdev), GFP_KERNEL);
	if((err = misc_register(&my_gpio_device))){
		printk("[koxui_gpio_test]:: misc failed\n");	
		goto fail_malloc;
	}
	INIT_DELAYED_WORK(&mt_gpio_cdev->my_delay_work_list,delay_work_func);
#if 0 //register char device
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
#endif

	return 0;
fail_malloc:
	//unregister_chrdev_region(device_node_nu, 1);
	return -1;
}
static void __exit koxui_gpio_test_exit(void)
{
	if(misc_deregister(&my_gpio_device)){
		printk("[koxui_gpio_test]:: misc failed\n");		
	}
#if 0
	cdev_del(&mt_gpio_cdev->my_cdev);	
	unregister_chrdev_region(device_node_nu, 1);
	device_destroy(mt_gpio_cdev->my_class, device_node_nu);
	class_destroy(mt_gpio_cdev->my_class);
	kfree(&mt_gpio_cdev->my_cdev);
	kfree(mt_gpio_cdev->my_class);
#endif
}
module_init(koxui_gpio_test_init);
module_exit(koxui_gpio_test_exit);
MODULE_LICENSE("GPL");