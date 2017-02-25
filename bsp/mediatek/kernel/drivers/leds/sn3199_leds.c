/*
 * drivers/leds/sn3199_leds.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 * sn3199 leds driver
 *
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include <linux/leds-mt65xx.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <mach/mt_gpio.h>
#include <linux/uaccess.h>
#include <linux/fb.h>
#include <linux/debugfs.h>


/****************************************************************************
 * defined
 ***************************************************************************/
#define LED_NAME "rgbled"
#define I2C_MASTER_CLOCK       400


/*register define*/
#define SN3199_SSD_EN_REG 0x00
#define SN3199_OUT_CTL1_REG 0x01
#define SN3199_OUT_CTL2_REG 0x02
#define SN3199_SETTING1_REG 0x03
#define SN3199_SETTING2_REG 0x04
#define SN3199_BREATH_DEAD_REG 0x05
#define SN3199_BREATH_STATE_REG 0x06


/*07h~0Fh is PWM level setting register*/
#define SN3199_PWM_OUT1_RGB1_R_REG 0x07
#define SN3199_PWM_OUT2_RGB1_G_REG 0x08
#define SN3199_PWM_OUT3_RGB1_B_REG 0x09

#define SN3199_PWM_OUT4_RGB2_R_REG 0x0A
#define SN3199_PWM_OUT5_RGB2_G_REG 0x0B
#define SN3199_PWM_OUT6_RGB2_B_REG 0x0C

#define SN3199_PWM_OUT7_RGB3_R_REG 0x0D
#define SN3199_PWM_OUT8_RGB3_G_REG 0x0E
#define SN3199_PWM_OUT9_RGB3_B_REG 0x0F

#define SN3199_PWM_DATA_REFRESH_REG 0x10 // write 0x00 is ok

/*11h~19h is T0 setting register*/
#define SN3199_T0_OUT1_RGB1_R_REG 0x11
#define SN3199_T0_OUT2_RGB1_G_REG 0x12
#define SN3199_T0_OUT3_RGB1_B_REG 0x13

#define SN3199_T0_OUT4_RGB2_R_REG 0x14
#define SN3199_T0_OUT5_RGB2_G_REG 0x15
#define SN3199_T0_OUT6_RGB2_B_REG 0x16

#define SN3199_T0_OUT7_RGB3_R_REG 0x17
#define SN3199_T0_OUT8_RGB3_G_REG 0x18
#define SN3199_T0_OUT9_RGB3_B_REG 0x19

/*1Ah~1Ch is T1 & T2 & T3 setting register*/
#define SN3199_T1_T2_T3_RGB1_REG 0x1A
#define SN3199_T1_T2_T3_RGB2_REG 0x1B
#define SN3199_T1_T2_T3_RGB3_REG 0x1C

/*1Dh~25h is T4 setting register*/
#define SN3199_T4_OUT1_RGB1_R_REG 0x1D
#define SN3199_T4_OUT2_RGB1_G_REG 0x1E
#define SN3199_T4_OUT3_RGB1_B_REG 0x1F

#define SN3199_T4_OUT4_RGB2_R_REG 0x20
#define SN3199_T4_OUT5_RGB2_G_REG 0x21
#define SN3199_T4_OUT6_RGB2_B_REG 0x22

#define SN3199_T4_OUT7_RGB3_R_REG 0x23
#define SN3199_T4_OUT8_RGB3_G_REG 0x24
#define SN3199_T4_OUT9_RGB3_B_REG 0x25

#define SN3199_TIME_REFRESH_REG 0x26

#define SN3199_RESET_REG 0xFF

/***** function offset define*****/
/*00h*/
#define SN3199_SSD_OFFSET_BIT 0 //software shut down bit; 0:software shut down; 1:working mode

/*03h*/
#define SN3199_RGB_MODE_OFFSET_BIT 4 // channel select bit; 0:RGB1; 1:RGB2; 2:RGB3;

#define SN3199_AE_OFFSET_BIT 2 // audio sync breath; 0:disable; 1:enable;
#define SN3199_AGCE_OFFSET_BIT 1 //AGC enable bit; 0:enable; 1:disable;
#define SN3199_AGCM_OFFSET_BIT 0 //AGC mode bit; 0:fast; 1:slow;

/*04h*/
#define SN3199_CM_OFFSET_BIT 7 //client mode set; 0:master mode; 1:client mode;
#define SN3199_CS_OFFSET_BIT 4 //current setting
#define SN3199_AGS_OFFSET_BIT 0 //AGS

/*05h*/
#define SN3199_RM_OFFSET_BIT 4 //breath dead enable; 0:disable; 1:enable
#define SN3199_HT_OFFSET_BIT 0 //dead mode; 0:dead on T2; 1:dead on T4

/*06h*/
#define SN3199_BME_OFFSET_BIT 4 //breath state enable; 0:disable; 1:enable
#define SN3199_CSS_OFFSET_BIT 0 //breath state channel select;

/*07h~0Fh pwm setting*/
/*10h pwm data refresh*/
/*11h~19h T0 setting*/
#define SN3199_T0B_OFFSET_BIT 4 //T0B 2bit
#define SN3199_T0A_OFFSET_BIT 0 //T0B 4bit

/*1Ah~1Ch T1 T2 T3 setting*/
#define SN3199_DT_OFFSET_BIT 7 // 0:T3=T1; 1: T3=2T1; 
#define SN3199_T2_OFFSET_BIT 4 // 3bit
#define SN3199_T1_T3_OFFSET_BIT 0 // 3bit  0~4; 5,6 breath disable; 7 T1=T3=0.1s

/*1Dh~25h T4 setting*/
#define SN3199_T4B_OFFSET_BIT 4 // 2bit
#define SN3199_T4A_OFFSET_BIT 0 // 4bit

/*26h timmer refresh*/
/*FFh reset*/

/****** function define**********/
#define SN3199_SSD_ENABLE 1<<SN3199_SSD_OFFSET_BIT //working
#define SN3199_SSD_DISABLE 0<<SN3199_SSD_OFFSET_BIT //shutdown

#define SN3199_RGB_MODE_ENABLE 7<<SN3199_RGB_MODE_OFFSET_BIT
#define SN3199_RGB_MODE_DISABLE 0<<SN3199_RGB_MODE_OFFSET_BIT
#define SN3199_AE_ENABLE 1<<SN3199_AE_OFFSET_BIT
#define SN3199_AE_DISABLE 0<<SN3199_AE_OFFSET_BIT
#define SN3199_AGCE_ENABLE 0<<SN3199_AGCE_OFFSET_BIT
#define SN3199_AGCE_DISABLE 1<<SN3199_AGCE_OFFSET_BIT
#define SN3199_AGCM_FAST 0<<SN3199_AGCM_OFFSET_BIT
#define SN3199_AGCM_SLOW 1<<SN3199_AGCM_OFFSET_BIT

