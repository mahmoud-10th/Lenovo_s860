#include "../miui_inter.h"
#include "../miui.h"
#include "../../../miui_intent.h"

#include "../../../recovery.h"/* lenovo-sw wangxf14 20130912 modify, modify for protect suddenness that phone power off in recovery system operate */

#define WIPE_FACTORY     1
#define WIPE_DATA        2
#define WIPE_CACHE       3
#define WIPE_DALVIK      4
#define FUSE_WIPE_DATA    5
#define FUSE_FORMAT_DATA    6

#define FORMAT_SYSTEM    11
#define FORMAT_DATA      12
#define FORMAT_CACHE     13
#define FORMAT_BOOT      14
#define FORMAT_SDCARD    15
#define FORMAT_ALL       16

STATUS wipe_item_show(menuUnit *p)
{
    if (RET_YES == miui_confirm(3, p->name, p->desc, p->icon)) {
        miui_busy_process();
        switch(p->result) {
            case WIPE_FACTORY:
                miuiIntent_send(INTENT_WIPE, 1, "/cache");
                miuiIntent_send(INTENT_WIPE, 1, "/data");
                break;
            case WIPE_DATA:
                miuiIntent_send(INTENT_WIPE, 1, "/data");
                break;
            case WIPE_CACHE:
                miuiIntent_send(INTENT_WIPE, 1, "/cache");
                break;
            case WIPE_DALVIK:
                miuiIntent_send(INTENT_WIPE, 1, "dalvik-cache");
                break;
            case FORMAT_SYSTEM:
                miuiIntent_send(INTENT_FORMAT, 1, "/system");
                break;
            case FORMAT_DATA:
                miuiIntent_send(INTENT_FORMAT, 1, "/data");
                break;
            case FORMAT_CACHE:
                miuiIntent_send(INTENT_FORMAT, 1, "/cache");
                break;
            case FORMAT_BOOT:
                miuiIntent_send(INTENT_FORMAT, 1, "/boot");
                break;
            case FORMAT_SDCARD:
                miuiIntent_send(INTENT_FORMAT, 1, "/sdcard");
                break;
            case FORMAT_ALL:
                miuiIntent_send(INTENT_FORMAT, 1, "/system");
                miuiIntent_send(INTENT_FORMAT, 1, "/data");
                miuiIntent_send(INTENT_FORMAT, 1, "/cache");
                break;
            default:
                assert_if_fail(0);
                break;
        }
    }
    return MENU_BACK;

}

/* Begin, lenovo-sw wangxf14 20130705 add, add for lenovo recovery wipe entry */
STATUS lenovo_wipe_item_show(menuUnit *p)//wangxf14_debug
{
    char flag[] = "1";
    if (RET_YES == miui_prompt(3, p->name, p->desc, flag)) {
//        set_ota_update_bootloader_message();/* lenovo-sw wangxf14 20130912 modify, modify for protect suddenness that phone power off in recovery system operate */
        switch(p->result) {
            case WIPE_FACTORY:
                set_data_and_cache_bootloader_message();				
                miuiIntent_send(INTENT_WIPE_LENOVO, 3, "1", "/cache", "/data");
                clean_bootloader_message();
                break;
            case WIPE_DATA:
                set_data_bootloader_message();
                miuiIntent_send(INTENT_WIPE_LENOVO, 2, "1", "/data");
		        clean_bootloader_message();
                break;
            case WIPE_CACHE:
		        set_cache_bootloader_message();
                miuiIntent_send(INTENT_WIPE_LENOVO, 2, "1", "/cache");
		        clean_bootloader_message();
                break;
            case WIPE_DALVIK:
                miuiIntent_send(INTENT_WIPE, 1, "dalvik-cache");
                break;
            case FUSE_WIPE_DATA:
                set_fuse_wipe_data_bootloader_message();				
                miuiIntent_send(INTENT_WIPE_LENOVO, 2, "1", "fuse_wipe_data");
		  clean_bootloader_message();				
                break;
            case FUSE_FORMAT_DATA:
                set_data_bootloader_message();				
                miuiIntent_send(INTENT_WIPE_LENOVO, 2, "1", "/data");
		  clean_bootloader_message();				
                break;				
            case FORMAT_SYSTEM:
                miuiIntent_send(INTENT_FORMAT, 1, "/system");
                break;
            case FORMAT_DATA:
                miuiIntent_send(INTENT_FORMAT, 1, "/data");
                break;
            case FORMAT_CACHE:
                miuiIntent_send(INTENT_FORMAT, 1, "/cache");
                break;
            case FORMAT_BOOT:
                miuiIntent_send(INTENT_FORMAT, 1, "/boot");
                break;
            case FORMAT_SDCARD:
                miuiIntent_send(INTENT_FORMAT, 1, "/sdcard");
                break;
            case FORMAT_ALL:
                miuiIntent_send(INTENT_FORMAT, 1, "/system");
                miuiIntent_send(INTENT_FORMAT, 1, "/data");
                miuiIntent_send(INTENT_FORMAT, 1, "/cache");
                break;
            default:
                assert_if_fail(0);
                break;
        }
//        clean_ota_update_bootloader_message();/* lenovo-sw wangxf14 20130912 modify, modify for protect suddenness that phone power off in recovery system operate */
    }
    return MENU_BACK;
}
/* End, lenovo-sw wangxf14 20130705 add, add for lenovo recovery wipe entry */


