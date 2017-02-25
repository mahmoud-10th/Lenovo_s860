/*
 * Copyright (C) 2014 lenovo MIUI
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Descriptions:
 * -------------
 * Main executable for MIUI Installer Binary
 *
 */
#define DEBUG //lenovo-sw wangxf14
 
#include "../miui_inter.h"
#include "../miui.h"
#include "../../../recovery.h"/* lenovo-sw wangxf14 20130913 add, add for lenovo recovery factory bug */

/* Begin, lenovo-sw wangxf14 20130822 add, add kmsg debug */
void ldklog_init(void);
void ldklog_set_level(int level);
void ldklog_write(int level, const char *fmt, ...)
    __attribute__ ((format(printf, 2, 3)));

#define LDKLOG_DEFAULT_LEVEL  4  /* messages <= this level are logged */

static int ldklog_fd = -1;
static int ldklog_level = LDKLOG_DEFAULT_LEVEL;

void ldklog_set_level(int level) {
    ldklog_level = level;
}

void ldklog_init(void) 
{
    static const char *name = "/dev/__kmsg__";
    if (mknod(name, S_IFCHR | 0600, (1 << 8) | 11) == 0) {
        ldklog_fd = open(name, O_WRONLY);
        fcntl(ldklog_fd, F_SETFD, FD_CLOEXEC);
        unlink(name);
    }
}

#define LDLOG_BUF_MAX 512

void ldklog_write(int level, const char *fmt, ...)
{
    char buf[LDLOG_BUF_MAX];
    va_list ap;

    if (level > ldklog_level) return;
    if (ldklog_fd < 0) return;

    va_start(ap, fmt);
    vsnprintf(buf, LDLOG_BUF_MAX, fmt, ap);
    buf[LDLOG_BUF_MAX - 1] = 0;
    va_end(ap);
    write(ldklog_fd, buf, strlen(buf));
}
/* End, lenovo-sw wangxf14 20130822 add, add kmsg debug */

struct _menuUnit *g_main_menu;//main menu
struct _menuUnit *g_root_menu;//language ui

static STATUS main_ui_clear(struct _menuUnit *p)
{
    //release tree, post order release
    if (p == NULL)
        return RET_OK;
    main_ui_clear(p->child);
    main_ui_clear(p->nextSilbing);
    free(p);
    return RET_OK;
}

static STATUS main_ui_clear_root()
{
    return main_ui_clear(g_root_menu);
}
static struct _menuUnit *tree_init()
{
    miui_debug("tree_init entery\n");
    g_root_menu = lang_ui_init();
    return_null_if_fail(g_root_menu != NULL);
    //main menu
    g_main_menu = common_ui_init();
    return_null_if_fail(g_main_menu != NULL);
    strncpy(g_main_menu->name, "<~mainmenu.name>", MENU_LEN);
//    menuUnit_set_show(g_main_menu, &common_ui_show); //lenovo-sw wangxf14 add 2013-06-20, add for button menu
    menuUnit_set_show(g_main_menu, &common_button_ui_show);//lenovo-sw wangxf14 add 2013-06-20, add for button menu
    g_root_menu->child = g_main_menu;
    g_main_menu->parent = g_root_menu;
    //add back operation
    g_main_menu = menuNode_init(g_main_menu);
    //inital mainmenu 
    //cancel reboot
    //assert_if_fail(menuNode_add(g_main_menu, reboot_ui_init()) == RET_OK);
    //add dual_reboot
//    assert_if_fail(menuNode_add(g_main_menu, dual_reboot_ui_init()) == RET_OK);//lenovo-sw wangxf14, add 20130607
    assert_if_fail(menuNode_add(g_main_menu, lenovo_reboot_ui_init()) == RET_OK);//lenovo-sw wangxf14, add 20130820    
    //add sd operation 
//    assert_if_fail(menuNode_add(g_main_menu, sd_ui_init()) == RET_OK);
//add lenovo_sd_ui_init
    assert_if_fail(menuNode_add(g_main_menu, lenovo_sd_ui_init()) == RET_OK);//lenovo-sw wangxf14, add 20130705

