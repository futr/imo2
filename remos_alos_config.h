#ifndef REMOS_ALOS_CONFIG_H_INCLUDED
#define REMOS_ALOS_CONFIG_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <math.h>

namespace remos {

class AlosConfig {
public:
	enum Type {
    	UNKNOWN,
    	ALOS,
        ALOS2,
    };

	bool loadLeader( char *filename );

	double getLat( double i, double j );
	double getLon( double i, double j );
	double getI( double lat, double lon );
	double getJ( double lat, double lon );
    bool hasTime();
    std::string getCenterTime();
    std::string getReadableCenterTime();
    Type getType();

private:
	FILE *ifp;
    Type type;

    void readALOS();
    void readALOS2();
    void readALOS2Detail();

    double calcPos( double *mat, double p, double l, double p0, double l0 );

	double phi[10];
	double lambda[10];
	double I[10];
	double J[10];
    double A[8];
    double B[8];

    double mat[4][25];
    double L0, P0;
    double PHI0, LAMBDA0;

    std::string timeStr;
};

}; // end of namespace remos

#endif