#define SN3199_CM_MASTER 0<<SN3199_CM_OFFSET_BIT
#define SN3199_CM_CLIENT 1<<SN3199_CM_OFFSET_BIT
#define SN3199_CS_SET_20mA 0<<SN3199_CS_OFFSET_BIT
#define SN3199_CS_SET_15mA 1<<SN3199_CS_OFFSET_BIT
#define SN3199_CS_SET_10mA 2<<SN3199_CS_OFFSET_BIT
#define SN3199_CS_SET_5mA 3<<SN3199_CS_OFFSET_BIT
#define SN3199_CS_SET_40mA 4<<SN3199_CS_OFFSET_BIT
#define SN3199_CS_SET_35mA 5<<SN3199_CS_OFFSET_BIT
#define SN3199_CS_SET_30mA 6<<SN3199_CS_OFFSET_BIT
#define SN3199_CS_SET_25mA 7<<SN3199_CS_OFFSET_BIT

#define SN3199_AGS_SET_0dB 0<<SN3199_AGS_OFFSET_BIT
#define SN3199_AGS_SET_3dB 1<<SN3199_AGS_OFFSET_BIT
#define SN3199_AGS_SET_6dB 2<<SN3199_AGS_OFFSET_BIT
#define SN3199_AGS_SET_9dB 3<<SN3199_AGS_OFFSET_BIT
#define SN3199_AGS_SET_12dB 4<<SN3199_AGS_OFFSET_BIT
#define SN3199_AGS_SET_15dB 5<<SN3199_AGS_OFFSET_BIT
#define SN3199_AGS_SET_18dB 6<<SN3199_AGS_OFFSET_BIT
#define SN3199_AGS_SET_21dB 7<<SN3199_AGS_OFFSET_BIT

#define SN3199_DT_ENABLE 1<<SN3199_DT_OFFSET_BIT
#define SN3199_DT_DISABLE 0<<SN3199_DT_OFFSET_BIT

/*************backlight value analyze************/
#define SN3199_BL_CUR_OFFSET_BIT 24// 3bit

#define SN3199_BL_CHANNEL_OFFSET_BIT 28// 1bit

#define SN3199_BL_RGB_1_OFFSET_BIT 28// 1bit
#define SN3199_BL_RGB_2_OFFSET_BIT 29// 1bit
#define SN3199_BL_RGB_3_OFFSET_BIT 30// 1bit

#define SN3199_BL_TURN_ON_OFFSET_BIT 27// 1bit

#define SN3199_BL_PWM_OFFSET_BIT 0// 24bit
#define SN3199_BL_R_OFFSET_BIT 16// 8bit
#define SN3199_BL_G_OFFSET_BIT 8// 8bit
#define SN3199_BL_B_OFFSET_BIT 0// 8bit
/*delay_on*/
#define SN3199_BL_T0_R_OFFSET_BIT 12 // 6bit
#define SN3199_BL_T0_G_OFFSET_BIT 6 // 6bit
#define SN3199_BL_T0_B_OFFSET_BIT 0 // 6bit
#define SN3199_BL_T1_T3_OFFSET_BIT 18 // 3bit
#define SN3199_BL_T2_OFFSET_BIT 21 // 3bit

/*delay_off*/
#define SN3199_BL_T4_R_OFFSET_BIT 12 // 6bit
#define SN3199_BL_T4_G_OFFSET_BIT 6 // 6bit
#define SN3199_BL_T4_B_OFFSET_BIT 0 // 6bit

/*masks*/
#define SN3199_BL_RGB_MASK 0xff
#define SN3199_BL_T0_MASK 0x3f
#define SN3199_BL_T1_T3_MASK 0x07
#define SN3199_BL_T2_MASK 0x07
#define SN3199_BL_T4_MASK 0x3f

static const char *cur_text[] = {
	"42mA",
	"10mA",
	"5mA",
	"30mA",
	"17.5mA",
};
static const char *t0_t4_text[] = {
	"0s",
	"0.13s",
	"0.26s",
	"0.52s",
	"1.04s",
	"2.08s",
	"4.16s",
	"8.32s",
	"16.64s",
	"33.28s",
	"66.56s",
};
static const char *t1_t3_text[] = {
	"0.13s",
	"0.26s",
	"0.52s",
	"1.04s",
	"2.08s",
	"4.16s",
	"8.32s",
	"16.64s",
};
static const char *t2_text[] = {
	"0s",
	"0.13s",
	"0.26s",
	"0.52s",
	"1.04s",
	"2.08s",
	"4.16s",
	"8.32s",
	"16.64s",
};



static int sn3199_is_init= 0;
static int debug_enable = 1;

#define LEDS_DEBUG(format, args...) do{ \
	if(debug_enable) \
	{\
		printk(KERN_EMERG format,##args);\
	}\
}while(0)

struct sn3199_leds_priv {
	struct led_classdev cdev;
	struct work_struct work;
	int gpio;
	int level;
	int delay_on;
	int delay_off;
	int rgb_1;
	int rgb1_on;
	int rgb1_off;
	int rgb_2;
	int rgb2_on;
	int rgb2_off;
	int rgb_3;
	int rgb3_on;
	int rgb3_off;
	int flash_music;
};

#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
typedef struct
{
	void (*Charging_RGB_LED)(unsigned int value);
} LENOVO_LED_CONTROL_FUNCS;
#endif

/****************************************************************************
 * local functions
 ***************************************************************************/

static int	sn3199_leds_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
//static int  sn3199_leds_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int  sn3199_leds_i2c_remove(struct i2c_client *client);


static int __init sn3199_leds_platform_probe(struct platform_device *pdev);
static int sn3199_leds_platform_remove(struct platform_device *pdev);
////////////////

#define SN3199_I2C_ADDR 0xC8
struct i2c_client *sn3199_i2c_cilent = NULL;

static struct i2c_board_info __initdata sn3199_i2c_board_info = { I2C_BOARD_INFO("sn3199", (SN3199_I2C_ADDR >> 1))};

static const struct i2c_device_id sn3199_i2c_id[] = {{"sn3199",0}, {}};
static struct i2c_driver sn3199_i2c_driver = {
    .probe = sn3199_leds_i2c_probe,
    .remove = sn3199_leds_i2c_remove,
    .driver.name = "sn3199",
    .id_table = sn3199_i2c_id,
    //.address_list = (const unsigned short *) forces,
};


static struct platform_driver sn3199_leds_platform_driver = {
	.driver		= {
		.name	= "leds-sn3199",
		//.owner	= THIS_MODULE,
	},
	.probe		= sn3199_leds_platform_probe,
	.remove		= sn3199_leds_platform_remove,
	//.suspend	= sn3199_leds_platform_suspend,
	//.shutdown   = sn3199_leds_platform_shutdown,
};

struct sn3199_leds_priv *g_sn3199_leds_data[2];

