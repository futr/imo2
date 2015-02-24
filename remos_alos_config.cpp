#include "remos_alos_config.h"

using namespace remos;

bool AlosConfig::loadLeader( char *filename )
{
	// リーダーファイルを読み込む
	char buf[128];
	int i;

	// 開く
	if ( NULL == ( ifp = fopen( filename, "r" ) ) ) {
		return false;
	}

    // タイプ確認
    fseek( ifp, 0x30, SEEK_SET );
    fread( buf, 7, 1, ifp );
    buf[7] = '\0';

    if ( !strncmp( "AL ", buf, 3 ) ) {
    	type = ALOS;
    } else if ( !strncmp( "AL2", buf, 3 ) ) {
    	type = ALOS2;
    } else {
    	type = UNKNOWN;

        fclose( ifp );

        return false;
    }

	// 読む
    switch ( type ) {
    case ALOS:
    	readALOS();
    	break;
    case ALOS2:
    	readALOS2();
    	break;
    default:
    	break;
    }

	// 閉じる
	fclose( ifp );
	
	return true;
}

AlosConfig::Type AlosConfig::getType()
{
	return type;
}

void AlosConfig::readALOS()
{
	char buf[128];
	int i;

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

    // シーン中心時刻を読み込む

    // 先頭に移動
	fseek( ifp, 4796, SEEK_SET );

    // 20文字読み込み
    fread( buf, 20, 1, ifp );

    buf[20] = '\0';

    // stringへ
    timeStr = buf;
}

void AlosConfig::readALOS2()
{
	char buf[128];
	int i;

	// 経度緯度座標変換係数を読み込む
    // 設備関連データレコード5 の2065-3064 バイトの係数の使用を推奨 らしいけど地図投影の方を使ってる

	// 先頭に移動
	fseek( ifp, 6080, SEEK_SET );

	// 終端文字付加
	buf[20] = '\0';

	// A
	for ( i = 0; i < 8; i++ ) {
		// 20文字読み込み
		fread( buf, 20, 1, ifp );

		// 実数へ変換して格納
		A[i] = strtod( buf, NULL );
	}

    // B
	for ( i = 0; i < 8; i++ ) {
		// 20文字読み込み
		fread( buf, 20, 1, ifp );

		// 実数へ変換して格納
		B[i] = strtod( buf, NULL );
	}

    // シーン中心時刻を読み込む

    // 先頭に移動
	fseek( ifp, 788, SEEK_SET );

    // 20文字読み込み
    fread( buf, 17, 1, ifp );

    buf[17] = '\0';

    // stringへ
    timeStr = buf;
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
	// 緯度取得
    switch ( type ) {
    case ALOS:
		return phi[0] + phi[1] * i + phi[2] * j + phi[3] * i * j + phi[4] * i * i + phi[5] * j * j + phi[6] * i * i * j + phi[7] * i * j * j + phi[8] * i * i * i + phi[9] * j * j * j;
    case ALOS2:
    	return A[4] + A[5] * j + A[6] * j + A[7] * i * j;
    default:
    	return 0;
    }
}

double AlosConfig::getLon( double i, double j )
{
	// 経度取得
	switch ( type ) {
    case ALOS:
		return lambda[0] + lambda[1] * i + lambda[2] * j + lambda[3] * i * j + lambda[4] * i * i + lambda[5] * j * j + lambda[6] * i * i * j + lambda[7] * i * j * j + lambda[8] * i * i * i + lambda[9] * j * j * j;
    case ALOS2:
    	return A[0] + A[1] * j + A[2] * i + A[3] * i * j;
    default:
    	return 0;
    }
}

double AlosConfig::getI( double lat, double lon )
{
	// 縦方向座標
	switch ( type ) {
    case ALOS:
		return I[0] + I[1] * lat + I[2] * lon + I[3] * lat * lon + I[4] * lat * lat + I[5] * lon * lon + I[6] * lat * lat * lon + I[7] * lat * lon * lon + I[8] * lat * lat * lat + I[9] * lon * lon * lon;
    case ALOS2:
    	return B[4] + B[5] * lon + B[6] * lat + B[7] * lat * lon;
    default:
    	return 0;
    }
}

double AlosConfig::getJ( double lat, double lon )
{
	// 横方向座標
	switch ( type ) {
    case ALOS:
		return J[0] + J[1] * lat + J[2] * lon + J[3] * lat * lon + J[4] * lat * lat + J[5] * lon * lon + J[6] * lat * lat * lon + J[7] * lat * lon * lon + J[8] * lat * lat * lat + J[9] * lon * lon * lon;
    case ALOS2:
        return B[0] + B[1] * lon + B[2] * lat + B[3] * lat * lon;
    default:
    	return 0;
    }
}

