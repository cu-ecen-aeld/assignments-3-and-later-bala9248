/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include <linux/slab.h> 
#include "aesdchar.h"

int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Balapranesh Elango");
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
	struct aesd_dev *dev;
	PDEBUG("open");
	dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
	filp->private_data = dev;
	return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
	PDEBUG("release");	
	return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
	ssize_t retval = 0;
	struct aesd_dev *dev = filp->private_data;
	struct aesd_buffer_entry *cbfifo = NULL;
	size_t offset_byte = 0, read_bytes = 0;
	int rc ;
	
	PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
	
	rc = mutex_lock_interruptible(&dev->lock);
	if(rc) 
		return -ERESTARTSYS;
	
	//find entry and offset for current f_pos
	cbfifo = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->cbfifo, *f_pos, &offset_byte);
	if(cbfifo == NULL)
		goto clean;
	
	//Check if bytes requested to be read out is lesser than available bytes for reading from offset
	if( (cbfifo->size - offset_byte) > count) 
		read_bytes = count;
	else 
		read_bytes = cbfifo->size - offset_byte;
	
	//read
	rc = copy_to_user(buf, cbfifo->buffptr+offset_byte, read_bytes);
	
	if (rc != 0) {
		retval = -EFAULT;
		goto clean;
	}
	
	//return the read bytes and update f_pos
	retval = read_bytes;
	*f_pos += read_bytes;
	

clean:	
	mutex_unlock(&dev->lock);
	return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	const char *new = NULL;
	ssize_t retval = -ENOMEM;
	size_t rem_bytes = 0;
	int rc ;
	struct aesd_dev *dev = filp->private_data;
	PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
	
	rc = mutex_lock_interruptible(&dev->lock);
	if(rc) 
		return -ERESTARTSYS;
	
	//If size is 0, allocate size = count
	if(!dev->buff_entry.size) {
		dev->buff_entry.buffptr = kmalloc(count*sizeof(char), GFP_KERNEL);
		
		if( dev->buff_entry.buffptr == NULL) 
			goto clean;
	}
	
	else { //previously written and \n was not detected, so increase size
		dev->buff_entry.buffptr = krealloc(dev->buff_entry.buffptr, (dev->buff_entry.size + count), GFP_KERNEL);
		if( dev->buff_entry.buffptr == NULL) 
			goto clean;
	}
	
	//copy from user space buffer 
	rem_bytes = copy_from_user((void *)&dev->buff_entry.buffptr[dev->buff_entry.size], buf, count);
	

	retval = count - rem_bytes; //rem bytes is subtracted in case of partial write
	dev->buff_entry.size += retval;
	
	//Check for \n
	if( strchr(dev->buff_entry.buffptr, '\n') != NULL ) {
		new = aesd_circular_buffer_add_entry(&dev->cbfifo, &dev->buff_entry);
	
		if (new) 
			kfree(new);
		
		//Data has been written to the circular buffer
		dev->buff_entry.buffptr = NULL;
		dev->buff_entry.size = 0;
	}
	
	 
clean:	

	mutex_unlock(&dev->lock);
	return retval;
}

struct file_operations aesd_fops = {
	.owner =    THIS_MODULE,
	.read =     aesd_read,
	.write =    aesd_write,
	.open =     aesd_open,
	.release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
	int err, devno = MKDEV(aesd_major, aesd_minor);

	cdev_init(&dev->cdev, &aesd_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &aesd_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	if (err) {
		printk(KERN_ERR "Error %d adding aesd cdev", err);
	}
	return err;
}


int aesd_init_module(void)
{
	dev_t dev = 0;
	int result;
	result = alloc_chrdev_region(&dev, aesd_minor, 1,
			"aesdchar");
	aesd_major = MAJOR(dev);
	if (result < 0) {
		printk(KERN_WARNING "Can't get major %d\n", aesd_major);
		return result;
	}
	memset(&aesd_device,0,sizeof(struct aesd_dev));

	//Init cbfifo and mutex
	mutex_init(&aesd_device.lock);
	aesd_circular_buffer_init(&aesd_device.cbfifo);
	result = aesd_setup_cdev(&aesd_device);

	if( result ) {
		unregister_chrdev_region(dev, 1);
	}
	return result;

}

void aesd_cleanup_module(void)
{
	dev_t devno = MKDEV(aesd_major, aesd_minor);

	cdev_del(&aesd_device.cdev);
        //deallocate cbfifo
	cbfifo_cleanup(&aesd_device.cbfifo);
	unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
