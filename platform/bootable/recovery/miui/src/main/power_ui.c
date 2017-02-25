//#define DEBUG //wangxf14_debug

#include <stdlib.h>
#include "../miui_inter.h"
#include "../miui.h"
#include "../../../miui_intent.h"

#define POWER_REBOOT				0
#define POWER_RECOVERY				1
#define POWER_BOOTLOADER			2
#define POWER_POWEROFF			3
//lenovo-sw wangxf14, add 20130607 begin, add dual reboot ui
#define POWER_REBOOT_1				4
#define POWER_REBOOT_2				5
//lenovo-sw wangxf14, add 20130607 end, add dual reboot ui

/*Begin, lenovo-sw wangxf14 2013.7.4 add, add get system flag function*/
#define LENOVO_OTA_PROC  "/proc/system_flag"
#define SYSTEM_FLAG '0'
#define SYSTEM_BACKUP_FLAG '2'

static char get_system_flag(void)
{
  int fd;
  size_t s;
  char buf[10];
  char flag;
  fd = open(LENOVO_OTA_PROC, O_RDONLY);
  if (fd < 0) {
      miui_debug("fail to open lenovo ota flag path!\n");
      return 0;
  }

  s = read(fd, buf, sizeof(buf));
  close(fd);

  if (s <= 0) {
      miui_debug("could not read system flag\n");
      return 0;
  }
  flag = buf[0];
  miui_debug("system flag=%c\n", flag);
  return flag;
}
/*End, lenovo-sw wangxf14 2013.7.4 add, add get system flag function*/

static STATUS power_child_show(menuUnit *p)
{
    //confirm
    if (RET_YES == miui_confirm(3, p->name, p->desc, p->icon)) {
        switch(p->result) {
            case POWER_REBOOT:
                miuiIntent_send(INTENT_REBOOT, 1, "reboot");
                break;
            case POWER_BOOTLOADER:
                miuiIntent_send(INTENT_REBOOT, 1, "bootloader");
                break;
            case POWER_RECOVERY:
                miuiIntent_send(INTENT_REBOOT, 1, "recovery");
                break;
            case POWER_POWEROFF:
                miuiIntent_send(INTENT_REBOOT, 1, "poweroff");
		  break;
//lenovo-sw wangxf14, add 20130607 begin, add dual reboot ui		  
//Begin, lenovo-sw wangxf14 add 20130621, fix system2 reboot failure
	     case POWER_REBOOT_1:
		  printf("POWER_REBOOT_1!\n");		 	
		  miuiIntent_send(INTENT_REBOOT, 1, "system1");
		  break;
	     case POWER_REBOOT_2:
		  printf("POWER_REBOOT_2!\n");
		  miuiIntent_send(INTENT_REBOOT, 1, "system2");
                break;
//End, lenovo-sw wangxf14 add 20130621, fix system2 reboot failure				
//lenovo-sw wangxf14, add 20130607 end, add dual reboot ui				
            default:
                assert_if_fail(0);
            break;
        }
    }
    return MENU_BACK;
}

/*Begin, lenovo-sw wangxf14 2013.7.4 add, add get system flag function*/
static STATUS lenovo_power_child_doing(menuUnit *p)
{
    switch(p->result) 
    {
        case POWER_REBOOT:
            miuiIntent_send(INTENT_REBOOT, 1, "reboot");
            break;
        case POWER_BOOTLOADER:
            miuiIntent_send(INTENT_REBOOT, 1, "bootloader");
            break;
        case POWER_RECOVERY:
            miuiIntent_send(INTENT_REBOOT, 1, "recovery");
            break;
        case POWER_POWEROFF:
            miuiIntent_send(INTENT_REBOOT, 1, "poweroff");
	  break;
     case POWER_REBOOT_1:
	  miuiIntent_send(INTENT_REBOOT, 1, "system1");
	  break;
     case POWER_REBOOT_2:
	  miuiIntent_send(INTENT_REBOOT, 1, "system2");
            break;
        default:
            assert_if_fail(0);
        break;
    }

    return MENU_BACK;
}
/*End, lenovo-sw wangxf14 2013.7.4 add, add get system flag function*/

struct _menuUnit* reboot_ui_init()
{
    struct _menuUnit *temp = common_ui_init();
    return_null_if_fail(temp != NULL);
    strncpy(temp->name, "<~reboot.null>", MENU_LEN);
    menuUnit_set_icon(temp, "@reboot");
    temp->result = POWER_REBOOT;
    temp->show = &power_child_show;
    return temp;
}

