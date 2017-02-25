#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

#include "roots.h"

#include <sys/mount.h>

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */


//static const char *LENOVO_OTA_RESULT_FILE ="/data/data/com.lenovo.ota/files/updateResult";
//static const char *LENOVO_OTA_RESULT_FILE_NEW ="/data/ota/updateResult";
static const char *LENOVO_OTA_RESULT_FILE ="/data/ota/updateResult";

static const char * LENOVO_FACTORY_OTA_FILE_NAME = "factory_sd_ota.zip";
static const char * LENOVO_EASYIMAGE_FILE_NAME = "easyimage.zip";
static const char * LENOVO_FACTORY_SD_OTA_RESULT_FILE = "/cache/recovery/factory_ota_result";

static const char * LENOVO_FUSE_INTERNAL_SD_REAL_LOCATION = "/data/media/0";

static const char * LENOVO_FUSE_INTERNAL_SD_NON_MULTI_LOCATION = "/data/media";

static const char * EASYIMAGE_IN_DATA = "/data/easyimage.zip";


static bool remove_mota_file(const char *file_name)
{
    int  ret = 0;

    ret = unlink(file_name);

    if (ret == 0)
		return true;

	if (ret < 0 && errno == ENOENT)
		return true;

    return false;
}

static void write_result_file(const char *file_name, int result)
{
    char  dir_name[256];

    ensure_path_mounted("/data");

    strcpy(dir_name, file_name);
    char *p = strrchr(dir_name, '/');
    *p = 0;

    //[MIDHNJ75] [miaotao1] 2012-11-14 fix file permission issue.
    mode_t preUmask = umask(0000);
    chmod(dir_name,0777);
    chmod(file_name,0666);
    //End, [MIDHNJ75]


    fprintf(stdout, "dir_name = %s\n", dir_name);

    if (opendir(dir_name) == NULL)  {
        fprintf(stdout, "dir_name = '%s' does not exist, create it.\n", dir_name);
        if (mkdir(dir_name, 0777))  {
            fprintf(stdout, "can not create '%s' : %s\n", dir_name, strerror(errno));
            //[MIDHNJ75] [miaotao1] 2012-11-14 fix file permission issue.
            umask(preUmask);
            //End, [MIDHNJ75]
            return;
        }
    }

    int result_fd = open(file_name, O_RDWR | O_CREAT, 0666);

    if (result_fd < 0) {
        fprintf(stdout, "cannot open '%s' for output : %s\n", file_name, strerror(errno));
        //[MIDHNJ75] [miaotao1] 2012-11-14 fix file permission issue.
        umask(preUmask);
        //End, [MIDHNJ75]
        return;
    }

    char tmp[12]={0x0};
    sprintf(tmp,"%d",result);
    write(result_fd, tmp, sizeof(tmp));
   
    close(result_fd);  
    //Start, [MIDHNJ75] [miaotao1] 2012-11-14 fix file permission issue.
    umask(preUmask);
    //End, [MIDHNJ75]
}



static int EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}


static int isFactorySdOTA(const char *update_package)
{
	return EndsWith(update_package, LENOVO_FACTORY_OTA_FILE_NAME)
	       || EndsWith(update_package,LENOVO_EASYIMAGE_FILE_NAME);
}

void lenovo_ota_remove_package_and_write_result(char *update_package, const int status)
{
    fprintf(stdout, "miaotao1 log: write result : lenovo_ota_remove_package_and_write_result\n");
    
    if(isFactorySdOTA(update_package))
    {
    	fprintf(stdout, "miaotao1 log: write result : lenovo_ota_remove_package_and_write_result it's factory sd ota\n");
    	write_result_file(LENOVO_FACTORY_SD_OTA_RESULT_FILE,status);
    	//For factory sd ota and easyimage in sdcard, don't delete the update package, it will be used again on another device
    }else{
	    write_result_file(LENOVO_OTA_RESULT_FILE, status);
	    
	    fprintf(stdout, "lenovo_ota_remove_package_and_write_result : remove_mota_file\n");
	    if (update_package)
	        remove_mota_file(update_package);
	}
}



static int fileExists(const char * file)
{
	if(!file)
		return 0;
	if(access(file,F_OK)==0)
	{
		fprintf(stdout, "File exsists %s\n", file);
		return 1;
	}else
	{
		fprintf(stdout, "File NOT exsists %s\n", file);
		return 0;
	}
	
}

