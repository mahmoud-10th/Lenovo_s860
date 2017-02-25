#include <cust_vibrator.h>
#include <linux/types.h>

static struct vibrator_hw cust_vibrator_hw = {
	.vib_timer = 50,
  #ifdef CUST_VIBR_LIMIT
	.vib_limit = 9,
  #endif
  #ifdef CUST_VIBR_VOL
       #ifdef LENOVO_VIBR_VOL_MAX
/*lenovo-sw lixh10 2014.4.9 add add for turnning down the vibrator begin */
	 #ifdef LENOVO_VIBR_VOL_PHAETON 
	    .vib_vol =0x05,
	    .vib_timer = 20, //normal
	 #else
	    .vib_vol = 0x7,//3.3//0x5,//2.8V // 0x04 //2.5V
	#endif
/*lenovo-sw lixh10 2014.4.9 add add for turnning down the vibrator end */
      #else
	.vib_vol =0x05,
      #endif
  #endif
};

struct vibrator_hw *get_cust_vibrator_hw(void)
{
    return &cust_vibrator_hw;
}

