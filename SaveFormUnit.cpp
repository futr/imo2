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
    double var[ECALC_VAR_COUNT];
    double *vars[ECALC_VAR_COUNT];

    /* �d�샆�j�b�g */
    struct ECALC_TOKEN *tok_r;
    struct ECALC_TOKEN *tok_g;
    struct ECALC_TOKEN *tok_b;

    int zoom_pos;
    AnsiString stime;
    unsigned int start;
    unsigned int now;

    // MainForm���玝���Ă���
    mf = SatViewMainForm;
    ColorMap *cmap = mf->color_map;
    TList *list_band = mf->list_band;
    bool drawModeExp = mf->b_draw_mode_exp;

    int i, j, k, l;
    int skip;
    int blockSize;
    RGBTRIPLE *rgb;
    unsigned char red;
    unsigned char blue;
    unsigned char green;
    unsigned char *buf;
    double mag;
    int img_start_x;
    int img_start_y;
    int img_read_xc;
    int img_read_yc;
    int img_use_xc;
    int img_use_yc;
    int draw_w;
    int draw_h;
    int draw_x;
    int draw_y;

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

    // �Y�[���͈�{�Œ�
    zoom_pos = ZOOM_MAX / 2;
    mag = mf->ZoomPosToMag( zoom_pos );

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
    draw_x = 0;
    draw_y = 0;
    draw_w = mf->img_w;
    draw_h = mf->img_h;

    // skip�i�ǂݍ��ݔ�΂��ʁj�Ɖ�f�u���b�N�T�C�Y����

    if ( mag >= 1 ) {
        skip = 1;
        blockSize = mag;
    } else {
        skip = 1 / mag;
        blockSize = 1;
    }

    /* �摜�ǂݍ��݊J�n�n�_���� */
    img_start_x = 0;
    img_start_y = 0;

	/* �ǂݍ��݌��i�͈́j���� */
	img_read_xc = ( draw_w - draw_x ) / mag;
	img_read_yc = ( draw_h - draw_y ) / mag;

    /* ���ۂɎg����� */
    img_use_xc = img_read_xc / skip;
    img_use_yc = img_read_yc / skip;

    // ��ʍs�o�b�t�@�쐬 ( R, G, B )
    buf = (unsigned char *)malloc( img_read_xc * sizeof( RGBTRIPLE ) * blockSize );

    /* �v���O���X�o�[�ݒ� */
    SaveProgressBar->Max = img_read_yc;
    SaveProgressBar->Position = 0;

    /* BMP�쐬 */
    ebmp_create_file_open( &ebmp, filename.c_str(), img_read_xc, img_read_yc );

    // �ǂݍ��݂ƕ`�惋�[�v
    for ( i = 0; i < img_use_yc; i++ ) {
        /* �e�o���h�ǂݍ��� */
        for ( j = 0; j < list_band->Count; j++ ) {
            /* �o���h�w��A�f�[�^�ǂݍ��� */
            band = (struct REMOS_FRONT_BAND *)(list_band->Items[j]);

            // �o���h�̃��C���o�b�t�@�ǂݍ���
            mf->ReadBandLineBuf( band, img_start_y + i * skip, img_start_x, img_read_xc );
        }

        /* ��f�[�^�ǂݏo�����[�v */
        for ( j = 0; j < img_use_xc; j++ ) {
            /* �F�쐬 */
            for ( k = 0; k < list_band->Count; k++ ) {
            	/* �o���h�擾 */
                band = (struct REMOS_FRONT_BAND *)(list_band->Items[k]);

                /* �l�o�^ */
                if ( band->canvas_mode ) {
                    // �L�����o�X���[�h
                	var[k] = remos_get_ranged_pixel( band->band, band->line_buf[j * skip] );
                } else {
                    var[k] = remos_get_ranged_pixel( band->band, remos_data_to_value_band( band->band, band->line_buf, j * skip ) );
                }
            }

            // �����[�h���J���[�o�[���[�h��
            if ( drawModeExp ) {
            	// �����[�h
                red   = mf->GetUCharValue( ecalc_get_tree_value( tok_r, vars, 0 ) );
                green = mf->GetUCharValue( ecalc_get_tree_value( tok_g, vars, 0 ) );
                blue  = mf->GetUCharValue( ecalc_get_tree_value( tok_b, vars, 0 ) );
            } else {
            	// �J���[�o�[���[�h
                cmap->evalExpression( vars, 0 );
                cmap->makeColor();

                red   = mf->GetUCharValue( cmap->getR() );
                green = mf->GetUCharValue( cmap->getG() );
                blue  = mf->GetUCharValue( cmap->getB() );
            }

            // �o�b�t�@�ɕۑ�
            rgb = (RGBTRIPLE *)buf;

            // �s�����
            for ( k = 0; k < blockSize; k++ ) {
                rgb[j * blockSize + k].rgbtBlue  = blue;
                rgb[j * blockSize + k].rgbtGreen = green;
                rgb[j * blockSize + k].rgbtRed   = red;
            }
        }

        // �sblockSize���`�惋�[�v
        for ( j = 0; j < blockSize; j++ ) {
            /* �摜�����o�� */
            ebmp_write_line( &ebmp, j + draw_y + i * blockSize, buf, ebmp.line_size );
        }

        // UI����

        /* �v���O���X */
        SaveProgressBar->Position = i;

        /* ���Ԍv�� */
        now = GetTickCount();

        if ( i % 10 == 0 && i ) {
            stime.sprintf( "�������� : %d�b / %d�b", ( now - start ) / 1000, (int)( ( now - start ) * ( (float)img_read_yc / i ) / 1000 ) );
            TimeLabel->Caption = stime;

            Application->ProcessMessages();
        }

        // �E�o����
        if ( !saving ) {
            break;
        }
    }

    // �o�b�t�@���
    free( buf );

    /* BMP�t�@�C������ */
    ebmp_create_file_close( &ebmp );

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