#if 0
char * lenovo_ota_fix_patch_location(char * update_package)
{
	if(!update_package){
		return update_package;
	}
	
	//if update_package exsit
	if(fileExists(update_package)){
		return update_package;
	}
	
	int len = strlen(update_package) + 64;
	char* modified_path = (char*)malloc(len*sizeof(char));
	memset(modified_path,0,len*sizeof(char));
	
	if(strncmp(update_package, "/sdcard", 7) == 0){
		
		//If it start with /sdcard0, /sdcard1, /sdcard2...
		int skip = 0;
		char * p = strchr(update_package+1, '/');
		if( p==NULL ){
			return update_package;
		}
		skip = p-update_package;
		
		//check if <LENOVO_FUSE_INTERNAL_SD_REAL_LOCATION>/xxxx exsit
		ensure_path_mounted(LENOVO_FUSE_INTERNAL_SD_NON_MULTI_LOCATION);
		strncpy(modified_path, LENOVO_FUSE_INTERNAL_SD_NON_MULTI_LOCATION, len);
		strncat(modified_path, update_package+skip, len-strlen(modified_path)-1);
		if(fileExists(modified_path)){
			return modified_path;
		}	
		
		//check if <LENOVO_FUSE_INTERNAL_SD_REAL_LOCATION>/xxxx exsit
		ensure_path_mounted(LENOVO_FUSE_INTERNAL_SD_REAL_LOCATION);
		strncpy(modified_path, LENOVO_FUSE_INTERNAL_SD_REAL_LOCATION, len);
		strncat(modified_path, update_package+skip, len-strlen(modified_path)-1);
		if(fileExists(modified_path)){
			return modified_path;
		}
		
		//check if /sdcard/xxxx exsit
		ensure_path_mounted("/sdcard");
		strncpy(modified_path, "/sdcard", len);
		strncat(modified_path, update_package+skip, len-strlen(modified_path)-1);
		if(fileExists(modified_path)){
			return modified_path;
		}
	
		//check if /sdcard1/xxxx exsit
		ensure_path_mounted("/sdcard1");
		strncpy(modified_path, "/sdcard1", len);
		strncat(modified_path, update_package+skip, len-strlen(modified_path)-1);
		if(fileExists(modified_path)){
			return modified_path;
		}
	
		//check if /sdcard2/xxxx exsit
		ensure_path_mounted("/sdcard2");
		strncpy(modified_path, "/sdcard2", len);
		strncat(modified_path, update_package+skip, len-strlen(modified_path)-1);
		if(fileExists(modified_path)){
			return modified_path;
		}
	}
	
	
	free(modified_path);
	return update_package;
}
#endif

/* Begin, lenovo-sw wangxf14 20131115 add, add for fix memory leak of the function of lenovo_ota_fix_patch_location */
int lenovo_ota_fix_patch_location_new(char * default_path, char * path, int len)
{
	if(!path){
		return 0;
	}

	memset(path,0,len*sizeof(char));
	
	//if update_package exsit
	if(fileExists(default_path)){
		strncpy(path, default_path, len);		
		return 1;
	}
	
	if(strncmp(default_path, "/sdcard", 7) == 0){
		
		//If it start with /sdcard0, /sdcard1, /sdcard2...
		int skip = 0;
		char * p = strchr(default_path+1, '/');
		if( p==NULL ){
			return 0;
		}
		skip = p-default_path;
		
		//check if <LENOVO_FUSE_INTERNAL_SD_REAL_LOCATION>/xxxx exsit
		ensure_path_mounted(LENOVO_FUSE_INTERNAL_SD_NON_MULTI_LOCATION);
		strncpy(path, LENOVO_FUSE_INTERNAL_SD_NON_MULTI_LOCATION, len);
		strncat(path, default_path+skip, len-strlen(path)-1);
		if(fileExists(path)){
			return 1;
		}	
		
		//check if <LENOVO_FUSE_INTERNAL_SD_REAL_LOCATION>/xxxx exsit
		ensure_path_mounted(LENOVO_FUSE_INTERNAL_SD_REAL_LOCATION);
		strncpy(path, LENOVO_FUSE_INTERNAL_SD_REAL_LOCATION, len);
		strncat(path, default_path+skip, len-strlen(path)-1);
		if(fileExists(path)){
			return 1;
		}
		
		//check if /sdcard/xxxx exsit
		ensure_path_mounted("/sdcard");
		strncpy(path, "/sdcard", len);
		strncat(path, default_path+skip, len-strlen(path)-1);
		if(fileExists(path)){
			return 1;
		}
	
		//check if /sdcard2/xxxx exsit
		ensure_path_mounted("/sdcard2");
		strncpy(path, "/sdcard2", len);
		strncat(path, default_path+skip, len-strlen(path)-1);
		if(fileExists(path)){
			return 1;
		}
	}

	return 0;
}
/* End, lenovo-sw wangxf14 20131115 add, add for fix memory leak of the function of lenovo_ota_fix_patch_location */

int is_dir(const char *path)
{
    struct stat buf;
    if (stat(path, &buf)==-1)
    {
        printf("not dir = %s \n", path);
        return -1;
    }
    return S_ISDIR(buf.st_mode);
}

int is_file(const char *path)
{
    struct stat buf;
    if (stat(path, &buf)==-1)
    {
        printf("not file = %s \n", path);
        return -1;
    }
    return S_ISREG(buf.st_mode);
}


int is_symlink(const char *path)
{
    struct stat buf;
    if (lstat(path, &buf)==-1)
    {
        printf("not link = %s \n", path);
        return -1;
    }
    return S_ISLNK(buf.st_mode);
}


