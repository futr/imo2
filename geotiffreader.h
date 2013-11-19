#ifndef GEOTIFFREADER_H
#define GEOTIFFREADER_H

#include "tifrec.h"

class GeotiffReader
{
public:
	GeotiffReader();

	void openFile( char *filename );
	void closeFile( char *filename );

private:
	struct TIFREC_FILE_CONTAINER tiff_file:
};

#endif // GEOTIFFREADER_H
