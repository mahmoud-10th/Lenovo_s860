/************************************************
         Lenovo-sw yexh1 add for Lenovo charging spec 
         Rule:
         (1) plase keep the lenovo charging code as nice as possible
         (2) please change the mediatek's code as less as possible
         (3) please add some comments in this head 
         Attention:
         (1) please include lenovo_charging.h to use the following functions
         (2) please use the macro LENOVO_CHARGING_STANDARD to control the code in MTK native code
         (3) add the lenovo_charging.o in makefile
         (4) add MACRO in cust_charging.h
              #define AC_CHARGER_CURRENT_LIMIT	CHARGE_CURRENT_600_00_MA   //lenovo standard 0.3C
              #define LENOVO_CHARGING_STANDARD
         (5)    
*************************************************/
#include "lenovo_charging.h"
#include <cust_charging.h>
#include <linux/errno.h>

#ifdef LENOVO_H_CHARGER
#include <mach/charging.h>

extern CHARGING_CONTROL battery_charging_control;
#endif
extern  CHR_CURRENT_ENUM g_temp_input_CC_value;

///////////Lenovo charging global variables//////////
static unsigned int g_battery_notify = 0;
static int g_battery_chg_current = 0;
static int g_battery_vol = 0;
static int g_battery_wake_count = 0;
static kal_bool  g_charger_1st_in_flag = KAL_TRUE;
static kal_bool  g_charger_in_flag = KAL_FALSE;
LENOVO_CHARGING_STATE lenovo_chg_state = CHARGING_STATE_PROTECT;
LENOVO_TEMP_STATE lenovo_temp_state = LENOVO_TEMP_POS_10_TO_POS_45;

static int g_temp_value; 
static int g_battery_charging_time = 0;

static int g_temp_protect_cnt =0;
static int g_temp_err_report_cnt = 0;
static int g_temp_err_resume_cnt= 0;
static int g_temp_to_cc_cnt= 0;
static int g_temp_to_limit_c_cnt= 0;


static int g_battery_debug_value = 0;
static int g_battery_debug_temp = 0;

static int lenovo_ext_led_drv_control_en = 1;//0-off;1-on

/*lenovo-sw weiweij added for bb test request of changing chg current manually*/
#ifdef LENOVO_CHARGING_TEST
static int g_test_charging_cur = 0;

extern kal_int32 battery_meter_get_battery_temperature(void);
static void lenovo_test_set_cur(void* data);
static void lenovo_test_resume_cur(void);
#endif
/*lenovo-sw weiweij added for bb test request of changing chg current manually end*/
/*lenovo_sw liaohj add for charging led diff call led 2013-11-14 ---begin*/
//#ifdef MTK_FAN5405_SUPPORT
int g_temp_charging_blue_flag = 0;
//#endif
/*lenovo_sw liaohj add for charging led diff call led 2013-11-14 ---end*/

/////////////////////////////////////////////////////////////////
                ///MTK dependency charging functions///// 
////////////////////////////////////////////////////////////////
static unsigned int g_bat_charging_state;
void lenovo_battery_backup_charging_state(void)
{
       if (BMT_status.bat_charging_state != CHR_ERROR)
	   	g_bat_charging_state = BMT_status.bat_charging_state;
}
void lenovo_battery_resume_charging_state(void)
{
        BMT_status.bat_charging_state = g_bat_charging_state;
	  
}
extern kal_int32 gFG_DOD0;    //mtk dependency
kal_bool lenovo_battery_is_deep_charging(void)
{
       if (gFG_DOD0 > 85)
	   	return KAL_TRUE;
       return KAL_FALSE;
}

/*lenvoo-sw weiweij added for ext led operation*/
#ifdef LENOVO_EXT_LED_SUPPORT

#define LENOVO_EXT_LED_CLOSE	0x0
#define LENOVO_EXT_LED_RED		0x78ffffff //0x000000ff
#define LENOVO_EXT_LED_ORANGE	0x78ffffff //0x000080ff
#define LENOVO_EXT_LED_GREEN	0x78ffffff //0x0000ff00

#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
//extern void SN3193_PowerOff_Charging_RGB_LED(unsigned int level);//jixu temp

typedef struct
{
    void (*Charging_RGB_LED)(unsigned int value);
} LENOVO_LED_CONTROL_FUNCS;
const LENOVO_LED_CONTROL_FUNCS * led_ctrl  = NULL;
void lenovo_register_led_control(LENOVO_LED_CONTROL_FUNCS * ctrl)
{
	if(ctrl)
		led_ctrl = ctrl;
	else
		printk("[JX] %s NO ctrl\n",__func__);
}
#else
extern void SN3193_PowerOff_Charging_RGB_LED(unsigned int level);
#endif

static void lenovo_ext_led_set_type(unsigned int type)
{
	if(lenovo_ext_led_drv_control_en==0)
		return;

	printk("%s type=0x%x\n", __func__, type);

#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
	if(led_ctrl->Charging_RGB_LED)
		led_ctrl->Charging_RGB_LED(type);
	else
		printk("[JX] %s NO ctrl\n",__func__);
#else
#if defined(LENOVO_SN3193_SUPPORT)||defined(LENOVO_SN3199_SUPPORT)
	SN3193_PowerOff_Charging_RGB_LED(type);
#endif	
#endif
}

static void lenovo_ext_led_set_en(int state)
{
	if(state==0)
	{
		lenovo_ext_led_drv_control_en = 0;
		//lenovo_ext_led_set_type(0);
	}else
	{
		lenovo_ext_led_drv_control_en = 1;
	}
}

static void lenovo_ext_led_set_color_as_cap(void)
{
	int soc = 0;

	if(lenovo_ext_led_drv_control_en==0)
		return;

	printk("%s BMT_status.UI_SOC=%d\n", __func__, BMT_status.UI_SOC);

	if((BMT_status.UI_SOC==0)&&(BMT_status.SOC>=30))//for first time det
		soc = BMT_status.SOC;
	else
		soc = BMT_status.UI_SOC;

	if(soc<30)
		lenovo_ext_led_set_type(LENOVO_EXT_LED_RED);
	else if((soc>=30)&&(soc<=75))
		lenovo_ext_led_set_type(LENOVO_EXT_LED_ORANGE);
	else if((soc>=75)&&(soc<=99))
		lenovo_ext_led_set_type(LENOVO_EXT_LED_GREEN);	
	else if((soc>=99))
		lenovo_ext_led_set_type(LENOVO_EXT_LED_CLOSE);		
}
#endif
/*lenvoo-sw weiweij added for ext led operation end*/

/*lenovo-sw weiweij added for h_charger support*/
#ifdef LENOVO_H_CHARGER

//#include <linux/uart/mtk_uart.h>
#include <linux/serial_core.h>
#include <mach/sync_write.h>
#include <mach/mt_gpio.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/fs.h>

