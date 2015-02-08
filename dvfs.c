#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/clk.h>

#include <mach/dvfs.h>

#define ATTR_RO(_name)	\
	static struct kobj_attribute _name##_interface = __ATTR(_name, 0444, _name##_show, NULL);

#define ATTR_RW(_name)	\
	static struct kobj_attribute _name##_interface = __ATTR(_name, 0644, _name##_show, _name##_store);

static ssize_t dvfs_table_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf, struct cpufreq_frequency_table *table)
{
	int count = 0;
	int i;
	if (!table) return 0;
	for (i=0; table[i].frequency != CPUFREQ_TABLE_END; i++) {
		count += sprintf(buf+count, "%u MHz: %u mV\n", table[i].frequency/1000,table[i].index/1000);
	}
	return count;
}

inline unsigned int round_v_up(v, r) {
	unsigned int d = v % r;
	if (d) return v + (r-d);
	return v;
}

static ssize_t dvfs_table_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count, struct cpufreq_frequency_table *table)
{
	int ret;
	int i, index;
	unsigned int v;

	if (!table) return -EINVAL;
	ret = sscanf(buf, "%d %u", &index, &v);
	if (!ret) {
		printk(KERN_ERR "invalid input %s", buf);
		return -EINVAL;
	}

	for (i=0; table[i].frequency != CPUFREQ_TABLE_END; i++)
		if (table[i].frequency/1000 == index) break;

	if (table[i].frequency == CPUFREQ_TABLE_END) {
		printk(KERN_ERR "invalid frequency %d", index);
		return -EINVAL;
	}

	if (v<600 || v>1500) {
		printk(KERN_ERR "voltage out of range: %u", v);
		return -EINVAL;
	}

	if (v<1200)
		v = round_v_up(25)
	else if (v<2400)
		v = round_v_up(50)
	else if (v<=3900)
		v = round_v_up(100)
	else
		return -EINVAL;
	v *= 1000;

	table[i].index = v;

	return count;
}

#define DVFS_TABLE_SHOW(_name) static ssize_t _name##_table_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) { \
	return dvfs_table_show(kobj, attr, buf, dvfs_get_freq_volt_table(clk_get(NULL, #_name))); \
}

#define DVFS_TABLE_STORE(_name) static ssize_t _name##_table_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) { \
	return dvfs_table_store(kobj, attr, buf, count, dvfs_get_freq_volt_table(clk_get(NULL, #_name))); \
}

DVFS_TABLE_SHOW(cpu);
DVFS_TABLE_STORE(cpu);
ATTR_RW(cpu_table);

DVFS_TABLE_SHOW(gpu);
DVFS_TABLE_STORE(gpu);
ATTR_RW(gpu_table);

DVFS_TABLE_SHOW(ddr);
DVFS_TABLE_STORE(ddr);
ATTR_RW(ddr_table);

static struct attribute *dvfs_attrs[] = {
	&cpu_table_interface.attr,
	&gpu_table_interface.attr,
	&ddr_table_interface.attr,
	NULL,
};

static struct attribute_group dvfs_interface_group = {
	.attrs = dvfs_attrs,
};

static struct kobject *dvfs_kobject;

static int __init dvfs_init(void)
{
	dvfs_kobject = kobject_create_and_add("dvfs", kernel_kobj);
	sysfs_create_group(dvfs_kobject, &dvfs_interface_group);
	return 0;
}
 
static void __exit dvfs_exit(void)
{
	sysfs_remove_group(dvfs_kobject, &dvfs_interface_group);
	kobject_put(dvfs_kobject);
}
 
module_init(dvfs_init);
module_exit(dvfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marcin Kaluza <marcin.kaluza@trioptimum.com>");
MODULE_DESCRIPTION("Test module");
