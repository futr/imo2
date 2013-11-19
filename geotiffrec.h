#ifndef GEOTIFFREC_H_INCLUDED
#define GEOTIFFREC_H_INCLUDED

/*
 * 直接Latlonに変換してから返してくる
 */

#include <vector>
#include "tifrec.h"
#include "UTM.h"

enum tifrec_geotiff_projection_mode {
	TIFREC_GEOTIFF_PROJECTION_MODE_UNKNOWN,
	TIFREC_GEOTIFF_PROJECTION_MODE_LATLON,
	TIFREC_GEOTIFF_PROJECTION_MODE_UTM,
};

enum tifrec_geotiff_key {
	TIFREC_GEOTIFF_KEY_MODEL_TYPE        = 1024,
	TIFREC_GEOTIFF_KEY_PROJECTED_CS_TYPE = 3072,
};

enum tifrec_geotiff_tag {
	TIFREC_GEOTIFF_TAG_MODEL_TIEPOINT    = 33922,		/* ラスター座標とモデル座標のペア */
	TIFREC_GEOTIFF_TAG_GEO_KEY_DIRECTORY = 34735,		/* 座標に関するいろんな情報がある */
	TIFREC_GEOTIFF_TAG_MODEL_PIXEL_SCALE = 33550,		/* スケール */
};

class Point2D {
public:
	Point2D( void ) {
		x = 0;
		y = 0;
	}
	
	Point2D( double x, double y ) {
		this->x = x;
		this->y = y;
	}

	double x;
	double y;
};

class GeotiffRec {
public:
	GeotiffRec();
	~GeotiffRec();

	bool open( char *filename );
	void close( void );
	
	bool initialize( void );
	
	bool isUTM( void );
	
	Point2D getScale( void );
	
	Point2D getLatLon( Point2D xy );
	Point2D getXY( Point2D latlon );
	Point2D utmToLatLon( Point2D utm );
	
	Point2D forward( Point2D xy );
	
private:
	struct TIFREC_FILE_CONTAINER m_tifrec;
	
	enum tifrec_geotiff_projection_mode m_proj_mode;
	std::vector< Point2D > tiepoints;
	std::vector< std::vector< double > > m_matrix;
	std::vector< double > scale;
	
	int cs_type;
	int zone_number;
	char zone_str[8];
	char sn;
	
	bool m_open;
};

#endif 