//#include <linux/sched.h>
//#include <linux/rtpm_prio.h>
//#include <linux/time.h>
//#include "cust_gpio_usage.h"
#include <asm/uaccess.h>

#include <mach/battery_meter.h>
#include <mach/mt_boot.h>

#define H_CHARGER_GPIO_SWITCH 	GPIO81

/*lenovo-sw weiweij modified for charging current of high volage charger mode*/
#define AC_H_CHARGER_CURRENT 		CHARGE_CURRENT_2800_00_MA
/*lenovo-sw weiweij modified for charging current of high volage charger mode end*/

int LENOVO_V_CHARGER_MAX = V_CHARGER_MAX;

int lenovo_is_h_charger = 0;
static int lenovo_h_charger_thread = 0;
static int lenovo_h_charger_charging_flag = 0;
static int lenovo_h_charger_prepare_flag = 0;
static int lenovo_h_charger_pre_chrtype = 0;
static int lenovo_h_charger_boot_flag = 1;

extern int lenovo_uart_config(int port, int baud);
extern int lenovo_uart_write(char* data, int len);

static struct file* lenovo_h_charger_uart_file = 0;

static void lenovo_h_charger_taggle_switch(int val)
{
	int ret[8];

	ret[0] = mt_set_gpio_mode(H_CHARGER_GPIO_SWITCH, GPIO_MODE_00);
	ret[1] = mt_set_gpio_dir(H_CHARGER_GPIO_SWITCH,GPIO_DIR_OUT);
	mt_get_gpio_pull_enable(H_CHARGER_GPIO_SWITCH);
	mt_set_gpio_pull_select(H_CHARGER_GPIO_SWITCH,GPIO_PULL_UP);	
	//mt_set_gpio_out(H_CHARGER_GPIO_SWITCH, GPIO_OUT_ONE);

	printk("%s ww_debug val=%d\n", __func__, val);
	if(val==0)
	{
		ret[2] = mt_set_gpio_out(H_CHARGER_GPIO_SWITCH, GPIO_OUT_ZERO);
	}else
	{
		ret[2] = mt_set_gpio_out(H_CHARGER_GPIO_SWITCH, GPIO_OUT_ONE);
	}
	//printk("%s ww_debug ret0=%d, ret1=%d, ret2=%d\n", __func__, ret[0], ret[1], ret[2]);
}

int lenovo_h_charger_get_chip(void)
{
	int ret[8];

	ret[0] = mt_set_gpio_mode(H_CHARGER_GPIO_SWITCH, GPIO_MODE_00);
	ret[1] = mt_set_gpio_dir(H_CHARGER_GPIO_SWITCH,GPIO_DIR_IN);
	mt_get_gpio_pull_enable(H_CHARGER_GPIO_SWITCH);
	mt_set_gpio_pull_select(H_CHARGER_GPIO_SWITCH,GPIO_PULL_UP);	
	//mt_set_gpio_out(H_CHARGER_GPIO_SWITCH, GPIO_OUT_ONE);

	printk("%s ww_debug val=%d\n", __func__);
	ret[2] = mt_get_gpio_out(H_CHARGER_GPIO_SWITCH);

 	printk("%s ww_debug ret0=%d, ret1=%d, ret2=%d\n", __func__, ret[0], ret[1], ret[2]);

	lenovo_h_charger_taggle_switch(0);
	return ret[2];
}

static int lenovo_h_charger_uart_config(int baud_level)
{
	struct termios settings;
	mm_segment_t oldfs;

	lenovo_h_charger_uart_file = filp_open("/dev/ttyMT0", O_RDWR|O_NDELAY, 0664); //open file
	if(IS_ERR(lenovo_h_charger_uart_file))
	{
		printk("%s Open /dev/ttyMT0 failed(%d)\n", __func__, lenovo_h_charger_uart_file);
		printk("%s errno=0x%x\n", __func__, PTR_ERR(lenovo_h_charger_uart_file));
		return -1;
	}

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	//set baud test
	lenovo_h_charger_uart_file->f_op->unlocked_ioctl(lenovo_h_charger_uart_file, TCGETS, (unsigned long)&settings);
	settings.c_cflag &= ~CBAUD;
	switch(baud_level)
	{
		case 300:
			settings.c_cflag |= B300;
			break;
		case 600:
			settings.c_cflag |= B600;
			break;
		case 0:
		case 921600:
			settings.c_cflag |= B921600;
			break;
		default:
			settings.c_cflag |= B921600;
			break;
	}

	settings.c_lflag &= ~ECHO;
	settings.c_lflag &= ~ICANON;

	lenovo_h_charger_uart_file->f_op->unlocked_ioctl(lenovo_h_charger_uart_file, TCSETS, (unsigned long)&settings);

	set_fs(oldfs);


	return 1;
}

static void lenovo_h_charger_uart_close()
{
	if(lenovo_h_charger_uart_file >0)
	{
		filp_close(lenovo_h_charger_uart_file, NULL);
		lenovo_h_charger_uart_file = 0 ;
	}
}

static int lenovo_h_charger_uart_write(unsigned char *data, int len)
{
	int num;
	mm_segment_t oldfs;
	
	{
		oldfs = get_fs();
		set_fs(KERNEL_DS);

		//write to TX
		num = lenovo_h_charger_uart_file->f_op->write(lenovo_h_charger_uart_file, data, len, &lenovo_h_charger_uart_file->f_pos);
		set_fs(oldfs);
		if(num != len)
		{
			printk("write_byte_test error: len =%d, num = %d", len,num);
		}
	}

	return num;
}

static int lenovo_h_charger_uart_read(unsigned char *data)
{
	int len;
	mm_segment_t oldfs;
	struct termios settings;

	{
		oldfs = get_fs();
		set_fs(KERNEL_DS);

		len = lenovo_h_charger_uart_file->f_op->read(lenovo_h_charger_uart_file, data, 1024, &lenovo_h_charger_uart_file->f_pos);

		set_fs(oldfs);
	}

	return len;
}


/*need to change as project code*/
static int lenovo_h_charger_resume()
{
	int hv_voltage = BATTERY_VOLT_07_000000_V;

	lenovo_h_charger_taggle_switch(0);
	msleep(500);
	printk("%s\n", __func__);
	battery_charging_control(CHARGING_CMD_SET_HV_THRESHOLD,&hv_voltage);	
	
	lenovo_h_charger_charging_flag = 0;
	lenovo_is_h_charger = 0;
	lenovo_h_charger_thread = 0;
	LENOVO_V_CHARGER_MAX = V_CHARGER_MAX;

	//lenovo_h_charger_taggle_switch(0);
}

