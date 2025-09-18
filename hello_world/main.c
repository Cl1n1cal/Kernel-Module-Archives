#include <linux/module.h>
#include <linux/init.h>



int hello_start(void)
{
    printk(KERN_INFO "Hello world!\n");
    return 0;
}

void hello_end(void)
{
    printk(KERN_INFO "Hello module unloaded\n");
}

module_init(hello_start);
module_exit(hello_end);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cl1nical");
MODULE_DESCRIPTION("Hello World Kernel Module");
MODULE_VERSION("0.1");