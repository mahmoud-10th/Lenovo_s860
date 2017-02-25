//#define DEBUG

#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../miui_inter.h"
#include "../miui.h"
#include "../../../miui_intent.h"
#include "../../../recovery.h"/* lenovo-sw wangxf14 20130812 add, add for lenovo recovery full and diff package update entry */

/*
 *_sd_show_dir show file system on screen
 *return MENU_BACK when pree backmenu
 */
#define SD_MAX_PATH 256
#define DEFAULT_MAX_PATH 128

//callback function , success return 0, non-zero fail
int file_install(char *file_name, int file_len, void *data)
{
    return_val_if_fail(file_name != NULL, RET_FAIL);
    return_val_if_fail(strlen(file_name) <= file_len, RET_INVALID_ARG);
    return_val_if_fail(data != NULL, RET_FAIL);
    struct _menuUnit *p = (pmenuUnit)data;
    if (RET_YES == miui_confirm(3, p->name, p->desc, p->icon)) {
        miuiIntent_send(INTENT_INSTALL, 3, file_name, "0", "1");
        return 0;
    }
    else return -1;
}
//callback funtion file filter, if access ,return 0; others return -1
int file_filter(char *file, int file_len)
{
    return_val_if_fail(file != NULL, RET_FAIL);
    int len = strlen(file);
    return_val_if_fail(len <= file_len, RET_INVALID_ARG);
    if (len >= 4 && strncasecmp(file + len -4, ".zip", 4) == 0)
        return 0;
    return -1;
     
}
static STATUS sd_menu_show(menuUnit *p)
{
    //ensure_mounte sd path
    struct _intentResult* result = miuiIntent_send(INTENT_MOUNT, 1, "/sdcard");
    //whatever wether sdd is mounted, scan sdcard and go on
    //assert_if_fail(miuiIntent_result_get_int() == 0);
    int ret ;
    ret = file_scan("/sdcard", sizeof("/sdcard"), p->name, strlen(p->name), &file_install, (void *)p, &file_filter);
    if (ret == -1) return MENU_BACK;
    return ret;
}

static STATUS sdext_menu_show(menuUnit *p)
{
    //ensure_mounte sd path
    struct _intentResult* result = miuiIntent_send(INTENT_MOUNT, 1, "/external_sd");
    //whatever wether sdd is mounted, scan sdcard and go on
    //assert_if_fail(miuiIntent_result_get_int() == 0);
    int ret ;
    ret = file_scan("/external_sd", sizeof("/external_sd"), p->name, strlen(p->name), &file_install, (void *)p, &file_filter);
    if (ret == -1) return MENU_BACK;
    return ret;
}
static STATUS sd_update_show(menuUnit *p)
{
    char new_path[SD_MAX_PATH] = "/sdcard/update.zip";
    int wipe_cache = 0;
    if (RET_YES == miui_confirm(3, p->name, p->desc, p->icon)) {
        miuiIntent_send(INTENT_INSTALL, 3, new_path, "0", "1");
    }
    return MENU_BACK;
}

/* Begin, lenovo-sw wangxf14 20130705 add, add for lenovo recovery update entry for debug */
static STATUS sd_lenovo_update_show(menuUnit *p)
{
//    char new_path[SD_MAX_PATH] = "/sdcard/recovery/dvt_update.zip";
    char new_path[SD_MAX_PATH] = "/sdcard/update.zip";
    int wipe_cache = 0;
    if (RET_YES == miui_confirm(3, p->name, p->desc, p->icon)) {	
        miuiIntent_send(INTENT_INSTALL_LENOVO, 3, new_path, "0", "1");
    }
    return MENU_BACK;
}
/* End, lenovo-sw wangxf14 20130705 add, add for lenovo recovery update entry for debug */

struct _menuUnit * sd_ui_init()
{
    struct _menuUnit *p = common_ui_init();
    return_null_if_fail(p != NULL);
    strncpy(p->name, "<~sd.name>", MENU_LEN);
    strncpy(p->title_name, "<~sd.title_name>", MENU_LEN);
    strncpy(p->icon, "@sd",  MENU_LEN);
    p->result = 0;
    return_null_if_fail(menuNode_init(p) != NULL);
	
    //install from sd
    struct _menuUnit  *temp = common_ui_init();
    return_null_if_fail(temp != NULL);
    menuUnit_set_icon(temp, "@sd.choose");
    strncpy(temp->name, "<~sd.install.name>", MENU_LEN);
    temp->show = &sd_menu_show;
    assert_if_fail(menuNode_add(p, temp) == RET_OK);

//lenovo-sw wangxf14, add 20130605 begin, shield default install path
#if 1	
    //install update.bin from sd
    temp = common_ui_init();
    menuUnit_set_icon(temp, "@sd.install");
    strncpy(temp->name,"<~sd.update.name>", MENU_LEN);
    temp->show = &sd_lenovo_update_show;
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
#endif
//lenovo-sw wangxf14, add 20130605 end, shield default install path