/*need to change as project code*/
static int lenovo_h_charger_config()
{
	if(lenovo_is_h_charger!=1)
	{
		lenovo_h_charger_resume();
		return;
	}		
	
	if(lenovo_h_charger_charging_flag==0)
	{
		printk("%s\n", __func__);

			printk("%d %d %d %d %d\n", lenovo_h_charger_charging_flag, lenovo_is_h_charger, lenovo_h_charger_thread, LENOVO_V_CHARGER_MAX);
			
	
		lenovo_h_charger_charging_flag = 1;
	}
}

static int lenovo_h_charger_kthread(void *x)
{
	int cnt = 0;
	int i;
	
	if(lenovo_h_charger_thread==1)
	{
		printk("ww_debug lenovo_h_charger_thread already started\n");
		return -1;
	}

	lenovo_h_charger_thread = 1;

	msleep(100);
	lenovo_h_charger_uart_config(600);
	msleep(100);
	for(i=0;i<5;i++)
	{	
		int len = 0;
		unsigned char buf[1024];
		int ret = 0;
		
		len = lenovo_h_charger_uart_write("SA", 2);
		printk("ww_debug tx %d cnt=%d\n", len, i);
		while(1)
		{
			len = 0;
			memset(buf, 0, 1024);

			len = lenovo_h_charger_uart_read(buf);
			if(len>0)
			{
				printk("ww_debug rx %d, %s\n", len, buf);
				if((strcmp(buf, "SAOK")==0)||(strcmp(buf, "OK")==0))
				{
					ret = 1;
					
					msleep(300);
    battery_charging_control(CHARGING_CMD_DUMP_REGISTER,NULL);					
					if(battery_meter_get_charger_voltage()>7000)
					{
						printk("ww_debug is h charger\n");
						lenovo_is_h_charger = 1;
					}else
					{
						printk("ww_debug not h charger\n");
						lenovo_is_h_charger = 0;
					}

					break;
				}				
			}

			msleep(10);
			cnt++;
			if(cnt>=100)
				break;
		}

		if(ret==1)
			break;
		
		msleep(1000);
	}								

	lenovo_h_charger_uart_close();
	lenovo_h_charger_thread = 0;

	lenovo_h_charger_config();
	
	return 0;
}

static void lenovo_h_charger_check_charger(void)
{
	int vol = 0;

	if(lenovo_is_h_charger==0)
		return;
		
	vol = battery_meter_get_charger_voltage();
	if(vol<7000)
	{
		printk("%s: vol fall to normal(%dv), resume to normal charge\n", __func__, vol);
		lenovo_h_charger_stop();
	}
}

int lenovo_h_charger_prepare()
{
	lenovo_h_charger_prepare_flag = 1;
}

void lenovo_h_charger_set_chrtype(void)
{
	if((lenovo_is_h_charger)||(lenovo_h_charger_thread))
		BMT_status.charger_type = STANDARD_CHARGER;
}

int lenovo_h_charger_start()
{
#if 1
	int hv_voltage = BATTERY_VOLT_10_000000_V;
 	struct timespec now_time;
	
	if(lenovo_h_charger_prepare_flag==0)
	{
		if(lenovo_h_charger_boot_flag==1)
		{
			printk("lenovo_h_charger boot in charger boot mode, check h charger, boot reason=%d\n", g_boot_reason);
		}else
		{
			return -3;
		}
	}
	
	if(BMT_status.charger_type==CHARGER_UNKNOWN)
		return -4;

	getrawmonotonic(&now_time);
	if(now_time.tv_sec<=10)
		return;

	lenovo_h_charger_boot_flag = 0;		
	lenovo_h_charger_prepare_flag = 0;

	//if((BMT_status.charger_type!=STANDARD_CHARGER)&&(BMT_status.charger_type!=3))//test
	if(BMT_status.charger_type!=STANDARD_CHARGER)		
	{
		return -2;
	}

	lenovo_h_charger_charging_flag = 0;

	if(lenovo_h_charger_thread==1)
	{
		printk("ww_debug lenovo_h_charger_kthread 1\n");
		return -1;
	}

	battery_charging_control(CHARGING_CMD_SET_HV_THRESHOLD,&hv_voltage);

	LENOVO_V_CHARGER_MAX = V_H_CHARGER_MAX;
		
	lenovo_h_charger_taggle_switch(1);

	kthread_run(lenovo_h_charger_kthread, NULL, "lenovo_h_charger_kthread"); 
	printk("lenovo_h_charger_kthread start\n");
	return 0;
#endif	
}

int lenovo_h_charger_stop(void)
{
	if(lenovo_is_h_charger==0)
		return;
	
	if(lenovo_h_charger_charging_flag==1)
	{
		printk("%s\n", __func__);
		lenovo_h_charger_resume();
	}
}
#endif
/*lenovo-sw weiweij added for h_charger support end*/

#ifdef LENOVO_SPECAIL_BATTERY_TEMP_PROTECT
unsigned int lenovo_specail_battery_temp_protest_set_cv_vol(void)
{
	BATTERY_VOLTAGE_ENUM cv_voltage = 0;
	printk("%s : state = %d\n", __func__, lenovo_temp_state);
	switch(lenovo_temp_state)
	{
		case LENOVO_TEMP_POS_0_TO_POS_10:
			cv_voltage = LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CV_VOL_0_TO_10;
			break;
		case LENOVO_TEMP_POS_10_TO_POS_45:	
			cv_voltage = LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CV_VOL_10_TO_45;
			break;
		case LENOVO_TEMP_POS_45_TO_POS_50:	
			cv_voltage = LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CV_VOL_45_TO_50;
			break;
		default:
 #ifdef HIGH_BATTERY_VOLTAGE_SUPPORT
                    cv_voltage = BATTERY_VOLT_04_350000_V;
#else
                    cv_voltage = BATTERY_VOLT_04_200000_V;
#endif			
			break;
	}
	return cv_voltage;
}
unsigned int lenovo_specail_battery_temp_protest_set_cur(void* data)
{
	CHR_CURRENT_ENUM cur = 0;
	printk("%s : state = %d\n", __func__, lenovo_temp_state);
	switch(lenovo_temp_state)
	{
		case LENOVO_TEMP_POS_0_TO_POS_10:
			cur = LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CUR_0_TO_10;
			break;
		case LENOVO_TEMP_POS_10_TO_POS_45:	
			cur = LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CUR_10_TO_45;
			break;
		case LENOVO_TEMP_POS_45_TO_POS_50:	
#ifdef LENOVO_H_CHARGER	
			if(lenovo_is_h_charger==1)
				cur = LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CUR_45_TO_50;
			else
				cur = LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CUR_45_TO_50;
#else
			cur = LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CUR_45_TO_50;
#endif
			break;
		default:
                    	cur = LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CUR_10_TO_45;
			break;
	}
	*((unsigned int*) data) = cur;
	return cur;
}
#endif
/*lenovo-sw weiweij added for specail battery temperature protect end*/
/*lenovo-sw weiweij added for charging current check in bq24196*/
#if defined(MTK_BQ24196_SUPPORT) || defined(MTK_BQ24250_SUPPORT)
#ifdef MTK_BQ27531_SUPPORT
extern short bq27531_get_averagecurrent(void);
#else
#ifdef LENOVO_BQ24250_CHG_CUR_DET
extern kal_int32 lenovo_read_charging_cur(void *data);
#else
extern kal_int32 oam_i_1;
#endif
#endif
#endif
/*lenovo-sw weiweij added for charging current check in bq24196 end*/