static void sn3199_udelay(UINT32 us)
{
	udelay(us);
}

static void sn3199_mdelay(UINT32 ms)
{
	msleep(ms);
}

static int sn3199_i2c_txdata(char *txdata, int len)
{
    int ret;
    struct i2c_msg msg[] = {
            {
                .addr = sn3199_i2c_cilent->addr,
                .flags = 0,
                .len =len,
                .buf = txdata,
            },
    };
    if(sn3199_i2c_cilent != NULL) {
        ret = i2c_transfer(sn3199_i2c_cilent->adapter, msg, 1);
        if(ret < 0)
            pr_err("%s i2c write erro: %d\n", __func__, ret);
    } else {
        LEDS_DEBUG("sn3199_i2c_cilent null\n");
    }
    return ret;
}

static int sn3199_write_reg(u8 addr, u8 para)
{
	LEDS_DEBUG("[LED]%s add:0x%x data:0x%x\n", __func__,addr,para);

    u8 buf[3];
    int ret = -1;

    buf[0] = addr;
    buf[1] = para;
    ret = sn3199_i2c_txdata(buf,2);
    if(ret < 0) {
        LEDS_DEBUG("%s write reg failed! addr=0x%x para=0x%x ret=%d\n", __func__,buf[0],buf[1],ret);
        return -1;
    }
    return 0;
}
#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
static int fake_ic = 0;
static int sn3199_test(void)
{
	int ret = -1;
    mt_set_gpio_out(/*GPIO_LED_EN*/GPIO80,GPIO_OUT_ONE);
    sn3199_mdelay(1);	
    ret = sn3199_write_reg(0x00, 0x01);
	if(ret<0)
		fake_ic = 1;
	return ret;
}
#endif

#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
static int sn3199_init(void)
#else
static void sn3199_init(void)
#endif
{
	LEDS_DEBUG("[LED]+%s\n", __func__);
#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
	if(sn3199_test()<0)
		return -1;
#endif

    mt_set_gpio_out(GPIO80/*GPIO_LED_EN*/,GPIO_OUT_ONE);
    sn3199_mdelay(10);

    sn3199_write_reg(0xFF, 0x01);
    sn3199_mdelay(10);
	
	sn3199_write_reg(0x00, 0x01);
    sn3199_write_reg(0x01, 0x77);
    sn3199_write_reg(0x02, 0x07);
	
	sn3199_write_reg(0x03, 0x00);//all breath
	sn3199_write_reg(0x04, 0x00);// 20mA
	sn3199_write_reg(0x05, 0x00);
	sn3199_write_reg(0x06, 0x00);
//rgb3
	sn3199_write_reg(0x07, 0x00);//out7
	sn3199_write_reg(0x08, 0x00);//out8
	sn3199_write_reg(0x09, 0x00);//out9
//rgb2
	sn3199_write_reg(0x0A, 0x00);//out4
	sn3199_write_reg(0x0B, 0x00);//out5
	sn3199_write_reg(0x0C, 0x00);//out6
//rgb1
	sn3199_write_reg(0x0D, 0x00);//out1
	sn3199_write_reg(0x0E, 0x00);//out2
	sn3199_write_reg(0x0F, 0x00);//out3

	sn3199_write_reg(0x11, 0x00);//t0
	sn3199_write_reg(0x12, 0x00);
	sn3199_write_reg(0x13, 0x00);
	sn3199_write_reg(0x14, 0x00);
	sn3199_write_reg(0x15, 0x00);
	sn3199_write_reg(0x16, 0x00);
	sn3199_write_reg(0x17, 0x00);
	sn3199_write_reg(0x18, 0x00);
	sn3199_write_reg(0x19, 0x00);

	sn3199_write_reg(0x1A, 0x00);//t1 t2 t3
	sn3199_write_reg(0x1B, 0x00);
	sn3199_write_reg(0x1C, 0x00);

	sn3199_write_reg(0x1D, 0x00);//t4
	sn3199_write_reg(0x1E, 0x00);
	sn3199_write_reg(0x1F, 0x00);
	sn3199_write_reg(0x20, 0x00);
	sn3199_write_reg(0x21, 0x00);
	sn3199_write_reg(0x22, 0x00);
	sn3199_write_reg(0x23, 0x00);
	sn3199_write_reg(0x24, 0x00);
	sn3199_write_reg(0x25, 0x00);

	sn3199_write_reg(0x10, 0x01);
	sn3199_write_reg(0x26, 0x01);
	    
	sn3199_is_init = 1;

    LEDS_DEBUG("[LED]-%s\n", __func__);
}
void sn3199_off(struct sn3199_leds_priv * leds_data)
{
	struct sn3199_leds_priv * led_data = leds_data;
	LEDS_DEBUG("[LED]%s\n", __func__);
	led_data->rgb_1 = 0;
	led_data->rgb1_on = 0;
	led_data->rgb1_off = 0;
	led_data->rgb_2 = 0;
	led_data->rgb2_on = 0;
	led_data->rgb2_off = 0;
	led_data->rgb_3 = 0;
	led_data->rgb3_on = 0;
	led_data->rgb3_off = 0;
	led_data->delay_on = 0;
	led_data->delay_off = 0;
	led_data->flash_music = 0;
					

	sn3199_write_reg(0x01, 0x00);
	sn3199_write_reg(0x02, 0x00);

	sn3199_write_reg(0x10, 0x01);
	sn3199_write_reg(0x26, 0x01);
	
    sn3199_write_reg(0x00, 0x00);
}

