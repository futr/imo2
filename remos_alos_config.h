#ifndef REMOS_ALOS_CONFIG_H_INCLUDED
#define REMOS_ALOS_CONFIG_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

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

	double phi[10];
	double lambda[10];
	double I[10];
	double J[10];
    double A[8];
    double B[8];

    std::string timeStr;
};

}; // end of namespace remos

#endif