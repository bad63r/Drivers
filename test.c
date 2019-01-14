#include <module.h>
#include <types.h>
#include <fs.h>

#define ADC_MAJOR 240

static struct cdev *adc_cdev = NULL;

static struct file_operations adc_fops = {
  .owner = THIS_MODULE,
  .open = adc_open,
  .release = adc_close,
  .read = adc_read,
};


static int __exit adc_mod_exit(void)
{
  dev_t dev;
  int ret;

  dev = MKDEV(ADC_MAJOR, 0 );
  cdev_del(adc_cdev);

  unregister_chrdev_region(dev, NUM_ADC_DEVICES);
}

static int __init adc_mod_init(void)
{
  dev_t dev;
  int ret;

  dev = MKDEV(ADC_MAJOR, 0);
  ret = register_chrdev_region(dev, NUM_ADC_DEVICES, "adc");

  if (ret) {
    printk("adc: can't register dev structure");
    return ret;
  }

  adc_cdev = cdev_alloc();
  adc_cdev->owner = THIS_MODULE;
  adc_cdev->fops = &adc_fops;

  ret = cdev_add(adc_cdev);
  if (ret) {
    printk("can't register cdev structure");
    return ret;
  }

  return ret;

}
