#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static struct proc_dir_entry *char_proc_file_debug = NULL;
static struct proc_dir_entry *char_proc_dir        = NULL;

static int debug_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "char_debug = %d\n", char_debug);
	return 0;
}

static int debug_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, debug_proc_show, NULL);
}

static ssize_t debug_proc_write(struct file *file, const char __user *buffer,
				    size_t count, loff_t *pos)
{
	char data8;

	if (count > 0) {
		if (get_user(data8, buffer))
			return -EFAULT;
		if (data8 >= '0' && data8 <= '9')
			char_debug = data8 - '0';
	}
	return count;
}

static const struct file_operations debug_fops = {
	.open		= debug_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.write		= debug_proc_write,
	.release	= single_release,
};

static int char_proc_init(void)
{
	char_proc_dir = proc_mkdir(CHAR_NAME, NULL);
	if (!char_proc_dir)
		return -ENOMEM;

	char_proc_file_debug = proc_create("debug", S_IWUGO | S_IRUGO, char_proc_dir,
			  &debug_fops);
	if (!char_proc_file_debug)
		return -ENOMEM;

    return 0;
}

static void cleanup_procfs(void)
{
    remove_proc_entry("debug", char_proc_dir);
    remove_proc_entry(CHAR_NAME, NULL);
    printk(KERN_INFO "%s %s Procfs removed\n",
                CHAR_NAME, CHAR_VER);
}

