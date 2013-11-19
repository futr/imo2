#ifndef COORDINATETRANSFORM_H
#define COORDINATETRANSFORM_H

#include <list>
#include <vector>

class CoordinateTransform
{
public:
    CoordinateTransform();

	addPoint( double x, double y, double X, double Y );
	deleteAllPoint( void );

	makeMatrix( void );

	vector<double> getTransformPoint( double x, double y );

private:
	double a;
	double b;
	double c;
	double d;
	double e;
	double f;

	std::list<vector<double>> points;
};

#endif // COORDINATETRANSFORM_H
