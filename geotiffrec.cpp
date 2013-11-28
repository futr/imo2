#include "geotiffrec.h"

GeotiffRec::GeotiffRec() : m_matrix( 3, std::vector< double >( 3, 0 ) ), scale( 3, 0 )
{
	// コンストラクター
	m_open = false;
}

GeotiffRec::~GeotiffRec()
{
	// デストラクター
	
	// 必要なら閉じる
	if ( m_open ) {
		close();
	}
}

bool GeotiffRec::open( char *filename )
{
	// ファイルを開く
	
	// すでに開かれていれば開き直す
	if ( m_open ) {
		close();
	}
	
	// tifrec初期化
	if ( tifrec_open( &m_tifrec, filename ) == TIFREC_RES_SUCCEED ) {
		m_open = true;
	} else {
		m_open = false;
	}
	
	return m_open;
}

void GeotiffRec::close( void )
{
	// ファイルを閉じる
	if ( !m_open ) {
		return;
	}
	
	tifrec_close( &m_tifrec );
	
	m_open = false;
}

bool GeotiffRec::initialize( void )
{
	// 初期化する
	struct TIFREC_IFD *next;
	int i;
	int j;
	int err = 0;
	int set_scale = 0;
	
	// 開かれてなければ何もしない
	if ( !m_open ) {
		return false;
	}
	
	// 初期化
	m_proj_mode = TIFREC_GEOTIFF_PROJECTION_MODE_UNKNOWN;
	
	/* ifdをたぐり、必要な情報を探す ( DEBUG : 今は1つ目のIFDだけ読むようにしている : next = NULL ) */
	for ( next = m_tifrec.ifd; next != NULL; next = NULL ) {
		for ( i = 0; i < next->count; i++ ) {
			if ( next->entry[i].tag == TIFREC_GEOTIFF_TAG_MODEL_TIEPOINT ) {
				/* タイポイント */
				
				/* タイポイントをすべて登録 */
				tiepoints.resize( next->entry[i].count / 3 );
				
				for ( j = 0; j < next->entry[i].count; j++ ) {
					if ( j % 3 == 0 ) {
						tiepoints[j / 3].x = next->entry[i].value_double[j];
					} else if ( j % 3 == 1 ) {
						tiepoints[j / 3].y = next->entry[i].value_double[j];
					}
				}
			} else if ( next->entry[i].tag == TIFREC_GEOTIFF_TAG_MODEL_PIXEL_SCALE ) {
				/* スケール */
				for ( j = 0; j < 3; j++ ) {
					scale[j] = next->entry[i].value_double[j];
				}
				
				set_scale = 1;
			} else if ( next->entry[i].tag == TIFREC_GEOTIFF_TAG_GEO_KEY_DIRECTORY ) {
				/* 座標に関するいろんな情報 */
				
				/* たぐって必要な情報があるか確認 */
				for ( j = 0; j < next->entry[i].count; j += 4 ) {
					if ( next->entry[i].value_usshort[j] == TIFREC_GEOTIFF_KEY_MODEL_TYPE ) {
						// 軽度緯度か投影か
						if ( next->entry[i].value_usshort[j + 3] == 2 ) {
							// 経度緯度モード
							m_proj_mode = TIFREC_GEOTIFF_PROJECTION_MODE_LATLON;
						} else if ( next->entry[i].value_usshort[j + 3] == 3 ) {
							// 経度緯度でも投影でもない
							err++;
						}
					} else if ( next->entry[i].value_usshort[j] == TIFREC_GEOTIFF_KEY_PROJECTED_CS_TYPE ) {
						// 投影方法
						cs_type = next->entry[i].value_usshort[j + 3];
						
						// WGS84UTMでなければ失敗
						if ( cs_type >= 32601 ) {
							m_proj_mode = TIFREC_GEOTIFF_PROJECTION_MODE_UTM;
						} else {
							err++;
						}
					}
				}
			}
		}
	}
	
	// エラーがあれば失敗
	if ( err ) {
		return false;
	}
	
	// タイポイントが2でなければ失敗 ( 今のところ )
	if ( tiepoints.size() != 2 ) {
		return false;
	}
	
	// 座標モードが不明なら失敗
	if ( m_proj_mode == TIFREC_GEOTIFF_PROJECTION_MODE_UNKNOWN ) {
		return false;
	}
	
	// スケールが設定されていなければ失敗
	if ( !set_scale ) {
		return false;
	}
	
	// 変換行列作成
	
	// 行列クリアー
	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			m_matrix[i][j] = 0;
		}
	}
	
	// 登録
	m_matrix[0][0] = scale[0];
	m_matrix[1][1] = -scale[1];
	
	m_matrix[0][2] = tiepoints[1].x;
	m_matrix[1][2] = tiepoints[1].y;
	
	// UTMモードなら必要な情報を作成 ( ZONE文字列のレターは適当に設定している )
	if ( m_proj_mode == TIFREC_GEOTIFF_PROJECTION_MODE_UTM ) {
		if ( cs_type >= 32601 && cs_type <= 32660 ) {
			// 北
			sn = 'N';
			
			zone_number = cs_type - 32601 + 1;
		} else {
			// 南
			sn = 'M';
			
			zone_number = cs_type - 32701 + 1;
		}
	}
	
	// ゾーン文字列作成
	sprintf( zone_str, "%d%c", zone_number, sn );
	
	return true;
}

Point2D GeotiffRec::getLatLon( Point2D xy )
{
	// ラスター座標をLonLatに変換
	Point2D fwd;
	
	fwd = forward( xy );
	
	// 必要ならUTMを変換
	if ( m_proj_mode == TIFREC_GEOTIFF_PROJECTION_MODE_UTM ) {
		fwd = utmToLatLon( fwd );
	}
	
	return fwd;
}

Point2D GeotiffRec::utmToLatLon( Point2D utm )
{
	// UMTをLatLonへ変換
	Point2D ret;
	
	UTM::UTMtoLL( utm.y, utm.x, zone_str, ret.y, ret.x );
	
	return ret;
}
	
Point2D GeotiffRec::forward( Point2D xy )
{
	// 変換行列を使ってラスター座標を地図座標に変換
	Point2D ret;
	
	ret.x = m_matrix[0][0] * xy.x + m_matrix[0][2];
	ret.y = m_matrix[1][1] * xy.y + m_matrix[1][2];
	
	return ret;
}

bool GeotiffRec::isUTM( void )
{
	// 座標モードがUTMかどうか
	if ( m_proj_mode == TIFREC_GEOTIFF_PROJECTION_MODE_UTM ) {
		return true;
	} else {
		return false;
	}
}

Point2D GeotiffRec::getXY( Point2D latlon )
{
	// ラスター座標取得
	Point2D xy;
	char zone[8];
	
	// UTMモードならUTMに変換
	if ( isUTM() ) {
		UTM::LLtoUTM_zone( latlon.y, latlon.x, latlon.y, latlon.x, zone, zone_number );
	}
	
	// ラスター座標作成
	xy.x = ( latlon.x - m_matrix[0][2] ) / m_matrix[0][0];
	xy.y = ( latlon.y - m_matrix[1][2] ) / m_matrix[1][1];
	
	// 返却
	return xy;
}

Point2D GeotiffRec::getScale( void )
{
	// スケール取得
	return Point2D( scale[0], scale[1] );
}

