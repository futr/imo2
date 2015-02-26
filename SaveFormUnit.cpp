//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SaveFormUnit.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSaveForm *SaveForm;
//---------------------------------------------------------------------------
__fastcall TSaveForm::TSaveForm(TComponent* Owner)
	: TForm(Owner)
{
	saving = false;
}
//---------------------------------------------------------------------------
void __fastcall TSaveForm::CancelButtonClick(TObject *Sender)
{
	// �ۑ����Ȃ狭����~���邩�m�F����
    if ( saving ) {
    	if ( Application->MessageBoxA( "�ۑ��𒆎~���܂����H\n�ۑ��𒆎~�����ꍇ�摜���������ۑ�����܂���B", "�ۑ��𒆎~", MB_YESNO | MB_ICONQUESTION ) == ID_YES ) {
        	saving = false;
            CancelButton->Enabled = false;
        }
    } else {
    	Close();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSaveForm::SaveButtonClick(TObject *Sender)
{
	/* �ۑ����� */
    SaveButton->Enabled = false;
    CancelButton->Enabled = true;
    SaveModeRadioGroup->Enabled = false;

    saving = true;

    if ( ImgSaveDialog->Execute() ) {
    	// �㏑���m�F
        if ( FileExists( ImgSaveDialog->FileName ) ) {
        	if( Application->MessageBoxA( "�t�@�������łɑ��݂��Ă��܂��B\n�㏑�����܂����H", "�㏑���m�F", MB_YESNO | MB_ICONWARNING ) == ID_NO ) {
            	goto L_EXIT_SAVE;
            }
        }

        if ( SaveModeRadioGroup->ItemIndex == 0 ) {
            /* ���̂܂� */
            SatViewMainForm->SatImage->Picture->SaveToFile( ImgSaveDialog->FileName );
        } else {
            /* ���{ ( DEBUG : ��Ԗڂ̈����͔{���̏��� ) */
			SaveToBmp( ImgSaveDialog->FileName, 0 );
        }
    }

L_EXIT_SAVE:
	saving = false;
    CancelButton->Enabled = true;
    SaveButton->Enabled = true;
    SaveModeRadioGroup->Enabled = true;

    SaveProgressBar->Position = 0;

    Close();
}
//---------------------------------------------------------------------------
bool __fastcall TSaveForm::SaveToBmp( AnsiString filename, int zpos )
{
	/* MainForm��Draw���ꕔ�R�s�[ */

	/* �`��֐� */
    TSatViewMainForm *mf;
    EBMP_FILE ebmp;

    struct REMOS_FRONT_BAND *band;
    unsigned int start;
    unsigned int now;
    double var[ECALC_VAR_COUNT];
    double *vars[ECALC_VAR_COUNT];
    AnsiString str;

    int i, j, k, l;
    int skip;
    int zoom_pos;
    unsigned char red;
    unsigned char blue;
    unsigned char green;

    /* �d�샆�j�b�g */
    struct ECALC_TOKEN *tok_r;
    struct ECALC_TOKEN *tok_g;
    struct ECALC_TOKEN *tok_b;

    int img_read_xc;
    int img_read_yc;

    double mag;


    unsigned char *buf;

    AnsiString stime;

    /* ���ԑ���J�n */
    start = GetTickCount();

    /* ���C���t�H�[���̃|�C���^ */
    mf = SatViewMainForm;

    tok_r = NULL;
    tok_b = NULL;
    tok_g = NULL;

    /* �J���ĂȂ���Ύ��s���Ȃ� */
    if ( !mf->b_open ) {
    	return false;
    }

    /* �`�撆�Ȃ���s���Ȃ� */
    if ( mf->b_drawing ) {
    	return false;
    }

    /* �`�拖���Ȃ���Ε`�悵�Ȃ� */
    if ( !mf->b_draw ) {
    	return false;
    }

    mf->b_drawing = true;

    /* �ϐ��|�C���^�o�^ */
    for ( i = 0; i < ECALC_VAR_COUNT; i++ ) {
    	vars[i] = &var[i];
    }

    /* �����j�b�g������ */
	ecalc_free_token( tok_b );
    tok_b = ecalc_make_token( mf->ExpEditB->Text.c_str() );
	tok_b = ecalc_make_tree( tok_b );

	ecalc_free_token( tok_g );
    tok_g = ecalc_make_token( mf->ExpEditG->Text.c_str() );
	tok_g = ecalc_make_tree( tok_g );

	ecalc_free_token( tok_r );
    tok_r = ecalc_make_token( mf->ExpEditR->Text.c_str() );
	tok_r = ecalc_make_tree( tok_r );

    /* ���W�v�Z */

	/* DEBUG : �����ł͋������{ */
    zoom_pos = ZOOM_MAX / 2;

    /* �{���ݒ� */
    if ( zoom_pos > ZOOM_MAX / 2 ) {
    	/* �k�� */
        mag  = 1.0 / Power( 2, zoom_pos - ZOOM_MAX / 2 );
        skip = 1.0 / mag;
    } else if ( zoom_pos < ZOOM_MAX / 2 ) {
    	/* �g�� */
    	mag  = Power( 2, ZOOM_MAX / 2 - zoom_pos );
        skip = mag;
    } else {
    	/* ���{ */
    	mag  = 1;
        skip = 1;
    }

    /* �g�傩�k�����ŕ`����@�ύX */
    if ( mag >= 1 ) {
    	/* �g�� */

    	/* �ǂݍ��݌����� */
    	img_read_xc = mf->img_w;
    	img_read_yc = mf->img_h;

        /* �v���O���X�o�[�ݒ� */
        SaveProgressBar->Max = img_read_yc;
        SaveProgressBar->Position = 0;

        /* BMP�쐬 */
        ebmp_create_file_open( &ebmp, filename.c_str(), img_read_xc * skip, img_read_yc * skip );

    	/* �o�b�t�@�[�m�� */
    	buf = (unsigned char *)malloc( ebmp.line_size );

        /* �ǂݍ��ݕ`�惋�[�v */
        for ( i = 0; i < img_read_yc; i++ ) {
        	/* ������~���ꂽ��~�܂� */
            if ( !saving ) {
            	break;
            }

            /* �e�o���h�ǂݍ��� */
            for ( j = 0; j < mf->list_band->Count; j++ ) {
                /* �o���h�w��A�f�[�^�ǂݍ��� */
                band = (struct REMOS_FRONT_BAND *)(mf->list_band->Items[j]);

                /* ���[�h�ŕ��@���Ⴄ */
                if ( !band->canvas_mode ) {
                	remos_get_line_pixels( band->band, band->line_buf, 0 + i, 0, img_read_xc );
                } else {
                	mf->GetLineData( band, band->line_buf, 0 + i, 0, img_read_xc );
                }
            }

            /* ��ʂɏ������� */

            /* ��f�[�^�ǂݏo�����[�v */
            for ( k = 0; k < img_read_xc; k++ ) {
                /* �F�쐬 */
                for ( l = 0; l < mf->list_band->Count; l++ ) {
                    /* �o���h�擾 */
                    band = (struct REMOS_FRONT_BAND *)(mf->list_band->Items[l]);

                    /* �l�o�^ */
                    if ( band->canvas_mode ) {
                        // �L�����o�X���[�h
                    	var[l] = remos_get_ranged_pixel( band->band, band->line_buf[k] );
                    } else {
                        var[l] = remos_get_ranged_pixel( band->band, remos_data_to_value_band( band->band, band->line_buf, k ) );
                    }
                }

                // �����[�h���J���[�o�[���[�h��
                if ( mf->b_draw_mode_exp ) {
                	// �����[�h
                    red   = mf->GetUCharValue( ecalc_get_tree_value( tok_r, vars, 0 ) );
                    green = mf->GetUCharValue( ecalc_get_tree_value( tok_g, vars, 0 ) );
                    blue  = mf->GetUCharValue( ecalc_get_tree_value( tok_b, vars, 0 ) );
                } else {
                	// �J���[�o�[���[�h
                    mf->color_map->evalExpression( vars, 0 );
                    mf->color_map->makeColor();

                    red   = mf->GetUCharValue( mf->color_map->getR() );
                    green = mf->GetUCharValue( mf->color_map->getG() );
                    blue  = mf->GetUCharValue( mf->color_map->getB() );
                }

                for ( l = 0; l < skip; l++ ) {
                    buf[l * 3 + k * 3 * skip + 0] = blue;
                    buf[l * 3 + k * 3 * skip + 1] = green;
                    buf[l * 3 + k * 3 * skip + 2] = red;
                }
            }

            /* �s���[�v */
            for ( j = 0; j < skip; j++ ) {
                /* �摜�����o�� */
                ebmp_write_line( &ebmp, i * skip + j, buf, ebmp.line_size );
            }

            /* �v���O���X */
            SaveProgressBar->Position = i;

            /* ���Ԍv�� */
            now = GetTickCount();

            if ( i ) {
            	stime.sprintf( "�������� : %d�b / %d�b", ( now - start ) / 1000, (int)( ( now - start ) * ( (float)img_read_yc / i ) / 1000 ) );
            }

            TimeLabel->Caption = stime;

            Application->ProcessMessages();
        }

        /* BMP�t�@�C������ */
        ebmp_create_file_close( &ebmp );
    } else {
    	/* �k�� */

    	/* �ǂݍ��݌����� */
        img_read_xc = ( mf->img_w ) / skip;
        img_read_yc = ( mf->img_h ) / skip;

        /* �v���O���X�o�[�ݒ� */
        SaveProgressBar->Max = img_read_yc;
        SaveProgressBar->Position = 0;

        /* BMP�쐬 */
        ebmp_create_file_open( &ebmp, filename.c_str(), img_read_xc, img_read_yc );

    	/* �o�b�t�@�[�m�� */
    	buf = (unsigned char *)malloc( ebmp.line_size );

        /* �ǂݍ��ݕ`�惋�[�v */
        for ( i = 0; i < img_read_yc; i++ ) {
        	// ������~���ꂽ��~�܂�
            if ( !saving ) {
            	break;
            }

            /* �e�o���h�ǂݍ��� */
            for ( j = 0; j < mf->list_band->Count; j++ ) {
                /* �o���h�w��A�f�[�^�ǂݍ��� */
                band = (struct REMOS_FRONT_BAND *)(mf->list_band->Items[j]);

               /* ���[�h�ŕ��@���Ⴄ */
                if ( !band->canvas_mode ) {
                	remos_get_line_pixels( band->band, band->line_buf, i * skip, 0, img_read_xc * skip );
                } else {
                	mf->GetLineData( band, band->line_buf, i * skip, 0, img_read_xc * skip );
                }
            }

            /* ��ʂɏ������� */

            /* ��f�[�^�ǂݏo�����[�v */
            for ( k = 0; k < img_read_xc; k++ ) {
                /* �F�쐬 */
                for ( l = 0; l < mf->list_band->Count; l++ ) {
                	/* �o���h�擾 */
                    band = (struct REMOS_FRONT_BAND *)(mf->list_band->Items[l]);

                    /* �l�o�^ */
                    if ( band->canvas_mode ) {
                        // �L�����o�X���[�h
                    	var[l] = remos_get_ranged_pixel( band->band, band->line_buf[k * skip] );
                    } else {
                        var[l] = remos_get_ranged_pixel( band->band, remos_data_to_value_band( band->band, band->line_buf, k * skip ) );
                    }
                }

                // �����[�h���J���[�o�[���[�h��
                if ( mf->b_draw_mode_exp ) {
                	// �����[�h
                    red   = mf->GetUCharValue( ecalc_get_tree_value( tok_r, vars, 0 ) );
                    green = mf->GetUCharValue( ecalc_get_tree_value( tok_g, vars, 0 ) );
                    blue  = mf->GetUCharValue( ecalc_get_tree_value( tok_b, vars, 0 ) );
                } else {
                	// �J���[�o�[���[�h
                    mf->color_map->evalExpression( vars, 0 );
                    mf->color_map->makeColor();

                    red   = mf->GetUCharValue( mf->color_map->getR() );
                    green = mf->GetUCharValue( mf->color_map->getG() );
                    blue  = mf->GetUCharValue( mf->color_map->getB() );
                }

                buf[k * 3 + 0] = blue;
                buf[k * 3 + 1] = green;
                buf[k * 3 + 2] = red;
            }

            /* �摜�����o�� */
            ebmp_write_line( &ebmp, i, buf, ebmp.line_size );

            /* �v���O���X */
            SaveProgressBar->Position = i;

            /* ���Ԍv�� */
            now = GetTickCount();

            if ( i ) {
            	stime.sprintf( "�������� : %d�b / %d�b", ( now - start ) / 1000, (int)( ( now - start ) * ( (float)img_read_yc / i ) / 1000 ) );
            }

            TimeLabel->Caption = stime;

            Application->ProcessMessages();
        }

        /* BMP�t�@�C������ */
        ebmp_create_file_close( &ebmp );
    }

    /* �o�b�t�@�J�� */
    free( buf );

    mf->b_drawing = false;

    return true;
}
//---------------------------------------------------------------------------
void __fastcall TSaveForm::FormShow(TObject *Sender)
{
	saving = false;
    CancelButton->Enabled = true;
    SaveButton->Enabled = true;
    SaveModeRadioGroup->Enabled = true;
    TimeLabel->Caption = "";
}
//---------------------------------------------------------------------------