void sn3199_rgb_factory_test(void)
{

	sn3199_write_reg(SN3199_SSD_EN_REG, SN3199_SSD_ENABLE);
	sn3199_write_reg(SN3199_OUT_CTL1_REG, 0x77);
	sn3199_write_reg(SN3199_OUT_CTL2_REG, 0x07);

	sn3199_write_reg(SN3199_SETTING2_REG, 3);//current 15mA
	
	
	sn3199_write_reg(SN3199_PWM_OUT1_RGB1_R_REG, 0xFF);
	sn3199_write_reg(SN3199_PWM_OUT2_RGB1_G_REG, 0xFF);
	sn3199_write_reg(SN3199_PWM_OUT3_RGB1_B_REG, 0xFF);
	
	sn3199_write_reg(SN3199_PWM_OUT4_RGB2_R_REG, 0xFF);
	sn3199_write_reg(SN3199_PWM_OUT5_RGB2_G_REG, 0xFF);
	sn3199_write_reg(SN3199_PWM_OUT6_RGB2_B_REG, 0xFF);
	
	sn3199_write_reg(SN3199_PWM_OUT7_RGB3_R_REG, 0xFF);
	sn3199_write_reg(SN3199_PWM_OUT8_RGB3_G_REG, 0xFF);
	sn3199_write_reg(SN3199_PWM_OUT9_RGB3_B_REG, 0xFF);

	
	sn3199_write_reg(SN3199_SETTING1_REG, SN3199_RGB_MODE_ENABLE );//PWM mode	and AE mode
	//t0~t4
	sn3199_write_reg(SN3199_T0_OUT1_RGB1_R_REG, 0);//t0 R
	sn3199_write_reg(SN3199_T0_OUT2_RGB1_G_REG, 1);//G
	sn3199_write_reg(SN3199_T0_OUT3_RGB1_B_REG, 2);//B
	
	sn3199_write_reg(SN3199_T0_OUT4_RGB2_R_REG, 0);
	sn3199_write_reg(SN3199_T0_OUT5_RGB2_G_REG, 1);
	sn3199_write_reg(SN3199_T0_OUT6_RGB2_B_REG, 2);
	
	sn3199_write_reg(SN3199_T0_OUT7_RGB3_R_REG, 0);
	sn3199_write_reg(SN3199_T0_OUT8_RGB3_G_REG, 1);
	sn3199_write_reg(SN3199_T0_OUT9_RGB3_B_REG, 2);//t0

	sn3199_write_reg(SN3199_T1_T2_T3_RGB1_REG, (1<<SN3199_T2_OFFSET_BIT) | (7<<SN3199_T1_T3_OFFSET_BIT));
	sn3199_write_reg(SN3199_T1_T2_T3_RGB2_REG, (1<<SN3199_T2_OFFSET_BIT) | (7<<SN3199_T1_T3_OFFSET_BIT));
	sn3199_write_reg(SN3199_T1_T2_T3_RGB3_REG, (1<<SN3199_T2_OFFSET_BIT) | (7<<SN3199_T1_T3_OFFSET_BIT));

	sn3199_write_reg(SN3199_T4_OUT1_RGB1_R_REG, 2);//t4
	sn3199_write_reg(SN3199_T4_OUT2_RGB1_G_REG, 2);
	sn3199_write_reg(SN3199_T4_OUT3_RGB1_B_REG, 2);
	
	sn3199_write_reg(SN3199_T4_OUT4_RGB2_R_REG, 2);
	sn3199_write_reg(SN3199_T4_OUT5_RGB2_G_REG, 2);
	sn3199_write_reg(SN3199_T4_OUT6_RGB2_B_REG, 2);
	
	sn3199_write_reg(SN3199_T4_OUT7_RGB3_R_REG, 2);
	sn3199_write_reg(SN3199_T4_OUT8_RGB3_G_REG, 2);
	sn3199_write_reg(SN3199_T4_OUT9_RGB3_B_REG, 2);//t4
	
	
	sn3199_write_reg(SN3199_PWM_DATA_REFRESH_REG, 0x00);//refresh data	
	sn3199_write_reg(SN3199_TIME_REFRESH_REG, 0x00);//refresh timmer

}






static int  sn3199_blink_set(struct led_classdev *led_cdev,
									unsigned long *delay_on,
									unsigned long *delay_off)
{
	LEDS_DEBUG("[LED]%s delay_on=0x%x, delay_off=0x%x\n", __func__,*delay_on,*delay_off);

	struct sn3199_leds_priv *led_data =
		container_of(led_cdev, struct sn3199_leds_priv, cdev);


	if (*delay_on != led_data->delay_on || *delay_off != led_data->delay_off) {
		led_data->delay_on = *delay_on;
		led_data->delay_off = *delay_off;

	
	}
	
	return 0;
}