void lenovo_battery_get_data(kal_bool in)
{
/*lenovo-sw weiweij added for charging current check in bq24196*/
#if defined(MTK_BQ24196_SUPPORT) || defined(MTK_BQ24250_SUPPORT)
#ifdef MTK_BQ27531_SUPPORT
	int ret =  (int) bq27531_get_averagecurrent();
	g_battery_chg_current = (ret>=0)? ret : 0;
#else	
#ifdef LENOVO_BQ24250_CHG_CUR_DET
	//g_battery_chg_current = (oam_i_1>=0)? 0 : 0-(oam_i_1/10);	
	lenovo_read_charging_cur(&g_battery_chg_current);
#else
	g_battery_chg_current = (oam_i_1>=0)? 0 : 0-(oam_i_1/10);	
#endif
#endif
#else
	g_battery_chg_current = (in ==KAL_TRUE) ? battery_meter_get_charging_current() : 0; 
#endif
/*lenovo-sw weiweij added for charging current check in bq24196 end*/

	g_temp_value = BMT_status.temperature;
	g_battery_vol =BMT_status.bat_vol;

	//DEBUG
	if (g_battery_debug_temp != 0)
		g_temp_value = g_battery_debug_temp;
}

void lenovo_battery_disable_charging(void)
{
    kal_uint32 disable = KAL_FALSE;
    battery_charging_control(CHARGING_CMD_ENABLE, &disable);
}

void lenovo_battery_enable_charging(void)
{
    kal_uint32 flag = KAL_TRUE;
    battery_charging_control(CHARGING_CMD_ENABLE, &flag);
}

//in lenovo factory mode(####1111#) test, in factory we need to display Ichg ASAP
void lenvovo_battery_wake_bat_thread(void)
{
     if (g_battery_wake_count++ < 3){
        	   printk("[BATTERY] wake up again. Need to display Icharging ASAP in lenovo fac. mode.\n"); 
              // msleep(50);  
        	   wake_up_bat ();
      }else
		       g_battery_wake_count = 5;
}

//MTK dependency
void lenovo_get_charging_curret(void *data)  // kernel panic, if you pass CHR_CURRENT_ENUM*
{    
       if(data == NULL) {
	   	return ;
       }	
	 if ( *(CHR_CURRENT_ENUM*)(data) == CHARGE_CURRENT_0_00_MA){
              return;     
	 }
	 if (lenovo_chg_state == CHARGING_STATE_PROTECT || lenovo_chg_state == CHARGING_STATE_ERROR)
	 	    *(CHR_CURRENT_ENUM*)(data) = CHARGE_CURRENT_0_00_MA;
	 
       if (BMT_status.charger_type == STANDARD_CHARGER)  //call or temp abnormal, we will limit the currenct
       	{     
      			g_temp_input_CC_value = CHARGE_CURRENT_2000_00_MA;

       		if (g_call_state == CALL_ACTIVE || lenovo_chg_state == CHARGING_STATE_LIMIT_CHARGING){
       	       #ifdef AC_CHARGER_CURRENT_LIMIT
/*lenovo-sw weiweij added for specail battery temperature protect*/
#ifdef LENOVO_SPECAIL_BATTERY_TEMP_PROTECT
				lenovo_specail_battery_temp_protest_set_cur((unsigned int*) data);
				printk("ww_debug data = %d\n", (*(unsigned int*)data));
#else				
			     *(CHR_CURRENT_ENUM*)(data)  =  AC_CHARGER_CURRENT_LIMIT;  //0.3C 
#endif
/*lenovo-sw weiweij added for specail battery temperature protect end*/
			 #else  
		            *(CHR_CURRENT_ENUM*)(data)  =  AC_CHARGER_CURRENT;  
		       #endif  
       	      }
#ifdef LENOVO_H_CHARGER	   
	   		else
		       {
		       	if(lenovo_is_h_charger==1)
		       	{
					*(CHR_CURRENT_ENUM*)(data) = AC_H_CHARGER_CURRENT;
					//g_temp_input_CC_value = CHARGE_CURRENT_1500_00_MA;
	
					lenovo_h_charger_check_charger();
/*lenovo-sw weiweij added for bb test request of changing chg current manually*/
#ifdef LENOVO_CHARGING_TEST
				lenovo_test_set_cur(data);
#endif
/*lenovo-sw weiweij added for bb test request of changing chg current manually end*/
		       	}		       		
		       }
#endif			
       	}	  
}

kal_bool lenovo_battery_is_battery_exist(kal_int64 R, kal_int64 Rdown)
{    //if the paralleling resistors almost equal to RBAT_PULL_DOWN_R, we asume the bat not exists
	if(R >= Rdown-2000){	
		lenovo_bat_printk(LENOVO_BAT_LOG_CRTI,  "Battery no exist, TRes_temp=%d, Rdown:%d \n", R,Rdown);
		return KAL_FALSE;
		}

	return KAL_TRUE;
}

////////////////////////////////////////////////////
           //////Lenovo charging debug functions//////////
///////////////////////////////////////////////////// 
#define LENOVO_START_CHARGING_VOLT  3500   //3.5V
kal_bool g_battery_discharging_flag = KAL_FALSE;
static struct task_struct *p_discharging_task = NULL;

int lenovo_battery_discharging_kthread(void *data)
{
      int life_and_everything; 
      // busy looper //
	do{
            life_and_everything = 99 * 99;
	}while(!kthread_should_stop());
}
void  lenovo_battery_start_discharging(void)
{      
      p_discharging_task = kthread_run(lenovo_battery_discharging_kthread, NULL, "lenovo_battery_discharging_kthread"); 
}
void lenovo_battery_stop_discharging(void)
{      if (p_discharging_task != NULL){
             kthread_stop(p_discharging_task);
		p_discharging_task = NULL;	 
		lenovo_bat_printk(LENOVO_BAT_LOG_CRTI, "stop debug discharging, bat vol:%d \n ",  g_battery_vol);

       }
}
kal_bool lenovo_battery_is_debug_discharging(void)
{
        if (g_battery_debug_value == 1)
        	{  if (g_battery_vol > LENOVO_START_CHARGING_VOLT){
        	          lenovo_bat_printk(LENOVO_BAT_LOG_CRTI, "is debug discharging, bat vol:%d \n ",  g_battery_vol);
       	          if(g_battery_discharging_flag == KAL_FALSE){
                                    lenovo_battery_start_discharging();
					    g_battery_discharging_flag = KAL_TRUE;
					  }
		           return KAL_TRUE;

		   }else{
		            g_battery_debug_value = 0;
		           if(g_battery_discharging_flag == KAL_TRUE){
		                   lenovo_battery_stop_discharging();
			            g_battery_discharging_flag = KAL_FALSE;  
		           	}                      
		   }
		}
	 return KAL_FALSE;
}


