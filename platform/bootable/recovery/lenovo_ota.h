#ifndef LENOVO_OTA_H_
#define LENOVO_OTA_H_

#ifdef __cplusplus
extern "C" {
#endif

void lenovo_ota_remove_package_and_write_result(char *update_package, const int status);
#if 0
char * lenovo_ota_fix_patch_location(char * update_package);
#endif
int lenovo_ota_fix_patch_location_new(char * default_path, char *path, int len);/* lenovo-sw wangxf14 20131115 add, add for fix memory leak of the function of lenovo_ota_fix_patch_location */
void delete_data(void);


#ifdef EASYIMAGE_SUPPORT    
void clean_easyimage_file_if_need(char * update_package);
#endif

#ifdef __cplusplus
}
#endif
#endif 
