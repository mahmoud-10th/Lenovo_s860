#include "../miui_inter.h"
#include "../miui.h"
static STATUS lang_menu_show(menuUnit *p)
{
    return_val_if_fail(p != NULL, RET_FAIL);
    miui_font( "0", "ttf/DroidSans.ttf;ttf/DroidSansFallback.ttf;", "12" );
    miui_font( "1", "ttf/DroidSans.ttf;ttf/DroidSansFallback.ttf;", "18" );
    miui_loadlang("langs/en.lang");

    /*
    int ret = miui_langmenu(p->title_name,p->icon, 
            "简体中文", "欢迎到recovery", "@lang",
            "English", "Welcome to recovery", "@lang", 2);
    */
    int ret = miui_langmenu(p->title_name, p->icon);

    if (0 == ret)
    {
        miui_loadlang("langs/cn.lang");
        miui_font( "0", "ttf/DroidSansFallback.ttf;ttf/DroidSans.ttf", "12" );
        miui_font( "1", "ttf/DroidSansFallback.ttf;ttf/DroidSans.ttf", "18" );
        p->result = 1;
    }
    else if (1 == ret)
    {
        miui_loadlang("langs/en.lang");
        miui_font( "0", "ttf/DroidSans.ttf", "12" );
        miui_font( "1", "ttf/DroidSans.ttf", "18" );
        p->result = 1;
    }
    else {
        miui_error("should not be here");
        p->result = 0;
    }
    return p->result;
}

/* Begin, lenovo-sw wangxf14 add 2013-06-20, add for button menu*/
static STATUS lang_button_menu_show(menuUnit *p)
{
    return_val_if_fail(p != NULL, RET_FAIL);
    miui_font( "0", "ttf/DroidSans.ttf;ttf/DroidSansFallback.ttf;", "18" );
    miui_font( "1", "ttf/DroidSans.ttf;ttf/DroidSansFallback.ttf;", "24" );
    miui_loadlang("langs/en.lang");

    int ret = miui_lang_buttonmenu(p->title_name, p->icon);

    if (0 == ret)
    {
        miui_loadlang("langs/cn.lang");
        miui_font( "0", "ttf/DroidSansFallback.ttf;ttf/DroidSans.ttf", "18" );
        miui_font( "1", "ttf/DroidSansFallback.ttf;ttf/DroidSans.ttf", "24" );
        p->result = 1;
    }
    else if (1 == ret)
    {
        miui_loadlang("langs/en.lang");
        miui_font( "0", "ttf/DroidSans.ttf", "18" );
        miui_font( "1", "ttf/DroidSans.ttf", "24" );
        p->result = 1;
    }
    else {
        miui_error("should not be here");
        p->result = 0;
    }
    return p->result;
}
/* End, lenovo-sw wangxf14 add 2013-06-20, add for button menu*/

struct _menuUnit * lang_ui_init()
{
    struct _menuUnit *p = common_ui_init();
    return_null_if_fail(p != NULL);
    strncpy(p->name, "<~lang.name>", MENU_LEN);
    strncpy(p->title_name, "<~lang.title_name>", MENU_LEN);
    menuUnit_set_icon(p, "@lang");
    p->result = 0;
// p->show = &lang_menu_show; //lenovo-sw wangxf14 add 2013-06-20, add for button menu
    p->show = &lang_button_menu_show;//lenovo-sw wangxf14 add 2013-06-20, add for button menu
    return p;
}

