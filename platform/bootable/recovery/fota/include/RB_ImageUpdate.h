#ifndef	__RB_IMAGEUPDATE_H__
#define	__RB_IMAGEUPDATE_H__

#include "RbErrors.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


	   /*********************************************************
	  **					C O N F I D E N T I A L             **
	 **						  Copyright 2002-2011                **
	  **                         Red Bend Ltd.                  **
	   **********************************************************/


/*
 * API definition for the Red Bend UPdate Installer (UPI)
 */

/* RB_UpdateParams - Defines advanced parameters for Image Update */
typedef struct RB_UpdateParams {
	unsigned long isBackgroundUpdate;
	unsigned long maxReaders;
	unsigned long written_sectors;
} RB_UpdateParams;

/* RB_SectionData - define section boundaries for old and new sections using offset and size */
typedef struct tag__RbSectionData {
	unsigned long old_offset;
	unsigned long old_size;
	unsigned long new_offset;
	unsigned long new_size;
	unsigned long ram_size;
	unsigned long bg_ram_size;
} RB_SectionData;

/* RB_ImageUpdate()
 *
 * This function should be invoked by the Update Agent to initialize the update process.
 * The user data argument (pbUserData) could be NULL.
 */
RB_RETCODE RB_ImageUpdate(
    void *pbUserData,					/* User data passed to all porting routines */
    unsigned long dwStartAddress,		/* start address of app to update */
    unsigned long dwEndAddress,			/* end address of app to update */
    unsigned char *pbyRAM,				/* pointer for the ram to use */
    unsigned long dwRAMSize,			/* size of the ram */
    unsigned char *pbyInternalRAM,
    unsigned long dwInternalRAMSize,
    unsigned long *dwBufferBlocks,		/* array of backup buffer block (sector) addresses */
    unsigned long dwNumOfBuffers,		/* number of backup buffer blocks */     
    unsigned long dwOperation,
    unsigned long dwUpdateParamsSize,	/* must be initialized to sizeof(RB_UpdateParams) */
    RB_UpdateParams *updateParams);		/* additional update parameters */

/* RB_ReadSourceBytes()
 *
 * This function should be used in background update to retrieve old content of flash.
 */
RB_RETCODE RB_ReadSourceBytes(
	void *pbUserData,				/* User data passed to all porting routines. Could be NULL */
    unsigned long address,			/* address of page in flash to retrieve */
    unsigned long size,				/* size of page in flash to retrieve */
    unsigned char *buff,			/* buffer large enough to contain size bytes */
	long section_id);

RB_RETCODE RB_ReadTargetBytes(
	void *pbUserData,				/* User data passed to all porting routines. Could be NULL */
    unsigned long address,			/* address of page in flash to retrieve */
    unsigned long size,				/* size of page in flash to retrieve */
    unsigned char *buff,			/* buffer large enough to contain size bytes */
	long section_id);			


//todo - what is this function???
long RB_OpenDelta(void* pbUserData, void* pRAM, long ram_sz);

/* API required by the UPI library to access device resources
 *
 * Note! Flash handling functions MUST be in RAM to enable flash write/erase
 * without any banking limitations. Flash writing is assumed to be
 * synchronous ('blocking') - when it has returned, all data was properly
 * copied. Erasing, the longer operation, can be asynchronous, in some
 * Flash types and a proper support for that feature will let the UPI
 * exploit the time that passes until the erased sector is required.
 * When it is required, it may still not been completely erased, so
 * erase waiting function must be called, which ensures that writing
 * will not occur before erase completed.
 */

/* RB_GetSectionsData() - Return sections definition
 *
 * This function should return information on flash sections for multi-section delta.
 */
long RB_GetSectionsData(
	void *pbUserData,					/* User data passed to all porting routines */
	long maxEntries,					/* Maximum number of entries in sectionsArray */
	RB_SectionData *sectionsArray,		/* Array to fill with section information */
	void *pRam,
	long ram_sz);

/* RB_GetDelta() - Get the Delta either as a whole or in block pieces */
long RB_GetDelta(
	void *pbUserData,				    /* User data passed to all porting routines */
	unsigned char *pbBuffer,			/* pointer to user buffer */
    unsigned long dwStartAddressOffset, /* offset from start of delta file */
    unsigned long dwSize);              /* buffer size in bytes */

