/*
 * Copyright (C) 2007 The Android Open Source Project
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
 * 
 * lenovo-sw wangxf14, add 20130607, add Poweroff_ui.c for power off phone item
 *
 */

#include <stdlib.h>
#include "../miui_inter.h"
#include "../miui.h"
#include "../../../miui_intent.h"

#define POWER_OFF                 0

static STATUS poweroff_show(menuUnit *p)
{
    //confirm
//    if (RET_YES == miui_confirm(3, p->name, p->desc, p->icon)) { //lenovo-sw wangxf14 20130705 disable for lenovo control
        switch(p->result) {
            case POWER_OFF:
                miuiIntent_send(INTENT_REBOOT, 1, "poweroff");
                break;
            default:
                assert_if_fail(0);
            break;
        }
//    } //lenovo-sw wangxf14 20130705 disable for lenovo control
    return MENU_BACK;
}

struct _menuUnit * poweroff_ui_init()
{
    struct _menuUnit *p = common_ui_init();
    return_null_if_fail(p != NULL);
    strncpy(p->name, "<~power.poweroff>", MENU_LEN);
    menuUnit_set_icon(p,"@power");
    p->result = POWER_OFF;
    p->show = &poweroff_show;
    return p;
}
