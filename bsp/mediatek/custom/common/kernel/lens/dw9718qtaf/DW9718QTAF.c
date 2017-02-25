/*
 *
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include "DW9718QTAF.h"
#include "../camera/kd_camera_hw.h"

#define LENS_I2C_BUSNUM 1
static struct i2c_board_info __initdata kd_lens_dev={ I2C_BOARD_INFO("DW9718QTAF", 0x19)};
//lenovo.sw wangsx3 20131022 the right address is 0x18
// use 0x19 to register i2c device,then use 0x18(DW9718QTAF_VCM_WRITE_ID) to transfer data

#define DW9718QTAF_DRVNAME "DW9718QTAF"
#define DW9718QTAF_VCM_WRITE_ID           0x18

#define DW9718QTAF_DEBUG
#ifdef DW9718QTAF_DEBUG
#define DW9718QTAFDB(fmt,arg...) printk("[DW9718QTAF]" fmt,##arg)
#else
#define DW9718QTAFDB(x,...)
#endif

static spinlock_t g_DW9718QTAF_SpinLock;

static struct i2c_client * g_pstDW9718QTAF_I2Cclient = NULL;

static dev_t g_DW9718QTAF_devno;
static struct cdev * g_pDW9718QTAF_CharDrv = NULL;
static struct class *actuator_class = NULL;

static int  g_s4DW9718QTAF_Opened = 0;
static long g_i4MotorStatus = 0;
static long g_i4Dir = 0;
static unsigned long g_u4DW9718QTAF_INF = 0;
static unsigned long g_u4DW9718QTAF_MACRO = 1023;
static unsigned long g_u4TargetPosition = 0;
static unsigned long g_u4CurrPosition   = 0;

static int g_sr = 3;

static int i2c_read(u8 a_u2Addr , u8 * a_puBuff)
{
    int  i4RetValue = 0;
    char puReadCmd[1] = {(char)(a_u2Addr)};
	i4RetValue = i2c_master_send(g_pstDW9718QTAF_I2Cclient, puReadCmd, 1);
	if (i4RetValue != 2) {
	    DW9718QTAFDB(" I2C write failed!! \n");
	    return -1;
	}
	//
	i4RetValue = i2c_master_recv(g_pstDW9718QTAF_I2Cclient, (char *)a_puBuff, 1);
	if (i4RetValue != 1) {
	    DW9718QTAFDB(" I2C read failed!! \n");
	    return -1;
	}
   
    return 0;
}

static u8 read_data(u8 addr)
{
	u8 get_byte=0;
    i2c_read( addr ,&get_byte);
    DW9718QTAFDB("[DW9718QTAF]  get_byte %d \n",  get_byte);
    return get_byte;
}

static int s4DW9718QTAF_ReadReg(unsigned short * a_pu2Result)
{
    //int  i4RetValue = 0;
    //char pBuff[2];

    *a_pu2Result = (read_data(0x02) << 8) + (read_data(0x03)&0xff);

    DW9718QTAFDB("[DW9718QTAF]  s4DW9718QTAF_ReadReg %d \n",  *a_pu2Result);
    return 0;
}

static int s4DW9718QTAF_WriteReg(u16 a_u2Data)
{
    int  i4RetValue = 0;

    char puSendCmd[3] = {0x02,(char)(a_u2Data >> 8) , (char)(a_u2Data & 0xFF)};

    DW9718QTAFDB("[DW9718QTAF]  write %d \n",  a_u2Data);
	
	g_pstDW9718QTAF_I2Cclient->ext_flag |= I2C_A_FILTER_MSG;
    i4RetValue = i2c_master_send(g_pstDW9718QTAF_I2Cclient, puSendCmd, 3);
	
    if (i4RetValue < 0) 
    {
        DW9718QTAFDB("[DW9718QTAF] I2C send failed!! \n");
        return -1;
    }

    return 0;
}

inline static int getDW9718QTAFInfo(__user stDW9718QTAF_MotorInfo * pstMotorInfo)
{
    stDW9718QTAF_MotorInfo stMotorInfo;
    stMotorInfo.u4MacroPosition   = g_u4DW9718QTAF_MACRO;
    stMotorInfo.u4InfPosition     = g_u4DW9718QTAF_INF;
    stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
    stMotorInfo.bIsSupportSR      = TRUE;

	if (g_i4MotorStatus == 1)	{stMotorInfo.bIsMotorMoving = 1;}
	else						{stMotorInfo.bIsMotorMoving = 0;}

	if (g_s4DW9718QTAF_Opened >= 1)	{stMotorInfo.bIsMotorOpen = 1;}
	else						{stMotorInfo.bIsMotorOpen = 0;}

    if(copy_to_user(pstMotorInfo , &stMotorInfo , sizeof(stDW9718QTAF_MotorInfo)))
    {
        DW9718QTAFDB("[DW9718QTAF] copy to user failed when getting motor information \n");
    }

    return 0;
}

inline static int moveDW9718QTAF(unsigned long a_u4Position)
{
    int ret = 0;
    
    if((a_u4Position > g_u4DW9718QTAF_MACRO) || (a_u4Position < g_u4DW9718QTAF_INF))
    {
        DW9718QTAFDB("[DW9718QTAF] out of range \n");
        return -EINVAL;
    }

    if (g_s4DW9718QTAF_Opened == 1)
    {
        unsigned short InitPos;
        ret = s4DW9718QTAF_ReadReg(&InitPos);
	    
        spin_lock(&g_DW9718QTAF_SpinLock);
        if(ret == 0)
        {
            DW9718QTAFDB("[DW9718QTAF] Init Pos %6d \n", InitPos);
            g_u4CurrPosition = (unsigned long)InitPos;
        }
        else
        {		
            g_u4CurrPosition = 0;
        }
        g_s4DW9718QTAF_Opened = 2;
        spin_unlock(&g_DW9718QTAF_SpinLock);
    }

    if (g_u4CurrPosition < a_u4Position)
    {
        spin_lock(&g_DW9718QTAF_SpinLock);	
        g_i4Dir = 1;
        spin_unlock(&g_DW9718QTAF_SpinLock);	
    }
    else if (g_u4CurrPosition > a_u4Position)
    {
        spin_lock(&g_DW9718QTAF_SpinLock);	
        g_i4Dir = -1;
        spin_unlock(&g_DW9718QTAF_SpinLock);			
    }
    else	
	{return 0;}

    spin_lock(&g_DW9718QTAF_SpinLock);    
    g_u4TargetPosition = a_u4Position;
    spin_unlock(&g_DW9718QTAF_SpinLock);	

    //DW9718QTAFDB("[DW9718QTAF] move [curr] %d [target] %d\n", g_u4CurrPosition, g_u4TargetPosition);

            spin_lock(&g_DW9718QTAF_SpinLock);
            g_sr = 3;
            g_i4MotorStatus = 0;
            spin_unlock(&g_DW9718QTAF_SpinLock);	
		
            if(s4DW9718QTAF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
            {
                spin_lock(&g_DW9718QTAF_SpinLock);		
                g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
                spin_unlock(&g_DW9718QTAF_SpinLock);				
            }
            else
            {
                DW9718QTAFDB("[DW9718QTAF] set I2C failed when moving the motor \n");			
                spin_lock(&g_DW9718QTAF_SpinLock);
                g_i4MotorStatus = -1;
                spin_unlock(&g_DW9718QTAF_SpinLock);				
            }

    return 0;
}

inline static int setDW9718QTAFInf(unsigned long a_u4Position)
{
    spin_lock(&g_DW9718QTAF_SpinLock);
    g_u4DW9718QTAF_INF = a_u4Position;
    spin_unlock(&g_DW9718QTAF_SpinLock);	
    return 0;
}

inline static int setDW9718QTAFMacro(unsigned long a_u4Position)
{
    spin_lock(&g_DW9718QTAF_SpinLock);
    g_u4DW9718QTAF_MACRO = a_u4Position;
    spin_unlock(&g_DW9718QTAF_SpinLock);	
    return 0;	
}

////////////////////////////////////////////////////////////////
static long DW9718QTAF_Ioctl(
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
{
    long i4RetValue = 0;

    switch(a_u4Command)
    {
        case DW9718QTAFIOC_G_MOTORINFO :
            i4RetValue = getDW9718QTAFInfo((__user stDW9718QTAF_MotorInfo *)(a_u4Param));
        break;

        case DW9718QTAFIOC_T_MOVETO :
            i4RetValue = moveDW9718QTAF(a_u4Param);
        break;
 
        case DW9718QTAFIOC_T_SETINFPOS :
            i4RetValue = setDW9718QTAFInf(a_u4Param);
        break;

        case DW9718QTAFIOC_T_SETMACROPOS :
            i4RetValue = setDW9718QTAFMacro(a_u4Param);
        break;
		
        default :
      	    DW9718QTAFDB("[DW9718QTAF] No CMD \n");
            i4RetValue = -EPERM;
        break;
    }

    return i4RetValue;
}

//Main jobs:
// 1.check for device-specified errors, device not ready.
// 2.Initialize the device if it is opened for the first time.
// 3.Update f_op pointer.
// 4.Fill data structures into private_data
//CAM_RESET
static int DW9718QTAF_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    long i4RetValue = 0;
	char puSendCmd2[2] = {0x01,0x39};
	char puSendCmd3[2] = {0x05,0x65};
	
    DW9718QTAFDB("[DW9718QTAF] DW9718QTAF_Open - Start\n");

    spin_lock(&g_DW9718QTAF_SpinLock);

    if(g_s4DW9718QTAF_Opened)
    {
        spin_unlock(&g_DW9718QTAF_SpinLock);
        DW9718QTAFDB("[DW9718QTAF] the device is opened \n");
        return -EBUSY;
    }

    g_s4DW9718QTAF_Opened = 1;
		
    spin_unlock(&g_DW9718QTAF_SpinLock);

    //puSendCmd2[2] = {(0x01),(0x39)};
   	i4RetValue = i2c_master_send(g_pstDW9718QTAF_I2Cclient, puSendCmd2, 2);
	//puSendCmd3[2] = {(char)(0x05),(char)(0x65)};
   	i4RetValue = i2c_master_send(g_pstDW9718QTAF_I2Cclient, puSendCmd3, 2);
	
    DW9718QTAFDB("[DW9718QTAF] DW9718QTAF_Open - End\n");

    return 0;
}

//Main jobs:
// 1.Deallocate anything that "open" allocated in private_data.
// 2.Shut down the device on last close.
// 3.Only called once on last time.
// Q1 : Try release multiple times.
static int DW9718QTAF_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
    DW9718QTAFDB("[DW9718QTAF] DW9718QTAF_Release - Start\n");

    if (g_s4DW9718QTAF_Opened)
    {
        DW9718QTAFDB("[DW9718QTAF] feee \n");
        g_sr = 5;
	    s4DW9718QTAF_WriteReg(200);
        msleep(10);
	    s4DW9718QTAF_WriteReg(100);
        msleep(10);
            	            	    	    
        spin_lock(&g_DW9718QTAF_SpinLock);
        g_s4DW9718QTAF_Opened = 0;
        spin_unlock(&g_DW9718QTAF_SpinLock);

    }

    DW9718QTAFDB("[DW9718QTAF] DW9718QTAF_Release - End\n");

    return 0;
}

static const struct file_operations g_stDW9718QTAF_fops = 
{
    .owner = THIS_MODULE,
    .open = DW9718QTAF_Open,
    .release = DW9718QTAF_Release,
    .unlocked_ioctl = DW9718QTAF_Ioctl
};

inline static int Register_DW9718QTAF_CharDrv(void)
{
    struct device* vcm_device = NULL;

    DW9718QTAFDB("[DW9718QTAF] Register_DW9718QTAF_CharDrv - Start\n");

    //Allocate char driver no.
    if( alloc_chrdev_region(&g_DW9718QTAF_devno, 0, 1,DW9718QTAF_DRVNAME) )
    {
        DW9718QTAFDB("[DW9718QTAF] Allocate device no failed\n");

        return -EAGAIN;
    }

    //Allocate driver
    g_pDW9718QTAF_CharDrv = cdev_alloc();

    if(NULL == g_pDW9718QTAF_CharDrv)
    {
        unregister_chrdev_region(g_DW9718QTAF_devno, 1);

        DW9718QTAFDB("[DW9718QTAF] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pDW9718QTAF_CharDrv, &g_stDW9718QTAF_fops);

    g_pDW9718QTAF_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pDW9718QTAF_CharDrv, g_DW9718QTAF_devno, 1))
    {
        DW9718QTAFDB("[DW9718QTAF] Attatch file operation failed\n");

        unregister_chrdev_region(g_DW9718QTAF_devno, 1);

        return -EAGAIN;
    }
   //lenovo.sw wangsx3 20131105 fix wrong lens class name,reason:cannot register lens driver.
    actuator_class = class_create(THIS_MODULE, "actuatorqtdrv");
    if (IS_ERR(actuator_class)) {
        int ret = PTR_ERR(actuator_class);
        DW9718QTAFDB("Unable to create class, err = %d\n", ret);
        return ret;            
    }

    vcm_device = device_create(actuator_class, NULL, g_DW9718QTAF_devno, NULL, DW9718QTAF_DRVNAME);

    if(NULL == vcm_device)
    {
        return -EIO;
    }
    
    DW9718QTAFDB("[DW9718QTAF] Register_DW9718QTAF_CharDrv - End\n");    
    return 0;
}

inline static void Unregister_DW9718QTAF_CharDrv(void)
{
    DW9718QTAFDB("[DW9718QTAF] Unregister_DW9718QTAF_CharDrv - Start\n");

    //Release char driver
    cdev_del(g_pDW9718QTAF_CharDrv);

    unregister_chrdev_region(g_DW9718QTAF_devno, 1);
    
    device_destroy(actuator_class, g_DW9718QTAF_devno);

    class_destroy(actuator_class);

    DW9718QTAFDB("[DW9718QTAF] Unregister_DW9718QTAF_CharDrv - End\n");    
}

//////////////////////////////////////////////////////////////////////

static int DW9718QTAF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int DW9718QTAF_i2c_remove(struct i2c_client *client);
static const struct i2c_device_id DW9718QTAF_i2c_id[] = {{DW9718QTAF_DRVNAME,0},{}};   
struct i2c_driver DW9718QTAF_i2c_driver = {                       
    .probe = DW9718QTAF_i2c_probe,                                   
    .remove = DW9718QTAF_i2c_remove,                           
    .driver.name = DW9718QTAF_DRVNAME,                 
    .id_table = DW9718QTAF_i2c_id,                             
};  

#if 0 
static int DW9718QTAF_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {         
    strcpy(info->type, DW9718QTAF_DRVNAME);                                                         
    return 0;                                                                                       
}      
#endif 
static int DW9718QTAF_i2c_remove(struct i2c_client *client) {
    return 0;
}

/* Kirby: add new-style driver {*/
static int DW9718QTAF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int i4RetValue = 0;

    DW9718QTAFDB("[DW9718QTAF] DW9718QTAF_i2c_probe\n");

    /* Kirby: add new-style driver { */
    g_pstDW9718QTAF_I2Cclient = client;
    g_pstDW9718QTAF_I2Cclient->addr=DW9718QTAF_VCM_WRITE_ID;
    g_pstDW9718QTAF_I2Cclient->addr = g_pstDW9718QTAF_I2Cclient->addr >> 1;
    
    //Register char driver
    i4RetValue = Register_DW9718QTAF_CharDrv();

    if(i4RetValue){

        DW9718QTAFDB("[DW9718QTAF] register char device failed!\n");

        return i4RetValue;
    }

    spin_lock_init(&g_DW9718QTAF_SpinLock);

    DW9718QTAFDB("[DW9718QTAF] Attached!! \n");

    return 0;
}

