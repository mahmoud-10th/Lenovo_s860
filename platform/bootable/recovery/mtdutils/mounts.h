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
 */

#ifndef MTDUTILS_MOUNTS_H_
#define MTDUTILS_MOUNTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LENOVO_RECOVERY_SUPPORT
struct MountedVolume {
    const char *device;
    const char *mount_point;
    const char *filesystem;
    const char *flags;
};

typedef struct MountedVolume MountedVolume;

typedef struct {
    MountedVolume *volumes;
    int volumes_allocd;
    int volume_count;
} MountsState;

#else
typedef struct MountedVolume MountedVolume;
#endif

int scan_mounted_volumes(void);

const MountedVolume *find_mounted_volume_by_device(const char *device);

const MountedVolume *
find_mounted_volume_by_mount_point(const char *mount_point);

int unmount_mounted_volume(const MountedVolume *volume);

int remount_read_only(const MountedVolume* volume);

#ifdef __cplusplus
}
#endif

#endif  // MTDUTILS_MOUNTS_H_
