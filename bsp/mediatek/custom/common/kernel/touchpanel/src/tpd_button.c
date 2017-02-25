#include "tpd.h"

extern struct tpd_device *tpd;

//#ifdef TPD_HAVE_BUTTON
//static int tpd_keys[TPD_KEY_COUNT] = TPD_KEYS;
//static int tpd_keys_dim[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;

/*lenovo-sw xuwen1 add 2014.01.06 for tp suspend status setting begin */ 
#if defined(LENOVO_CTP_SLIDE_WAKEUP)
static tpd_suspend_status_flag = 0;
#endif

/*lenovo-sw xuwen1 add 2014.01.06 for tp suspend status setting end*/ 

/*lenovo-sw lixh10 add 2014.04.14 for glove_func start*/ 
#ifdef LENOVO_CTP_GLOVE_CONTROL
int set_glove_func(bool flag);
int get_glove_func(void);
void set_tpd_glove_pre_status(int mode);
#endif

/*lenovo-sw lixh10 add 2014.04.14 for glove_func end*/ 

static unsigned int tpd_keycnt = 0;
static int tpd_keys[TPD_VIRTUAL_KEY_MAX]={0};
static int tpd_keys_dim[TPD_VIRTUAL_KEY_MAX][4];// = {0};
static ssize_t mtk_virtual_keys_show(struct kobject *kobj,
                   struct kobj_attribute *attr, char *buf) {
    int i, j;
    for(i=0, j=0;i<tpd_keycnt;i++)
        j+=sprintf(buf, "%s%s:%d:%d:%d:%d:%d%s",buf,
           __stringify(EV_KEY),tpd_keys[i],
           tpd_keys_dim[i][0],tpd_keys_dim[i][1],
           tpd_keys_dim[i][2],tpd_keys_dim[i][3],
           (i==tpd_keycnt-1?"\n":":"));
    return j;
}

static struct kobj_attribute mtk_virtual_keys_attr = {
    .attr = {
        .name = "virtualkeys.mtk-tpd",
        .mode = S_IRUGO,
    },
    .show = &mtk_virtual_keys_show,
};

/*Begin lenovo-sw xuwen1 add for tp suspend status setting 2014-1-6*/ 
#if defined(LENOVO_CTP_SLIDE_WAKEUP)
void set_tpd_suspend_status(int mode)
{
	tpd_suspend_status_flag = mode;
}
EXPORT_SYMBOL(set_tpd_suspend_status);

int get_tpd_suspend_status(void)
{
	return tpd_suspend_status_flag;
}
EXPORT_SYMBOL(get_tpd_suspend_status);

static ssize_t lenovo_tpd_suspend_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf,"%d.\n", get_tpd_suspend_status());
}

static ssize_t lenovo_tpd_suspend_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t size)
{
	int mode;
	int res = sscanf(buf, "%d", &mode);
	if (res != 1)
	{
		printk("%s: [wj]expect 1 numbers\n", __FUNCTION__);
	}
	else
	{
		set_tpd_suspend_status(mode);
	}
}

static struct kobj_attribute lenovo_tpd_suspend_attr = {
    .attr = {
        .name = "tpd_suspend_status",
        .mode = S_IRUGO | S_IWUSR,
    },
    .show = &lenovo_tpd_suspend_show,
	.store = &lenovo_tpd_suspend_store,
};
#endif

/*End lenovo-sw xuwen1 add for tp suspend status setting 2014-1-6*/ 

/*lenovo-sw lixh10 add 2014.04.14 for glove_func start*/ 
#ifdef LENOVO_CTP_GLOVE_CONTROL
void set_tpd_glove_status(int mode)
{
    if(mode)
    {
        set_tpd_glove_pre_status(1);
    }
    else
    {
        set_tpd_glove_pre_status(0);
    }
    set_glove_func(mode);
}
EXPORT_SYMBOL(set_tpd_glove_status);

int get_tpd_glove_status(void)
{
	return get_glove_func();
}
EXPORT_SYMBOL(get_tpd_glove_status);

static ssize_t lenovo_tpd_glove_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf,"%d.\n", get_tpd_glove_status());
}

static ssize_t lenovo_tpd_glove_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t size)
{
	int mode;
	int res = sscanf(buf, "%d", &mode);
	if (res != 1)
	{
		printk("%s: [wj]expect 1 numbers\n", __FUNCTION__);
	}
	else
	{
		set_tpd_glove_status(mode);
	}
}

static struct kobj_attribute lenovo_tpd_glove_attr = {
    .attr = {
        .name = "tpd_glove_status",
        .mode = S_IRUGO | S_IWUSR,
    },
    .show = &lenovo_tpd_glove_show,
	.store = &lenovo_tpd_glove_store,
};
#endif
/*lenovo-sw lixh10 add 2014.04.14 for glove_func end*/ 

static ssize_t lenovo_tpd_info_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{

    if(!have_correct_setting)
        return sprintf(buf,"[wj]Have not right setting.\n");
    else
        return sprintf(buf,"name=%s; types=%d;fw_num=0x%08x;\n", tpd_info_t->name, tpd_info_t->types, tpd_info_t->fw_num);
}

static struct kobj_attribute lenovo_tpd_info_attr = {
    .attr = {
        .name = "lenovo_tpd_info",
        .mode = S_IRUGO,
    },
    .show = &lenovo_tpd_info_show,
};

