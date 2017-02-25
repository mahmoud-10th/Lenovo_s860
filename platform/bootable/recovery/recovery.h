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
 * lenovo-sw wangxf14 add for lenovo recovery
 * 
 */

#ifndef RECOVERY_H_
#define RECOVERY_H_

#ifdef VERIFIER_TEST
int get_full_otapackage_flag(void);

#else

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

void lenovo_set_update_result(int status);
int lenovo_wipe_clean(const char *volume);
int lenovo_fuse_wipe_clean(const char *volume);
int get_full_otapackage_flag(void);
void set_full_otapackage_flag(int flag);
int is_full_update_zip(const char *dir);
void set_ota_update_bootloader_message(void);
void clean_ota_update_bootloader_message(void);
void set_data_and_cache_bootloader_message(void);
void set_data_bootloader_message(void);
void set_cache_bootloader_message(void);
void set_fuse_wipe_data_bootloader_message(void);/* lenovo-sw wangxf14 20140226 add, porting mtk6582 recovery optimize for data clean */
void clean_bootloader_message(void);
int try_and_get_ota_update_path(char * default_path, char * path, int len);
int get_language_flag(void);

#ifdef __cplusplus
}
#endif

#endif

#endif  // RECOVERY_H_