////////////////////////////////////////////////////
           //////Lenovo charging functions//////////
/////////////////////////////////////////////////////           
void lenovo_battery_reset_debounce(void)
{
}

void lenovo_battery_reset_vars(void)
{      g_battery_charging_time = 0;
        g_battery_wake_count = 0;
	 g_temp_protect_cnt = 0;
	 g_charger_1st_in_flag = KAL_TRUE;
	 lenovo_temp_state = LENOVO_TEMP_POS_10_TO_POS_45; 
	 lenovo_chg_state = CHARGING_STATE_PROTECT;
	 g_battery_notify &= BATTERY_TEMP_NORMAL_MASK;
	 g_battery_chg_current = 0;
}
unsigned int lenvovo_battery_notify_check(void)
{     unsigned int notify_code = 0x0000;
	switch (lenovo_temp_state){
             case LENOVO_TEMP_BELOW_0:
			notify_code |=  BATTERY_TEMP_LOW_STOP_CHARGING;
			break;
		case LENOVO_TEMP_POS_0_TO_POS_10:
			//notify_code |= BATTERY_TEMP_LOW_SLOW_CHARGING;
			break;			
		case LENOVO_TEMP_POS_10_TO_POS_45:
			notify_code = 0x0000;
			break;
		case LENOVO_TEMP_POS_45_TO_POS_50:
			//notify_code |=  BATTERY_TEMP_HIGH_SLOW_CHARGING;
			break;
		case LENOVO_TEMP_ABOVE_POS_50:
		      notify_code |=  BATTERY_TEMP_HIGH_STOP_CHARGING;
		      break;
		default:	
			break;
			}    
	return notify_code;
}

kal_bool lenovo_battery_is_valid_cc(int temp)
{    // (10,45]
      if ((temp > LENOVO_TEMP_POS_10_THRESHOLD) && (temp <= LENOVO_TEMP_POS_45_THRESHOLD))
	  	return KAL_TRUE;
	return KAL_FALSE;
}

kal_bool lenovo_battery_limit_to_cc(int temp)
{    // [11, 43]
      if ((temp >= LENOVO_TEMP_POS_10_UP) && (temp <= LENOVO_TEMP_POS_45_DOWN))
	  	return KAL_TRUE;
	return KAL_FALSE;
}

kal_bool lenovo_battery_is_valid_limit(int temp)
{
      if ((temp > LENOVO_TEMP_POS_0_THRESHOLD) && (temp <= LENOVO_TEMP_POS_10_THRESHOLD))
	  	return KAL_TRUE;  // (0, 10 ]
      if ((temp > LENOVO_TEMP_POS_45_THRESHOLD) && (temp <= LENOVO_TEMP_POS_50_THRESHOLD))
	  	return KAL_TRUE;  // (45, 50 ]  	
	return KAL_FALSE;
}

kal_bool lenovo_battery_cc_to_limit(int temp)
{
      if ((temp > LENOVO_TEMP_POS_0_THRESHOLD) && (temp <= LENOVO_TEMP_POS_10_DOWN))
	  	return KAL_TRUE;  // (0, 8]
      if ((temp >= LENOVO_TEMP_POS_45_UP) && (temp <= LENOVO_TEMP_POS_50_THRESHOLD))
	  	return KAL_TRUE;  // [46, 50 ]  	
	return KAL_FALSE;
}

kal_bool lenovo_battery_error_to_limit(int temp)
{    
      if ((temp >= LENOVO_TEMP_POS_0_UP) && (temp <= LENOVO_TEMP_POS_50_DOWN))
	  	return KAL_TRUE;   //  [2, 48]
	return KAL_FALSE;
}

kal_bool lenovo_battery_is_neg10_low_temp(int temp)
{    
      if (temp <= LENOVO_TEMP_NEG_10_THRESHOLD)
	  	return KAL_TRUE;   //  <= -10
	return KAL_FALSE;
}
kal_bool lenovo_battery_is_0_low_temp(int temp)
{    
      if (temp <= LENOVO_TEMP_POS_0_THRESHOLD)
	  	return KAL_TRUE;   //  <= 0
	return KAL_FALSE;
}

kal_bool lenovo_battery_is_50_high_temp(int temp)
{    
      if (temp > LENOVO_TEMP_POS_50_THRESHOLD)
	  	return KAL_TRUE;   //  >50
	return KAL_FALSE;
}

kal_bool lenovo_battery_is_0_50_normal_temp(int temp)
{    
      if ((temp > LENOVO_TEMP_POS_0_THRESHOLD) && (temp <= LENOVO_TEMP_POS_50_THRESHOLD))
	  	return KAL_TRUE;   //  (0,50]
	return KAL_FALSE;
}

kal_bool lenovo_battery_is_2_48_normal_temp(int temp)
{    
      if ((temp > LENOVO_TEMP_POS_0_UP) && (temp <= LENOVO_TEMP_POS_50_DOWN))
	  	return KAL_TRUE;   //  [2,48]
	return KAL_FALSE;
}


void lenovo_battery_limit_c_temp_state(int temp)
{
     if ((temp > LENOVO_TEMP_POS_0_THRESHOLD) && (temp <= LENOVO_TEMP_POS_10_THRESHOLD))
	  	lenovo_temp_state = LENOVO_TEMP_POS_0_TO_POS_10;  // (0, 10 ]
      if ((temp > LENOVO_TEMP_POS_45_THRESHOLD) && (temp <= LENOVO_TEMP_POS_50_THRESHOLD))
	  	lenovo_temp_state = LENOVO_TEMP_POS_45_TO_POS_50;  // (45, 50 ] 	 
}

