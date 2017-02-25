
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

#define FRAME_WIDTH  				(720)
#define FRAME_HEIGHT 				(1280)

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//#define LCM_DSI_CMD_MODE			0

#define LCM_ID    0x1283
#if 0//ined(LENOVO_PROJECT_SCOFIELD) || defined(LENOVO_PROJECT_SCOFIELDTD)
#define LCM_BACKLIGHT_EN GPIO18
#define LCM_ID_PIN GPIO114
#define LCM_COMPARE_BY_SW 1
#else
#define LCM_BACKLIGHT_EN GPIO116
#define LCM_ID_PIN GPIO92
#define LCM_COMPARE_BY_SW 1
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
static unsigned int lcm_esd_test = FALSE; ///only for ESD test

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    	(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 		(lcm_util.udelay(n))
#define MDELAY(n) 		(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq_V4(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq_V4(pdata, queue_size, force_update)

#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)						lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)			lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) 						lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   		lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    
static unsigned int lcm_cabcmode_index = 3;
       
static unsigned int lcm_compare_id(void);

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

		params->dsi.vertical_sync_active				= 2;
		params->dsi.vertical_backporch					= 14;
		params->dsi.vertical_frontporch					= 16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 2;
		params->dsi.horizontal_backporch				= 34;
		params->dsi.horizontal_frontporch				= 34;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
		params->dsi.pll_div1=0;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)
		params->dsi.fbk_div =16;    //fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)

//		params->dsi.HS_PRPR = 4;
//		params->dsi.pll_select=1;
		params->dsi.hs_read = 1;//need hs read
#if 1
		params->bl_app.min =1;
		params->bl_app.def =102;
		params->bl_app.max =255;

		params->bl_bsp.min =5;
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


