#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/timer.h>
#include <linux/hrtimer.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define CHRDEV_MAJOR 236
#define CHRDEV_MINOR 0
#define CHRDEV_COUNT 1
#define CHRDEV_NAME "ltd_dev"
#define THREAD1 1
#define THREAD2 2
#define START 1
#define PAUSE 9
#define CONTINUE 3
#define STOP 4
#define STARTALL 5
#define PAUSEALL 6
#define CONTINUEALL 7
#define STOPALL 8

struct chr_cdev{
	struct cdev cdev;
	int major;
	dev_t dev;
	struct class *dev_class;
};
struct chr_cdev *ltd_cdev;

struct timer_list timer1;
struct timer_list timer2;

void  timer_function1(unsigned long arg)
{
	printk("receive data from timer1: %d\n",arg);
}

void  timer_function2(unsigned long arg)
{
        printk("receive data from timer2: %d\n",arg);
}

void my_delay(unsigned int s)
{
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout( msecs_to_jiffies(s) );
}

struct semaphore sem1;
struct semaphore sem2;

static struct task_struct *my_task1=NULL;
static struct task_struct *my_task2=NULL;

static int my_kthread(void *arg)
{
	int i;
	while(!kthread_should_stop())
	{
		for(i=1;i<=100;i++){
                        down(&sem1);
                        set_current_state(TASK_INTERRUPTIBLE);
                        printk("Thread1:%d\n",i);
                        up(&sem1);
                }
                my_delay(1000);
	}
	return 0;
}
static int my_kthread2(void *arg)
{
	int i;
        while(!kthread_should_stop())
        {
		for(i=1;i<=100;i++){
			down(&sem2);
                	set_current_state(TASK_INTERRUPTIBLE);
                        printk("Thread2:%d\n",i);
                	up(&sem2);
		}
		my_delay(2000);
        }
	return 0;
}
int thread_run(int n)
{
	int err;
	if(n==1){
		my_task1=kthread_create(my_kthread,NULL,"Thread1");
		if(IS_ERR(my_task1)){
			printk("kthread1_create error\n");
			err=PTR_ERR(my_task1);
			return err;
		}
		printk("successfully create Thread1\n");
		wake_up_process(my_task1);
	}
	else if(n==2){
		my_task2=kthread_create(my_kthread2,NULL,"Thread2");
                if(IS_ERR(my_task2)){
                        printk("kthread2_create error\n");
                        err=PTR_ERR(my_task2);
                        return err;
                }
                printk("successfully create Thread2\n");
                wake_up_process(my_task2);
	}
	return 0;
}

int ltd_open(struct inode *inode,struct file *filp)
{
	printk("%s\r\n",__func__);
	return 0;
}

long ltd_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
	int ret=0;
	printk("cmd %d %ld\n",cmd,arg);
	switch(cmd){
	case START:
		if(arg==THREAD1){
			if(!my_task1)
				ret=thread_run(1);
		}
		else{
			if(!my_task2)
				ret=thread_run(2);
		}
		break;
	case PAUSE:
		if(arg==THREAD1)
			sema_init(&sem1,0);
		else
			sema_init(&sem2,0);
		break;
	case CONTINUE:
		if(arg==THREAD1)
                        up(&sem1);
                else
                        up(&sem2);
                break;
	case STOP:
		if(arg==THREAD1 && my_task1){
			printk("STOP THREAD1\n");
			kthread_stop(my_task1);
			my_task1=NULL;
		}
		else if(arg==THREAD2 && my_task2){
			printk("STOP THREAD2\n");
                        kthread_stop(my_task2);
                        my_task2=NULL;
		}
		break;
	case STARTALL:
		if(!my_task1)
			ret=thread_run(1);
		if(!my_task2)
			ret=thread_run(2);
		break;
	case PAUSEALL:
		sema_init(&sem1,0);
		sema_init(&sem2,0);
		break;
	case CONTINUEALL:
		up(&sem1);
		up(&sem2);
		break;
	case STOPALL:
		if(my_task1){
			printk("STOP THREAD1\n");
                        kthread_stop(my_task1);
                        my_task1=NULL;
		}
		if(my_task2){
			printk("STOP THREAD2\n");
                        kthread_stop(my_task2);
                        my_task2=NULL;
		}
		break;
	default:
		return -EINVAL;
	}
	return ret;
}

int ltd_release(struct inode *inode,struct file *filp)
{
	printk("%s\r\n",__func__);
	return 0;
}

const struct file_operations fops={
	.owner=THIS_MODULE,
	.open=ltd_open,
	.unlocked_ioctl=ltd_ioctl,
	.release=ltd_release,
};

static int __init demo_init(void)
{
	printk("%s:\n",__func__);
	if(register_chrdev_region(MKDEV(CHRDEV_MAJOR,CHRDEV_MINOR),CHRDEV_COUNT,CHRDEV_NAME)<0){
		printk("failed to register\n");
		return -EBUSY;
	}
	ltd_cdev=kmalloc(sizeof(*ltd_cdev),GFP_KERNEL);
	if(ltd_cdev==NULL){
		return -ENOMEM;
	}
	ltd_cdev->dev=MKDEV(CHRDEV_MAJOR,CHRDEV_MINOR);
	cdev_init(&ltd_cdev->cdev,&fops);
	ltd_cdev->cdev.owner=THIS_MODULE;
	cdev_add(&ltd_cdev->cdev,ltd_cdev->dev,CHRDEV_COUNT);

	sema_init(&sem1,1);
	sema_init(&sem2,1);

	return 0;
}

static void __exit demo_exit(void)
{
	printk("%s:\n",__func__);
	cdev_del(&ltd_cdev->cdev);
	kfree(ltd_cdev);
	unregister_chrdev_region(ltd_cdev->dev,CHRDEV_COUNT);

	if(my_task1){
		printk(KERN_ALERT "Stop thread1\n");
		kthread_stop(my_task1);
	}
	if(my_task2)
	{
		printk(KERN_ALERT "Stop thread2\n");
                kthread_stop(my_task2);
	}
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ltd");