    if (acfg()->sd_ext == 1)
    {
        //install from external_sd
        struct _menuUnit  *temp = common_ui_init();
        return_null_if_fail(temp != NULL);
        menuUnit_set_icon(temp, "@sd.choose");
        strncpy(temp->name, "<~sdext.install.name>", MENU_LEN);
        temp->show = &sdext_menu_show;
        assert_if_fail(menuNode_add(p, temp) == RET_OK);
    }
    return p;
}

/* Begin, lenovo-sw wangxf14 20130705 add, add for lenovo recovery update entry */
static STATUS lenovo_sd_prompt_show(menuUnit *p)
{
//    char new_path[SD_MAX_PATH] = "/sdcard/recovery/dvt_update.zip";
    char new_path[DEFAULT_MAX_PATH] = "/sdcard/update.zip";
    char flag[] = "0";
//    int wipe_cache = 0;

#ifdef LENOVO_SHARED_SDCARD
/* Begin, lenovo-sw wangxf14 20131115 add, add for fix memory leak of the function of lenovo_ota_fix_patch_location */
    char *update_package = new_path;
    if(update_package)
    {
        printf("lenovo_sd_prompt_show: default_path=%s\n",update_package);

	 int len = strlen(update_package) + 128;
        char* path = (char*)malloc(len*sizeof(char));

        printf("len = %d\n", len);
	 int ret = lenovo_ota_fix_patch_location_new(update_package, path, len);

        printf("lenovo_ota_fix_patch_location_new: ret=%d, default_path = %s, path=%s\n",ret, update_package, path);	 

	 if (1 == ret)
	 {
            strncpy(new_path, path, DEFAULT_MAX_PATH);
	 }

	 free(path);
		
        printf("lenovo_sd_prompt_show: path=%s\n",new_path);		
		
    }        
/* End, lenovo-sw wangxf14 20131115 add, add for fix memory leak of the function of lenovo_ota_fix_patch_location */

#else
/* Begin, lenovo-sw wangxf14 20130916 add, add the function of ota update zip package in sdcard and sdcard2 */
    char *default_path = new_path;
    if(default_path){
        printf("lenovo_sd_prompt_show: default_path=%s\n",default_path);

        int len = strlen(default_path) + 128;
        char* path = (char*)malloc(len*sizeof(char));

        printf("len = %d\n", len);
        int ret = try_and_get_ota_update_path(default_path, path, len);

        printf("try_and_get_ota_update_path: ret=%d, default_path = %s, path=%s\n",ret, default_path, path);

	 if (1 == ret)
	 {
            strncpy(new_path, path, DEFAULT_MAX_PATH);
	 }

	 free(path);
		
        printf("lenovo_sd_prompt_show: path=%s\n",new_path);
    }
/* End, lenovo-sw wangxf14 20130916 add, add the function of ota update zip package in sdcard and sdcard2 */	
#endif


    if (RET_YES == miui_prompt(3, p->name, p->desc, flag)) {
/* Begin, lenovo-sw wangxf14 20130812 add, add for lenovo recovery full and diff package update entry */		
        int tmpFlag = is_full_update_zip(new_path);
	 miui_debug("is_full_update_zip flag = %d\n", tmpFlag);
	 set_full_otapackage_flag(tmpFlag);
/* End, lenovo-sw wangxf14 20130812 add, add for lenovo recovery full and diff package update entry */

/* Begin, lenovo-sw wangxf14 20130912 modify, modify for protect suddenness that phone power off in recovery install */
        set_ota_update_bootloader_message();
        miuiIntent_send(INTENT_INSTALL_LENOVO, 3, new_path, "0", "1");
        clean_ota_update_bootloader_message();
/* End, lenovo-sw wangxf14 20130912 modify, modify for protect suddenness that phone power off in recovery install */

    }
    return MENU_BACK;
}

struct _menuUnit * lenovo_sd_ui_init()
{
    struct _menuUnit *p = common_ui_init();
    return_null_if_fail(p != NULL);
    strncpy(p->name, "<~sd.name>", MENU_LEN);
#ifdef LENOVO_MULTI_STORAGE	
    menuUnit_set_desc(p, "<~install.prompt2>");
#else
    menuUnit_set_desc(p, "<~install.prompt>");
#endif
    p->result = 0;
    p->show = &lenovo_sd_prompt_show;
    return p;
}
/* End, lenovo-sw wangxf14 20130705 add, add for lenovo recovery update entry */