long RB_GetRBDeltaOffset(
	void *pbUserData,				    /* User data passed to all porting routines */
	unsigned long signed_delta_offset,  /* offset of signed component delta in deployment package */
	unsigned long* delta_offset);		/* offset of component delta in deployment package */

/* RB_ReadBackupBlock() - Copy data from backup block to RAM. 
 *
 * Can copy data of specified size from any location in one of the buffer blocks and into specified RAM location
 */
long RB_ReadBackupBlock(
	void *pbUserData,					/* User data passed to all porting routines */
	unsigned char *pbBuffer,			/* pointer to user buffer in RAM where the data will be copied */
	unsigned long dwBlockAddress,		/* address of data to read into RAM. Must be inside one of the backup buffer blocks */
	unsigned long dwSize);				/* buffer size in bytes */

/* RB_EraseBackupBlock() - Erase a specific backup block.
 *
 * This function will erase the specified block starting from dwStartAddress.  The
 * block address must be aligned to sector size and a single sector should be erased.
 */ 
long RB_EraseBackupBlock(
	void *pbUserData,					/* User data passed to all porting routines */
	unsigned long dwStartAddress);		/* block start address in flash to erase */

/* RB_WriteBackupBlock() - Copy data from specified address in RAM to a backup block.
 *
 * This function copies data from specified address in RAM to a backup buffer block.
 * This will always write a complete block (sector) and the address will always be one of the addresses provided in RB_ImageUpdate() call.
 */ 
long RB_WriteBackupBlock(
	void *pbUserData,					/* User data passed to all porting routines */
	unsigned long dwBlockStartAddress,	/* address of the block to be updated */
	unsigned char *pbBuffer);  	        /* RAM to copy data from */

/* RB_WriteBackupPartOfBlock() - Copy data from specified address in RAM to part of a backup block.
 *
 * This function copies data from specified address in RAM to a backup buffer block.
 * This will write part of a block (sector) that was already written with RB_WriteBackupBlock().
 */ 
long RB_WriteBackupPartOfBlock(
	void *pbUserData,					/* User data passed to all porting routines */
	unsigned long dwStartAddress,		/* Start address in flash to write to */
	unsigned long dwSize,				/* Size of data to write */
	unsigned char* pbBuffer);			/* Buffer in RAM to write from */

/* RB_WriteBlock() - Writes one block to a given location in flash.
 *
 * Erases the image location and writes the data to that location 
 */ 
long RB_WriteBlock(
	void *pbUserData,					/* User data passed to all porting routines */
	unsigned long dwBlockAddress,		/* address of the block to be updated */
	unsigned char *pbBuffer);			/* pointer to data to be written */


/* RB_WriteMetadataOfBlock() - Write one metadata of a given block in flash
 *
 * writes the metadata in the matching place according to the represented block location
 */
long RB_WriteMetadataOfBlock(
	void *pbUserData,					/* User data passed to all porting routines */
	unsigned long dwBlockAddress,		/* address of the block of that the metadata describe */
	unsigned long dwMdSize,				/* Size of the data that the metadata describe*/
	unsigned char *pbMDBuffer);			/* pointer to metadata to be written */
							 

/* RB_ReadImage() - Reads data from flash.
 *
 * Number of bytes to read should be less or equal to block size. 
 * The data is used to create new block based on read data and diff file information 
 */ 
long RB_ReadImage(
	void *pbUserData,					/* User data passed to all porting routines */
	unsigned char *pbBuffer,			/* pointer to user buffer */
    unsigned long dwStartAddress,		/* memory address to read from */
    unsigned long dwSize);				/* number of bytes to copy */

/* RB_ReadImageNewKey() - Read content from flash assuming it is the updated image 
 *
 * This function is used when the update installer reads content from flash that is assumed to
 * be the updated version. This is useful when the image is encrypted and the key of the old
 * version and the new version might be different.
 */ 