/*lenovo-sw xuwen1 add 20140127 for area-touch-limit begin*/
#if NEED_TO_CREAT_LIMIT
static ssize_t lenovo_tpd_area_limit_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
if(!ready_to_use)
        return sprintf(buf,"[xw]Have not set yet,need to set.\n");
    else
        return sprintf(buf,"min_limit=%d;max_limit=%d;\n", tpd_limit->min_limit, tpd_limit->max_limit);
}

static struct kobj_attribute lenovo_tpd_area_limit_attr = {
         .attr = {
        .name = "lenovo_tpd_area_limit",
        .mode = S_IRUGO,
    },
    .show = &lenovo_tpd_area_limit_show,
};
#endif
/*lenovo-sw xuwen1 add 20140127 for area-touch-limit end*/

static struct attribute *mtk_properties_attrs[] = {
    &mtk_virtual_keys_attr.attr,
/*lenovo-sw xuwen1 add 2014-1-6 for tp suspend status setting begin*/ 
#if defined(LENOVO_CTP_SLIDE_WAKEUP)
	&lenovo_tpd_suspend_attr,
#endif
    &lenovo_tpd_info_attr,
/*lenovo-sw lixh10 add 2014.04.14 for glove_func start*/ 
#ifdef LENOVO_CTP_GLOVE_CONTROL
    &lenovo_tpd_glove_attr,
#endif
/*lenovo-sw lixh10 add 2014.04.14 for glove_func end*/ 	
/*lenovo-sw xuwen1 add 20140127 for area-touch-limit begin*/
 #if NEED_TO_CREAT_LIMIT
	&lenovo_tpd_area_limit_attr, 
 #endif
/*lenovo-sw xuwen1 add 20140127 for area-touch-limit end*/
    NULL
};

static struct attribute_group mtk_properties_attr_group = {
    .attrs = mtk_properties_attrs,
};

struct kobject *properties_kobj;

void tpd_button_init(void) {
    int ret = 0, i = 0;
#if 0
	for(i=0; i<TPD_VIRTUAL_KEY_MAX; i++)
	{
		for(j=0; j<4; j++)
		{
			tpd_keys_dim[i][j]=0;
		}
	}
#endif	
//    if((tpd->kpd=input_allocate_device())==NULL) return -ENOMEM;
    tpd->kpd=input_allocate_device();
    /* struct input_dev kpd initialization and registration */
    tpd->kpd->name = TPD_DEVICE "-kpd";
    set_bit(EV_KEY, tpd->kpd->evbit);
    //set_bit(EV_REL, tpd->kpd->evbit);
    //set_bit(EV_ABS, tpd->kpd->evbit);
    for(i=0;i<tpd_keycnt;i++)
        __set_bit(tpd_keys[i], tpd->kpd->keybit);
    tpd->kpd->id.bustype = BUS_HOST;
    tpd->kpd->id.vendor  = 0x0001;
    tpd->kpd->id.product = 0x0001;
    tpd->kpd->id.version = 0x0100;
    if(input_register_device(tpd->kpd))
        TPD_DMESG("input_register_device failed.(kpd)\n");
    set_bit(EV_KEY, tpd->dev->evbit);
    for(i=0;i<tpd_keycnt;i++)
        __set_bit(tpd_keys[i], tpd->dev->keybit);
    properties_kobj = kobject_create_and_add("board_properties", NULL);
    if(properties_kobj)
        ret = sysfs_create_group(properties_kobj,&mtk_properties_attr_group);
    if(!properties_kobj || ret)
    printk("failed to create board_properties\n");
}

void tpd_button(unsigned int x, unsigned int y, unsigned int down) {
    int i;
    if(down) {
        for(i=0;i<tpd_keycnt;i++) 
		{
            if(x>=tpd_keys_dim[i][0]-(tpd_keys_dim[i][2]/2) &&
               x<=tpd_keys_dim[i][0]+(tpd_keys_dim[i][2]/2) &&
               y>=tpd_keys_dim[i][1]-(tpd_keys_dim[i][3]/2) &&
               y<=tpd_keys_dim[i][1]+(tpd_keys_dim[i][3]/2) &&
               !(tpd->btn_state&(1<<i))) 
            {
                input_report_key(tpd->kpd, tpd_keys[i], 1);
				input_sync(tpd->kpd);
                tpd->btn_state|=(1<<i);
                TPD_DEBUG("[mtk-tpd] press key %d (%d)\n",i, tpd_keys[i]);
				printk("[mtk-tpd] press key %d (%d)\n",i, tpd_keys[i]);
            }
        }
    } else {
        for(i=0;i<tpd_keycnt;i++) {
            if(tpd->btn_state&(1<<i)) {
                input_report_key(tpd->kpd, tpd_keys[i], 0);
				input_sync(tpd->kpd);
                TPD_DEBUG("[mtk-tpd] release key %d (%d)\n",i, tpd_keys[i]);
				printk("[mtk-tpd] release key %d (%d)\n",i, tpd_keys[i]);
            }
        }
        tpd->btn_state=0;
    }
}
void tpd_button_setting(int keycnt, void *keys, void *keys_dim)
{
		tpd_keycnt = keycnt;
		memcpy(tpd_keys, keys, keycnt*4);
		memcpy(tpd_keys_dim, keys_dim, keycnt*4*4);
}
//#endif