void lenovo_battery_charing_protect(int temp)
{     static kal_bool has_been_neg10 = KAL_FALSE;

       g_temp_protect_cnt++;
       if (KAL_TRUE == lenovo_battery_is_0_50_normal_temp(temp)){
       	    if (KAL_TRUE == lenovo_battery_is_valid_cc(temp)){
				lenovo_chg_state = CHARGING_STATE_CHARGING;
				lenovo_temp_state = LENOVO_TEMP_POS_10_TO_POS_45;
       	    	} else{   
				lenovo_chg_state = CHARGING_STATE_LIMIT_CHARGING;
				 lenovo_battery_limit_c_temp_state(temp);
		    	}				
	       }
	 else if  (KAL_TRUE == lenovo_battery_is_neg10_low_temp(temp)){
	 	has_been_neg10 = KAL_TRUE;
	 	if (g_temp_protect_cnt > LENOVO_TEMP_NEG_10_COUNT){
                   lenovo_chg_state = CHARGING_STATE_ERROR;
	             lenovo_temp_state = LENOVO_TEMP_BELOW_0;
		}
	 } else if  (KAL_TRUE == lenovo_battery_is_0_low_temp(temp)){
	      if (has_been_neg10 == KAL_TRUE){
			has_been_neg10 = KAL_FALSE;
			g_temp_protect_cnt = 0;
		  }
             if (g_temp_protect_cnt > LENOVO_TEMP_NEG_0_COUNT){
                   lenovo_chg_state = CHARGING_STATE_ERROR;
	             lenovo_temp_state = LENOVO_TEMP_BELOW_0;
		}
	 }else if  (KAL_TRUE == lenovo_battery_is_50_high_temp(temp)){
             if (g_temp_protect_cnt > LENOVO_TEMP_POS_50_COUNT){
                   lenovo_chg_state = CHARGING_STATE_ERROR;
	             lenovo_temp_state = LENOVO_TEMP_ABOVE_POS_50;
		}
	 }

	 if (lenovo_chg_state != CHARGING_STATE_PROTECT)
	 	g_temp_protect_cnt = 0;
	 
}
void lenovo_battery_charging(int temp)
{
      if (KAL_TRUE == lenovo_battery_is_0_50_normal_temp(temp)){
	  	  if (lenovo_chg_state == CHARGING_STATE_CHARGING){
		  	if (KAL_TRUE == lenovo_battery_cc_to_limit(temp))
				  g_temp_to_limit_c_cnt++;
			else  g_temp_to_limit_c_cnt = 0;
			if (g_temp_to_limit_c_cnt > LENOVO_TEMP_LIMIT_C_DEBOUNCE_COUNT){
                            lenovo_chg_state = CHARGING_STATE_LIMIT_CHARGING;
				  lenovo_battery_limit_c_temp_state(temp);			
				  g_temp_to_limit_c_cnt = 0; 			
				}
		  }else{
		  	if (KAL_TRUE == lenovo_battery_limit_to_cc(temp))
				   g_temp_to_cc_cnt++;
			else    g_temp_to_cc_cnt = 0;
			if (g_temp_to_cc_cnt > LENOVO_TEMP_CC_DEBOUNCE_COUNT){
                            lenovo_chg_state = CHARGING_STATE_CHARGING;
				  lenovo_temp_state = LENOVO_TEMP_POS_10_TO_POS_45; 			
				  g_temp_to_cc_cnt = 0; 			
				}
		  }
		  g_temp_err_report_cnt = 0;	 

	  } else{  //temp error
	         g_temp_err_report_cnt++;
               if (g_temp_err_report_cnt > LENOVO_TEMP_ERROR_REPORT_COUNT){
			     g_temp_err_report_cnt = 0;
			     lenovo_chg_state = CHARGING_STATE_ERROR; 	 
                        if (KAL_TRUE == lenovo_battery_is_0_low_temp(temp)){                             
	                        lenovo_temp_state = LENOVO_TEMP_BELOW_0;			
				}else{
	                       lenovo_temp_state = LENOVO_TEMP_ABOVE_POS_50;
				}
				lenovo_bat_printk(LENOVO_BAT_LOG_CRTI, "error happens, temp:%d \n ", temp);				
			  }
                g_temp_to_cc_cnt= 0;
                g_temp_to_limit_c_cnt= 0;		   	   
	  }
}
void lenovo_battery_charging_error(int temp)
{
      if (KAL_TRUE == lenovo_battery_is_2_48_normal_temp(temp))
	  	   g_temp_err_resume_cnt++;
	else   g_temp_err_resume_cnt = 0;
	
	if (g_temp_err_resume_cnt > LENOVO_TEMP_ERROR_RESUME_COUNT){
                g_temp_err_resume_cnt = 0;
       	    if (KAL_TRUE == lenovo_battery_is_valid_cc(temp)){
				lenovo_chg_state = CHARGING_STATE_CHARGING;
				lenovo_temp_state = LENOVO_TEMP_POS_10_TO_POS_45;
       	    	}
		    else{   
				lenovo_chg_state = CHARGING_STATE_LIMIT_CHARGING;
				 lenovo_battery_limit_c_temp_state(temp);
		    	}
		   //reset mtk charging state
		   lenovo_battery_resume_charging_state();
		   lenovo_battery_enable_charging();
	       }
}

/*lenovo-sw weiweij added for charging terminate as 0.1c*/
#ifdef LENOVO_CHARGING_TERM
extern int lenovo_charging_charger_type ;
#endif
/*lenovo-sw weiweij added for charging terminate as 0.1c end*/

void lenovo_battery_charger_in(void)
{    int temp = g_temp_value;
      lenvovo_battery_wake_bat_thread();
	lenovo_battery_backup_charging_state();  

	if (g_charger_1st_in_flag == KAL_TRUE){
          g_charger_1st_in_flag = KAL_FALSE;
	    lenovo_chg_state = CHARGING_STATE_PROTECT;
	    lenovo_battery_set_Qmax_cali_status(1);	
	}

/*lenovo-sw weiweij added for h_charger support*/
#ifdef LENOVO_H_CHARGER
	lenovo_h_charger_start();
#endif
/*lenovo-sw weiweij added for h_charger support end*/

/*lenovo-sw weiweij added for charging terminate as 0.1c*/
#ifdef LENOVO_CHARGING_TERM
	lenovo_charging_charger_type = BMT_status.charger_type;
#endif
/*lenovo-sw weiweij added for charging terminate as 0.1c end*/

	switch (lenovo_chg_state){
             case CHARGING_STATE_PROTECT:
			lenovo_battery_charing_protect(temp); 	
			break;
		case CHARGING_STATE_LIMIT_CHARGING:
		case CHARGING_STATE_CHARGING:
			lenovo_battery_charging(temp);
			break;
		case CHARGING_STATE_ERROR:
			lenovo_battery_charging_error(temp);
			break;
		default:	
			break;
			}

}

/*lenovo-sw weiweij added for led-soc sync*/
#ifdef LENOVO_CHARGING_STANDARD
extern PMU_ChargerStruct BMT_status;

extern kal_uint32 charging_led_enable(kal_uint32 val);
extern kal_uint32 upmu_get_qi_va_en(void);
extern void upmu_set_rg_va_en(kal_uint32 val);

void lenovo_battery_charging_full_action(void)
{
	charging_led_enable(0);
}

void lenovo_battery_charging_resume_action(void)
{
	charging_led_enable(1);
}

