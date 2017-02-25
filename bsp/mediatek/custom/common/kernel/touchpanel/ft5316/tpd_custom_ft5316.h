#ifndef TOUCHPANEL_H__
#define TOUCHPANEL_H__

/* Pre-defined definition */
#define TPD_TYPE_CAPACITIVE
#define TPD_TYPE_RESISTIVE
#define TPD_POWER_SOURCE         
#define TPD_I2C_NUMBER           0
#define TPD_WAKEUP_TRIAL         60
#define TPD_WAKEUP_DELAY         100

#define TPD_VELOCITY_CUSTOM_X 15
#define TPD_VELOCITY_CUSTOM_Y 20

#define TPD_POWER_SOURCE_CUSTOM  MT6323_POWER_LDO_VGP1//	MT65XX_POWER_LDO_VGP4
#define TPD_POWER_SOURCE_1800	MT6323_POWER_LDO_VGP2//MT65XX_POWER_LDO_VGP6

#define TPD_DELAY                (2*HZ/100)
//#define TPD_RES_X                480
//#define TPD_RES_Y                800
#define TPD_CALIBRATION_MATRIX  {962,0,0,0,1600,0,0,0};

//#define TPD_HAVE_CALIBRATION
//#define TPD_HAVE_BUTTON
//#define TPD_HAVE_TREMBLE_ELIMINATION
#define TPD_HAVE_BUTTON
#define TPD_BUTTON_HEIGH        (40)
#define TPD_KEY_COUNT           3
/*
#define TPD_KEYS                { KEY_MENU, KEY_HOMEPAGE ,KEY_BACK, KEY_SEARCH}
#define TPD_KEYS_DIM            {{60,850,120,TPD_BUTTON_HEIGH},{180,850,120,TPD_BUTTON_HEIGH},{300,850,120,TPD_BUTTON_HEIGH}, {420,850,120,TPD_BUTTON_HEIGH}}
*/
#define TPD_KEYS                { KEY_MENU, KEY_HOMEPAGE ,KEY_BACK}
#define TPD_KEYS_DIM            {{120,1350,80,TPD_BUTTON_HEIGH},{330,1350,80,TPD_BUTTON_HEIGH},{590,1350,80,TPD_BUTTON_HEIGH}} //, {420,850,120,TPD_BUTTON_HEIGH}}
#endif /* TOUCHPANEL_H__ */