
#ifndef _CFG_CUSTOMSN_FILE_H
#define _CFG_CUSTOMSN_FILE_H

typedef struct
{
	unsigned char barcode[64];
}File_CustomSN_Struct;

#define CFG_FILE_CUSTOMSN_REC_SIZE    sizeof(File_CustomSN_Struct)
#define CFG_FILE_CUSTOMSN_REC_TOTAL   1

#endif