static void lenovo_battery_charging_set_led_state(void)
{
#ifdef LENOVO_EXT_LED_SUPPORT
	charging_led_enable(0);
	if(BMT_status.charger_exist == KAL_TRUE)
		lenovo_ext_led_set_color_as_cap();
	else
		lenovo_ext_led_set_type(LENOVO_EXT_LED_CLOSE);
#else
/*lenovo_sw liaohj modify for charging led diff call led 2013-11-14 ---begin*/
	if(BMT_status.UI_SOC==100)
	{
		charging_led_enable(0);
	}
	else
	{	
		if(g_temp_charging_blue_flag == 0)
		{
			charging_led_enable(1);
		}
		else
		{
			charging_led_enable(0);
		}
	}
/*lenovo_sw liaohj modify for charging led diff call led 2013-11-14 ---end*/
#endif

}
#endif
/*lenovo-sw weiweij added for led-soc sync end*/

void lenovo_battery_charger_out(void)
{
#ifdef LENOVO_CHARGING_TEST
	 lenovo_test_resume_cur();
#endif

       lenovo_battery_reset_vars();
	lenovo_battery_set_Qmax_cali_status(0);  
}

void lenovo_battery_debug_print(void)
{     g_battery_charging_time += LENOVO_CHARGING_THREAD_PERIOD; 
       lenovo_bat_printk(LENOVO_BAT_LOG_CRTI, "I:%d, Vbat:%d, Temp:%d, chg_state:%d, temp_state;%d \n ", 
	   	g_battery_chg_current, g_battery_vol, g_temp_value, lenovo_chg_state, lenovo_temp_state);
	 if (g_bat_charging_state == CHR_CC)
	 	lenovo_bat_printk(LENOVO_BAT_LOG_CRTI, "time:%d mins, state: CHR_CC \n ", g_battery_charging_time/60);
	 if (g_bat_charging_state == CHR_BATFULL)
	 	lenovo_bat_printk(LENOVO_BAT_LOG_CRTI, "time:%d mins, state: CHR_BATFULL \n ", g_battery_charging_time/60);
}

/*lenovo-sw weiweij added for bq27531 fireware update in first boot mode*/
#ifdef MTK_BQ27531_SUPPORT
	//27531 firmware update
extern int bq27531_check_fw_ver(void);
#endif
/*lenovo-sw weiweij added for bq27531 fireware update in first boot mode end*/

//mtk dependency : BMT_status.charger_exis
kal_bool lenovo_battery_charging_thread(unsigned int *notify)
{ 

/*lenovo-sw weiweij added for bq27531 fireware update in first boot mode*/
#ifdef MTK_BQ27531_SUPPORT
	//27531 firmware update
	bq27531_check_fw_ver();
#endif
/*lenovo-sw weiweij added for bq27531 fireware update in first boot mode end*/

    lenovo_battery_get_data(BMT_status.charger_exist);
	
    if( BMT_status.charger_exist == KAL_TRUE ){
	    if (lenovo_battery_is_debug_discharging() == KAL_TRUE) {	
			lenovo_battery_disable_charging();
			return KAL_FALSE;
	    	}
          lenovo_battery_charger_in();  

	   *notify &= BATTERY_TEMP_NORMAL_MASK;	    	
         *notify |= lenvovo_battery_notify_check();
		 
	    g_battery_notify = *notify; //update the notify value	 

           lenovo_battery_debug_print(); 		   
    }else{
           if (g_charger_in_flag == KAL_TRUE)
                      lenovo_battery_charger_out();
    	}  
    g_charger_in_flag = BMT_status.charger_exist;

/*lenovo-sw weiweij added for led-soc sync*/
#ifdef LENOVO_CHARGING_STANDARD	
	lenovo_battery_charging_set_led_state();
#endif
/*lenovo-sw weiweij added for led-soc sync*/

    return KAL_TRUE;
}





///////////////////////////////////////////////////////////////////////
                   /////Lenovo charging sys nodes//// 
//////////////////////////////////////////////////////////////////////

/// battery calibration status (start)
int battery_cali_start_status = 0;  //0: no Qmax cali; 1: on going Qmax cali (gFG_DOD0 > 85); 2: Qmax cali done
void lenovo_battery_set_Qmax_cali_status(int status)
{       
          if (status == 1){
		  if (lenovo_battery_is_deep_charging())		  	
		  	battery_cali_start_status = status; 
          	}
	    else	  
                    battery_cali_start_status = status; 

           lenovo_bat_printk(LENOVO_BAT_LOG_CRTI, "batt_calistatus:%d \n ", battery_cali_start_status);		
	 
}

static ssize_t batt_show_calistatus(struct device* dev,
				struct device_attribute *attr, char* buf)
{
    return sprintf(buf, "%d\n", battery_cali_start_status);
}
static DEVICE_ATTR(batt_calistatus, S_IRUGO|S_IWUSR, batt_show_calistatus, NULL);
/// battery calibration status (end)


/// battery show charging current (start)
 static ssize_t chg_show_i_current(struct device* dev,
				struct device_attribute *attr, char* buf)
{
   // xlog_printk(ANDROID_LOG_DEBUG, "Power/Battery", " lenovo_chg_show_i_current : %d\n", g_battery_chg_current);
    return sprintf(buf, "%d\n", g_battery_chg_current);
}
static DEVICE_ATTR(chg_current, S_IRUGO|S_IWUSR, chg_show_i_current, NULL);
/// battery show charging current (start)

//start
 static ssize_t chg_show_debug_value(struct device* dev,
				struct device_attribute *attr, char* buf)
{
    return sprintf(buf, "%d\n", g_battery_debug_value);
}

static ssize_t chg_store_debug_value(struct device *pdev, struct device_attribute *attr,
			    const char *buf, size_t size)
{
        sscanf(buf, "%d", &g_battery_debug_value);	
        return size; 
} 
static DEVICE_ATTR(debug_value, S_IRUGO|S_IWUSR, chg_show_debug_value, chg_store_debug_value);
//end

//start
 static ssize_t chg_show_debug_temp(struct device* dev,
				struct device_attribute *attr, char* buf)
{
    return sprintf(buf, "%d\n", g_battery_debug_temp);
}

static ssize_t chg_store_debug_temp(struct device *pdev, struct device_attribute *attr,
			    const char *buf, size_t size)
{
        sscanf(buf, "%d", &g_battery_debug_temp);	
        return size; 
} 
static DEVICE_ATTR(debug_temp, S_IRUGO|S_IWUSR, chg_show_debug_temp, chg_store_debug_temp);
//end

//start
 static ssize_t chg_show_notify_value(struct device* dev,
				struct device_attribute *attr, char* buf)
{
    return sprintf(buf, "%u\n", g_battery_notify);
}
static DEVICE_ATTR(notify_value, S_IRUGO|S_IWUSR, chg_show_notify_value, NULL);
//end

