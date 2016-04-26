#include "hsdinfo.h"

HSD_Status hsd_read_file( HSD_INFO *info, const char *filename )
{
    // Read hsd from filename
    HSD_Status status;
    
    // Open a file
    if ( ( info->fp = fopen( filename, "rb" ) ) == NULL ) {
        return HSD_Failed;
    }
    
    // Read a base block
    if ( ( status = hsd_base_block( info ) ) != HSD_Succeeded ) {
        fclose( info->fp );
        return status;
    }

    // Read a data info block
    if ( ( status = hsd_data_info_block( info ) ) != HSD_Succeeded ) {
        fclose( info->fp );
        return status;
    }

    // complete
    fclose( info->fp );

    return HSD_Succeeded;
}

HSD_Status hsd_base_block( HSD_INFO *info )
{
    // Read a base block
    char bytes[128];
    int headerBlockNumber;
    int blockLength;
    int endianInfo;
    int timeLine;
    
    // Read endian info
    // Why endian info is located after a few bytes from file start?
    fseek( info->fp, 5, SEEK_SET );
    fread( bytes, 1, 1, info->fp );
    endianInfo = hsd_convert( bytes, 1, HSD_LittleEndian, HSD_FORMAT_UINT );
    
    // Set endian info
    if ( endianInfo == 0 ) {
        info->endian = HSD_LittleEndian;
    } else {
        info->endian = HSD_BigEndian;
    }
    
    // Reset file position
    fseek( info->fp, 0, SEEK_SET );
    
    // Read header info
    fread( bytes, 1, 1, info->fp );
    headerBlockNumber = hsd_convert( bytes, 1, info->endian, HSD_FORMAT_UINT );
    
    fread( bytes, 1, 2, info->fp );
    blockLength = hsd_convert( bytes, 2, info->endian, HSD_FORMAT_UINT );
    
    fread( bytes, 1, 2, info->fp );
    info->headerBlockCount = hsd_convert( bytes, 2, info->endian, HSD_FORMAT_UINT );
    
    fread( bytes, 1, 1, info->fp );
    
    // Check header info
    if ( blockLength != 282 ) {
        return HSD_Not_HSD;
    }
    
    // Read
    hsd_read_char( info, info->satName,    16 );
    hsd_read_char( info, info->centerName, 16 );
    hsd_read_char( info, info->range,      4 );
    hsd_read_char( info, bytes, 2 );

    // Read time line info
    fread( bytes, 1, 2, info->fp );
    timeLine = hsd_convert( bytes, 2, info->endian, HSD_FORMAT_UINT );
    
    info->timeLineStartedHour = timeLine / 100;
    info->timeLineStartedMin  = timeLine - info->timeLineStartedHour * 100;
    
    // Read obs time
    fread( bytes, 1, 8, info->fp );
    info->startTime = hsd_convert( bytes, 8, info->endian, HSD_FORMAT_IEEEDP );
    fread( bytes, 1, 8, info->fp );
    info->endTime = hsd_convert( bytes, 8, info->endian, HSD_FORMAT_IEEEDP );
    
    // Read file info
    fread( bytes, 1, 8, info->fp );
    fread( bytes, 1, 4, info->fp );
    info->headerSize = hsd_convert( bytes, 4, info->endian, HSD_FORMAT_UINT );
    fread( bytes, 1, 4, info->fp );
    info->dataSize = hsd_convert( bytes, 4, info->endian, HSD_FORMAT_UINT );
    
    // skip
    fread( bytes, 1, 1, info->fp );
    fread( bytes, 1, 1, info->fp );
    fread( bytes, 1, 1, info->fp );
    fread( bytes, 1, 1, info->fp );
    hsd_read_char( info, bytes, 32 );
    
    // Read file name
    hsd_read_char( info, info->fileName, 128 );
    
    // Reserved
    fread( bytes, 1, 40, info->fp );
    
    return HSD_Succeeded;
}

