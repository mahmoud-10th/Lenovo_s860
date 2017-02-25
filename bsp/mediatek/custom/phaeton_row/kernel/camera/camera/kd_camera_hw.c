#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/xlog.h>
#include <linux/kernel.h>

#include "kd_camera_hw.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"
#include <mach/camera_isp.h>

/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    xlog_printk(ANDROID_LOG_INFO, PFX , fmt, ##arg)

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...)         xlog_printk(ANDROID_LOG_ERR, PFX , fmt, ##arg)
#define PK_XLOG_INFO(fmt, args...) \
                do {    \
                   xlog_printk(ANDROID_LOG_INFO, PFX , fmt, ##arg); \
                } while(0)
#else
#define PK_DBG(a,...)
#define PK_ERR(a,...)
#define PK_XLOG_INFO(fmt, args...)
#endif

kal_bool searchMainSensor = KAL_TRUE;

#define GPIO_CAMERA_MCLK (GPIO122|0x80000000)
extern void ISP_MCLK1_EN(bool En);

int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name)
{
	u32 pinSetIdx = 0;//default main sensor
	
#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4
	
#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3
	
	
	u32 pinSet[2][8] = {
			//for main sensor
			{
				GPIO_CAMERA_CMRST_PIN,
				GPIO_CAMERA_CMRST_PIN_M_GPIO,	/* mode */
				GPIO_OUT_ONE,					/* ON state */
				GPIO_OUT_ZERO,					/* OFF state */
				GPIO_CAMERA_CMPDN_PIN,
				GPIO_CAMERA_CMPDN_PIN_M_GPIO,
				GPIO_OUT_ONE,
				GPIO_OUT_ZERO,
			},
			//for sub sensor
			{
				GPIO_CAMERA_CMRST1_PIN,
				GPIO_CAMERA_CMRST1_PIN_M_GPIO,
				GPIO_OUT_ONE,
				GPIO_OUT_ZERO,
				GPIO_CAMERA_CMPDN1_PIN,
				GPIO_CAMERA_CMPDN1_PIN_M_GPIO,
				GPIO_OUT_ONE,
				GPIO_OUT_ZERO,
			},
		};
	
		if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
        pinSetIdx = 0;
		searchMainSensor = KAL_TRUE;
    }
    else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
        pinSetIdx = 1;
		searchMainSensor = KAL_FALSE;
    }

				//RST pin
    //power ON
    if (On) {

	   mt_set_gpio_mode(GPIO_CAMERA_MCLK,GPIO_MODE_00);		 		
	   mt_set_gpio_dir(GPIO_CAMERA_MCLK,GPIO_DIR_OUT);			
	   mt_set_gpio_out(GPIO_CAMERA_MCLK,GPIO_OUT_ZERO); 
			
		msleep(3);

		PK_DBG("kdCISModulePowerOn -on:currSensorName=%s,SensorIdx=%d\n",currSensorName,SensorIdx);
		if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx)
		{
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
				mdelay(2);
				
				if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
				{
					PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
					//return -EIO;
					goto _kdCISModulePowerOn_exit_;
				}
				PK_DBG("[ON_general 2.8V]sensorIdx:%d \n",SensorIdx);
				mdelay(1);
				
				if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
				{
					PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
					//return -EIO;
					goto _kdCISModulePowerOn_exit_;
				}
				mdelay(1);
	
				//PDN/STBY pin
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
				printk("set pwrdwn pin hight\n");
				mdelay(1);
	
#if !defined(MAIN_CAM_DVDD_EXTERNAL_LDO)
				if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200,mode_name))
				{
					PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
					//return -EIO;
					goto _kdCISModulePowerOn_exit_;
				}
				printk("main cam power up internal DVDD\n");
#else			
				printk("main cam power up external DVDD\n");
				mt_set_gpio_mode(GPIO_CAMERA_LDO_EN_PIN, 0);
				mt_set_gpio_dir(GPIO_CAMERA_LDO_EN_PIN, GPIO_DIR_OUT);
				mt_set_gpio_out(GPIO_CAMERA_LDO_EN_PIN, GPIO_OUT_ONE);