/******************************
*func:sn3199_proc_backlight_value

level:
 D[7-0] -> B(8bit)
 D[15-8] -> G(8bit)
 D[23-16] -> R(8bit)
 D[27-24] -> Current(3bit)
 D[31-28] -> multi rgbleds(4bit)
 0x f f ff ff ff
     | | | |  |---B
     | | | |-----G
     | | |-------R
     | |---------Current
     |-----------multi rgbleds D[28]:rgb1; D[29]:rgb2; D[30]:rgb3; D[31]:turn on led
 Example:
 set rgb1 as red; turn on rgb1; current set as 20mA
 level=0x90FF0000;
 set rgb2 as green; turn on rgb2; current set as 20mA
 level=0xa000FF00;
 set rgb3 as blue; turn on rgb3; current set as 20mA
 level=0xc00000FF;
 set rgb1 as green, set rgb2 as blue, set rgb3 as red, turn on rgb1/rgb2/rgb3; current set as 20mA
 level=0x1000FF00;
 level=0x200000FF;
 level=0x40FF0000;
 level=0x80000000;//on this situation the rgb value can not care; If not set D[31] as 1, then rgbled can not turn on; 

delay_on:every rgbled need itself T0 T1 and T3
 D[5-0] -> T0_Blue(6bit)
 D[11-6] -> T0_Green(6bit)
 D[17-12] -> T0_Red(6bit)

 D[20-18] -> T1_T3(3bit)
 D[23-21] -> T2(3bit)
 
delay_off:every regled need itself T4 
 D[5-0] -> T4_Blue(6bit)
 D[11-6] -> T4_Green(6bit)
 D[17-12] -> T4_Red(6bit)

period of time
			    ______			    ______
			   |		 |			   |		 |
	               |		  |			  |		  |
	              |         	   |                |         	   |
	             |                 |              |                  |
	            |                   |            |                    |
__________|                     |______|                      |______

           T0   |T1 |  T2    |T3|   T4    | T1|    T2  | T3|  T4    	
 
*******************************/
static void sn3199_proc_backlight_value(struct sn3199_leds_priv * leds_data,int level, int delay_on, int delay_off)
{
	int turn_on,rgb_1,rgb_2,rgb_3,led_cur,pwm_data;
	int pwm_rgb1_g,pwm_rgb1_r,pwm_rgb1_b,pwm_rgb2_g,pwm_rgb2_r,pwm_rgb2_b,pwm_rgb3_g,pwm_rgb3_r,pwm_rgb3_b;
	int t0_rgb1_g,t0_rgb1_r,t0_rgb1_b,t0_rgb2_g,t0_rgb2_r,t0_rgb2_b,t0_rgb3_g,t0_rgb3_r,t0_rgb3_b;
	int t1_t3_rgb1,t1_t3_rgb2,t1_t3_rgb3,t2_rgb1,t2_rgb2,t2_rgb3;
	int t4_rgb1_g,t4_rgb1_r,t4_rgb1_b,t4_rgb2_g,t4_rgb2_r,t4_rgb2_b,t4_rgb3_g,t4_rgb3_r,t4_rgb3_b;
	int ae;
	
	struct sn3199_leds_priv * led_data = NULL;
	if(leds_data==NULL){
		printk("%s ERROR leds_data is NULL \n",__func__);
		return;
	}
	led_data = leds_data;
	
	led_cur = (level>>SN3199_BL_CUR_OFFSET_BIT)&0x7;
	if(led_cur<1)
		led_cur=1;
	if(led_cur>3)
		led_cur=3;
	turn_on = (level>>SN3199_BL_TURN_ON_OFFSET_BIT)&0x1;
	rgb_1 = (level>>SN3199_BL_RGB_1_OFFSET_BIT)&0x1;
	rgb_2 = (level>>SN3199_BL_RGB_2_OFFSET_BIT)&0x1;
	rgb_3 = (level>>SN3199_BL_RGB_3_OFFSET_BIT)&0x1;
	
	pwm_data = (level>>SN3199_BL_PWM_OFFSET_BIT)&0xffffff;

	ae = led_data->flash_music;
	printk("[JX] %s led_cur:%d, turn_on:%d, rgb1:%d, rgb2:%d, rgb3:%d.\n",__func__,led_cur,turn_on,rgb_1,rgb_2,rgb_3);
printk("[JX] %s, pwm_data:0x%x, delay_on:0x%x, delay_off:ox%x\n",__func__,pwm_data,delay_on,delay_off);
	if(rgb_1){
		led_data->rgb_1 = pwm_data;
		led_data->rgb1_on = delay_on;
		led_data->rgb1_off = delay_off;
	}
	if(rgb_2){
		led_data->rgb_2 = pwm_data;
		led_data->rgb2_on = delay_on;
		led_data->rgb2_off = delay_off;
	}
	if(rgb_3){
		led_data->rgb_3 = pwm_data;
		led_data->rgb3_on = delay_on;
		led_data->rgb3_off = delay_off;
	}
	if(turn_on){
		pwm_rgb1_g = (led_data->rgb_1>>SN3199_BL_G_OFFSET_BIT)&SN3199_BL_RGB_MASK;
		pwm_rgb1_r = (led_data->rgb_1>>SN3199_BL_R_OFFSET_BIT)&SN3199_BL_RGB_MASK;
		pwm_rgb1_b = (led_data->rgb_1>>SN3199_BL_B_OFFSET_BIT)&SN3199_BL_RGB_MASK;
		pwm_rgb2_g = (led_data->rgb_2>>SN3199_BL_G_OFFSET_BIT)&SN3199_BL_RGB_MASK;
		pwm_rgb2_r = (led_data->rgb_2>>SN3199_BL_R_OFFSET_BIT)&SN3199_BL_RGB_MASK;
		pwm_rgb2_b = (led_data->rgb_2>>SN3199_BL_B_OFFSET_BIT)&SN3199_BL_RGB_MASK;
		pwm_rgb3_g = (led_data->rgb_3>>SN3199_BL_G_OFFSET_BIT)&SN3199_BL_RGB_MASK;
		pwm_rgb3_r = (led_data->rgb_3>>SN3199_BL_R_OFFSET_BIT)&SN3199_BL_RGB_MASK;
		pwm_rgb3_b = (led_data->rgb_3>>SN3199_BL_B_OFFSET_BIT)&SN3199_BL_RGB_MASK;

		t0_rgb1_g = (led_data->rgb1_on>>SN3199_BL_T0_G_OFFSET_BIT)&SN3199_BL_T0_MASK;
		t0_rgb1_r = (led_data->rgb1_on>>SN3199_BL_T0_R_OFFSET_BIT)&SN3199_BL_T0_MASK;
		t0_rgb1_b = (led_data->rgb1_on>>SN3199_BL_T0_B_OFFSET_BIT)&SN3199_BL_T0_MASK;
		t0_rgb2_g = (led_data->rgb2_on>>SN3199_BL_T0_G_OFFSET_BIT)&SN3199_BL_T0_MASK;
		t0_rgb2_r = (led_data->rgb2_on>>SN3199_BL_T0_R_OFFSET_BIT)&SN3199_BL_T0_MASK;
		t0_rgb2_b = (led_data->rgb2_on>>SN3199_BL_T0_B_OFFSET_BIT)&SN3199_BL_T0_MASK;
		t0_rgb3_g = (led_data->rgb3_on>>SN3199_BL_T0_G_OFFSET_BIT)&SN3199_BL_T0_MASK;
		t0_rgb3_r = (led_data->rgb3_on>>SN3199_BL_T0_R_OFFSET_BIT)&SN3199_BL_T0_MASK;
		t0_rgb3_b = (led_data->rgb3_on>>SN3199_BL_T0_B_OFFSET_BIT)&SN3199_BL_T0_MASK;

		t1_t3_rgb1 = (led_data->rgb1_on>>SN3199_BL_T1_T3_OFFSET_BIT)&SN3199_BL_T1_T3_MASK;
		t1_t3_rgb2 = (led_data->rgb2_on>>SN3199_BL_T1_T3_OFFSET_BIT)&SN3199_BL_T1_T3_MASK;
		t1_t3_rgb3 = (led_data->rgb3_on>>SN3199_BL_T1_T3_OFFSET_BIT)&SN3199_BL_T1_T3_MASK;

		t2_rgb1 = (led_data->rgb1_on>>SN3199_BL_T2_OFFSET_BIT)&SN3199_BL_T2_MASK;
		t2_rgb2 = (led_data->rgb2_on>>SN3199_BL_T2_OFFSET_BIT)&SN3199_BL_T2_MASK;
		t2_rgb3 = (led_data->rgb3_on>>SN3199_BL_T2_OFFSET_BIT)&SN3199_BL_T2_MASK;

		t4_rgb1_g = (led_data->rgb1_off>>SN3199_BL_T4_G_OFFSET_BIT)&SN3199_BL_T4_MASK;
		t4_rgb1_r = (led_data->rgb1_off>>SN3199_BL_T4_R_OFFSET_BIT)&SN3199_BL_T4_MASK;
		t4_rgb1_b = (led_data->rgb1_off>>SN3199_BL_T4_B_OFFSET_BIT)&SN3199_BL_T4_MASK;
		t4_rgb2_g = (led_data->rgb2_off>>SN3199_BL_T4_G_OFFSET_BIT)&SN3199_BL_T4_MASK;
		t4_rgb2_r = (led_data->rgb2_off>>SN3199_BL_T4_R_OFFSET_BIT)&SN3199_BL_T4_MASK;
		t4_rgb2_b = (led_data->rgb2_off>>SN3199_BL_T4_B_OFFSET_BIT)&SN3199_BL_T4_MASK;
		t4_rgb3_g = (led_data->rgb3_off>>SN3199_BL_T4_G_OFFSET_BIT)&SN3199_BL_T4_MASK;
		t4_rgb3_r = (led_data->rgb3_off>>SN3199_BL_T4_R_OFFSET_BIT)&SN3199_BL_T4_MASK;
		t4_rgb3_b = (led_data->rgb3_off>>SN3199_BL_T4_B_OFFSET_BIT)&SN3199_BL_T4_MASK;
		
		sn3199_write_reg(SN3199_SSD_EN_REG, SN3199_SSD_ENABLE);
		sn3199_write_reg(SN3199_OUT_CTL1_REG, 0x77);
		sn3199_write_reg(SN3199_OUT_CTL2_REG, 0x07);

		sn3199_write_reg(SN3199_SETTING2_REG, led_cur<<SN3199_CS_OFFSET_BIT);//current
		
		
		sn3199_write_reg(SN3199_PWM_OUT1_RGB1_R_REG, pwm_rgb1_r);
		sn3199_write_reg(SN3199_PWM_OUT2_RGB1_G_REG, pwm_rgb1_g);
		sn3199_write_reg(SN3199_PWM_OUT3_RGB1_B_REG, pwm_rgb1_b);
		
		sn3199_write_reg(SN3199_PWM_OUT4_RGB2_R_REG, pwm_rgb2_r);
		sn3199_write_reg(SN3199_PWM_OUT5_RGB2_G_REG, pwm_rgb2_g);
		sn3199_write_reg(SN3199_PWM_OUT6_RGB2_B_REG, pwm_rgb2_b);
		
		sn3199_write_reg(SN3199_PWM_OUT7_RGB3_R_REG, pwm_rgb3_r);
		sn3199_write_reg(SN3199_PWM_OUT8_RGB3_G_REG, pwm_rgb3_g);
		sn3199_write_reg(SN3199_PWM_OUT9_RGB3_B_REG, pwm_rgb3_b);

		
		if((t2_rgb1!=0) || (t2_rgb2!=0) || (t2_rgb3!=0)){
		sn3199_write_reg(SN3199_SETTING1_REG, SN3199_RGB_MODE_ENABLE | (ae<<SN3199_AE_OFFSET_BIT) );//PWM mode	and AE mode
		//t0~t4
		sn3199_write_reg(SN3199_T0_OUT1_RGB1_R_REG, t0_rgb1_r);//t0
		sn3199_write_reg(SN3199_T0_OUT2_RGB1_G_REG, t0_rgb1_g);
		sn3199_write_reg(SN3199_T0_OUT3_RGB1_B_REG, t0_rgb1_b);
		
		sn3199_write_reg(SN3199_T0_OUT4_RGB2_R_REG, t0_rgb2_r);
		sn3199_write_reg(SN3199_T0_OUT5_RGB2_G_REG, t0_rgb2_g);
		sn3199_write_reg(SN3199_T0_OUT6_RGB2_B_REG, t0_rgb2_b);
		
		sn3199_write_reg(SN3199_T0_OUT7_RGB3_R_REG, t0_rgb3_r);
		sn3199_write_reg(SN3199_T0_OUT8_RGB3_G_REG, t0_rgb3_g);
		sn3199_write_reg(SN3199_T0_OUT9_RGB3_B_REG, t0_rgb3_b);//t0
	
		sn3199_write_reg(SN3199_T1_T2_T3_RGB1_REG, (t2_rgb1<<SN3199_T2_OFFSET_BIT) | (t1_t3_rgb1<<SN3199_T1_T3_OFFSET_BIT));
		sn3199_write_reg(SN3199_T1_T2_T3_RGB2_REG, (t2_rgb2<<SN3199_T2_OFFSET_BIT) | (t1_t3_rgb2<<SN3199_T1_T3_OFFSET_BIT));
		sn3199_write_reg(SN3199_T1_T2_T3_RGB3_REG, (t2_rgb3<<SN3199_T2_OFFSET_BIT) | (t1_t3_rgb3<<SN3199_T1_T3_OFFSET_BIT));
	
		sn3199_write_reg(SN3199_T4_OUT1_RGB1_R_REG, t4_rgb1_r);//t4
		sn3199_write_reg(SN3199_T4_OUT2_RGB1_G_REG, t4_rgb1_g);
		sn3199_write_reg(SN3199_T4_OUT3_RGB1_B_REG, t4_rgb1_b);
		
		sn3199_write_reg(SN3199_T4_OUT4_RGB2_R_REG, t4_rgb2_r);
		sn3199_write_reg(SN3199_T4_OUT5_RGB2_G_REG, t4_rgb2_g);
		sn3199_write_reg(SN3199_T4_OUT6_RGB2_B_REG, t4_rgb2_b);
		
		sn3199_write_reg(SN3199_T4_OUT7_RGB3_R_REG, t4_rgb3_r);
		sn3199_write_reg(SN3199_T4_OUT8_RGB3_G_REG, t4_rgb3_g);
		sn3199_write_reg(SN3199_T4_OUT9_RGB3_B_REG, t4_rgb3_b);//t4
		}else{
			sn3199_write_reg(SN3199_SETTING1_REG, SN3199_RGB_MODE_DISABLE | (ae<<SN3199_AE_OFFSET_BIT));
		}
		
		sn3199_write_reg(SN3199_PWM_DATA_REFRESH_REG, 0x00);//refresh data
		sn3199_write_reg(SN3199_TIME_REFRESH_REG, 0x00);//refresh timmer
	
	}
	
}

