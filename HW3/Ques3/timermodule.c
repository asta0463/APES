/**
* @file - timermodule.c
* @brief - Implementation of a kernel module utilizing the kernel timer
* 
* @author - Ashish Tak
* @date 02/16/2019
**/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/timer.h>

#define MAX_ITERATIONS 10

struct timer_list my_timer;
char * Username="Tak";
uint period=500;	//Kernel timer period in ms


/*
 * @brief - Timer Callback function
 */
static void print_iterations (unsigned long data) {
	static int count=0;
	if (count<MAX_ITERATIONS) {
		printk("User name: %s", Username);
		printk("Iteration # %d", ++count);
		//Reactivate the timer and add the expiration period to the current tick count (jiffies)
		mod_timer(&my_timer, jiffies + msecs_to_jiffies(period));
	}
	else {
		flush_printk_to_syslog();
		del_timer(&my_timer);
	}
}


/*
 * @brief - Initialization function. Called when the module is loaded
 */ 
static int timer_initialize(void) {
	printk("Timer Module Init\n");
	setup_timer(&my_timer, print_iterations, 0);
	my_timer.expires= jiffies +  msecs_to_jiffies(period);
	//my_timer.data = 0;
	//my_timer.function = print_iterations;
	add_timer(&my_timer);
	return 0;
}

/*
 * @brief - Exit function. Called when the module is removed
 */ 
static void timer_exit(void) {
	printk("Timer Module Exit\n");
}

//Add the runtime module parameters
module_param(Username, charp, 0644);
module_param(period, uint, 0644);

//Register the module's entry and exit points
module_init(timer_initialize);
module_exit(timer_exit);

MODULE_DESCRIPTION("Basic Timer module");
MODULE_AUTHOR("Ashish Tak");
MODULE_LICENSE("GPL");
