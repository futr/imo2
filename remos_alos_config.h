#ifndef REMOS_ALOS_CONFIG_H_INCLUDED
#define REMOS_ALOS_CONFIG_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace remos {

class AlosConfig {
public:
	bool loadLeader( char *filename );
	
	double getLat( double i, double j );
	double getLon( double i, double j );
	double getI( double lat, double lon );
	double getJ( double lat, double lon );

private:
	double phi[10];
	double lambda[10];
	double I[10];
	double J[10];

};

}; // end of namespace remos

#endif