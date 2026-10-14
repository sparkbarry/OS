#include <linux/init.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/ktime.h>

MODULE_LICENSE("Dual BSD/GPL");

static unsigned long start_jiffies;
static ktime_t start_ktime;

static int hello_init(void){
    start_jiffies = jiffies;
    start_ktime = ktime_get_boottime();

    printk(KERN_ALERT "hello %d\n", 1000/HZ);
    return 0;
}
static void hello_exit(void){
    unsigned long end_jiffies=jiffies;
    ktime_t end_ktime=ktime_get_boottime();
    
    printk(KERN_ALERT "goodbye jiffies %lu\n", jiffies_to_msecs(end_jiffies-start_jiffies));
    printk(KERN_ALERT "goodbye ktime %lld\n", ktime_to_ms(ktime_sub(end_ktime, start_ktime)));
}
module_init(hello_init);
module_exit(hello_exit);