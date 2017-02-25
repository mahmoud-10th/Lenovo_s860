#ifndef _CUST_BAT_H_
#define _CUST_BAT_H_

/* stop charging while in talking mode */
#define STOP_CHARGING_IN_TAKLING
#define TALKING_RECHARGE_VOLTAGE 3800
#define TALKING_SYNC_TIME		   60

/* Battery Temperature Protection */
#define MTK_TEMPERATURE_RECHARGE_SUPPORT
#define MAX_CHARGE_TEMPERATURE  50
#define MAX_CHARGE_TEMPERATURE_MINUS_X_DEGREE	47
#define MIN_CHARGE_TEMPERATURE  0
#define MIN_CHARGE_TEMPERATURE_PLUS_X_DEGREE	6
#define ERR_CHARGE_TEMPERATURE  0xFF

/* Linear Charging Threshold */
#define V_PRE2CC_THRES	 		3400	//mV
#define V_CC2TOPOFF_THRES		4050
#define RECHARGING_VOLTAGE      4110
#define CHARGING_FULL_CURRENT    150	//mA

/* Charging Current Setting */
//#define CONFIG_USB_IF 						   
#define USB_CHARGER_CURRENT_SUSPEND			0		// def CONFIG_USB_IF
#define USB_CHARGER_CURRENT_UNCONFIGURED	CHARGE_CURRENT_70_00_MA	// 70mA
#define USB_CHARGER_CURRENT_CONFIGURED		CHARGE_CURRENT_500_00_MA	// 500mA

#define USB_CHARGER_CURRENT					CHARGE_CURRENT_500_00_MA	//500mA
/*Lenovo-sw weiweij add 2013-6-4,add for lenovo charging*/ 
#define AC_CHARGER_CURRENT					CHARGE_CURRENT_1800_00_MA
/*Lenovo-sw weiweij add 2013-6-4,add for lenovo charging end*/
#define NON_STD_AC_CHARGER_CURRENT			CHARGE_CURRENT_500_00_MA
#define CHARGING_HOST_CHARGER_CURRENT       CHARGE_CURRENT_650_00_MA
#define APPLE_0_5A_CHARGER_CURRENT          CHARGE_CURRENT_500_00_MA
#define APPLE_1_0A_CHARGER_CURRENT          CHARGE_CURRENT_650_00_MA
#define APPLE_2_1A_CHARGER_CURRENT          CHARGE_CURRENT_800_00_MA


/*Lenovo-sw begin yexh1 add 2013-6-4,add for lenovo charging*/ 
#define AC_CHARGER_CURRENT_LIMIT	CHARGE_CURRENT_800_00_MA   //lenovo standard 0.3C
/*Lenovo-sw end yexh1 add 2013-6-4,add for lenovo charging*/

/* Precise Tunning */
#define BATTERY_AVERAGE_DATA_NUMBER	3	
#define BATTERY_AVERAGE_SIZE 	30

#ifdef LENOVO_H_CHARGER
#define V_H_CHARGER_MAX	9500
extern int LENOVO_V_CHARGER_MAX;
extern int lenovo_h_charger_prepare(void);
extern int lenovo_h_charger_stop(void);
extern void lenovo_h_charger_set_chrtype(void);
#endif
#define LENOVO_SPECAIL_BATTERY_TEMP_PROTECT
#ifdef LENOVO_SPECAIL_BATTERY_TEMP_PROTECT
#define  LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CV_VOL_0_TO_10	BATTERY_VOLT_04_350000_V
#define  LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CV_VOL_10_TO_45	BATTERY_VOLT_04_350000_V
#define  LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CV_VOL_45_TO_50	BATTERY_VOLT_04_100000_V
#define  LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CUR_0_TO_10		AC_CHARGER_CURRENT_LIMIT
#define  LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CUR_10_TO_45		AC_CHARGER_CURRENT
#define  LENOVO_SPECAIL_BATTERY_TEMP_PROTECT_CUR_45_TO_50		AC_CHARGER_CURRENT
extern unsigned int lenovo_specail_battery_temp_protest_set_cv_vol(void);
#endif
/* charger error check */
//#define BAT_LOW_TEMP_PROTECT_ENABLE         // stop charging if temp < MIN_CHARGE_TEMPERATURE
#define V_CHARGER_ENABLE 0				// 1:ON , 0:OFF	
#define V_CHARGER_MAX 6200				// 6.2 V
#define V_CHARGER_MIN 4400				// 4.4 V

/* Tracking TIME */
#define ONEHUNDRED_PERCENT_TRACKING_TIME	10	// 10 second
#define NPERCENT_TRACKING_TIME	   			20	// 20 second
#define SYNC_TO_REAL_TRACKING_TIME  		60	// 60 second
#define V_0PERCENT_TRACKING							3450 //3450mV

/* Battery Notify */
/*Lenovo-sw begin yexh1 add 2013-6-24,add for lenovo charging*/ 
#define BATTERY_NOTIFY_CASE_0001_VCHARGER
//#define BATTERY_NOTIFY_CASE_0002_VBATTEMP
//#define BATTERY_NOTIFY_CASE_0003_ICHARGING
#define BATTERY_NOTIFY_CASE_0004_VBAT
#define BATTERY_NOTIFY_CASE_0005_TOTAL_CHARGINGTIME
/*Lenovo-sw end yexh1 add 2013-6-24,add for lenovo charging*/