#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
static void SN3199_PowerOff_Charging_RGB_LED(unsigned int level)
#else
void SN3193_PowerOff_Charging_RGB_LED(unsigned int level)
#endif
{
	int levels;
//return;
    if(!sn3199_is_init) {
        sn3199_init();
    }
	levels=level|(1<<SN3199_BL_TURN_ON_OFFSET_BIT)|(7<<SN3199_BL_CHANNEL_OFFSET_BIT);
#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
	printk("[JX] %s\n",__func__);	
#endif

	sn3199_proc_backlight_value(&g_sn3199_leds_data[0],level,0,0);

}
#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
// ms t2 for sn3199
const unsigned int multi_rgb_time_t2[] = {0,260,520,1040,2080,4160,8320,16640};
//t4 line 1 A=0~15, B=0
//t4 line 2 A=8~15, B=1
//t4 line 3 A=8~15, B=2
//t4 line 4 A=8~15, B=3
const unsigned int multi_rgb_time_t4[] = {0,260,520,780,1040,1300,1560,1820,2080,2340,2600,2860,3120,3380,3640,3900,/*0~15*/
											4160,4680,5200,5720,6240,6760,7280,7800,/*16~23*/
											8320,9360,10400,11440,12480,13520,14560,15600,/*24~31*/
											16640,18720,20800,22880,24960,27040,29120,31200/*32~39*/};
