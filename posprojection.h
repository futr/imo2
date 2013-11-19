#ifndef POSPROJECTION_H
#define POSPROJECTION_H

#include <list>

class Pos {
public;
	Pos();
	Pos( double x, double y );

	void setPos( double x, double y );
	void setX( double x );
	void setY( double y );

	double getX( void );
	double getY( void );

private:
	double x;
	double y;
};

class PosProjection
{
public:
	PosProjection();

	void addPos( Pos xy, Pos XY );
	void clearPos( void );

	void makeMatrix( void );

	Pos forward( Pos pos );
	Pos reverse( Pos pos );

	void setMatrixElement( int i, int j, double element );
	double getMatrixElement( int i, int j );

private:
	list<Pos> pos_list;

	double matrix[3][3];
	double inv_matrix[3][3];
};

#endif // POSPROJECTION_H
