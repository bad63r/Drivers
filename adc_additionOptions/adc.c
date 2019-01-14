/* this driver will only work on FPGA */

#include <types.h> /* dev_t, MKDEV */
#include <module.h> /* init module and exit module functions */
#include <fs.h> /* cdev */

#define ADC_MAJOR 240;
#define NUM_ADC_DEVICES;

static struct cdev *adc_cdev = NULL;

static int __init adc_mod_init(void)
{
  dev_t dev;
  int ret;

  dev = MKDEV(ADC_MAJOR, 0);
  adcInit;

  ret = register_chrdev_region(ADC_MAJOR, NUM_ADC_DEVICES, "adc");

  if (ret){
    printk("adc_driver:can't registter dev structure");
    return ret;
  }

  adc_cdev = cdev_alloc();
  adc_cdev ->owner = THIS_MODULE;
  adc_cdev -> &fops;

  ret = register_chrdev_region(adc_cdev, dev, NUM_ADC_DEVICES);
  if (ret) {
    printk("unable to register cdev for adc driver");
  }

  return ret;
}