    //add mount and toggle usb storage
//    assert_if_fail(menuNode_add(g_main_menu, mount_ui_init()) == RET_OK); //lenovo-sw wangxf14, shield function
    //add backup
//    assert_if_fail(menuNode_add(g_main_menu, backup_ui_init()) == RET_OK); //lenovo-sw wangxf14, shield function
    //add power
    //assert_if_fail(menuNode_add(g_main_menu, power_ui_init()) == RET_OK);
    //add tools operation
//    assert_if_fail(menuNode_add(g_main_menu, tool_ui_init()) == RET_OK); //lenovo-sw wangxf14, shield function
    //add info
//    assert_if_fail(menuNode_add(g_main_menu, info_ui_init()) == RET_OK); //lenovo-sw wangxf14, shield function
    //add wipe
    assert_if_fail(menuNode_add(g_main_menu, wipe_ui_init()) == RET_OK);
    //add poweroff
    assert_if_fail(menuNode_add(g_main_menu, poweroff_ui_init()) == RET_OK); //lenovo-sw wangxf14, add 20130607

    struct stat st;
    if (stat(RECOVERY_PATH, &st) != 0)
    {
        mkdir(RECOVERY_PATH, 0755);
    }
    return g_root_menu;
}
STATUS main_ui_init()
{
#ifndef _MIUI_NODEBUG
    miui_debug("function main_ui_init enter miui debug\n");
    remove_directory("/tmp/miui-memory");
    miui_memory_debug_init();
#endif
    miui_printf("\nInitializing...\n");
    miui_debug("wangxf14 test main_ui_init miui debug\n");//lenovo-sw wangxf14
    remove_directory(MIUI_TMP);
    unlink(MIUI_TMP_S);
    create_directory(MIUI_TMP);
    symlink(MIUI_TMP, MIUI_TMP_S);

/* Begin, lenovo-sw wangxf14 20130822 add, add kmsg debug init */
    ldklog_init();
    ldklog_set_level(7);
/* End, lenovo-sw wangxf14 20130822 add, add kmsg debug init */
    lenovo_debug("wangxf14 lenovo debug\n");

    //miui  config init
    miui_ui_init();
    //read config file and execute it
    miui_ui_config("/res/init.conf");
    //input thread start
    ui_init();
    //graphic thread start, print background
    ag_init();

    //miui_ui start
    miui_ui_start();
	//device config after miui_ui start
    miui_ui_config("/res/device.conf");//lenovo-sw wangxf14
    tree_init();


    miui_font( "0", "ttf/DroidSans.ttf;ttf/DroidSansFallback.ttf;", "18" );//lenovo-sw wangxf14 add 2013-06-20, add for lenovo font; modify font size at 20130814
    miui_font( "1", "ttf/DroidSans.ttf;ttf/DroidSansFallback.ttf;", "24" );//lenovo-sw wangxf14 add 2013-06-20, add for lenovo font; modify font size at 20130814

    if( 1 == get_language_flag() )
        miui_loadlang("langs/en.lang");
    else
        miui_loadlang("langs/cn.lang");
    return RET_OK;
}
STATUS main_ui_show()
{
    struct _menuUnit *node_show = g_root_menu;
    int index = 0;
    //show mainmenu

   clean_ota_update_bootloader_message();/* lenovo-sw wangxf14 20130913 add, add for lenovo recovery factory bug */

    while (index != MENU_QUIT)
    {
        return_val_if_fail(node_show != NULL, RET_FAIL);
        return_val_if_fail(node_show->show != NULL, RET_FAIL);
        miui_set_isbgredraw(1);
        index = node_show->show(node_show);
        if (index > 0 && index < MENU_BACK) {
            node_show = node_show->get_child_by_index(node_show, index);
        }
        else if (index == MENU_BACK || index == 0 )
        {
            if (node_show->parent != NULL)
                node_show = node_show->parent;
        }
        else {
            //TODO add MENU QUIT or some operation?
            miui_error("invalid index %d in %s\n", index, __FUNCTION__);
        }
    }
    return RET_FAIL;
}

STATUS main_ui_release()
{

#ifndef _MIUI_NODEBUG
  miui_dump_malloc();
#endif
  miui_ui_end();
  ag_close_thread();
  //clear ui tree
  main_ui_clear_root(); 
  //-- Release All
  ag_closefonts();  //-- Release Fonts
  miui_debug("Font Released\n");
  miui_ev_exit();        //-- Release Input Engine
  miui_debug("Input Released\n");
  ag_close();       //-- Release Graph Engine
  miui_debug("Graph Released\n");

  miui_debug("Cleanup Temporary\n");
  usleep(500000);
  unlink(MIUI_TMP_S);
  remove_directory(MIUI_TMP);
  return RET_OK;
}