static void lcm_init_register(void)
{
	unsigned int data_array[16];

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00043902; //EXTC=1
    	data_array[1] = 0x018312FF;                 
    	dsi_set_cmdq(&data_array, 2, 1);    

    	data_array[0] = 0x00023902; //ORISE mode enable
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x008312FF;    
    	dsi_set_cmdq(&data_array, 2, 1);    

//enable inverse mode 
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00008300;
    	dsi_set_cmdq(&data_array, 2, 1);
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000041B3;
    	dsi_set_cmdq(&data_array, 2, 1);
//enable end
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000a3902; //10
    	data_array[1] = 0x006400C0;//0x006400C0;
		data_array[2] = 0x6400110F;
		data_array[3] = 0x0000110F;
    	dsi_set_cmdq(&data_array, 4, 1);
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00073902; //07
    	data_array[1] = 0x005C00C0;
		data_array[2] = 0x00040001;
    	dsi_set_cmdq(&data_array, 3, 1);
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000A400;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000000C0; 
    	dsi_set_cmdq(&data_array, 2, 1); 

		data_array[0] = 0x00023902;
    	data_array[1] = 0x00008700;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000018C4; 
    	dsi_set_cmdq(&data_array, 2, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B300;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x005000c0; //0x005000c0
    	dsi_set_cmdq(&data_array, 2, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00008100;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000066C1;//55=60Hz ;66=65Hz
    	dsi_set_cmdq(&data_array, 2, 1); 

		data_array[0] = 0x00023902;
    	data_array[1] = 0x00008100;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000082C4; 
    	dsi_set_cmdq(&data_array, 2, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00008200;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000002C4; 
    	dsi_set_cmdq(&data_array, 2, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000049C4; 
    	dsi_set_cmdq(&data_array, 2, 1);

		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000C600;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000003B0; 
    	dsi_set_cmdq(&data_array, 2, 1); 		
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00053902; //5
    	data_array[1] = 0x021102F5;
		data_array[2] = 0x00000011;
	   	dsi_set_cmdq(&data_array, 3, 1);
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000050C5; 
    	dsi_set_cmdq(&data_array, 2, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00009400;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000055C5; //power ic frequence 650KHz
    	dsi_set_cmdq(&data_array, 2, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00009400;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000002F5; 
    	dsi_set_cmdq(&data_array, 2, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000BA00;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000003F5; 
    	dsi_set_cmdq(&data_array, 2, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B200;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x000000F5; 
    	dsi_set_cmdq(&data_array, 2, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B400;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x000000F5; 
    	dsi_set_cmdq(&data_array, 2, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B600;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x000000F5; 
    	dsi_set_cmdq(&data_array, 2, 1);

		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B800;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x000000F5; 
    	dsi_set_cmdq(&data_array, 2, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B400;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x0000C0C5; 
    	dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B200;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000040C5; 
    	dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000A000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000F3902; //15
    	data_array[1] = 0x061005C4;
		data_array[2] = 0x10150502;
		data_array[3] = 0x02071005;
		data_array[4] = 0x00101505;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x000000C4; 
    	dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00009100;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x005007C5; 
    	dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x00BCBCD8; 
    	dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x000804C5; //vdd1_8 1.7v
    	dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000BB00;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000080C5; 
    	dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000040D0; 
    	dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x000000D1; 
    	dsi_set_cmdq(&data_array, 2, 1);
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000C3902; //12
    	data_array[1] = 0x000000CB;
		data_array[2] = 0x00000000;
		data_array[3] = 0x00000000;
		dsi_set_cmdq(&data_array, 4, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x000000CB;
		data_array[2] = 0x00000000;
		data_array[3] = 0x00000000;
		data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000A000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x000000CB;
		data_array[2] = 0x00000000;
		data_array[3] = 0x00000000;
		data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x000000CB;
		data_array[2] = 0x00000000;
		data_array[3] = 0x00000000;
		data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000C000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x050505CB;
		data_array[2] = 0x00050505;
		data_array[3] = 0x00000000;
		data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000D000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x000000CB;
		data_array[2] = 0x05050000;
		data_array[3] = 0x05050505;
		data_array[4] = 0x00000505;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000E000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000F3902; //15
    	data_array[1] = 0x000000CB;
		data_array[2] = 0x00000000;
		data_array[3] = 0x00000000;
		data_array[4] = 0x00050500;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000F000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000C3902; //12
    	data_array[1] = 0xFFFFFFCB;
		data_array[2] = 0xFFFFFFFF;
		data_array[3] = 0xFFFFFFFF;
	   	dsi_set_cmdq(&data_array, 4, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x0D0B09CC;
		data_array[2] = 0x0003010F;
		data_array[3] = 0x00000000;
		data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x000000CC;
		data_array[2] = 0x2D2E0000;
		data_array[3] = 0x100E0C0A;
		data_array[4] = 0x00000402;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000A000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000F3902; //15
    	data_array[1] = 0x000000CC;
		data_array[2] = 0x00000000;
		data_array[3] = 0x00000000;
		data_array[4] = 0x002D2E00;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x0C0E10CC;
		data_array[2] = 0x0002040A;
		data_array[3] = 0x00000000;
		data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000C000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00103902; //16
    	data_array[1] = 0x000000CC;
		data_array[2] = 0x2E2D0000;
		data_array[3] = 0x090B0D0F;
		data_array[4] = 0x00000103;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000D000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000F3902; //15
    	data_array[1] = 0x000000CC;
		data_array[2] = 0x00000000;
		data_array[3] = 0x00000000;
		data_array[4] = 0x002E2D00;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000D3902; //13
    	data_array[1] = 0x00038FCE;
		data_array[2] = 0x8D00038E;
		data_array[3] = 0x038C0003;
		data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1);
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000F3902; //15
    	data_array[1] = 0x000000CE;
		data_array[2] = 0x00000000;
		data_array[3] = 0x00000000;
		data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000A000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000F3902; //15
    	data_array[1] = 0x050B38CE;
		data_array[2] = 0x0A0A0000;
		data_array[3] = 0x01050A38;
		data_array[4] = 0x000A0A00;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000F3902; //15
    	data_array[1] = 0x050938CE;
		data_array[2] = 0x0A0A0002;
		data_array[3] = 0x03050838;
		data_array[4] = 0x000A0A00;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000C000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000F3902; //15
    	data_array[1] = 0x050738CE;
		data_array[2] = 0x0A0A0004;
		data_array[3] = 0x05050638;
		data_array[4] = 0x000A0A00;
    	dsi_set_cmdq(&data_array, 5, 1); 
			
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000D000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000F3902; //15
    	data_array[1] = 0x050538CE;
		data_array[2] = 0x0A0A0006;
		data_array[3] = 0x07050438;
		data_array[4] = 0x000A0A00;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000F3902; //15
    	data_array[1] = 0x000000CF;
		data_array[2] = 0x00000000;
		data_array[3] = 0x00000000;
		data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00009000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000F3902; //15
    	data_array[1] = 0x000000CF;
		data_array[2] = 0x00000000;
		data_array[3] = 0x00000000;
		data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000A000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000F3902; //15
    	data_array[1] = 0x000000CF;
		data_array[2] = 0x00000000;
		data_array[3] = 0x00000000;
		data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000F3902; //15
    	data_array[1] = 0x000000CF;
		data_array[2] = 0x00000000;
		data_array[3] = 0x00000000;
		data_array[4] = 0x00000000;
    	dsi_set_cmdq(&data_array, 5, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000C000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x000C3902; //12
    	data_array[1] = 0x200101CF;
		data_array[2] = 0x01000020;
		data_array[3] = 0x08000002;
	   	dsi_set_cmdq(&data_array, 4, 1); 
		
		data_array[0] = 0x00023902;
    	data_array[1] = 0x0000B500;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00073902; //7
    	data_array[1] = 0xFFF133C5;
		data_array[2] = 0x00FFF133;
    	dsi_set_cmdq(&data_array, 3, 1); 
    	
    			data_array[0] = 0x00023902;
    	data_array[1] = 0x00008700;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000018C4;//
    	dsi_set_cmdq(&data_array, 2, 1); 
    	
    	
    			data_array[0] = 0x00023902;
    	data_array[1] = 0x0000C300;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000081F5;//
    	dsi_set_cmdq(&data_array, 2, 1); 
    	
    	/*
    			data_array[0] = 0x00023902;
    	data_array[1] = 0x00008200;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000000F4;
    	dsi_set_cmdq(&data_array, 2, 1); 
*/
		
		
		
/*	
//GAMMA2.2	
//--Gamma-------------------------
//E1
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00113902; //17
    	data_array[1] = 0x0b0700E1;    
    	data_array[2] = 0x0c0e060c;    
    	data_array[3] = 0x0a06020b;    
    	data_array[4] = 0x0c120f06;    
    	data_array[5] = 0x00000000;     
    	dsi_set_cmdq(&data_array, 6, 1);     

 
//E2
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00113902; //25
    	data_array[1] = 0x0c0700E2;    
    	data_array[2] = 0x0c0e060c;    
    	data_array[3] = 0x0a05020b;    
    	data_array[4] = 0x0c120f06;    
    	data_array[5] = 0x00000000;      
    	dsi_set_cmdq(&data_array, 6, 1);     

*/ 


/*
//GAMMA2.3
//--Gamma-------------------------
//E1
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00113902; //17
    	data_array[1] = 0x0c0600E1;    
    	data_array[2] = 0x0c0f050d;    
    	data_array[3] = 0x0a06020b;    
    	data_array[4] = 0x0c120d06;    
    	data_array[5] = 0x00000000;     
    	dsi_set_cmdq(&data_array, 6, 1);     

 
//E2
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00113902; //25
    	data_array[1] = 0x0c0600E2;    
    	data_array[2] = 0x0c0f050c;    
    	data_array[3] = 0x0a05020b;    
    	data_array[4] = 0x0c120d06;    
    	data_array[5] = 0x00000000;      
    	dsi_set_cmdq(&data_array, 6, 1);     

*/

/*
//GAMMA2.4
//--Gamma-------------------------
//E1
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00113902; //17
    	data_array[1] = 0x0c0600E1;    
    	data_array[2] = 0x0c0f050d;    
    	data_array[3] = 0x0a06020b;    
    	data_array[4] = 0x0b110d06;    
    	data_array[5] = 0x00000000;     
    	dsi_set_cmdq(&data_array, 6, 1);     

 
//E2
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00113902; //25
    	data_array[1] = 0x0c0700E2;    
    	data_array[2] = 0x0c0f050c;    
    	data_array[3] = 0x0a05020b;    
    	data_array[4] = 0x0b110d06;    
    	data_array[5] = 0x00000000;      
    	dsi_set_cmdq(&data_array, 6, 1);     

*/

//GAMMA2.5
//--Gamma-------------------------
//E1
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00113902; //17
    	data_array[1] = 0x0c0600E1;    
    	data_array[2] = 0x0c10050d;    
    	data_array[3] = 0x0906020b;    
    	data_array[4] = 0x0b110d06;    
    	data_array[5] = 0x00000000;     
    	dsi_set_cmdq(&data_array, 6, 1);     

 
//E2
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00113902; //25
    	data_array[1] = 0x0c0700E2;    
    	data_array[2] = 0x0c10050c;    
    	data_array[3] = 0x0905020b;    
    	data_array[4] = 0x0b110d06;    
    	data_array[5] = 0x00000000;      
    	dsi_set_cmdq(&data_array, 6, 1);     
//

    	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
		data_array[0] = 0x00023902;
    	data_array[1] = 0x000078D9;    
    	dsi_set_cmdq(&data_array, 2, 1); 

			
///////////////////////////////////////////////////// update by xuanquan.wang 2013/04/09
#if 0
data_array[0] = 0x00023902;
data_array[1] = 0x0000A000;
dsi_set_cmdq(&data_array, 2, 1);

data_array[0] = 0x00023902;
data_array[1] = 0x000002C1;
dsi_set_cmdq(&data_array, 2, 1);
MDELAY(1);

//data_array[0] = 0x00023902;
//data_array[1] = 0x0000A200;
//dsi_set_cmdq(&data_array, 2, 1);
//data_array[0] = 0x00023902;
//data_array[1] = 0x000008C1;
//dsi_set_cmdq(&data_array, 2, 1);
//data_array[0] = 0x00023902;
//data_array[1] = 0x0000A400;
//dsi_set_cmdq(&data_array, 2, 1);
//data_array[0] = 0x00023902;
//data_array[1] = 0x0000F0C1;
//dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x00008000;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000030C4;
dsi_set_cmdq(&data_array, 2, 1);
MDELAY(10);
data_array[0] = 0x00023902;
data_array[1] = 0x00008A00;
dsi_set_cmdq(&data_array, 2, 1);
data_array[0] = 0x00023902;
data_array[1] = 0x000040C4;
dsi_set_cmdq(&data_array, 2, 1);
MDELAY(10);
#else
data_array[0]=0x00023902;
data_array[1]=0x0000A000;
dsi_set_cmdq(&data_array, 2, 1);
MDELAY(2);

data_array[0]=0x00023902;
data_array[1]=0x000002C1;
dsi_set_cmdq(&data_array, 2, 1);
MDELAY(2);

data_array[0]=0x00023902;
data_array[1]=0x0000A200;
dsi_set_cmdq(&data_array, 2, 1);
MDELAY(2);

data_array[0]=0x00023902;
data_array[1]=0x0000FFC1;
dsi_set_cmdq(&data_array, 2, 1);
MDELAY(2);

data_array[0]=0x00023902;
data_array[1]=0x0000A400;
dsi_set_cmdq(&data_array, 2, 1);
MDELAY(2);

data_array[0]=0x00023902;
data_array[1]=0x0000FFC1;
dsi_set_cmdq(&data_array, 2, 1);
MDELAY(2);

data_array[0]=0x00023902;
data_array[1]=0x00009000;
dsi_set_cmdq(&data_array, 2, 1);
MDELAY(2);

data_array[0]=0x00103902;
data_array[1]=0xC0C0C0CB;
data_array[2]=0x00C0C0C0;
data_array[3]=0x00000000;
data_array[4]=0x00000000;
dsi_set_cmdq(&data_array, 5, 1);
MDELAY(2);

data_array[0]=0x00023902;
data_array[1]=0x0000A000;
dsi_set_cmdq(&data_array, 2, 1);
MDELAY(2);

data_array[0]=0x00103902;
data_array[1]=0x000000CB;
data_array[2]=0xC0C00000;
data_array[3]=0xC0C0C0C0;
data_array[4]=0x0000C0C0;
dsi_set_cmdq(&data_array, 5, 1);
MDELAY(2);

data_array[0]=0x00023902;
data_array[1]=0x0000B000;
dsi_set_cmdq(&data_array, 2, 1);
MDELAY(2);

data_array[0]=0x00103902;
data_array[1]=0x000000CB;
data_array[2]=0x00000000;
data_array[3]=0x00000000;
data_array[4]=0xC0C00000;
dsi_set_cmdq(&data_array, 5, 1);
MDELAY(2);
//pwm frequence is 68kHz
data_array[0]=0x00023902;
data_array[1]=0x0000B100;
dsi_set_cmdq(&data_array, 2, 1);
//MDELAY(2);
data_array[0]=0x00023902;
data_array[1]=0x000000C6;// 
dsi_set_cmdq(&data_array, 2, 1);
//MDELAY(2);
data_array[0]=0x00023902;
data_array[1]=0x0000B400;
dsi_set_cmdq(&data_array, 2, 1);
//MDELAY(2);
data_array[0]=0x00023902;
data_array[1]=0x000010C6;// 
dsi_set_cmdq(&data_array, 2, 1);
/*
data_array[0]=0x00023902;
data_array[1]=0x0000B400;
dsi_set_cmdq(&data_array, 2, 1);
MDELAY(2);

data_array[0]=0x00023902;
data_array[1]=0x000010C0;
dsi_set_cmdq(&data_array, 2, 1);

MDELAY(2);
*/
#endif
///////////////////////////////////////////////////////		
  
//orise mode disable
		data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00043902; //EXTC=1
    	data_array[1] = 0x0FFFFFFF;                 
    	dsi_set_cmdq(&data_array, 2, 1);  

	data_array[0] = 0x00351500;//te on
    	dsi_set_cmdq(&data_array, 1, 1);    

		data_array[0] = 0x24531500;//bl mode
			dsi_set_cmdq(&data_array, 1, 1); 

		data_array[0] = 0x03551500;//cabc 03:move 00:off
			dsi_set_cmdq(&data_array, 1, 1); 

     	data_array[0] = 0x00110500;                
    	dsi_set_cmdq(&data_array, 1, 1); 
    	MDELAY(150); 
    	
    	data_array[0] = 0x00290500;                
    	dsi_set_cmdq(&data_array, 1, 1);    
    	MDELAY(5); 

//    	data_array[0] = 0x002C0500;                
//    	dsi_set_cmdq(&data_array, 1, 1);    
//    	MDELAY(20); 

} 
static void lcm_setbacklight(unsigned int level);
 

static void lcm_init(void)
{


	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(5);
	SET_RESET_PIN(1);
	MDELAY(10);


#ifdef BUILD_LK
    printf("[wj]otm1283a init code.\n");
#endif
    lcm_init_register();
 

mt_set_gpio_out(LCM_BACKLIGHT_EN, 1);


}


static void lcm_suspend(void)
{
	unsigned int data_array[16];

#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, lcm_init \n", __func__);
#else
	printk("%s, lcm_init \n", __func__);
#endif	
//begin lenovo jixu add for power consume 20130131
mt_set_gpio_out(LCM_BACKLIGHT_EN, 0);//diable backlight ic
//end lenovo jixu add for power consume 20130131
    	data_array[0] = 0x00280500;                
    	dsi_set_cmdq(&data_array, 1, 1);    
		MDELAY(20);
    	data_array[0] = 0x00100500;                
    	dsi_set_cmdq(&data_array, 1, 1); 
		MDELAY(150); 
//begin lenovo jixu add for power consume 20130206
//		SET_RESET_PIN(0);
//end lenovo jixu add for power consume 20130206
}


static void lcm_resume(void)
{
int i;
unsigned int data_array[16];

#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, lcm_resume otm1283a\n", __func__);
#else
	printk("%s, lcm_resume otm1283a\n", __func__);
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
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(&data_array, 7, 0);

}

static void lcm_setbacklight(unsigned int level)
{
	unsigned int data_array[16];

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

static unsigned int lcm_esd_check(void)
{
unsigned char buffer[2],ret;

#ifndef BUILD_UBOOT
	if(lcm_esd_test)
	{
	lcm_esd_test = FALSE;
	return TRUE;
	}

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

static unsigned int lcm_compare_id(void)
{
#if LCM_COMPARE_BY_SW
	unsigned int id=0;
	unsigned char buffer[4];
	unsigned int data_array[16];  
     SET_RESET_PIN(1);
   MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(50);//Must over 6 ms

   	data_array[0] = 0x00023902;
    	data_array[1] = 0x00000000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00043902; //EXTC=1
    	data_array[1] = 0x018312FF;                 
    	dsi_set_cmdq(&data_array, 2, 1);    

    	data_array[0] = 0x00023902; //ORISE mode enable
    	data_array[1] = 0x00008000;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00033902;
    	data_array[1] = 0x008312FF;    
    	dsi_set_cmdq(&data_array, 2, 1);  
 
    	data_array[0] = 0x00023902; //ORISE mode enable
    	data_array[1] = 0x0000C600;    
    	dsi_set_cmdq(&data_array, 2, 1);     
    	data_array[0] = 0x00023902;
    	data_array[1] = 0x000003B0;    
    	dsi_set_cmdq(&data_array, 2, 1);    

	data_array[0] = 0x00043700;
	dsi_set_cmdq(&data_array, 1, 1);

	read_reg_v2(0xA1, buffer, 4);
	id = buffer[3]|(buffer[2]<<8); //we only need ID
//        id = read_reg(0xDA);
#if defined(BUILD_LK) || defined(BUILD_UBOOT)
	printf("%s, otm1283a id = 0x%08x\n", __func__, id);
#else
       printk("%s, otm1283a id = 0x%08x\n", __func__, id);
#endif

	return (LCM_ID == id)?1:0;
//	return 1;
#else
	unsigned int ret = 0;
	mt_set_gpio_mode(GPIO92,GPIO_MODE_GPIO); 
	mt_set_gpio_dir(GPIO92,GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO92,GPIO_PULL_DISABLE);

	ret = mt_get_gpio_in(LCM_ID_PIN);
#if defined(BUILD_LK)
	printf("%s, [jx]otm1283a LCM_ID_PIN = %d \n", __func__, ret);
#endif	

	return (ret == 0)?1:0;
#endif
}
static void lcm_setcabcmode(unsigned int mode)
{
	unsigned int data_array[16];

	#ifdef BUILD_LK
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

static void lcm_set_inverse(unsigned int mode)
{
	unsigned int data_array[16];

#ifdef BUILD_LK
	printf("%s mode=%d\n",__func__,mode);
#else
	printk("%s mode=%d\n",__func__,mode);
#endif

	if(mode){
		data_array[0] = 0x00210500;
		dsi_set_cmdq(data_array, 1, 1);
	}else
	{
		data_array[0] = 0x00200500;
		dsi_set_cmdq(data_array, 1, 1);
		
	}
	
	 MDELAY(10);
}


LCM_DRIVER otm1283a_hd720_dsi_vdo_boe_lcm_drv = 
{
	.name = "otm1283a_hd720_dsi_vdo_boe",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
	.esd_check   = lcm_esd_check,
  .esd_recover   = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
	.set_backlight	= lcm_setbacklight,
#ifdef LENOVO_LCD_BACKLIGHT_CONTROL_BY_LCM
	.set_cabcmode = lcm_setcabcmode,
	.get_cabcmode = lcm_getcabcstate,
#endif
	.set_inversemode = lcm_set_inverse,

};
