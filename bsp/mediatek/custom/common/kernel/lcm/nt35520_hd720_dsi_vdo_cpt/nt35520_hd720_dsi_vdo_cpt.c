#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)
#define LCM_ID (0x20)
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
#define REGFLAG_DELAY             							0xFE
#define REGFLAG_END_OF_TABLE      							0x00   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0
#define LCM_BACKLIGHT_EN GPIO116
#define LCM_ID_PIN GPIO92
#define LCM_COMPARE_BY_SW 1
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    
#define dsi_set_cmdq_V4(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq_V4(pdata, queue_size, force_update)

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static unsigned int lcm_cabcmode_index = 3;

static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/

	
	
//PAGE0
{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
{0xFF,4,{0xAA,0x55,0xA5,0x80}},
{0x6F,1,{0x13}},
{0xF7,1,{0x00}},
{0x6F,1,{0x01}},
{0xF3,1,{0x00}},
{0x6F,1,{0x08}},
{0xFC,1,{0x10}},
{0x6F,1,{0x02}},
{0xB8,1,{0x08}},
{0xBB,2,{0x74,0x44}},
{0xBC,2,{0x00,0x00}},
{0xB6,1,{0x08}},
{0xB1,2,{0xFA,0x21}},
{0xD9,2,{0x02,0x04}},//pwm //48k
{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},

//PAGE1
{0xB3,2,{0x32,0x32}},
{0xB4,2,{0x0F,0x0F}},
{0xB9,2,{0x53,0x53}},
{0xBA,2,{0x23,0x23}},
{0xB5,2,{0x05,0x05}},
{0xC0,1,{0x04}},
{0xBC,2,{0x90,0x01}},
{0xBD,2,{0x90,0x01}},
{0xBE,1,{0x6B}},
{0xCA,1,{0x00}},

//PAGE2

{0xF0,5,{0x55,0xAA,0x52,0x08,0x02}},
{0xEE,1,{0x01}},
{0xB0,16,{0x00,0x00,0x00,0x18,0x00,0x32,0x00,0x4E,0x00,0x60,0x00,0x81,0x00,0x9F,0x00,0xCE}},

{0xB1,16,{0x00,0xF8,0x01,0x3B,0x01,0x72,0x01,0xCF,0x02,0x19,0x02,0x1C,0x02,0x5E,0x02,0xA7}},

{0xB2,16,{0x02,0xD2,0x03,0x0D,0x03,0x32,0x03,0x60,0x03,0x80,0x03,0x9F,0x03,0xBF,0x03,0xE8}},

{0xB3,4,{0x03,0xFC,0x03,0xFF}},
//{0xB0,16,{0x00,0x00,0x00,0x18, 0x00,0x32,0x00,0x4E, 0x00,0x60,0x00,0x81, 0x00,0x9F,0x00,0xCE}},
                            
//{0xB1,16,{0x00,0xF8,0x01,0x3B, 0x01,0x72,0x01,0xCF, 0x02,0x19,0x02,0x1C, 0x02,0x5E,0x02,0xA7}},
                        
//{0xB2,16,{0x02,0xD2,0x03,0x0D, 0x03,0x32,0x03,0x60, 0x03,0x80,0x03,0x9F, 0x03,0xBF,0x03,0xE8}},
                        
//{0xB3,4,{0x03,0xFC,0x03,0xFF}},
//end puls
{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
{0x6F,1,{0x02}},
{0xBD,1,{0x0F}},



// GOA// Page3 

{0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
{0xB0,2,{0x20,0x00}},
{0xB1,2,{0x20,0x00}},
{0xB2,5,{0x05,0x00,0x17,0x00,0x00}},
{0xB3,5,{0x05,0x00,0x17,0x00,0x00}},
{0xB4,5,{0x05,0x00,0x17,0x00,0x00}},
{0xB5,5,{0x05,0x00,0x17,0x00,0x00}},
{0xB6,5,{0x12,0x00,0x19,0x00,0x00}},
{0xB7,5,{0x12,0x00,0x19,0x00,0x00}},
{0xB8,5,{0x12,0x00,0x19,0x00,0x00}},
{0xB9,5,{0x12,0x00,0x19,0x00,0x00}},
{0xBA,5,{0x53,0x00,0x1A,0x00,0x00}},
{0xBB,5,{0x53,0x00,0x1A,0x00,0x00}},
{0xBC,5,{0x53,0x10,0x1A,0x00,0x00}},
{0xBD,5,{0x53,0x10,0x1A,0x00,0x00}},
{0xC0,4,{0x00,0x34,0x00,0x00}},
{0xC1,4,{0x00,0x00,0x34,0x00}},
{0xC2,4,{0x00,0x00,0x34,0x00}},
{0xC3,4,{0x00,0x00,0x34,0x00}},
{0xC4,1,{0x60}},
{0xC5,1,{0x40}},
{0xC6,1,{0x40}},
{0xC7,1,{0x00}},
{0xEF,1,{0x00}},

// GOA// Page5

{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
{0xB0,2,{0x12,0x01}},
{0xB1,2,{0x12,0x01}},
{0xB2,2,{0x12,0x01}},
{0xB3,2,{0x12,0x01}},
{0xB4,2,{0x12,0x01}},
{0xB5,2,{0x12,0x01}},
{0xB6,2,{0x12,0x01}},
{0xB7,2,{0x12,0x01}},
{0xB8,1,{0x0C}},
{0xB9,1,{0x00}},
{0xBA,1,{0x00}},
{0xBB,1,{0x0A}},
{0xBC,1,{0x02}},
{0xBD,5,{0x0F,0x0F,0x0F,0x0F,0x0F}},
{0xC0,1,{0x07}},
{0xC1,1,{0x05}},
{0xC2,1,{0x82}},
{0xC3,1,{0x80}},
{0xC4,1,{0x00}},
{0xC5,1,{0x02}},
{0xC6,1,{0x22}},
{0xC7,1,{0x03}},
{0xC8,2,{0x03,0x20}},
{0xC9,2,{0x01,0x21}},
{0xCA,2,{0x01,0x60}},
{0xCB,2,{0x01,0x60}},
{0xCC,3,{0x00,0x00,0x02}},
{0xCD,3,{0x00,0x00,0x02}},
{0xCE,3,{0x00,0x00,0x02}},
{0xCF,3,{0x00,0x00,0x02}},
{0xD0,1,{0x00}},
{0xD1,11,{0x01,0x00,0x3D,0x07,0x10,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD2,11,{0x11,0x00,0x41,0x03,0x10,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD3,11,{0x20,0x00,0x43,0x07,0x10,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD4,11,{0x30,0x00,0x43,0x07,0x10,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xD5,5,{0x00,0x00,0x00,0x00,0x00}},
{0xD6,5,{0x00,0x00,0x00,0x00,0x00}},
{0xD7,5,{0x00,0x00,0x00,0x00,0x00}},
{0xD8,5,{0x00,0x00,0x00,0x00,0x00}},
{0xE5,1,{0x06}},
{0xE6,1,{0x06}},
{0xE7,1,{0x00}},
{0xE8,1,{0x06}},
{0xE9,1,{0x06}},
{0xEA,1,{0x06}},
{0xEB,1,{0x00}},
{0xEC,1,{0x00}},
{0xED,1,{0x33}},
{0xEE,1,{0x01}},

//SETPAGE 6 GOUT// Mapping  Page6  GOA Setting 

//GOUT Mapping
{0xF0,5,{0x55,0xAA,0x52,0x08,0x06}},
{0xB0,2,{0x00,0x02}},
{0xB1,2,{0x10,0x16}},
{0xB2,2,{0x12,0x18}},
{0xB3,2,{0x2F,0x2F}},
{0xB4,2,{0x2F,0x34}},
{0xB5,2,{0x34,0x34}},
{0xB6,2,{0x34,0x2F}},
{0xB7,2,{0x2F,0x2F}},
{0xB8,2,{0x04,0x06}},
{0xB9,2,{0x2D,0x2E}},
{0xBA,2,{0x2E,0x2D}},
{0xBB,2,{0x07,0x05}},
{0xBC,2,{0x2F,0x2F}},
{0xBD,2,{0x2F,0x34}},
{0xBE,2,{0x34,0x34}},
{0xBF,2,{0x34,0x2F}},
{0xC0,2,{0x2F,0x2F}},
{0xC1,2,{0x19,0x13}},
{0xC2,2,{0x17,0x11}},
{0xC3,2,{0x03,0x01}},
{0xC4,2,{0x07,0x05}},
{0xC5,2,{0x19,0x13}},
{0xC6,2,{0x17,0x11}},
{0xC7,2,{0x2F,0x2F}},
{0xC8,2,{0x2F,0x34}},
{0xC9,2,{0x34,0x34}},
{0xCA,2,{0x34,0x2F}},
{0xCB,2,{0x2F,0x2F}},
{0xCC,2,{0x03,0x01}},
{0xCD,2,{0x2E,0x2D}},
{0xCE,2,{0x2D,0x2E}},
{0xCF,2,{0x00,0x02}},
{0xD0,2,{0x2F,0x2F}},
{0xD1,2,{0x2F,0x34}},
{0xD2,2,{0x34,0x34}},
{0xD3,2,{0x34,0x2F}},
{0xD4,2,{0x2F,0x2F}},
{0xD5,2,{0x10,0x16}},
{0xD6,2,{0x12,0x18}},
{0xD7,2,{0x04,0x06}},
{0xD8,5,{0x00,0x00,0x00,0x00,0x00}},
{0xD9,5,{0x00,0x00,0x00,0x00,0x00}},
{0xE5,2,{0x2F,0x2F}},
{0xE6,2,{0x2F,0x2F}},
{0xE7,1,{0x00}},
//{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
// sleep out // display on//
//{0x36,1,{0xc8}},
{0x35,1,{0x00}},
{0x53,1,{0x24}},
{0x55,1,{0x03}},//cabc 03:move 00:off
//{0x51,1,{0xff}},
// RAM address 解析度 // 
{0x2A,4,{0x00,0x00,0x02,0xCF}},
{0x2B,4,{0x00,0x00,0x04,0xFF}},
{0x11,0,{0x00}},
{REGFLAG_DELAY,  120,{}},
{0x29,0,{0x00}},


	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.


	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};



static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
    {REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_in_setting[] = {
	// Display off sequence
	{0x28, 0, {0x00}},
	{REGFLAG_DELAY, 10, {}},

    // Sleep Mode On
	{0x10, 0, {0x00}},
	{REGFLAG_DELAY, 150, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};





static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
				//UDELAY(5);//soso add or it will fail to send register
       	}
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	unsigned int div1 = 0;
	unsigned int div2 = 0;
	unsigned int pre_div = 0;
	unsigned int post_div = 0;
	unsigned int fbk_sel = 0;
	unsigned int fbk_div = 0;
	unsigned int lane_no = 0;//lcm_params->dsi.LANE_NUM;

	unsigned int cycle_time;
	unsigned int ui;
	unsigned int hs_trail_m, hs_trail_n;

		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = BURST_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 0;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=720*3;	

		
//		params->dsi.compatibility_for_nvk	= 1;

		params->dsi.vertical_sync_active				= 5;
		params->dsi.vertical_backporch					= 30;
		params->dsi.vertical_frontporch					= 30;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 5;
		params->dsi.horizontal_backporch				= 30;
		params->dsi.horizontal_frontporch				= 30;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
		params->dsi.pll_div1=0;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)
		params->dsi.fbk_div =16;    //fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
		
		//params->dsi.pll_select = 1;//lenovo jixu add for frehopping
		params->dsi.hs_read = 1;//need hs read
#if 1
		params->bl_app.min =1;
		params->bl_app.def =102;
		params->bl_app.max =255;

		params->bl_bsp.min =7;
		params->bl_bsp.def =102;
		params->bl_bsp.max =255;
#endif
#if 1
        div1 = params->dsi.pll_div1;
        div2 = params->dsi.pll_div2;
        fbk_div = params->dsi.fbk_div;
		switch(div1)
   		{
	  		case 0:
		 	div1 = 1;
		 	break;

			case 1:
		 	div1 = 2;
		 	break;

			case 2:
			case 3:
			 div1 = 4;
			 break;

			default:
				div1 = 4;
				break;
		}

		switch(div2)
		{
			case 0:
				div2 = 1;
				break;
			case 1:
				div2 = 2;
				break;
			case 2:
			case 3:
				div2 = 4;
				break;
			default:
				div2 = 4;
				break;
		}

		switch(pre_div)
		{
		  case 0:
			 pre_div = 1;
			 break;

		  case 1:
			 pre_div = 2;
			 break;

		  case 2:
		  case 3:
			 pre_div = 4;
			 break;

		  default:
			 pre_div = 4;
			 break;
		}

		switch(post_div)
		{
		  case 0:
			 post_div = 1;
			 break;

		  case 1:
			 post_div = 2;
			 break;

		  case 2:
		  case 3:
			 post_div = 4;
			 break;

		  default:
			 post_div = 4;
			 break;
		}

		switch(fbk_sel)
		{
		  case 0:
			 fbk_sel = 1;
			 break;

		  case 1:
			 fbk_sel = 2;
			 break;

		  case 2:
		  case 3:
			 fbk_sel = 4;
			 break;

		  default:
			 fbk_sel = 4;
			 break;
		}
  		cycle_time=(1000*4*div2*div1)/(fbk_div*26)+0x01;

		ui=(1000*div2*div1)/(fbk_div*26*0x2)+0x01;
#define NS_TO_CYCLE(n, c)	((n) / (c))

   hs_trail_m=1;
   hs_trail_n= NS_TO_CYCLE(((hs_trail_m * 0x4) + 0x60 - 75), cycle_time);
   // +3 is recommended from designer becauase of HW latency
   params->dsi.HS_TRAIL = ((hs_trail_m > hs_trail_n) ? hs_trail_m : hs_trail_n);

   params->dsi.HS_PRPR	=  NS_TO_CYCLE((0x40 + 0x5 * ui + 25), cycle_time);
   // HS_PRPR can't be 1.
   if (params->dsi.HS_PRPR == 0)
	  params->dsi.HS_PRPR = 1;

#endif

}

static unsigned int lcm_compare_id(void);
static void lcm_setbacklight(unsigned int level);
static bool esd_check=false;

static void lcm_init(void)
{

	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(5);
	SET_RESET_PIN(1);
	MDELAY(10);


	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
mt_set_gpio_out(LCM_BACKLIGHT_EN, 1);


}


static void lcm_suspend(void)
{
	unsigned int data_array[16];

#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, \n", __func__);
#else
	printk("%s, \n", __func__);
#endif	
mt_set_gpio_out(LCM_BACKLIGHT_EN, 0);//diable backlight ic
		data_array[0] = 0x00280500; 			   
		dsi_set_cmdq(&data_array, 1, 1);	
		MDELAY(20);
		data_array[0] = 0x00100500; 			   
		dsi_set_cmdq(&data_array, 1, 1); 
		MDELAY(150); 
}


static void lcm_resume(void)
{
int i;
unsigned int data_array[16];

#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, \n", __func__);
#else
	printk("%s, \n", __func__);
#endif

#if 0
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(50);

	lcm_init_register();

		mt_set_gpio_out(LCM_BACKLIGHT_EN, 1);

#else
	data_array[0] = 0x00110500; 			   
	dsi_set_cmdq(&data_array, 1, 1); 
	MDELAY(150); 
	
	data_array[0] = 0x00290500; 			   
	dsi_set_cmdq(&data_array, 1, 1);	
	MDELAY(5); 
#endif
mt_set_gpio_out(LCM_BACKLIGHT_EN, 1);

}
static unsigned int lcm_compare_id(void)
{
//return 1;
#if LCM_COMPARE_BY_SW
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int data_array[16];  

	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(50);	

/*	
	data_array[0] = 0x00110500;		// Sleep Out
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);
*/
		
//*************Enable CMD2 Page1  *******************//
	data_array[0]=0x00063902;
	data_array[1]=0x52AA55F0;
	data_array[2]=0x00000108;
	dsi_set_cmdq(data_array, 3, 1);
	MDELAY(10); 

	data_array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10); 
	
	read_reg_v2(0xC5, buffer, 2);

	id = buffer[1];
#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, nt35520 id = 0x%x \n", __func__, id);
#else
       printk("%s, nt35520 id = 0x%x \n", __func__, id);
#endif

	return (LCM_ID == id)?1:0;
#else
	unsigned int ret = 0;
	ret = mt_get_gpio_in(LCM_ID_PIN);
#if defined(BUILD_LK)
	printf("%s, [jx]nt35520 cpt LCM_ID_PIN = %d \n", __func__, ret);
#endif	

	return (ret == 1)?1:0;
#endif
}

static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	//data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	//dsi_set_cmdq(data_array, 1, 1);
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
static unsigned int lcm_esd_check(void)
{
unsigned char buffer[2],ret;

#ifndef BUILD_UBOOT
/*	if(lcm_esd_test)
	{
	lcm_esd_test = FALSE;
	return TRUE;
	}
*/
	/// please notice: the max return packet size is 1
	/// if you want to change it, you can refer to the following marked code
	/// but read_reg currently only support read no more than 4 bytes....
	/// if you need to read more, please let BinHan knows.
	/*
	unsigned int data_array[16];
	unsigned int max_return_size = 1;

	data_array[0]= 0x00003700 | (max_return_size << 16); 

	dsi_set_cmdq(&data_array, 1, 1);
	*/
	read_reg_v2(0x0A, buffer,2);
	#ifndef BUILD_LK
	//printk("[JX] %s 0x0A 0=0x%x 1=0x%x \n",__func__,buffer[0],buffer[1]);
	#endif
	ret = buffer[0]==0x9C?0:1;
	#ifndef BUILD_LK
	//printk("[JX] %s ret=%d \n",__func__,ret);
	#endif
	if(ret){
		esd_check = true;
		return TRUE;
	}

	read_reg_v2(0x0E, buffer,2);
	#ifndef BUILD_LK
	//printk("[JX] %s 0x0E 0=0x%x 1=0x%x \n",__func__,buffer[0],buffer[1]);
	#endif
	ret = ((buffer[0])&(0xf0))==0x80?0:1;
	#ifndef BUILD_LK
	//printk("[JX] %s ret=%d \n",__func__,ret);
	#endif
	if(ret){
		esd_check = true;
		return TRUE;
	}
	else return FALSE;
#endif
}

static unsigned int lcm_esd_recover(void)
{
#ifndef BUILD_LK
	printk("[JX]+ %s \n",__func__);
#endif
	lcm_init();
#ifndef BUILD_LK
	printk("[JX]- %s \n",__func__);
#endif
}
static void lcm_setbacklight(unsigned int level)
{
	unsigned int data_array[16];

//return;
	if(level > 255) 
	level = 255;

	data_array[0] = 0x00023902;      
	data_array[1] = (0x51|(level<<8)); 
#ifdef BUILD_LK
	dsi_set_cmdq_V4(&data_array, 2, 1); 
#else
	dsi_set_cmdq(&data_array, 2, 1); 
#endif

}
static void lcm_setcabcmode(unsigned int mode)
{
	unsigned int data_array[16];

	#if BUILD_LK
	printf("%s mode=%d\n",__func__,mode);
	#else
	printk("%s mode=%d\n",__func__,mode);
	#endif

	switch(mode){
		case 0:
			lcm_cabcmode_index=0;
			break;
		case 1:
			lcm_cabcmode_index=1;
			break;
		case 2:
			lcm_cabcmode_index=3;
			break;
		default:
			break;
	}
	data_array[0] = 0x00023902;      
	data_array[1] = (0x55|(lcm_cabcmode_index<<8)); 
	dsi_set_cmdq(&data_array, 2, 1); 

	 MDELAY(10);
}



static void lcm_getcabcstate(unsigned int * state)
{

	if(lcm_cabcmode_index == 0){
		*state = 0;
	}else if(lcm_cabcmode_index == 1){
		*state = 1;
	}else{
		*state = 2;
	}
	return;
}


LCM_DRIVER nt35520_hd720_dsi_vdo_cpt_lcm_drv = 
{
    .name			= "nt35520_hd720_dsi_vdo_cpt",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
        .update         = lcm_update,
#endif
	.esd_check   = lcm_esd_check,
  .esd_recover   = lcm_esd_recover,
  .set_backlight	= lcm_setbacklight,
  
#ifdef LENOVO_LCD_BACKLIGHT_CONTROL_BY_LCM
  	.set_cabcmode = lcm_setcabcmode,
	.get_cabcmode = lcm_getcabcstate,
#endif
};

