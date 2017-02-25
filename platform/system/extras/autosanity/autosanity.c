#include <cutils/xlog.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <assert.h>

#define LOG_TAG "auto_sanity"
#define SANITY_PRINTF XLOGD

/* call it when test failed to trigger report. */
void autosanity_abort(void)
{
    int abort_fd = open("/sys/module/autosanity/parameters/do_autosanity", O_RDONLY);
    int value=0xff;

    read(abort_fd, value, sizeof(value));
}

#ifdef MTK_MEMORY_COMPRESSION_SUPPORT
int zram_test(void)
{
    char zram_disksize[16];
    int disksize = 0;
    int zram_fd = open("/sys/block/zram0/disksize", O_RDONLY);
    if(zram_fd < 0){
        SANITY_PRINTF("%s(%d) open device failed, zram_fd:%d, error: %s\n", __FUNCTION__, __LINE__, zram_fd, strerror(errno));
    	goto test_fail;
    }
        
    read(zram_fd, zram_disksize, sizeof(zram_disksize));
    disksize = (int)strtol(zram_disksize, NULL, 0);

    SANITY_PRINTF("%s(%d) open device success, zram_fd:%d, zram size:0x%x\n", __FUNCTION__, __LINE__, zram_fd, disksize);
    
    close(zram_fd);
    return 0;    

test_fail:    
    autosanity_abort();
    return -1;    
}
#endif //MTK_MEMORY_COMPRESSION_SUPPORT

#ifdef MTK_USE_RESERVED_EXT_MEM	
#define EXM_DEV "/dev/exm0"
#define EXM_ADDR "/sys/class/exm/exm0/maps/map0/addr"
#define EXM_SIZE "/sys/class/exm/exm0/maps/map0/size"
#define TESTSIZE (10*1024)

int ext_mem_alloc_test(void)
{
    int exm_fd, addr_fd, size_fd;
    int exm_size;
    void * exm_addr;
    char exm_addr_buf[16], exm_size_buf[16];
    
    exm_fd  = open(EXM_DEV,O_RDWR);
    addr_fd = open(EXM_ADDR, O_RDONLY);
    size_fd = open(EXM_SIZE, O_RDONLY);       

    if (exm_fd <0 || addr_fd <0 || size_fd <0){
    	SANITY_PRINTF("%s(%d) open device failed, exm_fd: %d, addr_fd: %d, size_fd: %d, error: %s\n", __FUNCTION__, __LINE__, exm_fd, addr_fd, size_fd, strerror(errno));
    	goto test_fail;
    }
    
    read(addr_fd, exm_addr_buf, sizeof(exm_addr_buf));
    read(size_fd, exm_size_buf, sizeof(exm_size_buf));
    exm_addr = (void*)strtoul(exm_addr_buf, NULL, 0);
    exm_size = (int)strtol(exm_size_buf, NULL, 0);
    
    SANITY_PRINTF("%s(%d) open device success, exm_addr:0x%x, exm_size:0x%x\n", __FUNCTION__, __LINE__, exm_addr, exm_size);

    void * vaddr = mmap(NULL, TESTSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, exm_fd, 0); 
    if (vaddr == MAP_FAILED){
        SANITY_PRINTF("%s[%d] mmap falied, error: %s\n", __FUNCTION__, __LINE__, strerror(errno));
        close(exm_fd);
        goto test_fail;
    }

    int ret = munmap(vaddr, TESTSIZE);
    if(ret != 0)
    {
        SANITY_PRINTF("%s(%d) munmap falied, ret:%d error: %s\n", __FUNCTION__, __LINE__, ret, strerror(errno));
        goto test_fail;
    }
    
    SANITY_PRINTF("%s(%d) mmap/munmap success\n", __FUNCTION__, __LINE__);    
    
    close(exm_fd);
    return 0;

test_fail:    
    autosanity_abort();
    return -1;
}
#endif //MTK_USE_RESERVED_EXT_MEM

int main(int argc, char *argv[]) {

#ifdef MTK_MEMORY_COMPRESSION_SUPPORT   
    zram_test();
#endif

#ifdef MTK_USE_RESERVED_EXT_MEM	
    ext_mem_alloc_test();
#endif    
        
    return 0;
}