static int DW9718QTAF_probe(struct platform_device *pdev)
{
    return i2c_add_driver(&DW9718QTAF_i2c_driver);
}

static int DW9718QTAF_remove(struct platform_device *pdev)
{
    i2c_del_driver(&DW9718QTAF_i2c_driver);
    return 0;
}

static int DW9718QTAF_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

static int DW9718QTAF_resume(struct platform_device *pdev)
{
    return 0;
}

// platform structure
static struct platform_driver g_stDW9718QTAF_Driver = {
    .probe		= DW9718QTAF_probe,
    .remove	= DW9718QTAF_remove,
    .suspend	= DW9718QTAF_suspend,
    .resume	= DW9718QTAF_resume,
    .driver		= {
        .name	= "lens_actuator_qt",
        .owner	= THIS_MODULE,
    }
};

static int __init DW9718QTAF_i2C_init(void)
{
    i2c_register_board_info(LENS_I2C_BUSNUM, &kd_lens_dev, 1);
	
    if(platform_driver_register(&g_stDW9718QTAF_Driver)){
        DW9718QTAFDB("failed to register DW9718QTAF driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit DW9718QTAF_i2C_exit(void)
{
	platform_driver_unregister(&g_stDW9718QTAF_Driver);
}

module_init(DW9718QTAF_i2C_init);
module_exit(DW9718QTAF_i2C_exit);

MODULE_DESCRIPTION("DW9718QTAF lens module driver");
MODULE_AUTHOR("KY Chen <ky.chen@Mediatek.com>");
MODULE_LICENSE("GPL");


