#ifndef HSDINFO_H_INCLUDED
#define HSDINFO_H_INCLUDED

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HSD_BigEndian,
    HSD_LittleEndian,
} HSD_Endian;

typedef enum {
    HSD_FORMAT_INT,
    HSD_FORMAT_UINT,
    HSD_FORMAT_IEEEFP,
    HSD_FORMAT_IEEEDP,
} HSD_Format;

typedef enum {
    HSD_Plain,
    HSD_gzip,
    HSD_bzip2,
} HSD_Compress;

typedef enum {
    HSD_Failed,
    HSD_Not_HSD,
    HSD_Succeeded,
} HSD_Status;

typedef struct {
    // FD
    FILE *fp;
    
    // BASE INFO BLOCK
    HSD_Endian endian;
    char satName[20];
    char centerName[20];
    char range[5];
    unsigned int timeLineStartedHour;
    unsigned int timeLineStartedMin;
    double startTime;
    double endTime;
    unsigned int headerBlockCount;
    unsigned int headerSize;
    unsigned int dataSize;
    char fileName[256];
    
    // DATA INFO BLOCK
    unsigned int bitCount;
    unsigned int columnCount;
    unsigned int lineCount;
    HSD_Compress compress;
} HSD_INFO;

// public
HSD_Status hsd_read_file( HSD_INFO *info, const char *filename );

// private
HSD_Status hsd_base_block( HSD_INFO *info );
HSD_Status hsd_data_info_block( HSD_INFO *info );

double hsd_convert( char *data, int len, HSD_Endian endian, HSD_Format format );
void hsd_read_char( HSD_INFO *info, char *dest, int len );

#ifdef __cplusplus
}
#endif

#endif