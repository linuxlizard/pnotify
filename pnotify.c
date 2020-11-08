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
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/file.h>
#include <linux/namei.h>
#include <linux/fsnotify.h>

MODULE_AUTHOR("David Poole <davep@mbuf.com>");
MODULE_DESCRIPTION("Trigger inotify event from userspace.");
MODULE_LICENSE("GPL");

#define PNOTIFY_MODULE_NAME		"pnotify"

static int pnotify_dev_open(struct inode *inode, struct file *filp)
{
	/* enforce read-only access to this chrdev */
	if ((filp->f_mode & FMODE_READ) == 0)
		return -EINVAL;
	if (filp->f_mode & FMODE_WRITE)
		return -EINVAL;
	printk( KERN_NOTICE "davep %s %d\n", __func__, __LINE__ );
	return 0;
}

static ssize_t pnotify_dev_read(struct file *filp, char __user *buf,
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

static const struct file_operations pnotify_chrdev_ops = {
	.owner		= THIS_MODULE,
	.open		= pnotify_dev_open,
	.read		= pnotify_dev_read,
	.llseek		= noop_llseek,
};

static const struct attribute_group *pnotify_dev_groups[];

static struct miscdevice pnot_miscdev = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= PNOTIFY_MODULE_NAME,
	.nodename	= "pnotify",
	.fops		= &pnotify_chrdev_ops,
	.groups		= pnotify_dev_groups,
};

static ssize_t pnotify_attr_trigger_notify(struct device *dev,
					struct device_attribute *attr,
					const char *filename, size_t len)
{
	int err;
	struct path path;

	err = 0;

	err = kern_path(filename, LOOKUP_FOLLOW, &path);
	if (err) {
		return err;
	}

	fsnotify_change(path.dentry, ATTR_MTIME);

	path_put(&path);

	return err ? : len;
}

static ssize_t pnotify_attr_current_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	ssize_t ret=-1;

	ret = snprintf(buf, PAGE_SIZE, "%s/%s\n", __func__, "hello, world");

	return ret;
}

static DEVICE_ATTR(trigger_notify, S_IRUGO | S_IWUSR,
		   pnotify_attr_current_show,
		   pnotify_attr_trigger_notify);

static struct attribute *pnotify_dev_attrs[] = {
	&dev_attr_trigger_notify.attr,
	NULL
};

ATTRIBUTE_GROUPS(pnotify_dev);

static int __init pnotify_modinit(void)
{
	int err;

	err = misc_register(&pnot_miscdev);

	return err;
}

static void __exit pnotify_modexit(void)
{
	misc_deregister(&pnot_miscdev);
}

module_init(pnotify_modinit);
module_exit(pnotify_modexit);

