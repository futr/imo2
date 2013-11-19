#include "remos_alos_config.h"

using namespace remos;

bool AlosConfig::loadLeader( char *filename )
{
	// リーダーファイルを読み込む
	FILE *ifp;
	char buf[128];
	int i;
	
	// 開く
	if ( NULL == ( ifp = fopen( filename, "rb" ) ) ) {
		return false;
	}
	
	// 経度緯度座標変換係数を読み込む
	
	// 先頭に移動
	fseek( ifp, 0x284C, SEEK_SET );
	
	// 終端文字付加
	buf[24] = '\0';
	
	// phi
	for ( i = 0; i < 10; i++ ) {
		// 24文字読み込み
		fread( buf, 24, 1, ifp );

		// 実数へ変換して格納
		phi[i] = strtod( buf, NULL );
	}
	
	// lambda
	for ( i = 0; i < 10; i++ ) {
		// 24文字読み込み
		fread( buf, 24, 1, ifp );

		// 実数へ変換して格納
		lambda[i] = strtod( buf, NULL );
	}
	
	// I
	for ( i = 0; i < 10; i++ ) {
		// 24文字読み込み
		fread( buf, 24, 1, ifp );

		// 実数へ変換して格納
		I[i] = strtod( buf, NULL );
	}
	
	// J
	for ( i = 0; i < 10; i++ ) {
		// 24文字読み込み
		fread( buf, 24, 1, ifp );

		// 実数へ変換して格納
		J[i] = strtod( buf, NULL );
	}
	
	// 閉じる
	fclose( ifp );
	
	return true;
}

double AlosConfig::getLat( double i, double j )
{
	// 緯度取得
	return phi[0] + phi[1] * i + phi[2] * j + phi[3] * i * j + phi[4] * i * i + phi[5] * j * j + phi[6] * i * i * j + phi[7] * i * j * j + phi[8] * i * i * i + phi[9] * j * j * j;
}

double AlosConfig::getLon( double i, double j )
{
	// 経度取得
	return lambda[0] + lambda[1] * i + lambda[2] * j + lambda[3] * i * j + lambda[4] * i * i + lambda[5] * j * j + lambda[6] * i * i * j + lambda[7] * i * j * j + lambda[8] * i * i * i + lambda[9] * j * j * j;
}

double AlosConfig::getI( double lat, double lon )
{
	// 縦方向座標
	return I[0] + I[1] * lat + I[2] * lon + I[3] * lat * lon + I[4] * lat * lat + I[5] * lon * lon + I[6] * lat * lat * lon + I[7] * lat * lon * lon + I[8] * lat * lat * lat + I[9] * lon * lon * lon;
}

double AlosConfig::getJ( double lat, double lon )
{
	// 横方向座標
	return J[0] + J[1] * lat + J[2] * lon + J[3] * lat * lon + J[4] * lat * lat + J[5] * lon * lon + J[6] * lat * lat * lon + J[7] * lat * lon * lon + J[8] * lat * lat * lat + J[9] * lon * lon * lon;
}