static void sn3199_value_preprocess(struct sn3199_leds_priv * leds_data)
{
	int color, delay_on, delay_off;
	int def_on,def_off;
	int level_time = 0;

	if(leds_data==NULL)
		return;
	
	color = leds_data->level;
	delay_on  = leds_data->delay_on;
	delay_off = leds_data->delay_off;
	if(delay_on==0 || delay_off==0){
		delay_on=0;
		delay_off=0;
	}
	printk("[JX] %s color=0x%x on=0x%x off=0x%x\n",__func__,color,delay_on,delay_off);

		//t2 and t4
		//multi_rgb [0]:rgb1; [1]:rgb2; [2]:rgb3; [3]:turn on rgb;
		//t0A=0000; t0B=00; t1=t3=001; t2=000~007;t4=00 0000~11 1111
		//default delay value
		def_on=0x40000;//D[5:0]=t0_blue; D[11-6]=t0_green; D[17:12]=t0_red; D[20:18]=t1=t3; D[23:21]=t2;
		def_off=0x00000;//D[5:0]=t4_blue; D[11:6]=t4_green; D[17:12]=t4_red;
		
		for(level_time=7;level_time>=0;level_time--){
			if(delay_on>=multi_rgb_time_t2[level_time])
				break;
		}
		delay_on= def_on | (level_time<<21) ;
		
		for(level_time=39;level_time>=0;level_time--){
			if((level_time>=31)&&(delay_off>=multi_rgb_time_t4[level_time])){
				level_time = (level_time-24)| (3<<4);
				break;
			}
			else if((level_time>=23)&&(delay_off>=multi_rgb_time_t4[level_time])){
				level_time = (level_time-16)| (2<<4);
				break;
			}
			else if((level_time>=15)&&(delay_off>=multi_rgb_time_t4[level_time])){
				level_time = (level_time-8)| (1<<4);
				break;
			}
			else if(delay_off>=multi_rgb_time_t4[level_time]){
				level_time = (level_time-0)| (0<<4);
				break;
			}
		}
		delay_off = def_off | (level_time<<12) | (level_time<<6) | (level_time<<0);

	if(!((color>>24)&0xff)){
		//if app not set SN3199_BL_TURN_ON_OFFSET_BIT and SN3199_BL_CHANNEL_OFFSET_BIT
		//we default set all channel on;
		leds_data->level=color|(1<<SN3199_BL_TURN_ON_OFFSET_BIT)|(7<<SN3199_BL_CHANNEL_OFFSET_BIT);
	}else{
		leds_data->level=color;
	}
	leds_data->delay_on=delay_on;
	leds_data->delay_off=delay_off;
}
#else
EXPORT_SYMBOL_GPL(SN3193_PowerOff_Charging_RGB_LED);
#endif

static void sn3199_led_work(struct work_struct *work)
{
	struct sn3199_leds_priv	*led_data =
		container_of(work, struct sn3199_leds_priv, work);

if((led_data->level)==0)
	sn3199_off(led_data);
else
#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
    {
	    sn3199_value_preprocess(led_data);
#endif
	sn3199_proc_backlight_value(led_data,led_data->level,led_data->delay_on,led_data->delay_off);
#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
    }
#endif
}

void sn3199_led_set(struct led_classdev *led_cdev,enum led_brightness value)
{
#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
	LEDS_DEBUG("[LED]%s value=0x%x\n", __func__,value);
#else
	LEDS_DEBUG("[LED]%s value=%d\n", __func__,value);
#endif

	struct sn3199_leds_priv *led_data =
		container_of(led_cdev, struct sn3199_leds_priv, cdev);
	

    if(sn3199_i2c_cilent == NULL) {
        printk("sn3199_i2c_cilent null\n");
        return;
    }
    cancel_work_sync(&led_data->work);
	led_data->level = value;
        
    if(!sn3199_is_init) {
        sn3199_init();
    }
    schedule_work(&led_data->work);
}

/*for factory test*/
static void sn3199_led_work_test(struct work_struct *work)
{
	struct sn3199_leds_priv	*led_data =
		container_of(work, struct sn3199_leds_priv, work);

if((led_data->level)==0)
	sn3199_off(led_data);
else
	sn3199_rgb_factory_test();
}

/*for factory test*/

void sn3199_led_set_test(struct led_classdev *led_cdev,enum led_brightness value)
{
	LEDS_DEBUG("[LED]%s value=%d\n", __func__,value);

	struct sn3199_leds_priv *led_data =
		container_of(led_cdev, struct sn3199_leds_priv, cdev);
	

    if(sn3199_i2c_cilent == NULL) {
        printk("sn3199_i2c_cilent null\n");
        return;
    }
    cancel_work_sync(&led_data->work);
	led_data->level = value;
        
    if(!sn3199_is_init) {
        sn3199_init();
    }
    schedule_work(&led_data->work);
}

static int  sn3199_leds_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
	int ret = -1;
#endif
	LEDS_DEBUG("[LED]%s\n", __func__);
	sn3199_i2c_cilent = client;

#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
    ret = sn3199_init();

    return ret;
#else
    sn3199_init();

    return 0;
#endif
 }

static int  sn3199_leds_i2c_remove(struct i2c_client *client)
{
   
   LEDS_DEBUG("[LED]%s\n", __func__);
    return 0;
}
static ssize_t sn3199_rgb_1_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	//struct led_classdev *led_cdev = dev_get_drvdata(dev);
	int rgb,on,off;
	rgb = g_sn3199_leds_data[0]->rgb_1;
	on = g_sn3199_leds_data[0]->rgb1_on;
	off =g_sn3199_leds_data[0]->rgb1_off; 
	return sprintf(buf, "%lu,%lu,%lu\n", rgb,on,off);
}

static ssize_t sn3199_rgb_1_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	//struct led_classdev *led_cdev = dev_get_drvdata(dev);
	int ret = -EINVAL;
	char *after;
	unsigned long state = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;

	if (count == size) {
		g_sn3199_leds_data[0]->rgb_1 = state;
		ret = count;
	}

	return ret;
}
static ssize_t sn3199_rgb_2_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	//struct led_classdev *led_cdev = dev_get_drvdata(dev);
	int rgb,on,off;
	rgb = g_sn3199_leds_data[0]->rgb_2;
	on = g_sn3199_leds_data[0]->rgb1_on;
	off =g_sn3199_leds_data[0]->rgb1_off; 
	return sprintf(buf, "%lu,%lu,%lu\n", rgb,on,off);

}

static ssize_t sn3199_rgb_2_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	//struct led_classdev *led_cdev = dev_get_drvdata(dev);
	int ret = -EINVAL;
	char *after;
	unsigned long state = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;

	if (count == size) {
		g_sn3199_leds_data[0]->rgb_2 = state;
		ret = count;
	}

	return ret;
}

static ssize_t sn3199_rgb_3_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	//struct led_classdev *led_cdev = dev_get_drvdata(dev);
	int rgb,on,off;
	rgb = g_sn3199_leds_data[0]->rgb_3;
	on = g_sn3199_leds_data[0]->rgb1_on;
	off =g_sn3199_leds_data[0]->rgb1_off; 
	return sprintf(buf, "%lu,%lu,%lu\n", rgb,on,off);

}