STATUS wipe_menu_show(menuUnit *p)
{
    return_val_if_fail(p != NULL, RET_FAIL);
    int n = p->get_child_count(p);
    int selindex = 0;
    return_val_if_fail(n >= 1, RET_FAIL);
    return_val_if_fail(n < ITEM_COUNT, RET_FAIL);
    struct _menuUnit *temp = p->child;
    return_val_if_fail(temp != NULL, RET_FAIL);
    char **menu_item = malloc(n * sizeof(char *));
    assert_if_fail(menu_item != NULL);
    int i = 0;
    for (i = 0; i < n; i++)
    {
        menu_item[i] = temp->name;
        temp = temp->nextSilbing;
    }
    selindex = miui_mainmenu(p->name, menu_item, NULL, NULL, n);
    p->result = selindex;
    if (menu_item != NULL) free(menu_item);
    return p->result;
}
struct _menuUnit* wipe_ui_init()
{
    struct _menuUnit* p = common_ui_init();
    return_null_if_fail(p != NULL);
    return_null_if_fail(menuUnit_set_name(p, "<~wipe.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_title(p, "<~wipe.title>") == RET_OK);
    return_null_if_fail(menuUnit_set_icon(p, "@wipe") == RET_OK);
//    return_null_if_fail(RET_OK == menuUnit_set_show(p, &wipe_menu_show));//lenovo-sw wangxf14 add 2013-06-20, add for button menu
    return_null_if_fail(RET_OK == menuUnit_set_show(p, &common_button_menu_show));//lenovo-sw wangxf14 add 2013-06-20, add for button menu
    return_null_if_fail(menuNode_init(p) != NULL);
#ifdef LENOVO_SHARED_SDCARD
    //wipe_data
    struct _menuUnit* temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~fuse.wipe.data.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_desc(temp, "<~fuse.wipe.data.desc>") == RET_OK);//lenovo-sw wangxf14 add 20130705 for lenovo wipe
    return_null_if_fail(menuUnit_set_result(temp, FUSE_WIPE_DATA) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &lenovo_wipe_item_show));//lenovo-sw wangxf14 add 20130705 for lenovo wipe
    //wipe_cache
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~fuse.delete.data.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_desc(temp, "<~fuse.delete.data.desc>") == RET_OK);//lenovo-sw wangxf14 add 20130705 for lenovo wipe
    return_null_if_fail(menuUnit_set_result(temp, FUSE_FORMAT_DATA) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &lenovo_wipe_item_show));//lenovo-sw wangxf14 add 20130705 for lenovo wipe
#else
    //wipe_data/factory reset
    struct _menuUnit* temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~wipe.factory.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_desc(temp, "<~wipe.factory.desc>") == RET_OK); //lenovo-sw wangxf14 add 20130705 for lenovo wipe
    return_null_if_fail(menuUnit_set_result(temp, WIPE_FACTORY) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &lenovo_wipe_item_show));//lenovo-sw wangxf14 add 20130705 for lenovo wipe
    //wipe_data
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~wipe.data.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_desc(temp, "<~wipe.data.desc>") == RET_OK);//lenovo-sw wangxf14 add 20130705 for lenovo wipe
    return_null_if_fail(menuUnit_set_result(temp, WIPE_DATA) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &lenovo_wipe_item_show));//lenovo-sw wangxf14 add 20130705 for lenovo wipe
    //wipe_cache
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~wipe.cache.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_desc(temp, "<~wipe.cache.desc>") == RET_OK);//lenovo-sw wangxf14 add 20130705 for lenovo wipe
    return_null_if_fail(menuUnit_set_result(temp, WIPE_CACHE) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &lenovo_wipe_item_show));//lenovo-sw wangxf14 add 20130705 for lenovo wipe
#endif    

//lenovo-sw wangxf14, add 20130605 begin, shield some function	
#if 0	
    //wipe dalvik-cache
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~wipe.dalvik-cache.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, WIPE_DALVIK) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
    //format system
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~format.system.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, FORMAT_SYSTEM) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
    //format data
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~format.data.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, FORMAT_DATA) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
    //format cache
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~format.cache.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, FORMAT_CACHE) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
    //format BOOT
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~format.boot.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, FORMAT_BOOT) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
    //format SDCARD
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~format.sdcard.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, FORMAT_SDCARD) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
    //format ALL
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~format.all.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, FORMAT_ALL) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
#endif
//lenovo-sw wangxf14, add 20130605 end, shield some function
    return p;
}