#endif
	
				if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name))
				{
					PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
					//return -EIO;
					goto _kdCISModulePowerOn_exit_;
				}
	
				mt_set_gpio_mode(GPIO_CAMERA_MCLK,GPIO_MODE_01);				
				
				mdelay(3);
				ISP_MCLK1_EN(true);
	
				//disable inactive sensor
				//PWDN pin is high				
				if(mt_set_gpio_mode(GPIO_CAMERA_CMPDN1_PIN,GPIO_CAMERA_CMPDN1_PIN_M_GPIO)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}				
				if(mt_set_gpio_dir(GPIO_CAMERA_CMPDN1_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");} 			
				if(mt_set_gpio_out(GPIO_CAMERA_CMPDN1_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}			
				printk("sub sensor powerdone pin is high\n");	
	
				//reset pin is low		
				if(mt_set_gpio_mode(GPIO_CAMERA_CMRST1_PIN,GPIO_CAMERA_CMRST1_PIN_M_GPIO)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}				
				if(mt_set_gpio_dir(GPIO_CAMERA_CMRST1_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");} 		
				if(mt_set_gpio_out(GPIO_CAMERA_CMRST1_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}			
				printk("sub sensor reset pin is low\n"); 
	
				mdelay(10); //wait sub sensor correctly reseted
	
				//Enable main camera
				//XSHUTDOWN pin 
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
				mdelay(5);
			} else if(DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
	
				//PDN/STBY pin
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
				mdelay(2);
	
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
				mdelay(2);
/*
				//PWDN pin is high				
				if(mt_set_gpio_mode(GPIO_CAMERA_CMPDN1_PIN,GPIO_CAMERA_CMPDN1_PIN_M_GPIO)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}				
				if(mt_set_gpio_dir(GPIO_CAMERA_CMPDN1_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");} 			
				if(mt_set_gpio_out(GPIO_CAMERA_CMPDN1_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}			
				printk("powerdone pin is high\n");	 
	
				//reset pin is high 	
				if(mt_set_gpio_mode(GPIO_CAMERA_CMRST1_PIN,GPIO_CAMERA_CMRST1_PIN_M_GPIO)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}				
				if(mt_set_gpio_dir(GPIO_CAMERA_CMRST1_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");} 		
				if(mt_set_gpio_out(GPIO_CAMERA_CMRST1_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}*/	   
			   //DOVDD		
			   PK_DBG("IOVDD is 1.8v \n");
				if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
				{
					PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
					//return -EIO;
					goto _kdCISModulePowerOn_exit_;
				}
				PK_DBG("[ON_general 2.8V]sensorIdx:%d \n",SensorIdx);
				
				if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
				{
					PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
					//return -EIO;
					goto _kdCISModulePowerOn_exit_;
				}
	
				//PDN pin
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
				mdelay(1);
				
				if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D3, VOL_1200,mode_name))
				{
					PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
					//return -EIO;
					goto _kdCISModulePowerOn_exit_;
				}
				
				mt_set_gpio_mode(GPIO_CAMERA_MCLK,GPIO_MODE_01);	
				msleep(3);
				ISP_MCLK1_EN(true);
				
				msleep(1);
	
			   //disable inactive sensor
			   if(mt_set_gpio_mode(GPIO_CAMERA_CMPDN_PIN,GPIO_CAMERA_CMPDN_PIN_M_GPIO)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");} 			   
			   if(mt_set_gpio_dir(GPIO_CAMERA_CMPDN_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}		   
			   if(mt_set_gpio_out(GPIO_CAMERA_CMPDN_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}			   
			   PK_DBG("main sensor powerdone pin is low\n");
			   mdelay(10); //wait sub sensor correctly reseted
	
			   if(mt_set_gpio_out(GPIO_CAMERA_CMRST1_PIN,GPIO_OUT_ONE))
					{PK_DBG("[CAMERA LENS] set gpio failed!! \n");} 		   
				msleep(5);
			   
			}
		} else {//power OFF
			ISP_MCLK1_EN(false);
			//msleep(3);
			mt_set_gpio_mode(GPIO_CAMERA_MCLK,GPIO_MODE_00);				
			mt_set_gpio_dir(GPIO_CAMERA_MCLK,GPIO_DIR_OUT); 		
			mt_set_gpio_out(GPIO_CAMERA_MCLK,GPIO_OUT_ZERO);	
		
			if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx)
			{
				printk("kdCISModulePowerOn -off:currSensorName=%s,SensorIdx=%d\n",currSensorName,SensorIdx);
				//reset pull down
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
	
				//AF			
				if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A2,mode_name))
				{
					PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
					//return -EIO;
					goto _kdCISModulePowerOn_exit_;
				}

#if !defined(MAIN_CAM_DVDD_EXTERNAL_LDO)
				if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D,mode_name)) 
				{			 
					PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");			 
					//return -EIO;			  
					goto _kdCISModulePowerOn_exit_; 	  
				}
#else
				mt_set_gpio_out(GPIO_CAMERA_LDO_EN_PIN, GPIO_OUT_ZERO);
#endif
				//PDN pull down
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
				
	//interface
				if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name))
				{
					PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
					//return -EIO;
					goto _kdCISModulePowerOn_exit_;
				}

				
	//analog
				if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
					PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
					//return -EIO;
					goto _kdCISModulePowerOn_exit_;
				}				
			}
			else if(DUAL_CAMERA_SUB_SENSOR == SensorIdx)
			{
				printk("kdCISModulePowerOn -off:currSensorName=%s,SensorIdx=%d\n",currSensorName,SensorIdx);
	
	
				//reset pull down
				if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
				if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D3,mode_name)) 
				{			 
					PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");			 
					//return -EIO;			  
					goto _kdCISModulePowerOn_exit_; 	  
				}
				//PDN pull down
				if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module0

				if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2, mode_name)) 
				{			 
					PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");			  
					//return -EIO;			  
					goto _kdCISModulePowerOn_exit_; 	   
				}
	
				if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name))		
				{			 
					PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");			
					//return -EIO;			  
					goto _kdCISModulePowerOn_exit_; 	   
				} 						
			}//
		}
			
		return 0;
	
	_kdCISModulePowerOn_exit_:
		return -EIO;
	}


EXPORT_SYMBOL(kdCISModulePowerOn);


//!--
//




