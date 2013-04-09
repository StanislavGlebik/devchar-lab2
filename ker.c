#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/moduleparam.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define BUFFER_MAX_SIZE 100
#define SUCCESS 0

#define PLUS 0
#define MINUS 1
#define MULTIPLY 2
#define DIVIDE 3

MODULE_AUTHOR("Glebik-Kolesov");
MODULE_LICENSE("GPL");

int result_proc;
int first_op_proc;
int second_op_proc;
int oper_proc;

char buff[BUFFER_MAX_SIZE];

static char * result = "result_device";
module_param(result, charp, 0);

static char * first_operand = "operand1_device";
module_param(first_operand, charp, 0);

static char * second_operand = "operand2_device";
module_param(second_operand, charp, 0);

static char * operation = "operation_device";
module_param(operation, charp, 0);

static int oper_type = PLUS;
static char first_oper[BUFFER_MAX_SIZE];
static char second_oper [BUFFER_MAX_SIZE];


static int atoi(char * a) 
{
	int res = 0;
	int i = 0;

	for (i = 0; a[i] != 0; ++i) {
		if (a[i] >= '0' && a[i] <= '9') {
			res = res * 10 + a[i] - '0';
		}
	}
	return res;
}

static int
 read_proc(struct seq_file *m, void *v) {
 	switch(oper_type) {
 		case PLUS:
 		seq_printf(m, "%d\n", atoi(first_oper) + atoi(second_oper));
 		break;
 		case MINUS:
 		seq_printf(m, "%d\n", atoi(first_oper) - atoi(second_oper));
 		break;
 		case MULTIPLY:
 		seq_printf(m, "%d\n", atoi(first_oper) * atoi(second_oper));
 		break;
 		case DIVIDE:
 		if (atoi(second_oper) == 0) {
 			seq_printf(m, "Disivion by zero!\n");
 		}
 		else {
 			seq_printf(m, "%d\n", atoi(first_oper) / atoi(second_oper));
 		}
 		break;
 	}

	return 0;
}

static int
jif_open(struct inode *inode, struct file *file) 
{
	return single_open(file, read_proc, NULL);
}

static const struct file_operations proc_file_fops = {
	.owner = THIS_MODULE,
	.open = jif_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static ssize_t 
procfs_read_oper(struct file *filp,
			char *buffer,
			size_t length,
			loff_t *offset)
{
	static int finished = 0;
	char sign;

	if (finished) {
		finished = 0;
		return 0;
	}

	finished = 1;

	switch(oper_type) {
		case PLUS:
			sign = '+';
			break;
		case MINUS:
			sign = '-';
			break;
		case DIVIDE:
			sign = '/';
			break;
		default:
			sign = '*';
			break;
	}

	if (copy_to_user(buffer, &sign, 1)) {
		return -EFAULT;
	}

	return 1;
}

static ssize_t 
procfs_read_second(struct file *filp,
			char *buffer,
			size_t length,
			loff_t *offset)
{
	static int finished = 0;

	if (finished) {
		finished = 0;
		return 0;
	}

	finished = 1;
	if (copy_to_user(buffer, second_oper, strlen(second_oper))) {
		return -EFAULT;
	}

	return strlen(second_oper);
}

static ssize_t 
procfs_read_first(struct file *filp,
			char *buffer,
			size_t length,
			loff_t *offset)
{
	static int finished = 0;

	if (finished) {
		finished = 0;
		return 0;
	}

	finished = 1;
	if (copy_to_user(buffer, first_oper, strlen(first_oper))) {
		return -EFAULT;
	}

	return strlen(first_oper);
}

static ssize_t 
operation_write(struct file *file, const char * buffer, size_t count,
					loff_t *data)
{
	if (count > BUFFER_MAX_SIZE)
	{
		count = BUFFER_MAX_SIZE-1;
	}
	
	if (copy_from_user(buff, buffer, count)) {
			return -EFAULT;
	}

	if (count > 0) {
		switch (buff[0]) {
			case '+':
			oper_type = PLUS;
			break;
			case '-':
			oper_type = MINUS;
			break;
			case '*':
			oper_type = MULTIPLY;
			break;
			case '/':
			oper_type = DIVIDE;
			break;
		}
	}
	else {
		oper_type = PLUS;
	}
	return count;
}

static ssize_t second_operand_write(struct file *file, const char * buffer, size_t count,
					loff_t *data)
{

	if (count >= BUFFER_MAX_SIZE)
	{
		count = BUFFER_MAX_SIZE-1;
	}

	if (copy_from_user(second_oper, buffer, count)) {
			return -EFAULT;
	}

	return count;
}
static ssize_t first_operand_write(struct file *file, const char * buffer, size_t count,
					loff_t *data)
{

	if (count >= BUFFER_MAX_SIZE)
	{
		count = BUFFER_MAX_SIZE-1;
	}

	if (copy_from_user(first_oper, buffer, count)) {
			return -EFAULT;
	}

	return count;
}


static int 
procfs_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static int procfs_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return SUCCESS;
}

static const struct file_operations operation_write_fops = {
	.owner = THIS_MODULE,
	.open = procfs_open,
	.release = procfs_release,
	.write = operation_write,
	.read = procfs_read_oper,
};

static const struct file_operations first_operand_write_fops = {
	.owner = THIS_MODULE,
	.open = procfs_open,
	.release = procfs_release,
	.write = first_operand_write,
	.read = procfs_read_first,
};

static const struct file_operations second_operand_write_fops = {
	.owner = THIS_MODULE,
	.open = procfs_open,
	.release = procfs_release,
	.write = second_operand_write,
	.read = procfs_read_second,
};

static int __init test_init( void )
{
	printk( KERN_ALERT "ker module loaded\n");
	result_proc = register_chrdev(0, result, &proc_file_fops);
	first_op_proc = register_chrdev(0, first_operand, &first_operand_write_fops);
	second_op_proc = register_chrdev(0, second_operand, &second_operand_write_fops);
	oper_proc = register_chrdev(0, operation, &operation_write_fops);

	strcpy(first_oper, "0");
	strcpy(second_oper, "0");

    printk(KERN_INFO "result_proc = %d", result_proc);
    printk(KERN_INFO "first_op_proc = %d", first_op_proc);
    printk(KERN_INFO "second_op_proc = %d", second_op_proc);
    printk(KERN_INFO "oper_proc =%d", oper_proc);

    /*
	if (result_proc == NULL || first_op_proc == NULL
		|| second_op_proc == NULL || oper_proc == NULL) {
		printk(KERN_INFO "Error: Could not initialize /proc/ker\n");
		return -ENOMEM;
	} else {
		printk(KERN_INFO "Success!\n");
	}
    */

	return 0;
}

static void __exit test_exit( void )
{
	unregister_chrdev(result_proc, result);
	unregister_chrdev(first_op_proc, first_operand);	
	unregister_chrdev(second_op_proc, second_operand);		
	unregister_chrdev(oper_proc, operation);		

	printk(KERN_ALERT "ker module is unloaded\n");
}

module_init( test_init );
module_exit( test_exit );
