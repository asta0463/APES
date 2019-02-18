/**
* @file - linkedListModule.c
* @brief - Implementation of a linked list module
* 
* @author - Ashish Tak
* @date 02/17/2019
**/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>

#define SEED_SIZE 50

//Command-line parameters
char * type;
uint count;

char * seedArray[SEED_SIZE]= {"fox", "dog", "cat", "rat", "cat", "tiger", "lion", "lion", "dog", "dog", \
	"chicken", "giraffe", "boar", "chicken", "eagle", "rat", "chicken", "boar", "boar", "giraffe", \
		"cat", "cat", "crow", "dog", "eagle", "eagle", "boar", "cow", "cow", "cat", \
		"crow", "cow", "chicken", "dog", "rat", "tiger", "lion", "lion", "tiger", "cow",\
	       "pigeon", "rat", "giraffe", "cow", "crow", "cow", "lion", "dog", "crow", "eagle"};	

struct animal {
	char * species;
	unsigned int count;
	struct list_head list;
};

//Declaration and Initialization of the head to the Linked List
LIST_HEAD(animal_list);

struct animal *ptr;
struct animal *currentAnimal;


/*
 * @brief - Function to print out the linked list based on the command line filters.
 */
void checkFilters(void) {
	uint mem_size=0, no_of_nodes=0;

	printk("\n\nWithout any filter:\n");
        list_for_each_entry(ptr, &animal_list, list) {
                printk("%s, count: %d", ptr->species, ptr->count);
        }

	printk("\n\nWith the type/species filter:");
        list_for_each_entry(ptr, &animal_list, list) {
		if(strcmp(ptr->species,type)==0) {
	                printk("%s, count: %d", ptr->species, ptr->count);
	                ++no_of_nodes;
	                mem_size+=sizeof(struct animal);
			break;
		}
        }
        printk("\n\nTotal no. of nodes: %d", no_of_nodes);
        printk("\nSize in memory: %d bytes",mem_size);

	mem_size=0; no_of_nodes=0;
	printk("\n\nWith the count filter:");
        list_for_each_entry(ptr, &animal_list, list) {
                if(ptr->count>=count) {
                        printk("%s, count: %d", ptr->species, ptr->count);
                        ++no_of_nodes;
                        mem_size+=sizeof(struct animal);
                }
        }
        printk("\n\nTotal no. of nodes: %d", no_of_nodes);
        printk("\nSize in memory: %d bytes",mem_size);

	mem_size=0; no_of_nodes=0;
	printk("\n\nWith both filters:");
        list_for_each_entry(ptr, &animal_list, list) {
                if(strcmp(ptr->species,type)==0 && ptr->count>=count) {
                        printk("%s, count: %d", ptr->species, ptr->count);
                        ++no_of_nodes;
			mem_size+=sizeof(struct animal);
                        break;
                }
        }
        printk("\n\nTotal no. of nodes: %d", no_of_nodes);
        printk("\nSize in memory: %d bytes",mem_size);

}


/*
 * @brief - Initialization function. Called when the module is loaded
 */
static int listInitialize(void) {
        printk("Initializing the linked list\n");

	//Insert the seed elements into the Linked List and perform insertion sort in parallel
        unsigned int i, flag=0;
        for (i=0; i<SEED_SIZE; i++) {
                flag=0;
                list_for_each_entry(ptr, &animal_list, list) {
                        if(!strcmp(ptr->species,seedArray[i])) {
                                ptr->count=ptr->count+1;
                                flag=1;
                                break;
                        }
			else if (strcmp(ptr->species,seedArray[i])>0) {
                                currentAnimal= kmalloc(sizeof(struct animal), GFP_KERNEL);
                                currentAnimal->species= seedArray[i];
                                currentAnimal->count=1;
                                list_add_tail(&currentAnimal->list, &ptr->list);
				if (&animal_list==&ptr->list)
					animal_list=currentAnimal->list;
				flag=1;
				break;
			}
                }
                if(!flag) {
                                currentAnimal= kmalloc(sizeof(struct animal), GFP_KERNEL);
                                currentAnimal->species= seedArray[i];
                                currentAnimal->count=1;
                                list_add_tail(&currentAnimal->list, &animal_list);
		}
        }

	uint mem_size=0, no_of_nodes=0;
        printk("Final Linked List, after removal of duplicate occurrences:");
        list_for_each_entry(ptr, &animal_list, list) {
		++no_of_nodes;
		mem_size+=sizeof(struct animal);
                printk("%s, count: %d", ptr->species, ptr->count);
        }
	printk("\n\nTotal no. of nodes: %d", no_of_nodes);
	printk("\nSize in memory: %d bytes",mem_size);

	checkFilters();

        return 0;
}


/*
 * @brief - Exit function. Called when the module is removed
 */ 
static void listExit(void) {
	printk("Linked List Module Exits\n");
	uint mem_size=0;
	list_for_each_entry(ptr, &animal_list, list) {
		if (ptr!=NULL) {
			kfree(ptr);
		}
                mem_size+=sizeof(struct animal);
        }
	printk("\nAmount of memory freed: %d", mem_size);

}

//Add the runtime module parameters
module_param(type, charp, 0644);
module_param(count, uint, 0644);

//Register the module's entry and exit points
module_init(listInitialize);
module_exit(listExit);

MODULE_DESCRIPTION("Linked List module");
MODULE_AUTHOR("Ashish Tak");
MODULE_LICENSE("GPL");