long RB_ReadImageNewKey(
	void *pbUserData,					/* User data passed to all porting routines */
	unsigned char *pbBuffer,
	unsigned long dwStartAddress,
	unsigned long dwSize);

/* RB_GetBlockSize() - Returns the size of one memory block. 
 *
 * This function must be called before any block reading or writing can be done.
 * Note: For a given update, the sequence of blocks that is being updated is always the same. 
 * This assumption is made in order to be able to successfully continue update after failure situation.
 */ 
long RB_GetBlockSize(void);/* returns the size of a memory-block */

/* RB_GetUPIVersion() - get the UPI version string */
long RB_GetUPIVersion(unsigned char *pbVersion);			/* Buffer to return the version string in */

//Protocol versions
//get the UPI version string
unsigned long RB_GetUPIProtocolVersion(void);
//get the UPI scout version string
unsigned long RB_GetUPIScoutProtocolVersion(void);
//get the delta UPI version
unsigned long RB_GetDeltaProtocolVersion(void* pbUserData, void* pbyRAM, unsigned long dwRAMSize);		/* User data passed to all porting routines, pointer for the ram to use, size of the ram */
//get the delta scout version
unsigned long RB_GetDeltaScoutProtocolVersion(void* pbUserData, void* pbyRAM, unsigned long dwRAMSize); /* User data passed to all porting routines, pointer for the ram to use, size of the ram */
//get the minimal RAM use for delta expand
unsigned long RB_GetDeltaMinRamUse(void* pbUserData, void* pbyRAM, unsigned long dwRAMSize);			/* User data passed to all porting routines, pointer for the ram to use, size of the ram */

/* RB_ResetTimerA() - Reset watchdog timer A
 *
 * This function is being called periodically within the 30-second period
 */
long RB_ResetTimerA(void);

/* RB_ResetTimerB() - Reset watchdog timer B
 *
 * This function is reserved for future use
 */
long RB_ResetTimerB(void);

/* RB_Progress() - Report the current stage of the update
 *
 * Provides information in percents on the update process progress
 */
void RB_Progress(
	void *pbUserData,					/* User data passed to all porting routines */
	unsigned long uPercent);			/* progress info in percents */

/* RB_Trace() - Display trace messages on the console for debug purposes
 *
 * Format and print data using the C printf() format
 */
unsigned long RB_Trace(
	void *pbUserData,					/* User data passed to all porting routines */
	const char *aFormat,...);			/* format string to printf */

/* Background update requires the implementation of the following:
 * o Critical sections enter and leave
 * o Memory allocation and free
 */

typedef enum {
RB_SEMAPHORE_READER = 0,   /* Lock a reader semaphore before reading flash content */
RB_SEMAPHORE_LAST = 50                  /* Last and unused enum value, for sizing arrays, etc. */
} RB_SEMAPHORE;

/*Add the following porting routines (replace the critical section routines):*/

/* RB_SemaphoreInit() - Initialize the specified semaphore to number of available shared resources */
long RB_SemaphoreInit(void *pbUserData, RB_SEMAPHORE semaphore, long num_resources);

/* RB_SemaphoreDestroy() - Destroy the specified semaphore */
long RB_SemaphoreDestroy(void *pbUserData, RB_SEMAPHORE semaphore);

/* RB_SemaphoreDown() - Request to use the resource protected by specified semaphore */
long RB_SemaphoreDown(void *pbUserData, RB_SEMAPHORE semaphore,long num);

/* RB_SemaphoreUp() - Free the resource protected by specified semaphore */
long RB_SemaphoreUp(void *pbUserData, RB_SEMAPHORE semaphore,long num);

/* RB_Malloc() - Allocate a memory block
 *
 * Allocate the required memory block. Return pointer on success and NULL on failure.
 */
void *RB_Malloc(unsigned long size);				/* size in bytes of memory block to allocate */

/* RB_Free() - Free a memory block allocated by RB_Alloc()
 *
 * Free the memory block.
 */
void RB_Free(void *pMemBlock);					/* pointer to memory block allocated by RB_Malloc() */

#ifdef __cplusplus
	}
#endif  /* __cplusplus */

#endif	/*	__RB_UPDATE_IMAGE_H__	*/
