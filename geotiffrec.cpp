#include "geotiffrec.h"

GeotiffRec::GeotiffRec() : m_matrix( 3, std::vector< double >( 3, 0 ) ), scale( 3, 0 )
{
	// �R���X�g���N�^�[
	m_open = false;
}

GeotiffRec::~GeotiffRec()
{
	// �f�X�g���N�^�[
	
	// �K�v�Ȃ����
	if ( m_open ) {
		close();
	}
}

bool GeotiffRec::open( char *filename )
{
	// �t�@�C�����J��
	
	// ���łɊJ����Ă���ΊJ������
	if ( m_open ) {
		close();
	}
	
	// tifrec������
	if ( tifrec_open( &m_tifrec, filename ) == TIFREC_RES_SUCCEED ) {
		m_open = true;
	} else {
		m_open = false;
	}
	
	return m_open;
}

void GeotiffRec::close( void )
{
	// �t�@�C�������
	if ( !m_open ) {
		return;
	}
	
	tifrec_close( &m_tifrec );
	
	m_open = false;
}

bool GeotiffRec::initialize( void )
{
	// ����������
	struct TIFREC_IFD *next;
	int i;
	int j;
	int err = 0;
	int set_scale = 0;
	
	// �J����ĂȂ���Ή������Ȃ�
	if ( !m_open ) {
		return false;
	}
	
	// ������
	m_proj_mode = TIFREC_GEOTIFF_PROJECTION_MODE_UNKNOWN;
	
	/* ifd��������A�K�v�ȏ���T�� ( DEBUG : ����1�ڂ�IFD�����ǂނ悤�ɂ��Ă��� : next = NULL ) */
	for ( next = m_tifrec.ifd; next != NULL; next = NULL ) {
		for ( i = 0; i < next->count; i++ ) {
			if ( next->entry[i].tag == TIFREC_GEOTIFF_TAG_MODEL_TIEPOINT ) {
				/* �^�C�|�C���g */
				
				/* �^�C�|�C���g�����ׂēo�^ */
				tiepoints.resize( next->entry[i].count / 3 );
				
				for ( j = 0; j < next->entry[i].count; j++ ) {
					if ( j % 3 == 0 ) {
						tiepoints[j / 3].x = next->entry[i].value_double[j];
					} else if ( j % 3 == 1 ) {
						tiepoints[j / 3].y = next->entry[i].value_double[j];
					}
				}
			} else if ( next->entry[i].tag == TIFREC_GEOTIFF_TAG_MODEL_PIXEL_SCALE ) {
				/* �X�P�[�� */
				for ( j = 0; j < 3; j++ ) {
					scale[j] = next->entry[i].value_double[j];
				}
				
				set_scale = 1;
			} else if ( next->entry[i].tag == TIFREC_GEOTIFF_TAG_GEO_KEY_DIRECTORY ) {
				/* ���W�Ɋւ��邢���ȏ�� */
				
				/* �������ĕK�v�ȏ�񂪂��邩�m�F */
				for ( j = 0; j < next->entry[i].count; j += 4 ) {
					if ( next->entry[i].value_usshort[j] == TIFREC_GEOTIFF_KEY_MODEL_TYPE ) {
						// �y�x�ܓx�����e��
						if ( next->entry[i].value_usshort[j + 3] == 2 ) {
							// �o�x�ܓx���[�h
							m_proj_mode = TIFREC_GEOTIFF_PROJECTION_MODE_LATLON;
						} else if ( next->entry[i].value_usshort[j + 3] == 3 ) {
							// �o�x�ܓx�ł����e�ł��Ȃ�
							err++;
						}
					} else if ( next->entry[i].value_usshort[j] == TIFREC_GEOTIFF_KEY_PROJECTED_CS_TYPE ) {
						// ���e���@
						cs_type = next->entry[i].value_usshort[j + 3];
						
						// WGS84UTM�łȂ���Ύ��s
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
	
	// �G���[������Ύ��s
	if ( err ) {
		return false;
	}
	
	// �^�C�|�C���g��2�łȂ���Ύ��s ( ���̂Ƃ��� )
	if ( tiepoints.size() != 2 ) {
		return false;
	}
	
	// ���W���[�h���s���Ȃ玸�s
	if ( m_proj_mode == TIFREC_GEOTIFF_PROJECTION_MODE_UNKNOWN ) {
		return false;
	}
	
	// �X�P�[�����ݒ肳��Ă��Ȃ���Ύ��s
	if ( !set_scale ) {
		return false;
	}
	
	// �ϊ��s��쐬
	
	// �s��N���A�[
	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			m_matrix[i][j] = 0;
		}
	}
	
	// �o�^
	m_matrix[0][0] = scale[0];
	m_matrix[1][1] = -scale[1];
	
	m_matrix[0][2] = tiepoints[1].x;
	m_matrix[1][2] = tiepoints[1].y;
	
	// UTM���[�h�Ȃ�K�v�ȏ����쐬 ( ZONE������̃��^�[�͓K���ɐݒ肵�Ă��� )
	if ( m_proj_mode == TIFREC_GEOTIFF_PROJECTION_MODE_UTM ) {
		if ( cs_type >= 32601 && cs_type <= 32660 ) {
			// �k
			sn = 'N';
			
			zone_number = cs_type - 32601 + 1;
		} else {
			// ��
			sn = 'M';
			
			zone_number = cs_type - 32701 + 1;
		}
	}
	
	// �]�[��������쐬
	sprintf( zone_str, "%d%c", zone_number, sn );
	
	return true;
}

Point2D GeotiffRec::getLatLon( Point2D xy )
{
	// ���X�^�[���W��LonLat�ɕϊ�
	Point2D fwd;
	
	fwd = forward( xy );
	
	// �K�v�Ȃ�UTM��ϊ�
	if ( m_proj_mode == TIFREC_GEOTIFF_PROJECTION_MODE_UTM ) {
		fwd = utmToLatLon( fwd );
	}
	
	return fwd;
}

Point2D GeotiffRec::utmToLatLon( Point2D utm )
{
	// UMT��LatLon�֕ϊ�
	Point2D ret;
	
	UTM::UTMtoLL( utm.y, utm.x, zone_str, ret.y, ret.x );
	
	return ret;
}
	
Point2D GeotiffRec::forward( Point2D xy )
{
	// �ϊ��s����g���ă��X�^�[���W��n�}���W�ɕϊ�
	Point2D ret;
	
	ret.x = m_matrix[0][0] * xy.x + m_matrix[0][2];
	ret.y = m_matrix[1][1] * xy.y + m_matrix[1][2];
	
	return ret;
}

bool GeotiffRec::isUTM( void )
{
	// ���W���[�h��UTM���ǂ���
	if ( m_proj_mode == TIFREC_GEOTIFF_PROJECTION_MODE_UTM ) {
		return true;
	} else {
		return false;
	}
}

Point2D GeotiffRec::getXY( Point2D latlon )
{
	// ���X�^�[���W�擾
	Point2D xy;
	char zone[8];
	
	// UTM���[�h�Ȃ�UTM�ɕϊ�
	if ( isUTM() ) {
		UTM::LLtoUTM_zone( latlon.y, latlon.x, latlon.y, latlon.x, zone, zone_number );
	}
	
	// ���X�^�[���W�쐬
	xy.x = ( latlon.x - m_matrix[0][2] ) / m_matrix[0][0];
	xy.y = ( latlon.y - m_matrix[1][2] ) / m_matrix[1][1];
	
	// �ԋp
	return xy;
}

Point2D GeotiffRec::getScale( void )
{
	// �X�P�[���擾
	return Point2D( scale[0], scale[1] );
}