static ssize_t sn3199_rgb_3_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	//struct led_classdev *led_cdev = dev_get_drvdata(dev);
	int ret = -EINVAL;
	char *after;
	unsigned long state = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;

	if (count == size) {
		g_sn3199_leds_data[0]->rgb_3 = state;
		ret = count;
	}

	return ret;
}
static ssize_t sn3199_flash_music_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	//struct led_classdev *led_cdev = dev_get_drvdata(dev);
	int rgb;
	rgb = g_sn3199_leds_data[0]->flash_music;
	return sprintf(buf, "%lu\n", rgb);
}

static ssize_t sn3199_flash_music_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	//struct led_classdev *led_cdev = dev_get_drvdata(dev);
	int ret = -EINVAL;
	char *after;
	unsigned long state = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;

	if (count == size) {
		g_sn3199_leds_data[0]->flash_music = state;
		ret = count;
	}

	return ret;
}

static DEVICE_ATTR(rgb_1, 0777, sn3199_rgb_1_show, sn3199_rgb_1_store);
static DEVICE_ATTR(rgb_2, 0777, sn3199_rgb_2_show, sn3199_rgb_2_store);
static DEVICE_ATTR(rgb_3, 0777, sn3199_rgb_3_show, sn3199_rgb_3_store);
static DEVICE_ATTR(flash_music, 0777, sn3199_flash_music_show, sn3199_flash_music_store);

static void sn3199_add_arrts(struct led_classdev *led_cdev)
{
	int rc;

	if(device_create_file(led_cdev->dev, &dev_attr_rgb_1))
		printk("SN3199 ERROR for creat rgb_1");
	if(device_create_file(led_cdev->dev, &dev_attr_rgb_2))
		printk("SN3199 ERROR for creat rgb_2");
	if(device_create_file(led_cdev->dev, &dev_attr_rgb_3))
		printk("SN3199 ERROR for creat rgb_3");
	if(device_create_file(led_cdev->dev, &dev_attr_flash_music))
		printk("SN3199 ERROR for creat flash_music");
}

#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
static LENOVO_LED_CONTROL_FUNCS led_ctrl  = 
{
	.Charging_RGB_LED = SN3199_PowerOff_Charging_RGB_LED,
};

extern void lenovo_register_led_control(LENOVO_LED_CONTROL_FUNCS * ctrl);
#endif

static int __init sn3199_leds_platform_probe(struct platform_device *pdev)
{
	int ret=0;
	int i;
	LEDS_DEBUG("[LED]%s\n", __func__);
#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
	if(i2c_add_driver(&sn3199_i2c_driver))
	{
		printk("add i2c driver error %s\n",__func__);
		return -1;
	} 
	if(fake_ic)
		goto err;
#endif

	g_sn3199_leds_data[0] = kzalloc(sizeof(struct sn3199_leds_priv), GFP_KERNEL);
	if (!g_sn3199_leds_data[0]) {
		ret = -ENOMEM;
		goto err;
	}

	
	g_sn3199_leds_data[0]->cdev.name = LED_NAME;
	g_sn3199_leds_data[0]->cdev.brightness_set = sn3199_led_set;
	g_sn3199_leds_data[0]->cdev.max_brightness = 0x7fffffff;
	g_sn3199_leds_data[0]->cdev.blink_set = sn3199_blink_set;
	INIT_WORK(&g_sn3199_leds_data[0]->work, sn3199_led_work);
	g_sn3199_leds_data[0]->gpio = GPIO80;//GPIO_LED_EN;
	g_sn3199_leds_data[0]->level = 0;
	
	ret = led_classdev_register(&pdev->dev, &g_sn3199_leds_data[0]->cdev);
	if (ret)
		goto err;

	//for factory test
	g_sn3199_leds_data[1] = kzalloc(sizeof(struct sn3199_leds_priv), GFP_KERNEL);
	if (!g_sn3199_leds_data[1]) {
		ret = -ENOMEM;
		goto err;
	}

	g_sn3199_leds_data[1]->cdev.name = "test-led";
	g_sn3199_leds_data[1]->cdev.brightness_set = sn3199_led_set_test;
	g_sn3199_leds_data[1]->cdev.max_brightness = 0xff;
	INIT_WORK(&g_sn3199_leds_data[1]->work, sn3199_led_work_test);
	g_sn3199_leds_data[1]->gpio = GPIO80;//GPIO_LED_EN;
	g_sn3199_leds_data[1]->level = 0;
	
	ret = led_classdev_register(&pdev->dev, &g_sn3199_leds_data[1]->cdev);
	//end for factory test
	
	if (ret)
		goto err;
#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
	lenovo_register_led_control(&led_ctrl);
#else

	if(i2c_add_driver(&sn3199_i2c_driver))
	{
		printk("add i2c driver error %s\n",__func__);
		return -1;
	} 
	sn3199_add_arrts(&g_sn3199_leds_data[0]->cdev);
#endif

	return 0;
	
err:

	for (i = 1; i >=0; i--) {
			if (!g_sn3199_leds_data[i])
				continue;
			led_classdev_unregister(&g_sn3199_leds_data[i]->cdev);
			cancel_work_sync(&g_sn3199_leds_data[i]->work);
			kfree(g_sn3199_leds_data[i]);
			g_sn3199_leds_data[i] = NULL;
		}
#ifdef LENOVO_LED_COMPATIBLE_SUPPORT
	i2c_del_driver(&sn3199_i2c_driver);
#endif

	return ret;
}

static int sn3199_leds_platform_remove(struct platform_device *pdev)
{
	struct sn3199_leds_priv *priv = dev_get_drvdata(&pdev->dev);
	LEDS_DEBUG("[LED]%s\n", __func__);

	dev_set_drvdata(&pdev->dev, NULL);

	kfree(priv);

	i2c_del_driver(&sn3199_i2c_driver);

	return 0;
}


/***********************************************************************************
* please add platform device in mt_devs.c
*
************************************************************************************/
static int __init sn3199_leds_init(void)
{
	int ret;

	LEDS_DEBUG("[LED]%s\n", __func__);

	if (i2c_register_board_info(2, &sn3199_i2c_board_info, 1) !=0) {
		printk(" cann't register i2c %s\n",__func__);
		return -1;
	}

	ret = platform_driver_register(&sn3199_leds_platform_driver);
	if (ret)
	{
		printk("[LED]%s:drv:E%d\n", __func__,ret);
		return ret;
	}

	return ret;
}

static void __exit sn3199_leds_exit(void)
{
	LEDS_DEBUG("[LED]%s\n", __func__);

	i2c_del_driver(&sn3199_i2c_driver);

	platform_driver_unregister(&sn3199_leds_platform_driver);

	sn3199_is_init = 0;
}

module_param(debug_enable, int,0644);

module_init(sn3199_leds_init);
module_exit(sn3199_leds_exit);

MODULE_AUTHOR("jixu@lenovo.com");
MODULE_DESCRIPTION("sn3199 led driver");
MODULE_LICENSE("GPL");



