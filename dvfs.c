#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/clk.h>

#include <mach/dvfs.h>

#define ATTR_RO(_name)	\
	static struct kobj_attribute _name##_interface = __ATTR(_name, 0444, _name##_show, NULL);

#define ATTR_RW(_name)	\
	static struct kobj_attribute _name##_interface = __ATTR(_name, 0644, _name##_show, _name##_store);

static ssize_t cpu_table_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct cpufreq_frequency_table *table = dvfs_get_freq_volt_table(clk_get(NULL, "cpu"));
	int count = 0;
	int i;
	for (i=0; table[i].frequency != CPUFREQ_TABLE_END; i++) {
		count += sprintf(buf+count, "%d: freq: %u MHz, volt: %u mV\n", i, table[i].frequency/1000,table[i].index/1000);
	}
	return count;
}

static ssize_t cpu_table_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	struct cpufreq_frequency_table *table = dvfs_get_freq_volt_table(clk_get(NULL, "cpu"));
	int ret;
	int i, index;
	unsigned int v;

	ret = sscanf(buf, "%d", &index);
	if (!ret) return -EINVAL;

	for (i=0; table[i].frequency != CPUFREQ_TABLE_END; i++)
		if (i == index) break;
	if (table[i].frequency == CPUFREQ_TABLE_END) return -EINVAL;

	buf+=ret+1;

	ret = sscanf(buf, "%u", &v);
	if (!ret || v<800 || v>1450) return -EINVAL;
	table[i].index = (v/25)*25*1000;

	return count;
}

ATTR_RW(cpu_table);

static struct attribute *dvfs_attrs[] = {
	&cpu_table_interface.attr,
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