/* JEITA parameter */
//#define MTK_JEITA_STANDARD_SUPPORT
#define CUST_SOC_JEITA_SYNC_TIME 30
#define JEITA_RECHARGE_VOLTAGE  4110	// for linear charging
#define JEITA_TEMP_ABOVE_POS_60_CV_VOLTAGE		BATTERY_VOLT_04_100000_V
#define JEITA_TEMP_POS_45_TO_POS_60_CV_VOLTAGE	BATTERY_VOLT_04_100000_V
#define JEITA_TEMP_POS_10_TO_POS_45_CV_VOLTAGE	BATTERY_VOLT_04_200000_V
#define JEITA_TEMP_POS_0_TO_POS_10_CV_VOLTAGE	BATTERY_VOLT_04_100000_V
#define JEITA_TEMP_NEG_10_TO_POS_0_CV_VOLTAGE	BATTERY_VOLT_03_900000_V
#define JEITA_TEMP_BELOW_NEG_10_CV_VOLTAGE		BATTERY_VOLT_03_900000_V

/* For JEITA Linear Charging only */
#define JEITA_NEG_10_TO_POS_0_FULL_CURRENT  120	//mA 
#define JEITA_TEMP_POS_45_TO_POS_60_RECHARGE_VOLTAGE  4000
#define JEITA_TEMP_POS_10_TO_POS_45_RECHARGE_VOLTAGE  4100
#define JEITA_TEMP_POS_0_TO_POS_10_RECHARGE_VOLTAGE   4000
#define JEITA_TEMP_NEG_10_TO_POS_0_RECHARGE_VOLTAGE   3800
#define JEITA_TEMP_POS_45_TO_POS_60_CC2TOPOFF_THRESHOLD	4050
#define JEITA_TEMP_POS_10_TO_POS_45_CC2TOPOFF_THRESHOLD	4050
#define JEITA_TEMP_POS_0_TO_POS_10_CC2TOPOFF_THRESHOLD	4050
#define JEITA_TEMP_NEG_10_TO_POS_0_CC2TOPOFF_THRESHOLD	3850


/* High battery support */
/*lenovo-sw weiweij added for stella high vol battery*/
#if 1
//#define HIGH_BATTERY_VOLTAGE_SUPPORT
#else
//#define HIGH_BATTERY_VOLTAGE_SUPPORT
#endif
/*lenovo-sw weiweij added for stella high vol battery end*/

#define LENOVO_CHARGING_SLEEP 1

/* Disable Battery check for HQA */
#ifdef MTK_DISABLE_POWER_ON_OFF_VOLTAGE_LIMITATION
#define CONFIG_DIS_CHECK_BATTERY
#endif

#ifdef MTK_FAN5405_SUPPORT
#define FAN5405_BUSNUM 1
#endif

#ifdef MTK_BQ24158_SUPPORT
#define BQ24158_BUSNUM 1
#endif

#ifdef MTK_BQ24250_SUPPORT
#define BQ24250_BUSNUM 1
#endif

#ifdef MTK_MAX17058_SUPPORT
#define MAX17058_BUSNUM	2

#ifdef _max17058_SW_H_
unsigned char max17058_model_data[] = 
{
	
};
#endif

#endif

/*lenovo-sw weiweij added for bq24196 support*/
#ifdef MTK_BQ24196_SUPPORT
#ifndef MTK_BQ27531_CONTROL_24196_SUPPORT
#define BQ24196_BUSNUM 1
#endif
#endif
/*lenovo-sw weiweij added for bq24196 support end*/

/*lenovo-sw weiweij added for bq27531 support*/
#ifdef MTK_BQ27531_SUPPORT
#define BQ27531_BUSNUM 1
#endif
/*lenovo-sw weiweij added for bq27531 support end*/

//#define LENOVO_EXT_LED_SUPPORT

//#define LENOVO_USING_ISENSE_AS_VBAT

//#define LENOVO_BQ24250_CHG_CUR_DET
#ifdef LENOVO_BQ24250_CHG_CUR_DET
#define LENOVO_CHARGING_CUR_R 240
#define LENOVO_CHARGING_CUR_CHANNEL 0
#endif

/*lenovo-sw weiweij added for charging terminate as 0.1c*/
#define LENOVO_CHARGING_TERM
#ifdef LENOVO_CHARGING_TERM
#define LENOVO_CHARGING_TERM_CUR_STAGE_1	2	//400ma
#define LENOVO_CHARGING_TERM_CUR_STAGE_2	0	//120ma
#endif
/*lenovo-sw weiweij added for charging terminate as 0.1c end*/

/*lenovo-sw weiweij added for otg ocp*/
#define LENOVO_OTG_OCP
/*lenovo-sw weiweij added for otg ocp end*/

/*lenovo-sw weiweij added for zcv table initail*/
#define LENOVO_ZCV_TABLE_INIT
/*lenovo-sw weiweij added for zcv table initail end*/

/*lenovo-swe weiweij added for battery temp update in charging sleep stage*/
#define LENOVO_BATTERY_TEMP_AVERAGE
#ifdef LENOVO_BATTERY_TEMP_AVERAGE
#define LENOVO_BATTERY_TEMP_AVERAGE_SIZE	10
#endif
/*lenovo-swe weiweij added for battery temp update in charging sleep stage end*/

#endif /* _CUST_BAT_H_ */ 
