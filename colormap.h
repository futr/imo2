#ifndef COLORMAP_H_INCLUDED
#define COLORMAP_H_INCLUDED

#include <vcl.h>

#include "ecalc.h"

class ColorLevel {
public:
    ColorLevel( double level, int r, int g, int b );

	void setLevel( double level );
    void setColor( int r, int g, int b );

    double getLevel( void );
    int getR( void );
    int getG( void );
    int getB( void );

	int r;
    int g;
    int b;

    double level;
};

class ColorMap {
public:
    ColorMap();
    ~ColorMap();

    ColorMap &operator =( const ColorMap &r_cm );

	void setExpression( AnsiString exp );
    AnsiString getExpression( void );
    void evalExpression( double **args, double ans );

    void setValue( double value );
    double getValue( void );

    void setUnitString( AnsiString unit );
    AnsiString getUnitString( void );

    void setRange( double bottom, double top );
	void setSmooth( bool smooth = true );
    bool getSmooth( void );

    double getR( void );
    double getG( void );
    double getB( void );

    void sortLevel( void );
    void makeColor( void );

    void addColorLevel( ColorLevel *level );
    void deleteColorLevel( int index );
    void deleteAllColorLevel( void );
    int getColorLevelCount( void );
    int getColorLevelIndex( ColorLevel *level );
    ColorLevel *getColorLevel( int index );

    void setTopLevel( double level );
    void setBottomLevel( double level );

private:
    void setColor( int r, int g, int b );

    ColorLevel *getUnderColorLevel( double level );
    ColorLevel *getUpperColorLevel( double level );

private:
    TList *levels;

    struct ECALC_TOKEN *tok_exp;

	AnsiString exp;
    AnsiString unit_str;

    bool smooth;

    double bottom;
    double top;

    // èoóÕíl
    double value;
    double r;
    double g;
    double b;

};

int __fastcall sortLevelFunction( void *item1, void *item2 );

#endif