static ssize_t chg_show_ext_led_drv_control(struct device* dev, struct device_attribute *attr, char* buf)
{
    return sprintf(buf, "%u\n", lenovo_ext_led_drv_control_en);
}

 static ssize_t chg_store_ext_led_drv_control(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    char *pvalue = NULL;
    //unsigned int state = 0;
    printk("[Battery] %s\n", __func__);
    if(buf != NULL && size != 0)
    {
        printk( "[Battery] buf is %s and size is %d \n",buf,size);
        lenovo_ext_led_drv_control_en = simple_strtoul(buf,&pvalue,10);
        printk("[Battery] store code : %d \n",lenovo_ext_led_drv_control_en);     	
    }        

    return size;
}
static DEVICE_ATTR(ext_led_drv_control, 0664, chg_show_ext_led_drv_control, chg_store_ext_led_drv_control);

/*lenovo-sw weiweij added for h_charger state sysfile point*/
#ifdef LENOVO_H_CHARGER
 static ssize_t chg_show_h_charger_state(struct device* dev, struct device_attribute *attr, char* buf)
{
    return sprintf(buf, "%u\n", lenovo_is_h_charger);
}

 static ssize_t chg_store_h_charger_state(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    char *pvalue = NULL;
    unsigned int state = 0;
    printk("[Battery] chg_store_h_charger_state\n");
    if(buf != NULL && size != 0)
    {
        printk( "[Battery] buf is %s and size is %d \n",buf,size);
        state = simple_strtoul(buf,&pvalue,16);
        printk("[Battery] store code : %d \n",state);     
	if(state==0)
	{
		lenovo_h_charger_taggle_switch(0);
		lenovo_h_charger_stop();
	}
    }        

    return size;
}
static DEVICE_ATTR(h_charger_state, 0664, chg_show_h_charger_state, chg_store_h_charger_state);
#endif
/*lenovo-sw weiweij added for h_charger state sysfile point end*/

/*lenovo-sw weiweij added for bb test request of changing chg current manually*/
#ifdef LENOVO_CHARGING_TEST
static void lenovo_test_set_cur(void* data)
{
	int factor = CHARGE_CURRENT_2000_00_MA / 2000;
		
	if((g_test_charging_cur<2000)||(g_test_charging_cur>4000))
	{
		lenovo_bat_printk(LENOVO_BAT_LOG_CRTI, "%s: %d \n ", __func__, g_test_charging_cur);		
		return ;
	}

	*(CHR_CURRENT_ENUM*)(data) = factor * g_test_charging_cur;
}

static void lenovo_test_resume_cur(void)
{
	g_test_charging_cur = 0;
}

 static ssize_t chg_show_set_chg_cur(struct device* dev, struct device_attribute *attr, char* buf)
{
    return sprintf(buf, "%u\n", g_test_charging_cur);
}

 static ssize_t chg_store_set_chg_cur(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    char *pvalue = NULL;
    //unsigned int state = 0;
    printk("[Battery] %s\n", __func__);
    if(buf != NULL && size != 0)
    {
        printk( "[Battery] buf is %s and size is %d \n",buf,size);
        g_test_charging_cur = simple_strtoul(buf,&pvalue,10);
        printk("[Battery] store code : %d \n",g_test_charging_cur);     	
    }        

    return size;
}
static DEVICE_ATTR(set_chg_cur, 0664, chg_show_set_chg_cur, chg_store_set_chg_cur);

 static ssize_t chg_show_ntc_val(struct device* dev, struct device_attribute *attr, char* buf)
{
	unsigned int temp  = battery_meter_get_battery_temperature();
	
    return sprintf(buf, "%u\n", temp);
}

static DEVICE_ATTR(ntc_val, 0664, chg_show_ntc_val, NULL);
#endif
/*lenovo-sw weiweij added for bb test request of changing chg current manually end*/

/*lenovo-sw weiweij added for shutdown phone manually end*/
 static ssize_t chg_store_set_shutdown_cmd(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
	extern void mt_power_off(void);

    char *pvalue = NULL;
    unsigned int cmd = 0;
    printk("[Battery] %s\n", __func__);
    if(buf != NULL && size != 0)
    {
        printk( "[Battery] buf is %s and size is %d \n",buf,size);
        cmd = simple_strtoul(buf,&pvalue,10);
        printk("[Battery] store code : %d \n",cmd);    

		if(cmd==1) 	
			mt_power_off();
    }        

    return size;
}

 static ssize_t chg_show_set_shutdown_cmd(struct device* dev, struct device_attribute *attr, char* buf)
{
	unsigned int temp  = 0;
	
    return sprintf(buf, "%u\n", temp);
}

static DEVICE_ATTR(set_shutdown_cmd, 0664, chg_show_set_shutdown_cmd,  chg_store_set_shutdown_cmd);
/*lenovo-sw weiweij added for shutdown phone manually end*/

int lenovo_battery_create_sys_file(struct device *dev)
{     int ret;
	if (ret = device_create_file(dev, &dev_attr_chg_current))
	{
		return ret;
	}
	if (ret = device_create_file(dev, &dev_attr_batt_calistatus))
	{
		return ret;
	}
      if (ret = device_create_file(dev, &dev_attr_debug_value))
	{
		return ret;
	}
	if (ret = device_create_file(dev, &dev_attr_debug_temp))
	{
		return ret;
	} 
	if (ret = device_create_file(dev, &dev_attr_notify_value))
	{
		return ret;
	}
/*lenovo-sw weiweij added for h_charger state sysfile point*/
#ifdef LENOVO_H_CHARGER
	if (ret = device_create_file(dev, &dev_attr_h_charger_state))
	{
		return ret;
	}
#endif
/*lenovo-sw weiweij added for h_charger state sysfile point end*/	
/*lenovo-sw weiweij added for bb test request of changing chg current manually*/
#ifdef LENOVO_CHARGING_TEST
	if (ret = device_create_file(dev, &dev_attr_set_chg_cur))
	{
		return ret;
	}
	if (ret = device_create_file(dev, &dev_attr_ntc_val))
	{
		return ret;
	}	
#endif
/*lenovo-sw weiweij added for bb test request of changing chg current manually end*/
/*lenvoo-sw weiweij added for ext led operation*/
//#ifdef LENOVO_EXT_LED_SUPPORT
	if (ret = device_create_file(dev, &dev_attr_ext_led_drv_control))
	{
		return ret;
	}
//#endif
/*lenvoo-sw weiweij added for ext led operation end*/
/*lenovo-sw weiweij added for shutdown phone manually*/
	if (ret = device_create_file(dev, &dev_attr_set_shutdown_cmd))
	{
		return ret;
	}
/*lenovo-sw weiweij added for shutdown phone manually end*/
}

/////////////////////////////////////////////////////////
                 // Lenovo misc fuctions
/////////////////////////////////////////////////////////