struct _menuUnit * lenovo_reboot_ui_init()
{
    struct _menuUnit *temp = common_ui_init();
    return_null_if_fail(temp != NULL);
    strncpy(temp->name, "<~reboot.null>", MENU_LEN);
    menuUnit_set_icon(temp, "@reboot");
    temp->result = POWER_REBOOT;
    temp->show = &lenovo_power_child_doing;
    return temp;
}

//lenovo-sw wangxf14, add 20130607 begin, add dual reboot ui
struct _menuUnit * dual_reboot_ui_init()
{
    struct _menuUnit *p = common_ui_init();
    return_null_if_fail(p != NULL);
    strncpy(p->name, "<~dual_reboot.name>", MENU_LEN);
    menuUnit_set_icon(p,"@backup");
//    menuUnit_set_show(p, &common_menu_show); //lenovo-sw wangxf14 add 2013-06-20, add for button menu
    menuUnit_set_show(p, &common_button_menu_show);//lenovo-sw wangxf14 add 2013-06-20, add for button menu
    p->result = 0;
    assert_if_fail(menuNode_init(p) != NULL);
    //reboot1
    struct _menuUnit *temp = common_ui_init();
    return_null_if_fail(temp != NULL);

/*Begin, lenovo-sw wangxf14 2013.7.4 add, add set menu by system flag */	
    if( SYSTEM_FLAG == get_system_flag() )
        strncpy(temp->name, "<~dual_reboot.system1.recently>", MENU_LEN);		
    else
        strncpy(temp->name, "<~dual_reboot.system1>", MENU_LEN);
/*End, lenovo-sw wangxf14 2013.7.4 add, add set menu by system flag */	

    menuUnit_set_icon(temp, "@backup");
    temp->result = POWER_REBOOT_1;
    temp->show = &lenovo_power_child_doing;//lenovo-sw wangxf14 20130705 add, add for reset function
    assert_if_fail(menuNode_add(p, temp) == RET_OK);

    //reboot2
    temp = common_ui_init();
    return_null_if_fail(temp != NULL);
	
/*Begin, lenovo-sw wangxf14 2013.7.4 add, add set menu by system flag */	
    if( SYSTEM_BACKUP_FLAG == get_system_flag() )	
        strncpy(temp->name, "<~dual_reboot.system2.recently>", MENU_LEN);
    else
        strncpy(temp->name, "<~dual_reboot.system2>", MENU_LEN);
/*End, lenovo-sw wangxf14 2013.7.4 add, add set menu by system flag */	
	
    menuUnit_set_icon(temp, "@backup");
    temp->result = POWER_REBOOT_2;
    temp->show = &lenovo_power_child_doing;//lenovo-sw wangxf14 20130705 add, add for reset function
    assert_if_fail(menuNode_add(p, temp) == RET_OK);	
 
    return p;
}
//lenovo-sw wangxf14, add 20130607 end, add dual reboot ui

struct _menuUnit * power_ui_init()
{
    struct _menuUnit *p = common_ui_init();
    return_null_if_fail(p != NULL);
    strncpy(p->name, "<~power.name>", MENU_LEN);
    menuUnit_set_title(p, "<~power.title>");
    menuUnit_set_icon(p,"@power");
    menuUnit_set_show(p, &common_menu_show);
    p->result = 0;
    assert_if_fail(menuNode_init(p) != NULL);
    //reboot
    struct _menuUnit *temp = common_ui_init();
    return_null_if_fail(temp != NULL);
    strncpy(temp->name, "<~reboot.null>", MENU_LEN);
    menuUnit_set_title(temp, "<~reboot.null.title>");
    menuUnit_set_icon(temp, "@reboot");
    temp->result = POWER_REBOOT;
    temp->show = &power_child_show;
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    //reboot bootloader
    temp = common_ui_init();
    return_null_if_fail(temp != NULL);
    strncpy(temp->name, "<~reboot.bootloader>", MENU_LEN);
    menuUnit_set_icon(temp, "@reboot.bootloader");
    temp->result = POWER_BOOTLOADER;
    temp->show = &power_child_show;
    assert_if_fail(menuNode_add(p, temp) == RET_OK);

    //reboot recovery
    temp = common_ui_init();
    return_null_if_fail(temp != NULL);
    strncpy(temp->name, "<~reboot.recovery>", MENU_LEN);
    menuUnit_set_icon(temp, "@reboot.recovery");
    temp->result = POWER_RECOVERY;
    temp->show = &power_child_show;
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    //poweroff
    temp = common_ui_init();
    return_null_if_fail(temp != NULL);
    strncpy(temp->name, "<~reboot.poweroff>", MENU_LEN);
    menuUnit_set_icon(temp, "@power");
    temp->result = POWER_POWEROFF;
    temp->show = &power_child_show;
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return p;
}