HSD_Status hsd_data_info_block( HSD_INFO *info )
{
    // Read a data info block
    char bytes[128];
    int headerBlockNumber;
    int blockLength;
    int compress;
    
    // Read header info
    fread( bytes, 1, 1, info->fp );
    headerBlockNumber = hsd_convert( bytes, 1, info->endian, HSD_FORMAT_UINT );
    
    fread( bytes, 1, 2, info->fp );
    blockLength = hsd_convert( bytes, 2, info->endian, HSD_FORMAT_UINT );

    // Read image info
    fread( bytes, 1, 2, info->fp );
    info->bitCount = hsd_convert( bytes, 2, info->endian, HSD_FORMAT_UINT );
    fread( bytes, 1, 2, info->fp );
    info->columnCount = hsd_convert( bytes, 2, info->endian, HSD_FORMAT_UINT );
    fread( bytes, 1, 2, info->fp );
    info->lineCount = hsd_convert( bytes, 2, info->endian, HSD_FORMAT_UINT );
    
    // Read compress info
    fread( bytes, 1, 1, info->fp );
    compress = hsd_convert( bytes, 1, info->endian, HSD_FORMAT_UINT );
    
    switch ( compress ) {
    case 0:
        info->compress = HSD_Plain;
        break;
    case 1:
        info->compress = HSD_gzip;
        break;
    case 2:
        info->compress = HSD_bzip2;
        break;
    default:
        info->compress = HSD_Plain;
        break;
    }
    
    return HSD_Succeeded;
}

void hsd_read_char( HSD_INFO *info, char *dest, int len )
{
    // Read a string
    fread( dest, 1, len, info->fp );
    dest[len] = '\0';
}

double hsd_convert( char *data, int len, HSD_Endian endian, HSD_Format format )
{
	/* エンディアンと書式に従ってデータを数値にする */
	double ret;
	char *ret_ptr;
	float ret_f;
	double ret_d;
	char ret_c;
	unsigned char ret_uc;
	short ret_s;
	unsigned short ret_us;
	int ret_i;
	unsigned int ret_ui;
	int i;
	
	/* 書き込み先ポインターを特定 */
	switch ( format ) {
		case HSD_FORMAT_INT:
			/* signed integer */
			switch ( len ) {
				case 4:
					ret_ptr = &ret_i;
					break;
				case 2:
					ret_ptr = &ret_s;
					break;
				case 1:
					ret_ptr = &ret_c;
					break;
			}
			
			break;

		case HSD_FORMAT_UINT:
			/* unsigned integer */
			switch ( len ) {
				case 4:
					ret_ptr = &ret_ui;
					break;
				case 2:
					ret_ptr = &ret_us;
					break;
				case 1:
					ret_ptr = &ret_uc;
					break;
			}
			
			break;

		case HSD_FORMAT_IEEEFP:
			/* float */
			switch ( len ) {
				case 4:
					ret_ptr = &ret_f;
					break;
			}
			
			break;
			
		case HSD_FORMAT_IEEEDP:
			/* double */
			switch ( len ) {
				case 8:
					ret_ptr = &ret_d;
					break;
			}
			
			break;
		
		default:
			ret_ptr = &ret_i;
			break;
	}
	
	/* エンディアンにしたがってデーターをコピー */
	if ( endian == HSD_BigEndian ) {
		for ( i = 0; i < len; i++ ) {
			ret_ptr[i] = data[len - 1 - i];
		}
	} else {
		for ( i = 0; i < len; i++ ) {
			ret_ptr[i] = data[i];
		}
	}

	/* 値をfloatに変換する */
	switch ( format ) {
		case HSD_FORMAT_INT:
			/* signed integer */
			switch ( len ) {
				case 4:
					ret = ret_i;
					break;
				case 2:
					ret = ret_s;
					break;
				case 1:
					ret = ret_c;
					break;
			}

			break;

		case HSD_FORMAT_UINT:
			/* unsigned integer */
			switch ( len ) {
				case 4:
					ret = ret_ui;
					break;
				case 2:
					ret = ret_us;
					break;
				case 1:
					ret = ret_uc;
					break;
			}

			break;

		case HSD_FORMAT_IEEEFP:
			/* float */
			switch ( len ) {
				case 4:
					ret = ret_f;
					break;
			}

			break;
			
		case HSD_FORMAT_IEEEDP:
			/* double */
			switch ( len ) {
				case 8:
					ret = ret_d;
					break;
			}

			break;

		default:
			ret = ret_i;
			break;
	}

	return ret;
}
