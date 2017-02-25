
#ifndef __CUSTOM_MEMORYDEVICE__
#define __CUSTOM_MEMORYDEVICE__

/*
 ****************************************************************************
 [README , VERY IMPORTANT NOTICE]
 --------------------------------
 After user configured this C header file, not only C compiler compile it but
 also auto-gen tool parse user's configure setting.
 Here are recommend configure convention to make both work fine.

 1. All configurations in this file form as #define MACRO_NAME MACRO_VALUE format.
    Note the #define must be the first non-space character of a line

 2. To disable the optional configurable item. Please use // before #define,
    for example: //#define MEMORY_DEVICE_TYPE

 3. Please don't use #if , #elif , #else , #endif conditional macro key word here.
    Such usage might cause compile result conflict with auto-gen tool parsing result.
    Auto-Gen tool will show error and stop.
    3.1.  any conditional keyword such as #if , #ifdef , #ifndef , #elif , #else detected.
          execpt this #ifndef __CUSTOM_MEMORYDEVICE__
    3.2.  any duplicated MACRO_NAME parsed. For example auto-gen tool got 
          2nd MEMORY_DEVICE_TYPE macro value.
 ****************************************************************************
*/

/*
 ****************************************************************************
 Step 1: Specify memory device type and its complete part number
         Possible memory device type: LPSDRAM (SDR, DDR)
 ****************************************************************************
*/

#define BOARD_ID                MT6582_EVB

//#define CS_PART_NUMBER[0]       H9TP32A8JDACPR_KGM
//#define CS_PART_NUMBER[1]       KMK5U000VM_B309
#define CS_PART_NUMBER[0]       H9TP18A8JDMCPR_KGM    //lenovo.sw : add to support 1+16G hynix emmc for phaeton 
#define CS_PART_NUMBER[1]       KMI8U000MA_B605    //lenovo.sw : add to support 2+16G samsung emmc  for Pheaton_row
#define CS_PART_NUMBER[2]       KMK8X000VM_B412 //lenovo.sw jixj : add to support 1+16G samsung emmc for phaeton 

#endif /* __CUSTOM_MEMORYDEVICE__ */
