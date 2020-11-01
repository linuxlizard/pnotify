/*
 * Poke inotify from userspace to trigger a userspace notify event on
 * filesystems that don't support inotify (such as NFS, SMBFS).
 *
 * Copyright 2020 David Poole <davep@mbuf.com>
 *
 * Started from code from drivers/char/hw_random/core.c
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 *
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/hw_random.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
//#include <linux/sched/signal.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/file.h>
#include <linux/namei.h>

MODULE_AUTHOR("David Poole <davep@mbuf.com>");
MODULE_DESCRIPTION("Trigger inotify event from userspace.");
MODULE_LICENSE("GPL");

#define PNOT_MODULE_NAME		"pnot"

static int rng_dev_open(struct inode *inode, struct file *filp)
{
	/* enforce read-only access to this chrdev */
	if ((filp->f_mode & FMODE_READ) == 0)
		return -EINVAL;
	if (filp->f_mode & FMODE_WRITE)
		return -EINVAL;
	printk( KERN_NOTICE "davep %s %d\n", __func__, __LINE__ );
	return 0;
}

static ssize_t rng_dev_read(struct file *filp, char __user *buf,
			    size_t size, loff_t *offp)
{
	ssize_t ret = 0;
	int err = 0;
	char rng_buffer[64];
	int len;

	printk( KERN_NOTICE "davep %s %d\n", __func__, __LINE__ );

	strcpy(rng_buffer, __func__);
	len = strlen(rng_buffer);

	if (copy_to_user(buf, rng_buffer, len)) {
		printk( KERN_NOTICE "davep %s %d\n", __func__, __LINE__ );
		err = -EFAULT;
		goto out;
	}
	ret += len;

out:
	return ret ? : err;
}

static const struct file_operations rng_chrdev_ops = {
	.owner		= THIS_MODULE,
	.open		= rng_dev_open,
	.read		= rng_dev_read,
	.llseek		= noop_llseek,
};

static const struct attribute_group *rng_dev_groups[];

static struct miscdevice pnot_miscdev = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= PNOT_MODULE_NAME,
	.nodename	= "pnot",
	.fops		= &rng_chrdev_ops,
	.groups		= rng_dev_groups,
};

static ssize_t hwrng_attr_current_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t len)
{
	int err;
//	struct kstat stat;
//	struct file* file;
	unsigned int lookup_flags = LOOKUP_FOLLOW ;
	struct path path;

	err = 0;

	printk( KERN_NOTICE "davep %s buf=%s len=%zu\n", __func__, buf, len);

//	err = vfs_stat(buf, &stat);
//	printk( KERN_NOTICE "davep %s vfs_stat err=%d\n", __func__, err);

	// fs/open.c
//	file = filp_open(buf, O_RDONLY, 0);
//	if (file) {
//		printk( KERN_NOTICE "davep %s %d\n", __func__, __LINE__ );
//		fput(file);
//	}

	/* reading do_sys_truncate() fs/open.c
	 */
	err = user_path_at(AT_FDCWD, buf, lookup_flags, &path);
	if (err) {
		printk( KERN_NOTICE "davep %s %d err=%d\n", __func__, __LINE__, err );
	}
	else {
		printk( KERN_NOTICE "davep %s %d\n", __func__, __LINE__ );
		path_put(&path);
	}

	return err ? : len;
}

static ssize_t hwrng_attr_current_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	ssize_t ret=-1;

	ret = snprintf(buf, PAGE_SIZE, "%s/%s\n", __func__, "hello, world");

	return ret;
}

static ssize_t hwrng_attr_available_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	int ret;

	ret = snprintf(buf, PAGE_SIZE, "%s/%s\n", __func__, "hello, world");

	return strlen(buf);
}

static DEVICE_ATTR(rng_current, S_IRUGO | S_IWUSR,
		   hwrng_attr_current_show,
		   hwrng_attr_current_store);
static DEVICE_ATTR(rng_available, S_IRUGO,
		   hwrng_attr_available_show,
		   NULL);

static struct attribute *rng_dev_attrs[] = {
	&dev_attr_rng_current.attr,
	&dev_attr_rng_available.attr,
	NULL
};

ATTRIBUTE_GROUPS(rng_dev);

static int __init pnotify_modinit(void)
{
	int err;

	printk(KERN_INFO "Hello world 1.\n");

	err = misc_register(&pnot_miscdev);
	printk( KERN_NOTICE "davep %s %d err=%d\n", __func__, __LINE__, err );

	return 0;
}

static void __exit pnotify_modexit(void)
{
	printk(KERN_INFO "Goodbye world 1.\n");
	misc_deregister(&pnot_miscdev);
}

module_init(pnotify_modinit);
module_exit(pnotify_modexit);