int del_dir(const char *delete_path, const char * exclude_path, const char * exclude_file)
{

    DIR * dir;
    struct dirent *ptr;

    char topath[256];

    if( strcmp(exclude_path, delete_path) == 0)
    {
        printf("exclude_path = %s, \n", delete_path );
        return 0;		
    }

    if( strcmp(exclude_file, delete_path) == 0)
    {
        printf("exclude_file 0 = %s, \n", exclude_file );
        return 0;		
    }

    if (is_file(delete_path))
    {	
        remove(delete_path);        //delete file
        printf("delete_file step 0 = %s, \n", delete_path );	
        return 0;
    } 
    else if(!is_dir(delete_path))
    {
       unlink(delete_path);   //unlink file  
       remove(delete_path);        //delete file
       printf("unlink_file step 0 = %s, \n", delete_path );	
       return 0;	   
    }	
	
    dir = opendir(delete_path);
    while((ptr=readdir(dir))!=NULL) {
        memset(topath, 0, 256);
        strcpy(topath, delete_path);
        if (topath[strlen(topath)-1]!='/')
        {
            strcat(topath, "/");
        }
        
        strcat(topath, ptr->d_name);
        if ((is_dir(topath))==1) 
       {
            if (strcmp(strrchr(topath, '/'), "/.") !=0
                && strcmp(strrchr(topath, '/'), "/..") != 0 && strcmp(topath, delete_path) != 0) 
           {
                del_dir(topath, exclude_path, exclude_file);
                if( strcmp(exclude_path, topath) != 0)
                {
                     rmdir(topath);
                     printf("delete dir = %s, \n", topath );				
                }			
            }
        }
        else if (is_file(topath)) 
        {
             if( strcmp(exclude_file, topath) == 0)
             {
                  printf("exclude_file 0 = %s, \n", exclude_file );	
             }		  
             else		 	
             {
                 remove(topath);
                 printf("delete_file step 2 = %s, \n", topath );
             }
        }
        else if(!is_dir(topath))
        {
          unlink(topath);   remove(topath);
	  printf("unlink_file step 2 = %s, \n", topath );		  
        }	
    }

    closedir(dir);
    return 0;
}




int del_link(const char *delete_path, const char * exclude_path)
{

    DIR * dir;
    struct dirent *ptr;

    char topath[256];

    if( strcmp(exclude_path, delete_path) == 0)
    {
        printf("exclude_path = %s, \n", delete_path );
        return 0;		
    }

    if(is_symlink(delete_path))
    {
       unlink(delete_path);   //unlink file  
       printf("del_link = %s, \n", delete_path );	
       return 0;	   
    }	
	
    dir = opendir(delete_path);
    while((ptr=readdir(dir))!=NULL) {
        memset(topath, 0, 256);
        strcpy(topath, delete_path);
        if (topath[strlen(topath)-1]!='/')
        {
            strcat(topath, "/");
        }
        
        strcat(topath, ptr->d_name);
        if ((is_dir(topath))==1)
        {
            if (strcmp(strrchr(topath, '/'), "/.") !=0
                && strcmp(strrchr(topath, '/'), "/..") != 0 && strcmp(topath, delete_path) != 0)
            {
                del_link(topath, exclude_path);
            }		
        }
       else if(is_symlink(topath))
       {
            unlink(topath);   
            printf("del_link 2 = %s, \n", topath );		  
        }	
    }

    closedir(dir);
    return 0;
}


int residue_dir(const char *delete_path)
{

    DIR * dir;
    struct dirent *ptr;

    char topath[256];
	
    dir = opendir(delete_path);
    while((ptr=readdir(dir))!=NULL) {
        memset(topath, 0, 256);
        strcpy(topath, delete_path);
        if (topath[strlen(topath)-1]!='/')
        {
            strcat(topath, "/");
        }
        strcat(topath, ptr->d_name);

        if ((is_dir(topath))==1)
        {
            if (strcmp(strrchr(topath, '/'), "/.") !=0
                && strcmp(strrchr(topath, '/'), "/..") != 0 && strcmp(topath, delete_path) != 0)
            {
                residue_dir(topath);
		printf("residue_dir = %s, \n", topath );			
            }		
        }
	else	
	    printf("residue_file = %s, \n", topath );	
    }

    closedir(dir);
    return 0;
}


void delete_data(void)
{
    //delete link file , 
    //delete link target file  before delete link file , will cause delete link file failed. 
    del_link("/data", "/data/media");
    //"/data/media" is  internal sdcard : /storage/sdcard0  , 
    //"data/.layout_version" is a flag to migrate /data/media contents into /data/media/0, 
    //delete "data/.layout_version" will cause sdcard0 rebuild in :installd.c : initialize_directories();
    del_dir("/data", "/data/media", "/data/.layout_version");   
    //after delete "/data", save the remaining dir&files to debug log  	
    //only "/data/media/0" and "/data/.layout_version" should be remaining 
    residue_dir("/data");
}

#ifdef EASYIMAGE_SUPPORT    
void clean_easyimage_file_if_need(char * update_package)
{
    if(update_package && 0==strncmp(update_package,EASYIMAGE_IN_DATA,strlen(EASYIMAGE_IN_DATA)))
    {
        unlink(EASYIMAGE_IN_DATA);
        sync();
    }
}
#endif


#ifdef __cplusplus
}
#endif	/* __cplusplus */

