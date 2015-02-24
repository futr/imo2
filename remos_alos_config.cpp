#include "remos_alos_config.h"

using namespace remos;

bool AlosConfig::loadLeader( char *filename )
{
	// ���[�_�[�t�@�C����ǂݍ���
	FILE *ifp;
	char buf[128];
	int i;
	
	// �J��
	if ( NULL == ( ifp = fopen( filename, "rb" ) ) ) {
		return false;
	}
	
	// �o�x�ܓx���W�ϊ��W����ǂݍ���
	
	// �擪�Ɉړ�
	fseek( ifp, 0x284C, SEEK_SET );

	// �I�[�����t��
	buf[24] = '\0';
	
	// phi
	for ( i = 0; i < 10; i++ ) {
		// 24�����ǂݍ���
		fread( buf, 24, 1, ifp );

		// �����֕ϊ����Ċi�[
		phi[i] = strtod( buf, NULL );
	}
	
	// lambda
	for ( i = 0; i < 10; i++ ) {
		// 24�����ǂݍ���
		fread( buf, 24, 1, ifp );

		// �����֕ϊ����Ċi�[
		lambda[i] = strtod( buf, NULL );
	}
	
	// I
	for ( i = 0; i < 10; i++ ) {
		// 24�����ǂݍ���
		fread( buf, 24, 1, ifp );

		// �����֕ϊ����Ċi�[
		I[i] = strtod( buf, NULL );
	}
	
	// J
	for ( i = 0; i < 10; i++ ) {
		// 24�����ǂݍ���
		fread( buf, 24, 1, ifp );

		// �����֕ϊ����Ċi�[
		J[i] = strtod( buf, NULL );
	}

    // �V�[�����S������ǂݍ���

    // �擪�Ɉړ�
	fseek( ifp, 4796, SEEK_SET );

    // 20�����ǂݍ���
    fread( buf, 20, 1, ifp );

    buf[20] = '\0';

    // string��
    timeStr = buf;

	// ����
	fclose( ifp );
	
	return true;
}

bool AlosConfig::hasTime()
{
	if ( timeStr.length() == 0 || timeStr[0] == ' ' ) {
    	return false;
    }

    return true;
}

std::string AlosConfig::getReadableCenterTime()
{
	std::string year, month, day, h, m, s;

    year  = timeStr.substr( 0, 4 );
    month = timeStr.substr( 4, 2 );
    day   = timeStr.substr( 6, 2 );
    h     = timeStr.substr( 8, 2 );
    m     = timeStr.substr( 10, 2 );
    s     = timeStr.substr( 12, 2 );

	return year + "/" + month + "/" + day + " " + h + ":" + m + ":" + s;
}

std::string AlosConfig::getCenterTime()
{
	return timeStr;
}

double AlosConfig::getLat( double i, double j )
{
	// �ܓx�擾
	return phi[0] + phi[1] * i + phi[2] * j + phi[3] * i * j + phi[4] * i * i + phi[5] * j * j + phi[6] * i * i * j + phi[7] * i * j * j + phi[8] * i * i * i + phi[9] * j * j * j;
}

double AlosConfig::getLon( double i, double j )
{
	// �o�x�擾
	return lambda[0] + lambda[1] * i + lambda[2] * j + lambda[3] * i * j + lambda[4] * i * i + lambda[5] * j * j + lambda[6] * i * i * j + lambda[7] * i * j * j + lambda[8] * i * i * i + lambda[9] * j * j * j;
}

double AlosConfig::getI( double lat, double lon )
{
	// �c�������W
	return I[0] + I[1] * lat + I[2] * lon + I[3] * lat * lon + I[4] * lat * lat + I[5] * lon * lon + I[6] * lat * lat * lon + I[7] * lat * lon * lon + I[8] * lat * lat * lat + I[9] * lon * lon * lon;
}

double AlosConfig::getJ( double lat, double lon )
{
	// ���������W
	return J[0] + J[1] * lat + J[2] * lon + J[3] * lat * lon + J[4] * lat * lat + J[5] * lon * lon + J[6] * lat * lat * lon + J[7] * lat * lon * lon + J[8] * lat * lat * lat + J[9] * lon * lon * lon;
}

