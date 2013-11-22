#include "ColorBar.h"
#include "SaveFormUnit.h"
#include "PresetLS_THM.h"
#include "PresetHTM.h"
#include "PresetNDVI.h"
#include "About.h"
#include "PixValue.h"

/* �q�X�g�O�����̏�����啝�ɏ����������A�ǂ�ȃr�b�g���ł�256���x���ɕ������� */
/* �q�X�g�O�����̖��͏C�������͂� */
/* Canvas���[�h�̃q�X�g�O�����Ԉ���ĂȂ��H */
/* Canvas���[�h�̔r���������K�� */
/* b_canvas�ňꉞ�r�����������Ă��邪�A�ꉞ�Ȃ̂ł����ƍ��Ƃ��͏��������� */

/* �ꏊ�l�_�C�A���O��float�Ƃ�usint�Ƃ����ނ��Ⴍ����ɂ��Ă��܂��� */
/* remos�ɕs���S��pal1.1���t���Ă��� */
/* �ꏊ�l�_�C�A���O�Ŋg�僂�[�h���ʒu�����������A���Ɋg�債�č��ɋ󔒂�����ꍇ */

/* �`��֐��̃R�s�[����ʂɂ���̂ƁAremos����l�����o�����@����v���ĂȂ���ɕ��U���Ă��邱�� */
/* ���W�l�擾�̃R�[�h�����U���Ă��邱�ƂȂǂ̖�肪����B */
/* �����W�̐ݒ���قƂ�ǋ@�\���Ă��Ȃ� */

#include "Status.h"
//---------------------------------------------------------------------------

#include <vcl.h>
#include <Jpeg.hpp>
#pragma hdrstop

#include "Main.h"

#include <math.h>
#include <Math.hpp>

#include "ecalc.h"
#include "remos.h"
#include "econf.h"
#include "ColorMapConfigUnit.h"
#include "PresetFormAGDEMUnit.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CSPIN"
#pragma resource "*.dfm"
TSatViewMainForm *SatViewMainForm;

/* �摜�N�_ */
int img_x_start = 0;
int img_y_start = 0;

//---------------------------------------------------------------------------
__fastcall TSatViewMainForm::TSatViewMainForm(TComponent* Owner)
	: TForm(Owner)
{
	/* �q���g�ݒ� */
	Application->OnHint = ShowHint;

	/* ���X�g�m�� */
    list_band = new TList();
    list_file = new TList();

    /* �o�b�N�o�b�t�@�쐬 */
    img_back = new Graphics::TBitmap();

    /* Canvas���[�h�p */

    /* �l������ */
    b_open = false;
    b_click = false;
    b_drawing = false;
    b_zoom_click = false;
    b_canvas = false;
    b_sync = false;
    b_draw = true;
    b_config = false;
    b_config_land = false;   					/* ����͌o�x�ܓx�p */
    b_config_resolution = false;
    b_mode_length = false;
    b_mode_stamp_cb = false;
    b_draw_mode_exp = true;

    // �ݒ菉����
    clearSetting();

    tok_r = NULL;
    tok_g = NULL;
    tok_b = NULL;
    tok_color_bar = NULL;

    color_bar_exp = "a";

    img_w = 1;
    img_h = 1;

    mag = 1;

    zoom_pos = ZOOM_MAX / 2;

	/* �������}�l�[�W�������� */
	ecalc_memman_init();

    /* �^�C�g�� */
    Application->Title = "SatelliteEye";

    /* �q���g�����o�^ */
    Application->OnHint = DisplayHint;

    /* �h���b�O�󂯎�� */
    DragAcceptFiles( Handle, true );

    /* ���b�Z�[�W�����֐��ݒ� */
    Application->OnMessage = AppMessage;

    // �z�C�[���C�x���g�n���h���[�ݒ� ( �����E�B���h�E���b�Z�[�W���������Ȃ��Ɩ��� )
    // SatImage->OnMouseWheel = ImageMouseWheelEvent;
    SatViewMainForm->OnMouseWheel = ImageMouseWheelEvent;

    /* �u���[�h�L���X�g���b�Z�[�W�o�^ */
    umsg_x       = RegisterWindowMessage( "SATIMGVIEW_SCR_X_9145" );
    umsg_y       = RegisterWindowMessage( "SATIMGVIEW_SCR_Y_9145" );
    umsg_mag     = RegisterWindowMessage( "SATIMGVIEW_SCR_MAG_9145" );
    umsg_connect = RegisterWindowMessage( "SATIMGVIEW_CONNECT_9145" );
    umsg_lat     = RegisterWindowMessage( "SATIMGVIEW_LAT_9145" );
    umsg_lon     = RegisterWindowMessage( "SATIMGVIEW_LON_9145" );
    umsg_draw    = RegisterWindowMessage( "SATIMGVIEW_DRAW_9145" );

    // �J���[�}�b�v������
    color_map = new ColorMap;

    // �J���[�}�b�v�ɔ����������l�Ƃ��Đݒ�
    color_map->deleteAllColorLevel();
    color_map->addColorLevel( new ColorLevel(   0,   0,   0,   0 ) );
    color_map->addColorLevel( new ColorLevel( 255, 255, 255, 255 ) );
    color_map->setSmooth();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	/* ���X�g��� */
    int i;

    /* �t�@�C���J�� */
    for ( i = 0; i < list_file->Count; i++ ) {
    	remos_close( (struct REMOS_FILE_CONTAINER *)list_file->Items[i] );
        free( list_file->Items[i] );
    }

    /* �o���h�{�b�N�X�J�� */
    for ( i = 0; i < list_band->Count; i++ ) {
    	FreeBandBox( (struct REMOS_FRONT_BAND *)list_band->Items[i] );
        free( list_band->Items[i] );
    }

    // �J���[�}�b�v�j��
    delete color_map;

    delete list_band;
    delete list_file;
    delete img_back;

    ecalc_free_token( tok_r );
    ecalc_free_token( tok_g );
    ecalc_free_token( tok_b );
    ecalc_free_token( tok_color_bar );
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ShowHint( TObject *sender )
{
	/* �q���g��\�� */
    StatusBar->SimpleText = Application->Hint;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::TBOpenClick(TObject *Sender)
{
	/* �t�@�C�����J�� */

    if ( OpenDialog->Execute() ) {
    	OpenFiles();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::TBSaveClick(TObject *Sender)
{
    /* �J���ĂȂ���Ύ��s���Ȃ� */
    if ( !b_open ) {
    	return;
    }

	/* �摜�ۑ� ( �ςɑ��삳���ƕ|���̂�Modal ) */
    SaveForm->ShowModal();

    /*
    if ( SaveDialog->Execute() ) {
    	SatImage->Picture->SaveToFile( SaveDialog->FileName );
    }
	*/
}
//---------------------------------------------------------------------------
struct REMOS_FRONT_BAND *TSatViewMainForm::MakeBandBox( struct REMOS_BAND *band, AnsiString fln, int index )
{
    /* �o���h�{�b�N�X�쐬 */
    char num[2];
    struct REMOS_FRONT_BAND *box;

    /* �o���h�ԍ��쐬 */
    num[0] = 0x61 + index;
    num[1] = '\0';

    /* �m�� */
    box = (struct REMOS_FRONT_BAND *)malloc( sizeof(struct REMOS_FRONT_BAND) );

    box->img_hist    = new TImage( SatViewMainForm );
    box->label_name  = new TLabel( SatViewMainForm );
    box->panel_cont  = new TPanel( SatViewMainForm );

    box->btn_close = new TBitBtn( SatViewMainForm );
    box->btn_auto  = new TBitBtn( SatViewMainForm );
    box->btn_max   = new TBitBtn( SatViewMainForm );
    box->btn_red   = new TBitBtn( SatViewMainForm );
    box->btn_green = new TBitBtn( SatViewMainForm );
    box->btn_blue  = new TBitBtn( SatViewMainForm );
    box->btn_wb    = new TBitBtn( SatViewMainForm );

    box->label_bottom = new TLabel( SatViewMainForm );
    box->label_top = new TLabel( SatViewMainForm );
    box->edit_bottom = new TEdit( SatViewMainForm );
    box->updown_bottom = new TUpDown( SatViewMainForm );
    box->edit_top = new TEdit( SatViewMainForm );
    box->updown_top = new TUpDown( SatViewMainForm );

    /* �ݒ� */
    box->index = index;
    box->band = band;
    box->fln = new AnsiString( fln );

    box->panel_cont->Align  = alTop;
    box->panel_cont->Parent = BandList;
    box->panel_cont->Height = 100;
    box->panel_cont->Tag    = (int)box;

    box->btn_close->Parent     = box->panel_cont;
    box->btn_auto->Parent      = box->panel_cont;
    box->btn_max->Parent       = box->panel_cont;
    box->btn_wb->Parent        = box->panel_cont;
    box->btn_red->Parent       = box->panel_cont;
    box->btn_green->Parent     = box->panel_cont;
    box->btn_blue->Parent      = box->panel_cont;
    box->img_hist->Parent      = box->panel_cont;
    box->label_name->Parent    = box->panel_cont;
    box->label_bottom->Parent  = box->panel_cont;
    box->label_top->Parent     = box->panel_cont;
    box->edit_bottom->Parent   = box->panel_cont;
    box->updown_bottom->Parent = box->panel_cont;
    box->edit_top->Parent      = box->panel_cont;
    box->updown_top->Parent    = box->panel_cont;

    box->btn_close->TabOrder   = 0;
    box->edit_bottom->TabOrder = 1;
    box->edit_top->TabOrder    = 2;
    box->btn_auto->TabOrder    = 3;
    box->btn_max->TabOrder     = 4;
    box->btn_red->TabOrder     = 5;
    box->btn_green->TabOrder   = 6;
    box->btn_blue->TabOrder    = 7;
    box->btn_wb->TabOrder      = 8;


    box->img_hist->OnMouseDown = HistMouseDown;
    box->img_hist->OnMouseMove = HistMouseMove;
    box->img_hist->OnMouseUp   = HistMouseUp;

    box->btn_close->OnClick = BandCloseBtnClick;

    box->edit_bottom->OnChange = BandEditDownChange;
    box->edit_top->OnChange    = BandEditUpChange;

    box->btn_blue->OnClick  = BlueBtnClick;
    box->btn_red->OnClick   = RedBtnClick;
    box->btn_green->OnClick = GreenBtnClick;
    box->btn_auto->OnClick  = AutoBtnClick;
    box->btn_wb->OnClick    = WBBtnClick;
    box->btn_max->OnClick   = MaxBtnClick;

    box->img_hist->Top  = 24;
    box->img_hist->Left = 4;

    box->img_hist->Height = 45;
    box->img_hist->Width  = 256;

    box->img_hist->Canvas->Brush->Color = clWhite;
    box->img_hist->Canvas->FillRect( Rect( 0, 0, 177, 45 ) );

    box->label_bottom->Left = 8;
    box->label_bottom->Top = 76;
    box->label_bottom->Caption = "��";

    box->label_top->Left = 68;
    box->label_top->Top = 76;
    box->label_top->Caption = "��";

    box->edit_bottom->Left = 24;
    box->edit_bottom->Top = 72;
    box->edit_bottom->Width = 40;

    box->edit_top->Top = 72;
    box->edit_top->Left = 84;
    box->edit_top->Width = 40;

    // box->updown_bottom->Associate = box->edit_bottom;
    // box->updown_top->Associate = box->edit_top;
    box->updown_bottom->Visible = false;
    box->updown_top->Visible    = false;

    box->updown_bottom->Max = 255;
    box->updown_top->Max = 255;
    box->updown_top->Position = 255;

    box->label_name->Top = 8;
    box->label_name->Left = 28;
    // box->label_name->Font->Color = clBlue;
    box->label_name->Caption = num;
    box->label_name->Caption = box->label_name->Caption + " : " + fln;;

    box->btn_close->Top         = 4;
    box->btn_close->Left        = 4;
    box->btn_close->Height      = 17;
    box->btn_close->Width       = 21;
    box->btn_close->Caption     = "�~";
    box->btn_close->Font->Color = clRed;

    box->btn_blue->Top         = 72;
    box->btn_blue->Left        = 224;
    box->btn_blue->Height      = 21;
    box->btn_blue->Width       = 17;
    box->btn_blue->Caption     = "��";
    box->btn_blue->Font->Color = clBlue;
    box->btn_blue->Font->Size  = 10;

    box->btn_red->Top         = 72;
    box->btn_red->Left        = 184;
    box->btn_red->Height      = 21;
    box->btn_red->Width       = 17;
    box->btn_red->Caption     = "��";
    box->btn_red->Font->Color = clRed;
    box->btn_red->Font->Size  = 10;

    box->btn_green->Top         = 72;
    box->btn_green->Left        = 204;
    box->btn_green->Height      = 21;
    box->btn_green->Width       = 17;
    box->btn_green->Caption     = "��";
    box->btn_green->Font->Color = clGreen;
    box->btn_green->Font->Size  = 10;

    box->btn_wb->Top         = 72;
    box->btn_wb->Left        = 244;
    box->btn_wb->Height      = 21;
    box->btn_wb->Width       = 17;
    box->btn_wb->Caption     = "��";
    box->btn_wb->Font->Color = clBlack;
    box->btn_wb->Font->Size  = 10;

    box->btn_auto->Top         = 72;
    box->btn_auto->Left        = 128;
    box->btn_auto->Height      = 21;
    box->btn_auto->Width       = 29;
    box->btn_auto->Caption     = "����";
    box->btn_auto->Font->Size  = 8;

    box->btn_max->Top         = 72;
    box->btn_max->Left        = 156;
    box->btn_max->Height      = 21;
    box->btn_max->Width       = 21;
    box->btn_max->Caption     = "<>";
    box->btn_max->Font->Size  = 8;

    box->band->range_bottom = box->band->range_bottom;
    box->band->range_top    = box->band->range_top;

    box->hist_click = false;

    /* Canvas���[�h�p */
    box->img_canvas = NULL;

    /* ���C���o�b�t�@�쐬 */
    box->line_buf = (unsigned char *)malloc( box->band->line_img_width * box->band->byte_per_sample * box->band->sample_per_pix );	// DEBUG �o�O�������\��������

    return box;
}
//---------------------------------------------------------------------------
void TSatViewMainForm::FreeBandBox( struct REMOS_FRONT_BAND *box )
{
	/* �o���h�{�b�N�X�J�� */
    delete box->fln;

	delete box->img_hist;
    delete box->label_name;
    delete box->btn_blue;
    delete box->btn_red;
    delete box->btn_green;
    delete box->btn_auto;
    delete box->btn_wb;
    delete box->btn_max;

    // DEBUG �v���O�����I�����ɍ폜���Ă����̂łق��Ƃ�delete box->btn_close;
    box->btn_close->Visible = false;
    box->btn_close->Parent = SatViewMainForm;

    delete box->panel_cont;

    free( box->line_buf );
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::DrawImg( TImage *screen, Graphics::TBitmap *back_screen, bool drawModeExp, int zoompos, int img_cent_x, int img_cent_y, ColorMap *cmap )
{
	/* �`��֐� */
    struct REMOS_FRONT_BAND *band;
    unsigned char *line_buf;
    double var[ECALC_VAR_COUNT];
    double *vars[ECALC_VAR_COUNT];
    AnsiString str;

    int i, j, k, l;
    int skip;
    int xpos;
    RGBTRIPLE *rgb;
    unsigned char red;
    unsigned char blue;
    unsigned char green;

    unsigned char *buf;

    double mag;	// �O���[�o���Ƃ��Ԃ��Ă���̋������[�J��

    int sc_w;
    int sc_h;
    int sc_x;
    int sc_y;

    int img_start_x;
    int img_start_y;
    int img_read_xc;
    int img_read_yc;
    int img_read_xw;
    int img_read_yw;

    int draw_w;
    int draw_h;
    int draw_x;
    int draw_y;
    double draw_x_b;
    double draw_y_b;

    int bb_w;
    int bb_h;

    /* �J���ĂȂ���Ύ��s���Ȃ� */
    if ( !b_open ) {
    	return;
    }

    /* �`�撆�Ȃ���s���Ȃ� */
    if ( b_drawing ) {
    	return;
    }

    /* �`�拖���Ȃ���Ε`�悵�Ȃ� */
    if ( !b_draw ) {
    	return;
    }

    b_drawing = true;

    /* �ϐ��|�C���^�o�^ */
    for ( i = 0; i < ECALC_VAR_COUNT; i++ ) {
    	vars[i] = &var[i];
    }

    /* �����j�b�g������ */
	ecalc_free_token( tok_b );
    tok_b = ecalc_make_token( ExpEditB->Text.c_str() );
	tok_b = ecalc_make_tree( tok_b );

	ecalc_free_token( tok_g );
    tok_g = ecalc_make_token( ExpEditG->Text.c_str() );
	tok_g = ecalc_make_tree( tok_g );

	ecalc_free_token( tok_r );
    tok_r = ecalc_make_token( ExpEditR->Text.c_str() );
	tok_r = ecalc_make_tree( tok_r );

    /* �X�N���[���T�C�Y�擾 */
    sc_w = screen->Width;
    sc_h = screen->Height;

	/* �L�����o�X����点�� */
    screen->Canvas->Brush->Color = clWhite;
    screen->Picture->Bitmap->Width = sc_w;
    screen->Picture->Bitmap->Height = sc_h;

    screen->Picture->Bitmap->PixelFormat = pf24bit;
    screen->Picture->Bitmap->HandleType = bmDIB;

    /* ������ */
    str = "�`�撆";
    screen->Canvas->Font->Size   = 12;
    screen->Canvas->Font->Color  = clRed;
    screen->Canvas->Brush->Color = clWhite;
    screen->Canvas->TextOutA( sc_w / 2 - screen->Canvas->TextWidth( str ) / 2, sc_h / 2 - screen->Canvas->TextHeight( str ) - 10, str );
    Application->ProcessMessages();

    /* ���W�v�Z */

    /* �{���ݒ� */
    if ( zoompos > ZOOM_MAX / 2 ) {
    	/* �k�� */
        mag  = 1.0 / Power( 2, zoompos - ZOOM_MAX / 2 );
        skip = 1.0 / mag;
    } else if ( zoompos < ZOOM_MAX / 2 ) {
    	/* �g�� */
    	mag  = Power( 2, ZOOM_MAX / 2 - zoompos );
        skip = mag;
    } else {
    	/* ���{ */
    	mag  = 1;
        skip = 1;
    }

    /* �����`���Ԑݒ� */
    draw_w = sc_w;
    draw_h = sc_h;

    /* �摜�ǂݍ��݊J�n�n�_���� */
    img_start_x = img_cent_x - ( draw_w / 2.0 ) * ( 1.0 / mag );
    img_start_y = img_cent_y - ( draw_h / 2.0 ) * ( 1.0 / mag );

    /* �������p�ɕۑ� */
    img_start_x_line = img_start_x;
    img_start_y_line = img_start_y;

    /* �`��J�n�_�ݒ� */
    if ( img_start_x < 0 ) {
    	draw_x_b = fabsl( img_cent_x - ( draw_w / 2.0 ) * ( 1.0 / mag ) ) * mag;
        draw_x = draw_x_b;
        img_start_x = 0;
        line_add_x = 0;
    } else {
    	draw_x = 0;
        line_add_x = ( sc_w / 2 ) - ( img_cent_x - img_start_x ) * mag;
    }

    if ( img_start_y < 0 ) {
    	draw_y_b = fabsl( img_cent_y - ( draw_h / 2.0 ) * ( 1.0 / mag ) ) * mag;
        draw_y = draw_y_b;
        img_start_y = 0;
        line_add_y = 0;
    } else {
    	draw_y = 0;
        line_add_y = ( sc_h / 2 ) - ( img_cent_y - img_start_y ) * mag;
    }

    /* �L�����p�X�N���A */
    screen->Canvas->Brush->Color = clWhite;
    screen->Canvas->FillRect( Rect( 0, 0, sc_w, sc_h ) );

    /* �g�傩�k�����ŕ`����@�ύX */
    if ( mag >= 1 ) {
    	/* �g�� */

    	/* �ǂݍ��݌����� */
    	img_read_xc = ( draw_w - draw_x ) / skip;
    	img_read_yc = ( draw_h - draw_y ) / skip;

        /* �͈͕␳ */
        if ( img_start_x + img_read_xc > img_w ) {
            /* x�̓ǂݍ��ݔ͈͂��I�[�o�[ */
            img_read_xc = img_w - img_start_x;
        }

        if ( img_start_y + img_read_yc > img_h ) {
            /* y�̓ǂݍ��ݔ͈͂��I�[�o�[ */
            img_read_yc = img_h - img_start_y;
        }

        /* �摜�o�b�t�@�쐬 */
        buf = (unsigned char *)malloc( img_read_xc * 3 );

        /* �ǂݍ��ݕ`�惋�[�v */
        for ( i = 0; i < img_read_yc; i++ ) {
            /* �e�o���h�ǂݍ��� */
            for ( j = 0; j < list_band->Count; j++ ) {
                /* �o���h�w��A�f�[�^�ǂݍ��� */
                band = (struct REMOS_FRONT_BAND *)(list_band->Items[j]);

                /* ���[�h�ŕ��@���Ⴄ */
                if ( !band->canvas_mode ) {
                	remos_get_line_pixels( band->band, band->line_buf, img_start_y + i, img_start_x, img_read_xc );
                } else {
                	GetLineData( band, band->line_buf, img_start_y + i, img_start_x, img_read_xc );
                }

                /* �����W�K�p */
                remos_get_ranged_pixels( band->band, band->line_buf, img_read_xc );
            }

            /* ��ʂɏ������� */

            /* ��f�[�^�ǂݏo�����[�v */
            for ( k = 0; k < img_read_xc; k++ ) {
                /* �F�쐬 */
                for ( l = 0; l < list_band->Count; l++ ) {
                    /* �o���h�擾 */
                    band = (struct REMOS_FRONT_BAND *)(list_band->Items[l]);

                    /* �l�o�^ */
                    // var[l] = band->line_buf[k];
                    if ( band->canvas_mode ) {
                        // �L�����o�X���[�h
                    	var[l] = band->line_buf[k];
                    } else {
                    	// remos���[�h DEBUG : �G���f�B�A�������ߑł��ɂ��Ă���̂Ńo�O�邩��
                        // var[l] = remos_data_to_value( band->line_buf + band->band->bits / 8 * k, band->band->bits / 8, REMOS_ENDIAN_LITTLE );

                    	// DEBUG : �l�̎擾���@��ύX ( �}�C�i�X�ɑΉ������邽�� )
                        var[l] = remos_data_to_value_band( band->band, band->line_buf + band->band->bits / 8 * k );
                    }
                }

                // �����[�h���J���[�o�[���[�h��
                if ( drawModeExp ) {
                	// �����[�h
                    red   = GetUCharValue( ecalc_get_tree_value( tok_r, vars, 0 ) );
                    green = GetUCharValue( ecalc_get_tree_value( tok_g, vars, 0 ) );
                    blue  = GetUCharValue( ecalc_get_tree_value( tok_b, vars, 0 ) );
                } else {
                	// �J���[�o�[���[�h
                    cmap->evalExpression( vars, 0 );
                    cmap->makeColor();

                    red   = GetUCharValue( cmap->getR() );
                    green = GetUCharValue( cmap->getG() );
                    blue  = GetUCharValue( cmap->getB() );
                }

                buf[k * 3 + 0] = red;
                buf[k * 3 + 1] = green;
                buf[k * 3 + 2] = blue;
            }

            /* �s���[�v */
            for ( j = 0; j < skip; j++ ) {
                /* �s�|�C���^�擾 */
                rgb = (RGBTRIPLE *)screen->Picture->Bitmap->ScanLine[j + draw_y + i * skip];

            	for ( k = 0; k < img_read_xc; k++ ) {
                    for ( l = 0; l < skip; l++ ) {
                        rgb[l + draw_x + k * skip].rgbtBlue  = buf[k * 3 + 2];
                        rgb[l + draw_x + k * skip].rgbtRed   = buf[k * 3 + 0];
                        rgb[l + draw_x + k * skip].rgbtGreen = buf[k * 3 + 1];
                    }
                }
            }
        }

        /* �o�b�t�@�J�� */
        free( buf );
    } else {
    	/* �k�� */

    	/* �ǂݍ��݌����� */
    	img_read_xc = draw_w - draw_x;
    	img_read_yc = draw_h - draw_y;

        /* �͈͕␳ */
        if ( img_start_x + img_read_xc * skip > img_w ) {
            /* x�̓ǂݍ��ݔ͈͂��I�[�o�[ */
            img_read_xc = ( img_w - img_start_x ) / skip;
        }

        if ( img_start_y + img_read_yc * skip > img_h ) {
            /* y�̓ǂݍ��ݔ͈͂��I�[�o�[ */
            img_read_yc = ( img_h - img_start_y ) / skip;
        }

        /* �ǂݍ��ݕ`�惋�[�v */
        for ( i = 0; i < img_read_yc; i++ ) {
            /* �e�o���h�ǂݍ��� */
            for ( j = 0; j < list_band->Count; j++ ) {
                /* �o���h�w��A�f�[�^�ǂݍ��� */
                band = (struct REMOS_FRONT_BAND *)(list_band->Items[j]);

               /* ���[�h�ŕ��@���Ⴄ */
                if ( !band->canvas_mode ) {
                	remos_get_line_pixels( band->band, band->line_buf, img_start_y + i * skip, img_start_x, img_read_xc * skip );
                } else {
                	GetLineData( band, band->line_buf, img_start_y + i * skip, img_start_x, img_read_xc * skip );
                }

                /* �����W�K�p */
                // remos_get_ranged_pixels( band->band, band->line_buf, img_read_xc * skip );
            }

            /* ��ʂɏ������� */

            /* �s�|�C���^�擾 */
            rgb = (RGBTRIPLE *)screen->Picture->Bitmap->ScanLine[draw_y + i];

            /* ��f�[�^�ǂݏo�����[�v */
            for ( k = 0; k < img_read_xc; k++ ) {
                /* �F�쐬 */
                for ( l = 0; l < list_band->Count; l++ ) {
                	/* �o���h�擾 */
                    band = (struct REMOS_FRONT_BAND *)(list_band->Items[l]);

                    /* �l�o�^ ( �����W�̏����͂����ōs�� ) */
                    // var[l] = remos_get_ranged_pixel( band->band, band->line_buf[k * skip] );
                    /* �l�o�^ */
                    if ( band->canvas_mode ) {
                        // �L�����o�X���[�h
                    	var[l] = remos_get_ranged_pixel( band->band, band->line_buf[k * skip] );
                    } else {
                    	// remos���[�h DEBUG : �G���f�B�A�������ߑł��ɂ��Ă���̂Ńo�O�邩��
                        // var[l] = remos_get_ranged_pixel( band->band, remos_data_to_value( band->line_buf + band->band->bits / 8 * k * skip, band->band->bits / 8, REMOS_ENDIAN_LITTLE ) );
                    	// DEBUG : �l�̎擾���@��ύX ( �}�C�i�X�ɑΉ������邽�� )
                        var[l] = remos_get_ranged_pixel( band->band, remos_data_to_value_band( band->band, band->line_buf + band->band->bits / 8 * k * skip ) );
                    }
                }

                // �����[�h���J���[�o�[���[�h��
                if ( drawModeExp ) {
                	// �����[�h
                    red   = GetUCharValue( ecalc_get_tree_value( tok_r, vars, 0 ) );
                    green = GetUCharValue( ecalc_get_tree_value( tok_g, vars, 0 ) );
                    blue  = GetUCharValue( ecalc_get_tree_value( tok_b, vars, 0 ) );
                } else {
                	// �J���[�o�[���[�h
                    cmap->evalExpression( vars, 0 );
                    cmap->makeColor();

                    red   = GetUCharValue( cmap->getR() );
                    green = GetUCharValue( cmap->getG() );
                    blue  = GetUCharValue( cmap->getB() );
                }

                rgb[draw_x + k].rgbtBlue  = blue;
                rgb[draw_x + k].rgbtRed   = red;
                rgb[draw_x + k].rgbtGreen = green;
            }
        }
    }

    // �o�b�t�@�փR�s�[
    if ( back_screen != NULL ) {
    	back_screen->Canvas->CopyRect( Rect( 0, 0, screen->Width, screen->Height ), screen->Canvas, Rect( 0, 0, screen->Width, screen->Height ) );
	}

    b_drawing = false;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::DrawImg( void )
{
    /* �ėp�`��֐����g���ă��C���摜��`�� */

    /* mag���O���[�o���ϐ��Ȃ̂ŉ����N���邩�킩��Ȃ�����ꉞ�{���v�Z�������Ă��� */
    if ( zoom_pos > ZOOM_MAX / 2 ) {
    	/* �k�� */
        mag  = 1.0 / Power( 2, zoom_pos - ZOOM_MAX / 2 );
    } else if ( zoom_pos < ZOOM_MAX / 2 ) {
    	/* �g�� */
    	mag  = Power( 2, ZOOM_MAX / 2 - zoom_pos );
    } else {
    	/* ���{ */
    	mag  = 1;
    }

    /* �`�� */
    DrawImg( SatImage, img_back, b_draw_mode_exp, zoom_pos, ScrImgHor->Position, ScrImgVert->Position, color_map );
}
//---------------------------------------------------------------------------
unsigned char TSatViewMainForm::GetUCharValue( double value )
{
	/* unsigned char �Ɋۂߍ��� */
    if ( value < 0 ) {
    	return 0;
    } else if ( value > 255 ) {
    	return 255;
    } else {
    	return (unsigned char)value;
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::TBCloseClick(TObject *Sender)
{
	/* �J���Ă������̂�S�ĕ��� */
    int i;
    struct REMOS_FRONT_BAND *box;
    TList *del_list;
    void *del;

    del_list = new TList();

    if ( b_open ) {
        /* �o���h�{�b�N�X�J�� */
        for ( i = 0; i < list_band->Count; i++ ) {
        	box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

        	/* Canvas���[�h�Ȃ�폜�v���ɓo�^ */
            if ( box->canvas_mode == true ) {
            	del_list->Add( box->img_canvas );
            }

            FreeBandBox( box );
            free( list_band->Items[i] );
        }

        list_band->Clear();

        /* �t�@�C���J�� */
        for ( i = 0; i < list_file->Count; i++ ) {
            remos_close( (struct REMOS_FILE_CONTAINER *)list_file->Items[i] );
            free( list_file->Items[i] );
        }

        list_file->Clear();

        /* canvas���[�h��canvas��j�� */
        for ( i = 0; i < del_list->Count; i++ ) {
        	/* �ۑ� */
        	del = del_list->Items[i];

            /* NULL�Ȃ�Ȃɂ����Ȃ� */
            if ( del == NULL ) {
            	continue;
            }

            /* �j�� */
        	delete del_list->Items[i];

            /* �����Ɠ������̂��폜 */
        	while ( del_list->IndexOf( del ) != -1 ) {
            	del_list->Items[del_list->IndexOf( del )] = NULL;
            }
        }
    }

    /* �L�����o�X�N���A */
    SatImage->Canvas->Brush->Color = clWhite;
    SatImage->Canvas->FillRect( Rect( 0, 0, SatImage->Width, SatImage->Height ) );

    /* �t���O���� */
    b_open = false;

    clearSetting();

    /* ���X�g�j�� */
    delete del_list;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::SatImageMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	TPoint point;

    // DEBUG : �t�H�[�J�X��ݒ肵�Ă݂�
    PanelImage->SetFocus();

	/* �J�[�\���ړ������� */
	b_move = false;

    /* �Ђ炢�ĂȂ���΂Ȃɂ����Ȃ� */
    if ( !b_open ) {
    	return;
    }

	/* ���{�^�� */
    if ( Button == mbLeft ) {
        cp_x = X;
        cp_y = Y;

        cp_vert = ScrImgVert->Position;
        cp_hor  = ScrImgHor->Position;

        /* �o�b�t�@������ */
    	img_back->Width  = SatImage->Width;
   		img_back->Height = SatImage->Height;

    	img_back->PixelFormat = pf24bit;
    	img_back->HandleType = bmDIB;

        /* �o�b�t�@�ɃR�s�[ */
        img_back->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), SatImage->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

    	b_click = true;
    } else {
    	int vert;
    	int hor;
    	int i;
    	int cent_c_x;
    	int cent_c_y;
        AnsiString str;
        unsigned int val;
    	struct REMOS_FRONT_BAND *box;
        char band[2];
        TPixForm *pix_form;

        // �N���b�N����
    	b_click = false;

        // ���W�l�t�H�[���쐬
        pix_form = new TPixForm( this );
        pix_form->do_delete = true;

        /* �J�[�\���ʒu�Z�� */
        vert = ScrImgVert->Position;
        hor  = ScrImgHor->Position;

        /* �g���C�A���h�G���[�ł����Ȃ��� */
        cent_c_x = hor + ( X + line_add_x - SatImage->Width / 2 ) / mag;
        cent_c_y = vert + ( Y + line_add_y - SatImage->Height / 2 ) / mag;

        if ( cent_c_x < 0 ) {
            cent_c_x = 0;
        }

        if ( cent_c_y < 0 ) {
            cent_c_y = 0;
        }

        if ( cent_c_y >= img_h ) {
            cent_c_y = img_h - 1;
        }

        if ( cent_c_x >= img_w ) {
            cent_c_x = img_w - 1;
        }

        str = "";
        str = str + "X, Y : " + IntToStr( cent_c_x ) + ", " + IntToStr( cent_c_y ) + "\n";

        // �o�x�ܓx�\��
        if ( b_config ) {
            str = str + "�o�x, �ܓx : " + GetLon( cent_c_x, cent_c_y ) + ", " + GetLat( cent_c_x, cent_c_y ) + "\n\n";
        }

        /* �e�o���h���f�[�^�擾 */
        for ( i = 0; i < list_band->Count; i++ ) {
            box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

            /* ���[�h�Ŏ擾���@���Ⴄ */
            if ( box->canvas_mode ) {
                val = GetPixel( box, img_w * cent_c_y + cent_c_x );
            } else {
                val = remos_get_pixel( box->band, img_w * cent_c_y + cent_c_x );
            }

            band[0] = tolower( (char)( box->index + 0x41 ) );
            band[1] = '\0';

            str = str + band;
            str = str + " : " + IntToStr( val ) + "\n";
        }

        pix_form->ValueLabel->Caption = str;

        if ( pix_form->ValueLabel->Height + 30 > pix_form->Height ) {
            pix_form->Height = pix_form->ValueLabel->Height + 30;
        }

        /* �J�[�\���ʒu�Ƀs�N�Z�������ړ� */
        GetCursorPos( &point );

        pix_form->Left = point.x + 10;
        pix_form->Top  = point.y + 10;

        pix_form->Show();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::SatImageMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	int lon;
    int lat;

    if ( b_click ) {
    	// �������胂�[�h�Ȃ�Ȃɂ����Ȃ�
        if ( b_mode_length ) {
        	img_back->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), SatImage->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

        	b_click = false;

        	return;
        }

        // �J���[�o�[�X�^���v���[�h�Ȃ�`�悵�ăo�b�t�@�փR�s�[
        if ( b_mode_stamp_cb ) {
        	b_click = false;

            // �o�b�t�@�R�s�[
        	//SatImage->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), img_back->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

            // �J���[�o�[�]��
            //SatImage->Canvas->CopyRect( Rect( X + 10, Y + 10, X + 10 + ColorBarForm->ColorBarImage->Width, Y + 10 + ColorBarForm->ColorBarImage->Height ), ColorBarForm->ColorBarImage->Canvas, ColorBarForm->ColorBarImage->Canvas->ClipRect );

            // �o�b�t�@�֓]��
        	//img_back->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), SatImage->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

            return;
        }

		/* �ȈՕ`�悵�Ă݂� */
		SatImage->Canvas->Brush->Color = clWhite;
    	SatImage->Canvas->FillRect( Rect( 0, 0, SatImage->Width, SatImage->Height ) );
    	SatImage->Canvas->CopyRect( Rect( - 1 * ( cp_x - X ), - 1 * ( cp_y - Y ), SatImage->Width - ( ( cp_x - X ) ), SatImage->Height - ( ( cp_y - Y ) ) ), img_back->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

    	b_click = false;

        /* �V���N�����K�v�Ȃ�V���N������ ( Sync�Ńh���[��������̂Ńh���[���Ȃ� ) */
        if ( b_sync ) {
            if ( b_config ) {
            	/* �o�x�ܓx�\ */
                lon = 10000 * GetLon( ScrImgHor->Position, ScrImgVert->Position ).ToDouble();
                lat = 10000 * GetLat( ScrImgHor->Position, ScrImgVert->Position ).ToDouble();

                PostMessage( HWND_BROADCAST, umsg_lat, lat, 0 );
                PostMessage( HWND_BROADCAST, umsg_lon, lon, 0 );
            } else {
            	/* XY���[�h */
        		PostMessage( HWND_BROADCAST, umsg_x, ScrImgHor->Position, 0 );
            	PostMessage( HWND_BROADCAST, umsg_y, ScrImgVert->Position, 0 );
            }

            PostMessage( HWND_BROADCAST, umsg_draw, ScrImgVert->Position, 0 );
        } else {
        	DrawImg();
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::BandCloseBtnClick(TObject *Sender)
{
	/* �o���h����� */
    /* �{�^�������͍폜�����A�v���O�����I�����ɍ폜���Ă��炤 : DEBUG */
    struct REMOS_FRONT_BAND *box;
    struct REMOS_FILE_CONTAINER *fc;
    int i;
    bool b_canvas_mode;
    Graphics::TBitmap *bmp;
    AnsiString fln;
    AnsiString fc_fln;

    /* BNAD���� */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    fln = *box->fln;
    bmp = box->img_canvas;

    /* ���X�g�폜 */
    list_band->Delete( box->index );

    /* �o���h�폜 */
    FreeBandBox( box );
    free( box );

    /* �{�b�N�X�X�V */
    for ( i = 0; i < list_band->Count; i++ ) {
    	UpdateBandBox( (struct REMOS_FRONT_BAND *)list_band->Items[i] );
    }

    /* �o���h���ɓ���t�@�C���������݂��邩�`�F�b�N */
    for ( i = 0; i < list_band->Count; i++ ) {
        box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

        /* �����t�@�C��������Ζ��Ȃ� */
    	if ( fln == *box->fln ) {
        	return;
        }
    }

    /* ����̂��̂��Ȃ������̂ŕ��� */
    for ( i = 0; i < list_file->Count; i++ ) {
    	fc = (struct REMOS_FILE_CONTAINER *)list_file->Items[i];

        fc_fln = fc->file_name;

    	if ( fc_fln == fln ) {
    		remos_close( fc );
        	free( fc );
        }

		list_file->Delete( i );

        /* DEBUG : Canvas���[�h��������Bitmap�j�� */
        delete bmp;
    }

    /* �S������ */
    if ( list_file->Count == 0 ) {
    	// DEBUG : �{�^���������Ȃ���΂Ȃ�Ȃ��֌W����A�폜�R�[�h�������ɃR�s�[ ( �֐��ɂ��ׂ� )

        /* �L�����o�X�N���A */
        SatImage->Canvas->Brush->Color = clWhite;
        SatImage->Canvas->FillRect( Rect( 0, 0, SatImage->Width, SatImage->Height ) );

        /* �t���O���� */
        b_open = false;

       	clearSetting();
    }
}
//---------------------------------------------------------------------------
void TSatViewMainForm::UpdateBandBox( struct REMOS_FRONT_BAND *box )
{
	/* �{�b�N�X�����l�X�V */
    char num[2];

    /* �����̃C���f�b�N�X�Ď擾 */
    box->index = list_band->IndexOf( box );

    /* �o���h�ԍ��쐬 */
    num[0] = 0x61 + box->index;
    num[1] = '\0';

    /* ���O�����Ȃ��� */
    box->label_name->Caption = num;
    box->label_name->Caption = box->label_name->Caption + " : " + *box->fln;
    box->label_name->Caption = box->label_name->Caption + " - " + IntToStr( box->band->band_num );

    /* Canvas���[�h���Aremos���[�h�ł��J���[�Ȃ�F������ */
    if ( box->canvas_mode || box->band->color == REMOS_BAND_COLOR_RGB ) {
    	switch ( box->band->band_num ) {
        	case 0:
            	box->label_name->Caption = box->label_name->Caption + " - R";
                box->label_name->Font->Color = clRed;
                break;

        	case 1:
            	box->label_name->Caption = box->label_name->Caption + " - G";
                box->label_name->Font->Color = clGreen;
                break;

        	case 2:
            	box->label_name->Caption = box->label_name->Caption + " - B";
                box->label_name->Font->Color = clBlue;
                break;
        }
    }

    /* Hint�ݒ� */
    box->label_name->Hint = box->label_name->Caption;

    /* �����W�ǂݍ��� */
    box->edit_bottom->Text = FloatToStr( box->band->range_bottom );
    box->edit_top->Text    = FloatToStr( box->band->range_top );

    /* �q�X�g�O�����`�� */
    DrawHist( box );
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::HistMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	/* �q�X�g�O�����N���b�N�C�x���g */
    struct REMOS_FRONT_BAND *box;

    /* BNAD���� */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    box->hist_click  = true;
    box->hist_button = Button;

    HistMouseMove( Sender, Shift, X, Y );
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::HistMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	/* �q�X�g�O�����N���b�N�C�x���g */
    struct REMOS_FRONT_BAND *box;

    /* BNAD���� */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    box->hist_click = false;

    /* �X�V */
    DrawHist( box );

    /* �`�悳���� */
    DrawImg();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::HistMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
	/* �q�X�g�O�����|�C���^�ړ��C�x���g */
    int w, h;
    int y;
    int i;
    float val;
    struct REMOS_FRONT_BAND *box;

    /* BNAD���� */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    if ( X < 0 ) {
    	X = 0;
    } else if ( X > 255 ) {
    	X = 255;
    }

    /* �N���b�N����ĂȂ�������X�L�b�v */
    if ( !box->hist_click ) {
    	return;
    }

    /* X�̈ʒu����l�쐬 */
    val = X / 255.0 * ( box->band->range_max - box->band->range_min ) + box->band->range_min;

    /* �����W�ݒ� */
    if ( box->hist_button == mbLeft ) {
    	if ( val < box->band->range_top ) {
            box->band->range_bottom = val;
        } else {
            box->band->range_bottom = box->band->range_top;

            X = ( box->band->range_top - box->band->range_min ) / ( box->band->range_max - box->band->range_min ) * 255;
        }
    } else if ( box->hist_button == mbRight ) {
    	if ( val > box->band->range_bottom ) {
        	box->band->range_top = val;
        } else {
            box->band->range_top = box->band->range_bottom;

            X = ( box->band->range_top - box->band->range_min ) / ( box->band->range_max - box->band->range_min ) * 255;
        }
    }

    /* �l���G�f�B�b�g�ɓǂݍ��� */
    box->edit_bottom->Text = FloatToStr( box->band->range_bottom );
    box->edit_top->Text    = FloatToStr( box->band->range_top );

    /* ������`�� */
    box->img_hist->Canvas->Font->Color = clBlack;
    box->img_hist->Canvas->TextOutA( 2 + X, 2, IntToStr( box->band->hist[X] ) );
}
//---------------------------------------------------------------------------
void TSatViewMainForm::DrawHist( struct REMOS_FRONT_BAND *box )
{
	/* �q�X�g�O�����`�� */
    int h;
    int y;
    int i;
    int pos_l;
    int pos_r;
    double max;

    h = box->img_hist->Height;

    /* 0���Z�h�~ */
    if ( box->band->hist_max == 0 ) {
    	box->band->hist_max = 1;
    }

    if ( box->band->hist_max_reduce_topbottom == 0 ) {
    	box->band->hist_max_reduce_topbottom = 1;
    }

    /* �`��ʒu�v�Z */
    pos_l = ( box->band->range_bottom - box->band->range_min ) / ( box->band->range_max - box->band->range_min ) * 255;
    pos_r = ( box->band->range_top - box->band->range_min ) / ( box->band->range_max - box->band->range_min ) * 255;

    /* �q�X�g�O�����`�� */
    for ( i = 0; i <= pos_l; i++ ) {
        if ( HistReduceMaxCheckBox->Checked ) {
            max = box->band->hist[i] / (double)box->band->hist_max_reduce_topbottom;
        } else {
            max = box->band->hist[i] / (double)box->band->hist_max;
        }

    	y = h * ( max );

        box->img_hist->Canvas->Pen->Color = (TColor)RGB( 50, 50, 130 );
        box->img_hist->Canvas->MoveTo( i, h );
        box->img_hist->Canvas->LineTo( i, h - y );

        box->img_hist->Canvas->Pen->Color = (TColor)RGB( 80, 80, 80 );
        box->img_hist->Canvas->MoveTo( i, h - y );
        box->img_hist->Canvas->LineTo( i, 0 );
    }

    for ( i = pos_l; i <= pos_r; i++ ) {
        if ( HistReduceMaxCheckBox->Checked ) {
            max = box->band->hist[i] / (double)box->band->hist_max_reduce_topbottom;
        } else {
            max = box->band->hist[i] / (double)box->band->hist_max;
        }

    	y = h * ( max );

        box->img_hist->Canvas->Pen->Color = (TColor)RGB( 30, 30, 255 );
        box->img_hist->Canvas->MoveTo( i, h );
        box->img_hist->Canvas->LineTo( i, h - y );

        box->img_hist->Canvas->Pen->Color = clWhite;
        box->img_hist->Canvas->MoveTo( i, h - y );
        box->img_hist->Canvas->LineTo( i, 0 );
    }

    for ( i = pos_r; i < 256; i++ ) {
        if ( HistReduceMaxCheckBox->Checked ) {
            max = box->band->hist[i] / (double)box->band->hist_max_reduce_topbottom;
        } else {
            max = box->band->hist[i] / (double)box->band->hist_max;
        }

    	y = h * ( max );

        box->img_hist->Canvas->Pen->Color = (TColor)RGB( 50, 50, 130 );
        box->img_hist->Canvas->MoveTo( i, h );
        box->img_hist->Canvas->LineTo( i, h - y );

        box->img_hist->Canvas->Pen->Color = (TColor)RGB( 80, 80, 80 );
        box->img_hist->Canvas->MoveTo( i, h - y );
        box->img_hist->Canvas->LineTo( i, 0 );
    }

    /* �q�X�g�O����������ĂȂ���Όx�� */
    if ( !box->hist_maked ) {
        box->img_hist->Canvas->Font->Size   = 9;
        box->img_hist->Canvas->Font->Color  = clRed;
        box->img_hist->Canvas->Brush->Color = clWhite;
        box->img_hist->Canvas->TextOutA( 1, 1, "�q�X�g�O�������쐬����Ă��܂���" );
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::BandEditUpChange(TObject *Sender)
{
	/* �q�X�g�O�����ύX */
    struct REMOS_FRONT_BAND *box;
    float val;

    /* BNAD���� */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    /* ���݂̒l */
    try {
    	val = box->edit_top->Text.ToDouble();
    } catch( ... ) {
    	// �����łȂ���Ή������Ȃ�
        return;
    }

    /* �I�[�o�[���Ă���΍ŏ��Ɠ��������� */
    if ( box->band->range_bottom > val ) {
    	val = box->band->range_bottom;

        // �G�f�B�b�g�͂�����Ȃ�
        // box->edit_top->Text = FloatToStr( val );
    }

    /* �l�X�V */
    box->band->range_top = val;

    /* ��ʍX�V */
    DrawHist( box );

	PleaseClick();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::BandEditDownChange(TObject *Sender)
{
	/* �q�X�g�O�����ύX */
    struct REMOS_FRONT_BAND *box;
    float val;

    /* BNAD���� */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    /* ���݂̒l */
    try {
    	val = box->edit_bottom->Text.ToDouble();
    } catch( ... ) {
    	// �����łȂ���Ή������Ȃ�
        return;
    }

    /* �I�[�o�[���Ă���΍ő�Ɠ��������� */
    if ( box->band->range_top < val ) {
    	val = box->band->range_top;

        // �G�f�B�b�g�͂�����Ȃ�
    	// box->edit_bottom->Text  = FloatToStr( val );
    }

    /* �l�X�V */
    box->band->range_bottom = val;

    /* ��ʍX�V */
    DrawHist( box );

	PleaseClick();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::PleaseClick( void )
{
    /* �X�V���b�Z�[�W */
    AnsiString str;

    str = "�ĕ`�悵�Ă�������";
    SatImage->Canvas->Font->Size   = 9;
    SatImage->Canvas->Font->Color  = clRed;
    SatImage->Canvas->Brush->Color = clWhite;
    SatImage->Canvas->TextOutA( 10, 10, str );
}
//---------------------------------------------------------------------------
AnsiString TSatViewMainForm::GetLon( int x, int y )
{
	/* �o�x�𕶎��� */
    AnsiString ret;
    double lon;
    double lat;
    double utm_n;
    double utm_e;

    // UTM���[�h�ł����UTM�Ōv�Z����
    if ( b_config_utm ) {
    	lon = config_utm.getLatLon( Point2D( x, y ) ).x;
    } else {
    	// �����������Ȃ炾�����p�̐ݒ���g��
        if ( b_config_alos ) {
        	lon = conf_alos.getLon( x, y );
        } else {
        	// LatLon���[�h
        	lon = conf_lt_lon + ( ( conf_rt_lon - conf_lt_lon ) / ( img_w - 1 ) ) * x + ( ( conf_lb_lon - conf_lt_lon ) / ( img_h - 1 ) ) * y + ( ( conf_rb_lon - conf_lb_lon - conf_rt_lon + conf_lt_lon ) / ( ( img_w - 1 ) * ( img_h - 1 ) ) ) * ( x * y );
        }
    }

    ret = ret.sprintf( "%.4lf", lon );

    return ret;
}
//---------------------------------------------------------------------------
AnsiString TSatViewMainForm::GetLat( int x, int y )
{
	/* �ܓx�𕶎��� */
    AnsiString ret;
    double lat;
    double lon;
    double utm_n;
    double utm_e;

    // UTM���[�h�ł����UTM�Ōv�Z����
    if ( b_config_utm ) {
    	lat = config_utm.getLatLon( Point2D( x, y ) ).y;
    } else {
    	// �����������Ȃ炾�����p�̐ݒ���g��
        if ( b_config_alos ) {
        	lat = conf_alos.getLat( x, y );
        } else {
        	// LatLon���[�h
        	lat = conf_lt_lat + ( ( conf_rt_lat - conf_lt_lat ) / ( img_w - 1 ) ) * x + ( ( conf_lb_lat - conf_lt_lat ) / ( img_h - 1 ) ) * y + ( ( conf_rb_lat - conf_lb_lat - conf_rt_lat + conf_lt_lat ) / ( ( img_w - 1 ) * ( img_h - 1 ) ) ) * ( x * y );
        }
    }

    ret = ret.sprintf( "%.4lf", lat );

    return ret;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::SatImageMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
	int r;
    int g;
    int b;
    int sc_h;
    int sc_w;
    int mm_s_x;
    int mm_s_y;
    int vert;
    int hor;
    int i;
    int cent_c_x;
    int cent_c_y;
    int line_x;
    int line_y;
    float val;
    struct REMOS_FRONT_BAND *box;
    TColor cl;
    AnsiString str;
    char band[2];
    double lon;
    double lat;
    double var[ECALC_VAR_COUNT];
    double *vars[ECALC_VAR_COUNT];


    /* �Ђ炢�ĂȂ���΂Ȃɂ����Ȃ� */
    if ( !b_open ) {
    	return;
    }

    // ���p�|�C���^�[�o�^
    for ( i = 0; i < ECALC_VAR_COUNT; i++ ) {
    	vars[i] = &var[i];
    }

    /* �J�[�\���ړ� */
    b_move = true;

    sc_h = SatImage->Height;
    sc_w = SatImage->Width;

	/* �N���b�N����Ă���H */
	if ( b_click ) {
    	// �������胂�[�h��
        if ( b_mode_length ) {
        	// �������胂�[�h

        	// �o�b�t�@�`��
            SatImage->Canvas->CopyRect( Rect( 0, 0, img_back->Width, img_back->Height ), img_back->Canvas, Rect( 0, 0, img_back->Width, img_back->Height ) );

            // ���`��
            SatImage->Canvas->Pen->Color = clRed;
            SatImage->Canvas->Pen->Width = 5;
            SatImage->Canvas->MoveTo( cp_x, cp_y );
            SatImage->Canvas->LineTo( X, Y );

            // �{���v�Z
            double real_mag;

            if ( zoom_pos > ZOOM_MAX / 2 ) {
                /* �k�� */
                real_mag  = 1.0 / Power( 2, zoom_pos - ZOOM_MAX / 2 );
            } else if ( zoom_pos < ZOOM_MAX / 2 ) {
                /* �g�� */
                real_mag = Power( 2, ZOOM_MAX / 2 - zoom_pos );
            } else {
                /* ���{ */
                real_mag = 1;
            }

            // �����`��
            double length = sqrt( ( X - cp_x ) * ( X - cp_x ) * conf_resolution_x * conf_resolution_x / ( real_mag * real_mag ) + ( Y - cp_y ) * ( Y - cp_y ) * conf_resolution_y * conf_resolution_y / ( real_mag * real_mag ) );

            SatImage->Canvas->Font->Color = clRed;
            SatImage->Canvas->TextOutA( X + 5, Y - 5 - SatImage->Canvas->TextHeight( "height" ), IntToStr( (int)length ) + "[m]" );
        } else if ( b_mode_stamp_cb ) {
        	// �X�^���v���[�h�Ȃ�Ȃɂ����Ȃ�
        } else {
        	// �ړ����[�h
            SatImage->Canvas->Pen->Width = 1;

            /* �摜�ړ� */
            ScrImgHor->Position  = cp_hor  + ( cp_x - X ) / mag;
            ScrImgVert->Position = cp_vert + ( cp_y - Y ) / mag;

            vert = ScrImgVert->Position;
            hor  = ScrImgHor->Position;

            /* �ȈՕ`�� */
            SatImage->Canvas->Brush->Color = clWhite;
            SatImage->Canvas->FillRect( Rect( 0, 0, SatImage->Width, SatImage->Height ) );
            SatImage->Canvas->CopyRect( Rect( - 1 * ( cp_x - X ), - 1 * ( cp_y - Y ), SatImage->Width - ( ( cp_x - X ) ), SatImage->Height - ( ( cp_y - Y ) ) ), img_back->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

            /* �~�j�}�b�v�`�� */
            mm_s_x = 100 * ( hor / (float)img_w );
            mm_s_y = ( ( img_h / (float)img_w ) * 100 ) * ( vert / (float)img_h );

            SatImage->Canvas->Pen->Color   = clRed;
            SatImage->Canvas->Brush->Color = (TColor)RGB( 255, 200, 200 );
            SatImage->Canvas->Rectangle( 0, 0, 100, ( img_h / (float)img_w ) * 100 );
            SatImage->Canvas->Brush->Color = clRed;

            SatImage->Canvas->MoveTo( mm_s_x, 0 );
            SatImage->Canvas->LineTo( mm_s_x, ( img_h / (float)img_w ) * 100 );
            SatImage->Canvas->MoveTo( 0, mm_s_y );
            SatImage->Canvas->LineTo( 100, mm_s_y );

            /* �\���� �␳�� : line_add_ : DEBUG */
            SatImage->Canvas->MoveTo( sc_w / 2 - line_add_x, 0 );
            SatImage->Canvas->LineTo( sc_w / 2 - line_add_x, sc_h );
            SatImage->Canvas->MoveTo( 0, sc_h / 2 - line_add_y );
            SatImage->Canvas->LineTo( sc_w, sc_h / 2 - line_add_y );

            str = "X, Y : " + IntToStr( hor ) + ", " + IntToStr( vert );

            SatImage->Canvas->Font->Size = 9;
            SatImage->Canvas->Font->Color = clRed;
            SatImage->Canvas->Brush->Color = (TColor)RGB( 255, 200, 200 );
            SatImage->Canvas->TextOutA( sc_w / 2 - line_add_x + 5, sc_h / 2 - line_add_y + 5, str );

            /* �o�x�ܓx���v�Z�o����Ε\�� */
            if ( b_config ) {
                str = "�o�x, �ܓx :  " + GetLon( hor, vert ) + ", " + GetLat( hor, vert );
                SatImage->Canvas->TextOutA( sc_w / 2 - line_add_x + 5, sc_h / 2 - line_add_y + 5 + SatImage->Canvas->TextHeight( str ), str );
            }

            for ( i = 0; i < list_band->Count; i++ ) {
                box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

                /* ���[�h�Ŏ擾���@���Ⴄ */
                if ( box->canvas_mode ) {
                    val = GetPixel( box, img_w * vert + hor  );
                } else {
                    val = remos_get_pixel( box->band, img_w * vert + hor );
                }

                band[0] = tolower( (char)( box->index + 0x41 ) );
                band[1] = '\0';

                str = band;
                str = str + " : " + IntToStr( (int)val );

                SatImage->Canvas->Font->Size = 9;
                SatImage->Canvas->Font->Color = clRed;
                SatImage->Canvas->Brush->Color = (TColor)RGB( 255, 200, 200 );
                SatImage->Canvas->TextOutA( sc_w / 2 - line_add_x + 5, sc_h / 2 + 5 + ( i + 2 ) * SatImage->Canvas->TextHeight( str ) - line_add_y, str );
            }
        }

    } else {
    	/* ���W�l�X�V */
        cl = SatImage->Canvas->Pixels[X][Y];

        r = ( 0x0000FF & cl );
        g = ( 0x00FF00 & cl ) >> 8;
        b = ( 0xFF0000 & cl ) >> 16;

        str = "";
        str = str + "( R, G, B ) = " + "( " + IntToStr( r ) + ", "
    	                    + IntToStr( g ) + ", " + IntToStr( b ) + " )";

    	StatusBar->Panels->Items[2]->Text = str;

        /* �J�[�\���ʒu�Z�� */
        vert = ScrImgVert->Position;
        hor  = ScrImgHor->Position;

        /* �g���C�A���h�G���[�ł����Ȃ��� */
        cent_c_x = hor + ( X + line_add_x - SatImage->Width / 2 ) / mag;
        cent_c_y = vert + ( Y + line_add_y - SatImage->Height / 2 ) / mag;

        if ( cent_c_x < 0 ) {
            cent_c_x = 0;
        }

        if ( cent_c_y < 0 ) {
            cent_c_y = 0;
        }

        if ( cent_c_y >= img_h ) {
            cent_c_y = img_h - 1;
        }

        if ( cent_c_x >= img_w ) {
            cent_c_x = img_w - 1;
        }

        // �o�x�ܓx�\��
        if ( b_config ) {
        	StatusBar->Panels->Items[3]->Text = "X, Y : " + IntToStr( cent_c_x ) + ", " + IntToStr( cent_c_y ) + " " + "�o�x, �ܓx : " + GetLon( cent_c_x, cent_c_y ) + ", " + GetLat( cent_c_x, cent_c_y );
        }

        /* �J�[�\���ʒu�t�H�[���ɕ\�� */
        if ( PixForm->Visible ) {
            // cent_c_x = img_start_x_line + X / mag;
            // cent_c_y = img_start_y_line + Y / mag;

        	str = "";
            str = str + "X, Y : " + IntToStr( cent_c_x ) + ", " + IntToStr( cent_c_y ) + "\n";

            // �o�x�ܓx�\��
            if ( b_config ) {
            	str = str + "�o�x, �ܓx : " + GetLon( cent_c_x, cent_c_y ) + ", " + GetLat( cent_c_x, cent_c_y ) + "\n\n";
			}

			/* �e�o���h���f�[�^�擾 */
            for ( i = 0; i < list_band->Count; i++ ) {
                box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

            	/* ���[�h�Ŏ擾���@���Ⴄ */
            	if ( box->canvas_mode ) {
              		val = GetPixel( box, img_w * cent_c_y + cent_c_x );
            	} else {
            		val = remos_get_pixel( box->band, img_w * cent_c_y + cent_c_x );
            	}

                band[0] = tolower( (char)( box->index + 0x41 ) );
                band[1] = '\0';

                str = str + band;
                str = str + " : " + IntToStr( (int)val ) + "\n";
        	}

        	PixForm->ValueLabel->Caption = str;

            if ( PixForm->ValueLabel->Height + 30 >PixForm->Height ) {
            	PixForm->Height = PixForm->ValueLabel->Height + 30;
            }
        }

        // �J���[�o�[�\��
        if ( ColorBarForm->Visible || b_mode_stamp_cb ) {
        	/* �J�[�\���ʒu�Z�� */
        	vert = ScrImgVert->Position;
    		hor  = ScrImgHor->Position;

            /* �g���C�A���h�G���[�ł����Ȃ��� */
            cent_c_x = hor + ( X + line_add_x - SatImage->Width / 2 ) / mag;
            cent_c_y = vert + ( Y + line_add_y - SatImage->Height / 2 ) / mag;

            if ( cent_c_x < 0 ) {
            	cent_c_x = 0;
            }

            if ( cent_c_y < 0 ) {
            	cent_c_y = 0;
            }

            // ���쐬
            ecalc_free_token( tok_color_bar );
    		tok_color_bar = ecalc_make_token( color_bar_exp.c_str() );
			tok_color_bar = ecalc_make_tree( tok_color_bar );

			/* �e�o���h���f�[�^�擾 */
            for ( i = 0; i < list_band->Count; i++ ) {
                box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

            	/* ���[�h�Ŏ擾���@���Ⴄ */
            	if ( box->canvas_mode ) {
              		val = GetPixel( box, img_w * cent_c_y + cent_c_x );
            	} else {
            		val = remos_get_pixel( box->band, img_w * cent_c_y + cent_c_x );
            	}

            	// ���ϐ��ɓo�^
                var[i] = val;
        	}

            // �J���[�}�b�v���L���ł���΃J���[�}�b�v�̌��ݒl�v�Z
            if ( color_map != NULL ) {
            	color_map->evalExpression( vars, 0 );
            }

            // �J���[�o�[�̃��[�h�ɂ���Ēl�̑����������
            if ( ColorBarForm->getExpMode() ) {
            	// ������擾�����l��ݒ肵�ĕ`��
                ColorBarForm->setNowValue( ecalc_get_tree_value( tok_color_bar, vars, 0 ) );
                ColorBarForm->draw();
            } else {
            	// �J���[�}�b�v����擾�����l��ݒ肵�ĕ`��
            	ColorBarForm->setNowValue( color_map->getValue() );
            	ColorBarForm->draw();
            }
        }

        // �J���[�o�[�X�^���v���[�h
        if ( b_mode_stamp_cb ) {
        	// �o�b�t�@�R�s�[
        	SatImage->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), img_back->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

            // �\�Ȃ���W������
        	str = "";
            // str = str + "X, Y : " + IntToStr( cent_c_x ) + ", " + IntToStr( cent_c_y ) + "\n";

            if ( b_config ) {
            	str = str + "�o�x, �ܓx : " + GetLon( cent_c_x, cent_c_y ) + ", " + GetLat( cent_c_x, cent_c_y ) + "\n\n";
			}

            int size_buf = SatImage->Canvas->Font->Size;
            SatImage->Canvas->Font->Size = 10;
            SatImage->Canvas->TextOutA( X + 10, Y + 10 + ColorBarForm->ColorBarImage->Height, str );
            SatImage->Canvas->Font->Size = size_buf;

            // �\��������
            SatImage->Canvas->Pen->Color = clRed;
            SatImage->Canvas->Pen->Width = 2;
            SatImage->Canvas->MoveTo( X - 10, Y );
            SatImage->Canvas->LineTo( X - 5,  Y );
            SatImage->Canvas->MoveTo( X + 10, Y );
            SatImage->Canvas->LineTo( X + 5,  Y );
            SatImage->Canvas->MoveTo( X, Y - 10 );
            SatImage->Canvas->LineTo( X, Y -  5 );
            SatImage->Canvas->MoveTo( X, Y + 10 );
            SatImage->Canvas->LineTo( X, Y +  5 );

            // �J���[�o�[�]��
            SatImage->Canvas->CopyRect( Rect( X + 10, Y + 10, X + 10 + ColorBarForm->ColorBarImage->Width, Y + 10 + ColorBarForm->ColorBarImage->Height ), ColorBarForm->ColorBarImage->Canvas, ColorBarForm->ColorBarImage->Canvas->ClipRect );

        }

		/* �o�b�t�@���R�s�[ */
		// SatImage->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), img_back->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ImageMouseWheelEvent( TObject *Sender, TShiftState Shift,
	int WheelDelta, const TPoint &MousePos, bool &Handled )
{
	// Image�p�̃z�C�[���C�x���g

    // WheelDalta�̕����Ō����𔻒f
    if ( WheelDelta >= 1 ) {
    	// �Y�[���A�b�v
    	zoom_pos--;

    	if ( zoom_pos < 0 ) {
    		zoom_pos = 0;
    	}

    	DrawZoomBox( 0, 0 );
		DrawImg();
    } else if ( WheelDelta <= -1 ) {
        // �Y�[���_�E��
        zoom_pos++;

    	if ( zoom_pos > ZOOM_MAX ) {
    		zoom_pos = ZOOM_MAX;
    	}

    	DrawZoomBox( 0, 0 );
		DrawImg();
    }

    // ��������
    Handled = true;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ScrImgHorScroll(TObject *Sender,
      TScrollCode ScrollCode, int &ScrollPos)
{
	int r;
    int g;
    int b;
    int sc_h;
    int sc_w;
    int mm_s_x;
    int mm_s_y;
    int mm_e_x;
    int mm_e_y;
    int vert;
    int hor;
    AnsiString str;

    /* �Ђ炢�ĂȂ���΂Ȃɂ����Ȃ� */
    if ( !b_open ) {
    	return;
    }

	if ( ScrollCode == scEndScroll || ScrCheckBox->Checked == true ) {
        /* �V���N�����K�v�Ȃ�V���N������ ( Sync�Ńh���[��������̂Ńh���[���Ȃ� ) */
        if ( b_sync ) {
        	PostMessage( HWND_BROADCAST, umsg_x, ScrImgHor->Position, 0 );
            PostMessage( HWND_BROADCAST, umsg_draw, ScrImgVert->Position, 0 );
        } else {
        	DrawImg();
        }
    } else if ( ScrollCode == scTrack || ScrollCode == scLineUp || ScrollCode == scLineDown || ScrollCode == scPageDown || ScrollCode == scPageUp ) {
    	/* �~�j�}�b�v�`�� */
        sc_h = SatImage->Height;
        sc_w = SatImage->Width;
        vert = ScrImgVert->Position;
        hor  = ScrollPos;

        mm_s_x = 100 * ( hor / (float)img_w );
        mm_s_y = ( ( img_h / (float)img_w ) * 100 ) * ( vert / (float)img_h );
        mm_e_x = mm_s_x + 1;
        mm_e_y = mm_s_y + 1;

        SatImage->Canvas->Pen->Width   = 1;
        SatImage->Canvas->Pen->Color   = clRed;
        SatImage->Canvas->Brush->Color = (TColor)RGB( 255, 200, 200 );
        SatImage->Canvas->Rectangle( 0, 0, 100, ( img_h / (float)img_w ) * 100 );
        SatImage->Canvas->Brush->Color = clRed;
        SatImage->Canvas->Rectangle( Rect( mm_s_x, mm_s_y, mm_e_x, mm_e_y ) );
        SatImage->Canvas->MoveTo( mm_s_x, 0 );
        SatImage->Canvas->LineTo( mm_s_x, ( img_h / (float)img_w ) * 100 );
        SatImage->Canvas->MoveTo( 0, mm_s_y );
        SatImage->Canvas->LineTo( 100, mm_s_y );

        SatImage->Canvas->MoveTo( sc_w / 2, 0 );
        SatImage->Canvas->LineTo( sc_w / 2, sc_h );
        SatImage->Canvas->MoveTo( 0, sc_h / 2 );
        SatImage->Canvas->LineTo( sc_w, sc_h / 2 );

        str = IntToStr( hor ) + ", " + IntToStr( vert ) + "          ";

        SatImage->Canvas->Font->Size = 9;
        SatImage->Canvas->Font->Color = clRed;
        SatImage->Canvas->Brush->Color = (TColor)RGB( 255, 200, 200 );
        SatImage->Canvas->TextOutA( sc_w / 2 + 5, sc_h / 2 + 5, str );
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ScrImgVertScroll(TObject *Sender,
      TScrollCode ScrollCode, int &ScrollPos)
{
	int r;
    int g;
    int b;
    int sc_h;
    int sc_w;
    int mm_s_x;
    int mm_s_y;
    int mm_e_x;
    int mm_e_y;
    int vert;
    int hor;
    AnsiString str;

    /* �Ђ炢�ĂȂ���΂Ȃɂ����Ȃ� */
    if ( !b_open ) {
    	return;
    }

	if ( ScrollCode == scEndScroll || ScrCheckBox->Checked == true ) {
        /* �V���N�����K�v�Ȃ�V���N������ ( Sync�Ńh���[��������̂Ńh���[���Ȃ� ) */
        if ( b_sync ) {
        	PostMessage( HWND_BROADCAST, umsg_y, ScrImgVert->Position, 0 );
            PostMessage( HWND_BROADCAST, umsg_draw, ScrImgVert->Position, 0 );
        } else {
        	DrawImg();
        }
    } else if ( ScrollCode == scTrack || ScrollCode == scLineUp || ScrollCode == scLineDown || ScrollCode == scPageDown || ScrollCode == scPageUp ) {
    	/* �~�j�}�b�v�`�� */
        sc_h = SatImage->Height;
        sc_w = SatImage->Width;
        vert = ScrollPos;
        hor  = ScrImgHor->Position;

        mm_s_x = 100 * ( hor / (float)img_w );
        mm_s_y = ( ( img_h / (float)img_w ) * 100 ) * ( vert / (float)img_h );
        mm_e_x = mm_s_x + 1;
        mm_e_y = mm_s_y + 1;

        SatImage->Canvas->Pen->Width   = 1;
        SatImage->Canvas->Pen->Color   = clRed;
        SatImage->Canvas->Brush->Color = (TColor)RGB( 255, 200, 200 );
        SatImage->Canvas->Rectangle( 0, 0, 100, ( img_h / (float)img_w ) * 100 );
        SatImage->Canvas->Brush->Color = clRed;
        SatImage->Canvas->Rectangle( Rect( mm_s_x, mm_s_y, mm_e_x, mm_e_y ) );
        SatImage->Canvas->MoveTo( mm_s_x, 0 );
        SatImage->Canvas->LineTo( mm_s_x, ( img_h / (float)img_w ) * 100 );
        SatImage->Canvas->MoveTo( 0, mm_s_y );
        SatImage->Canvas->LineTo( 100, mm_s_y );

        SatImage->Canvas->MoveTo( sc_w / 2, 0 );
        SatImage->Canvas->LineTo( sc_w / 2, sc_h );
        SatImage->Canvas->MoveTo( 0, sc_h / 2 );
        SatImage->Canvas->LineTo( sc_w, sc_h / 2 );

        str = IntToStr( hor ) + ", " + IntToStr( vert ) + "          ";

        SatImage->Canvas->Font->Size = 9;
        SatImage->Canvas->Font->Color = clRed;
        SatImage->Canvas->Brush->Color = (TColor)RGB( 255, 200, 200 );
        SatImage->Canvas->TextOutA( sc_w / 2 + 5, sc_h / 2 + 5, str );
    }
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ExpEditRChange(TObject *Sender)
{
	/* ��������� */
	ecalc_free_token( tok_r );

	/* �����X�V���� */
    tok_r = ecalc_make_token( ExpEditR->Text.c_str() );

	/* �c���[�ɕύX */
	tok_r = ecalc_make_tree( tok_r );

    if ( tok_r == NULL ) {
    	ExpEditR->Color = clBlack;
    } else {
    	ExpEditR->Color = clRed;
    }
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ExpEditBChange(TObject *Sender)
{
	/* ��������� */
	ecalc_free_token( tok_b );

	/* �����X�V���� */
    tok_b = ecalc_make_token( ExpEditB->Text.c_str() );

	/* �c���[�ɕύX */
	tok_b = ecalc_make_tree( tok_b );

    if ( tok_b == NULL ) {
    	ExpEditB->Color = clBlack;
    } else {
    	ExpEditB->Color = clBlue;
    }
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ExpEditGChange(TObject *Sender)
{
	/* ��������� */
	ecalc_free_token( tok_g );

	/* �����X�V���� */
    tok_g = ecalc_make_token( ExpEditG->Text.c_str() );

	/* �c���[�ɕύX */
	tok_g = ecalc_make_tree( tok_g );

    if ( tok_g == NULL ) {
    	ExpEditG->Color = clBlack;
    } else {
    	ExpEditG->Color = clGreen;
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::BlueBtnClick(TObject *Sender)
{
	/* ������� */
    struct REMOS_FRONT_BAND *box;
    char band[2];

    /* BNAD���� */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    band[0] = tolower( (char)( box->index + 0x41 ) );
    band[1] = '\0';

    ExpEditB->Text = band;

    // �K�v�Ȃ玮���[�h��
    if ( !ExpDrawRadioButton->Checked ) {
    	ExpDrawRadioButton->Checked = true;
    } else {
    	DrawImg();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::GreenBtnClick(TObject *Sender)
{
	/* ������΂� */
    struct REMOS_FRONT_BAND *box;
    char band[2];

    /* BNAD���� */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    band[0] = tolower( (char)( box->index + 0x41 ) );
    band[1] = '\0';

    ExpEditG->Text = band;

    // �K�v�Ȃ玮���[�h��
    if ( !ExpDrawRadioButton->Checked ) {
    	ExpDrawRadioButton->Checked = true;
    } else {
    	DrawImg();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::RedBtnClick(TObject *Sender)
{
	/* ������Ԃ� */
    struct REMOS_FRONT_BAND *box;
    char band[2];

    /* BNAD���� */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    band[0] = tolower( (char)( box->index + 0x41 ) );
    band[1] = '\0';

    ExpEditR->Text = band;

    // �K�v�Ȃ玮���[�h��
    if ( !ExpDrawRadioButton->Checked ) {
    	ExpDrawRadioButton->Checked = true;
    } else {
    	DrawImg();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::WBBtnClick(TObject *Sender)
{
	/* ������ */
    struct REMOS_FRONT_BAND *box;
    char band[2];

    /* BNAD���� */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    band[0] = tolower( (char)( box->index + 0x41 ) );
    band[1] = '\0';

    ExpEditR->Text = band;
    ExpEditG->Text = band;
	ExpEditB->Text = band;

    // �K�v�Ȃ玮���[�h��
    if ( !ExpDrawRadioButton->Checked ) {
    	ExpDrawRadioButton->Checked = true;
    } else {
    	DrawImg();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::MaxBtnClick(TObject *Sender)
{
	/* �����W�ݒ���� */
    struct REMOS_FRONT_BAND *box;
    char band[2];

    /* BNAD���� */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    /* �ő�ŏ��� */
    box->band->range_bottom = box->band->range_min;
    box->band->range_top    = box->band->range_max;

    /* �X�V */
    UpdateBandBox( box );

    DrawImg();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::AutoBtnClick(TObject *Sender)
{
	/* �I�[�g�����W */
    struct REMOS_FRONT_BAND *box;
    double range;
    int ret;

    /* BNAD���� */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    /* �q�X�g�O���������ς݁H */
    if ( !box->hist_maked ) {
    	ret = Application->MessageBoxA( "�q�X�g�O�������쐬����Ă��܂���B�쐬���܂����H", "�m�F", MB_YESNO | MB_ICONINFORMATION );

    	if ( ret == IDYES ) {
        	box->hist_maked = true;
        	remos_make_hist( box->band );
        }
    }

    range = AutoRangeEdit->Text.ToDouble();

    /* �I�[�g�����W�K�p */
    if ( AutoRangeCheckBox->Checked ) {
    	/* �㉺������ */
        remos_calc_auto_range( box->band, range, 1 );
    } else {
    	/* �̂����Ȃ� */
    	remos_calc_auto_range( box->band, range, 0 );
    }

    /* �X�V */
    UpdateBandBox( box );

    DrawImg();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ZoomImageMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
	TPoint points[3];
    int skip;
    int cent_y;
    int track_pos;
    int i;
    int zoom_change;
    double zoom_mag;

    /* �ȈՊg�� */
    if ( !b_zoom_click ) {
    	return;
    }

    /* �͈͒��� */
    if ( X < 0 ) {
    	X = 0;
    }

    if ( X > ZoomImage->Width ) {
    	X = ZoomImage->Width - 1;
    }

    if ( Y < 0 ) {
    	Y = 0;
    }

    if ( Y > ZoomImage->Height ) {
    	Y = ZoomImage->Height - 1;
    }

    skip = ( ZoomImage->Height - 30 ) / ZOOM_MAX;

    /* �Y�[������ */
    zoom_pos = ( Y - 15.0 + skip / 2 ) / ( ZoomImage->Height - 30 ) * ZOOM_MAX;

    if ( zoom_pos < 0 ) {
       	zoom_pos = 0;
    } else if ( zoom_pos > ZOOM_MAX ) {
    	zoom_pos = ZOOM_MAX;
    }

    /* �{���ݒ� */
    if ( zoom_pos > ZOOM_MAX / 2 ) {
    	/* �k�� */
        mag  = 1.0 / Power( 2, zoom_pos - ZOOM_MAX / 2 );
    } else if ( zoom_pos < ZOOM_MAX / 2 ) {
    	/* �g�� */
    	mag  = Power( 2, ZOOM_MAX / 2 - zoom_pos );
    } else {
    	/* ���{ */
    	mag  = 1;
    }

    /* �`�� */
    DrawZoomBox( X, Y );

    zoom_change = zoom_pos_click - zoom_pos + ZOOM_MAX / 2;

    /* �{���ݒ� */
    if ( zoom_change > ( ZOOM_MAX / 2 ) ) {
    	/* �k�� */
        zoom_mag  = 1.0 / Power( 2, zoom_change - ( ZOOM_MAX / 2 ) );
    } else if ( zoom_change < ( ZOOM_MAX / 2 ) ) {
    	/* �g�� */
    	zoom_mag  = Power( 2, ( ZOOM_MAX / 2 ) - zoom_change );
    } else {
    	/* ���{ */
    	zoom_mag  = 1;
    }

    /* �K�v�Ȃ�`�� */
    if ( zoom_pos_bef != zoom_pos ) {
    	SatImage->Canvas->Brush->Color = clWhite;
    	SatImage->Canvas->FillRect( Rect( 0, 0, SatImage->Width, SatImage->Height ) );
    	SatImage->Canvas->CopyRect( Rect( SatImage->Width / 2 - SatImage->Width / 2 / zoom_mag,
    	                            	SatImage->Height / 2 - SatImage->Height / 2 / zoom_mag,
    	                                SatImage->Width / 2 + SatImage->Width / 2 / zoom_mag,
    	                                SatImage->Height / 2 + SatImage->Height / 2 / zoom_mag ),
    	                            img_back->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );
    }

    /* �Y�[���ʒu�ۑ� */
    zoom_pos_bef = zoom_pos;

}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::DrawZoomBox( int X, int Y )
{
	TPoint points[3];
    int skip;
    int cent_y;
    int track_pos;
    int i;
    int zoom_change;
    double zoom_mag;

    /* �͈͒��� */
    if ( X < 0 ) {
    	X = 0;
    }

    if ( X > ZoomImage->Width ) {
    	X = ZoomImage->Width - 1;
    }

    if ( Y < 0 ) {
    	Y = 0;
    }

    if ( Y > ZoomImage->Height ) {
    	Y = ZoomImage->Height - 1;
    }

    skip = ( ZoomImage->Height - 30 ) / ZOOM_MAX;

    track_pos = 15 + zoom_pos * skip;

    ZoomImage->Picture->Bitmap->Width = ZoomImage->Width;
    ZoomImage->Picture->Bitmap->Height = ZoomImage->Height;

    points[0] = Point( 12, 5 );
    points[1] = Point( ZoomImage->Width - 12, 5 );
    points[2] = Point( ZoomImage->Width / 2, ZoomImage->Height - 5 );

	ZoomImage->Canvas->Brush->Color = clWhite;
    ZoomImage->Canvas->Pen->Color = clBlue;
	ZoomImage->Canvas->Rectangle( Rect( 0, 0, ZoomImage->Width, ZoomImage->Height ) );

	ZoomImage->Canvas->Brush->Color = (TColor)RGB( 180, 180, 255 );
    ZoomImage->Canvas->Pen->Color = clBlue;
    ZoomImage->Canvas->Polygon( points, 3 );

    for ( i = 0; i < ZOOM_MAX + 1; i++ ) {
    	if ( i != ZOOM_MAX / 2 ) {
    		ZoomImage->Canvas->MoveTo( 10, 15 + i * skip );
    		ZoomImage->Canvas->LineTo( ZoomImage->Width - 10, 15 + i * skip );
        } else {
    		ZoomImage->Canvas->MoveTo( 0, 15 + i * skip );
    		ZoomImage->Canvas->LineTo( ZoomImage->Width, 15 + i * skip );
        }
    }

    /* �n�R�`�� */

    /* �N���b�N���Ă�΂Ȃ߂炩�ɓ����� */
    if ( b_zoom_click ) {
    	track_pos = Y;
    }

	ZoomImage->Canvas->Brush->Color = (TColor)RGB( 100, 100, 255 );
    ZoomImage->Canvas->Pen->Color = clBlue;

    ZoomImage->Canvas->Rectangle( 5, track_pos - 10, ZoomImage->Width - 5, track_pos + 10 );
    ZoomImage->Canvas->Pen->Color = (TColor)RGB( 200, 200, 255 );
    ZoomImage->Canvas->MoveTo( 5, track_pos - 10 );
    ZoomImage->Canvas->LineTo( ZoomImage->Width - 5, track_pos - 10 );
    ZoomImage->Canvas->MoveTo( 5, track_pos - 10 );
    ZoomImage->Canvas->LineTo( 5, track_pos + 10 );

    /* �M�U�M�U */
    for ( i = 0; i < 4; i++ ) {
    	ZoomImage->Canvas->Pen->Color = (TColor)RGB( 50, 50, 200 );
    	ZoomImage->Canvas->MoveTo( 8, track_pos - 10 + ( i + 1 ) * ( 20 / 5 ) );
    	ZoomImage->Canvas->LineTo( ZoomImage->Width - 8, track_pos - 10 + ( i + 1 ) * ( 20 / 5 ) );

    	ZoomImage->Canvas->Pen->Color = (TColor)RGB( 190, 190, 255 );
    	ZoomImage->Canvas->MoveTo( 8, track_pos - 10 + ( i + 1 ) * ( 20 / 5 ) - 1 );
    	ZoomImage->Canvas->LineTo( ZoomImage->Width - 8, track_pos - 10 + ( i + 1 ) * ( 20 / 5 ) - 1 );
    	ZoomImage->Canvas->MoveTo( 8, track_pos - 10 + ( i + 1 ) * ( 20 / 5 ) - 1 );
    	ZoomImage->Canvas->LineTo( 8, track_pos - 10 + ( i + 1 ) * ( 20 / 5 ) + 1 );
    }
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ZoomImageMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if ( Button == mbLeft ) {
    	b_zoom_click = true;

        zoom_pos_click = zoom_pos;
        zoom_pos_bef   = zoom_pos;

        /* �o�b�t�@������ */
    	img_back->Width  = SatImage->Width;
   		img_back->Height = SatImage->Height;

    	img_back->PixelFormat = pf24bit;
    	img_back->HandleType = bmDIB;

        /* �o�b�t�@�ɃR�s�[ */
        img_back->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), SatImage->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

        /* �}�E�X�ړ��C�x���g���� */
        ZoomImageMouseMove( Sender, Shift, X, Y );
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ZoomImageMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	b_zoom_click = false;

    DrawZoomBox( X, Y );

    /* �V���N�����K�v�Ȃ�V���N������ ( Sync�Ńh���[��������̂Ńh���[���Ȃ� ) */
    /*
    if ( b_sync ) {
    	PostMessage( HWND_BROADCAST, umsg_mag, zoom_pos, 0 );
    } else {
       	DrawImg();
    }
    */

    /* Zoom�����O���� */
    DrawImg();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::FormShow(TObject *Sender)
{
    /* �Y�[������ */
    DrawZoomBox( 0, 0 );
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::TBSideClick(TObject *Sender)
{
	if ( !TBSide->Down ) {
    	BandPanel->Visible = false;
        MI_V_BAND->Checked = false;
    } else {
    	BandPanel->Visible = true;
        MI_V_BAND->Checked = true;
    }

    // �������T�C�Y�ōĕ`�悳���邽��
    Application->ProcessMessages();

    DrawImg();
    // PleaseClick();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_QUITClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_OPENClick(TObject *Sender)
{
	TBOpen->OnClick( Sender );
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_SAVEClick(TObject *Sender)
{
	TBSave->OnClick( Sender );	
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_CLOSEClick(TObject *Sender)
{
	TBClose->OnClick( Sender );	
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::BtnDrawClick(TObject *Sender)
{
	/* �t���O���֎~�ł������I�ɕ`�� */

	if ( !b_draw ) {
    	b_draw = true;

    	DrawImg();

        b_draw = false;
    } else {
    	DrawImg();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::MI_V_BANDClick(TObject *Sender)
{
	/* �o���h���X�gON/OFF */
    if ( BandPanel->Visible ) {
    	TBSide->Down = false;
        BandPanel->Visible = false;
    } else {
    	TBSide->Down = true;
        BandPanel->Visible = true;
    }

    PleaseClick();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::MI_V_TOOLClick(TObject *Sender)
{
	/* �c�[���o�[ON/OFF */
    if ( ButtonPanel->Visible ) {
        ButtonPanel->Visible = false;
    } else {
        ButtonPanel->Visible = true;
    }

    PleaseClick();
}
//---------------------------------------------------------------------------


void __fastcall TSatViewMainForm::V1Click(TObject *Sender)
{
    if ( BandPanel->Visible == true ) {
        MI_V_BAND->Checked = true;

        if ( ConfigPanel->Visible == true ) {
        	MI_V_CONF->Checked = true;
        } else {
        	MI_V_CONF->Checked = false;
        }
    } else {
        MI_V_BAND->Checked = false;
        MI_V_CONF->Checked = false;
    }

    if ( ButtonPanel->Visible == true ) {
        MI_V_TOOL->Checked = true;
    } else {
        MI_V_TOOL->Checked = false;
    }

    if ( ZoomPanel->Visible == true ) {
        MI_V_ZOOM->Checked = true;
    } else {
        MI_V_ZOOM->Checked = false;
    }

    if ( StatusBar->Visible == true ) {
        MI_V_STATUS->Checked = true;
    } else {
        MI_V_STATUS->Checked = false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_V_ZOOMClick(TObject *Sender)
{
	/* �Y�[���o�[ON/OFF */
    if ( ZoomPanel->Visible ) {
        ZoomPanel->Visible = false;
    } else {
        ZoomPanel->Visible = true;
    }

    PleaseClick();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_V_STATUSClick(TObject *Sender)
{
	/* �Y�[���o�[ON/OFF */
    if ( StatusBar->Visible ) {
        StatusBar->Visible = false;
    } else {
        StatusBar->Visible = true;
    }

    PleaseClick();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ZoomUpImageMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	/* �Y�[���A�b�v */
    zoom_pos--;

    if ( zoom_pos < 0 ) {
    	zoom_pos = 0;
    }

    DrawZoomBox( X, Y );
    // ZoomImage->OnMouseMove( Sender, Shift, 0, 0 );

    /* �V���N�����K�v�Ȃ�V���N������ ( Sync�Ńh���[��������̂Ńh���[���Ȃ� ) */
    /*
    if ( b_sync ) {
    	PostMessage( HWND_BROADCAST, umsg_mag, zoom_pos, 0 );
    } else {
       	DrawImg();
    }
    */

    /* Zoom�����O���� */
	DrawImg();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::Image3MouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	/* �Y�[���_�E�� */
    zoom_pos++;

    if ( zoom_pos > ZOOM_MAX ) {
    	zoom_pos = ZOOM_MAX;
    }

    DrawZoomBox( 0, 0 );

    /* �V���N�����K�v�Ȃ�V���N������ ( Sync�Ńh���[��������̂Ńh���[���Ȃ� ) */
    /*
    if ( b_sync ) {
    	PostMessage( HWND_BROADCAST, umsg_mag, zoom_pos, 0 );
    } else {
       	DrawImg();
    }
    */

    /* Zoom�����O���� */
	DrawImg();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::MI_W_LTClick(TObject *Sender)
{
	/* �E�C���h�E���ړ� */
	TSize desktop;

    /* �f�X�N�g�b�v�T�C�Y�擾 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

    /* �ړ� */
    SatViewMainForm->Left = 0;
    SatViewMainForm->Top  = 0;

    /* �T�C�Y�ύX */
    SatViewMainForm->Width  = desktop.cx / 2;
    SatViewMainForm->Height = desktop.cy / 2;
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_W_RTClick(TObject *Sender)
{
	/* �E�C���h�E���ړ� */
	TSize desktop;

    /* �f�X�N�g�b�v�T�C�Y�擾 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

    /* �ړ� */
    SatViewMainForm->Left = desktop.cx / 2;
    SatViewMainForm->Top  = 0;

    /* �T�C�Y�ύX */
    SatViewMainForm->Width  = desktop.cx / 2;
    SatViewMainForm->Height = desktop.cy / 2;
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_W_LBClick(TObject *Sender)
{
	/* �E�C���h�E���ړ� */
	TSize desktop;

    /* �f�X�N�g�b�v�T�C�Y�擾 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

    /* �ړ� */
    SatViewMainForm->Left = 0;
    SatViewMainForm->Top  = desktop.cy / 2;

    /* �T�C�Y�ύX */
    SatViewMainForm->Width  = desktop.cx / 2;
    SatViewMainForm->Height = desktop.cy / 2;
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_W_RBClick(TObject *Sender)
{
	/* �E�C���h�E���ړ� */
	TSize desktop;

    /* �f�X�N�g�b�v�T�C�Y�擾 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

    /* �ړ� */
    SatViewMainForm->Left = desktop.cx / 2;
    SatViewMainForm->Top  = desktop.cy / 2;

    /* �T�C�Y�ύX */
    SatViewMainForm->Width  = desktop.cx / 2;
    SatViewMainForm->Height = desktop.cy / 2;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ZoomPanelResize(TObject *Sender)
{
	DrawZoomBox( 0, 0 );
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::DisplayHint(TObject *Sender)
{
    StatusBar->Panels->Items[0]->Text=GetLongHint(Application->Hint);
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::MI_V_CONFClick(TObject *Sender)
{
	if ( BandPanel->Visible == false ) {
    	BandPanel->Visible   = true;
        ConfigPanel->Visible = true;
        TBSide->Down = true;

        PleaseClick();
    } else if ( ConfigPanel->Visible == true ) {
    	ConfigPanel->Visible = false;
    } else {
    	ConfigPanel->Visible = true;
    }
}
//---------------------------------------------------------------------------
void TSatViewMainForm::MakeHist( struct REMOS_FRONT_BAND *box )
{
	/* Canvas���[�h�p�q�X�g�O�����쐬 */
    Graphics::TBitmap *bmp;
    RGBTRIPLE *rgb;
    int i, j;
    unsigned int max;

    max = 0;

    /* �|�C���^�Z�k */
    bmp = box->img_canvas;

	/* �������N���A */
	for ( i = 0; i < 256; i++ ) {
		box->band->hist[i] = 0;
	}

    for ( i = 0; i < box->band->line_count; i++ ) {
    	rgb = (RGBTRIPLE *)bmp->ScanLine[i];

    	for ( j = 0; j < box->band->line_img_width; j++ ) {
            if ( box->band->band_num == 0 ) {
                box->band->hist[rgb[j].rgbtRed]++;
            }

            if ( box->band->band_num == 1 ) {
                box->band->hist[rgb[j].rgbtGreen]++;
            }

            if ( box->band->band_num == 2 ) {
                box->band->hist[rgb[j].rgbtBlue]++;
            }
        }
    }

	/* �ő�l�v�� */
	for ( i = 0; i < 256; i++ ) {
		if ( max < box->band->hist[i] ) {
			max = box->band->hist[i];
		}
	}

	/* �ő�l�ۑ� */
	box->band->hist_max = max;

	/* �ő�l�v�� */
	for ( i = 1, max = 0; i < 255; i++ ) {
		if ( max < box->band->hist[i] ) {
			max = box->band->hist[i];
		}
	}

	/* �ő�l�ۑ� */
	box->band->hist_max_reduce_topbottom = max;

    /* 0���ƍ���̂Œl�����Ă��� */
    if ( box->band->hist_max_reduce_topbottom == 0 ) {
    	box->band->hist_max_reduce_topbottom = 1;
    }

    if ( box->band->hist_max == 0 ) {
    	box->band->hist_max = 1;
    }
}
//---------------------------------------------------------------------------
void TSatViewMainForm::GetLineData( struct REMOS_FRONT_BAND *box, unsigned char *buf, int line, int from, int count )
{
	/* �L�����o�X���[�h�p�Q�b�g���C�� */
    Graphics::TBitmap *bmp;
    RGBTRIPLE *rgb;
    int i;

    /* �|�C���^�Z�k */
    bmp = box->img_canvas;

    /* �͈͊O�ɂ���΋����͈͓� */
    if ( line >= box->band->line_count ) {
    	line = box->band->line_count - 1;
    }

    if ( box->band->line_img_width <= from ) {
    	from = box->band->line_img_width - 1;
    }

    if ( count + from >= box->band->line_img_width ) {
    	count = box->band->line_img_width - from;
    }

    /* �s�|�C���^�擾 */
    rgb = (RGBTRIPLE *)bmp->ScanLine[line];

    for ( i = from; i < count + from; i++ ) {
    	if ( box->band->band_num == 0 ) {
        	buf[i - from] = rgb[i].rgbtRed;
        }

    	if ( box->band->band_num == 1 ) {
        	buf[i - from] = rgb[i].rgbtGreen;
        }

    	if ( box->band->band_num == 2 ) {
        	buf[i - from] = rgb[i].rgbtBlue;
        }
    }
}
//---------------------------------------------------------------------------
unsigned char TSatViewMainForm::GetPixel( struct REMOS_FRONT_BAND *box, int pos )
{
	/* Canvas���[�h�ł�1pix�擾�֐� */
    Graphics::TBitmap *bmp;
    RGBTRIPLE *rgb;
    int i;
    int x;
    int line;

    /* 0���Z�΍� */
    if ( pos < 1 ) {
    	pos = 0;
    	x   = 0;
    } else {
    	x = pos % box->band->line_img_width % pos;
    }

    if ( pos >= box->band->line_img_width * box->band->line_count ) {
    	pos = box->band->line_img_width * box->band->line_count - 1;
    }

    /* �|�C���^�Z�k */
    bmp = box->img_canvas;

    /* �s�|�C���^�擾 */
    rgb = (RGBTRIPLE *)bmp->ScanLine[pos / box->band->line_img_width];

    if ( box->band->band_num == 0 ) {
        return rgb[x].rgbtRed;
    }

    if ( box->band->band_num == 1 ) {
        return rgb[x].rgbtGreen;
    }

    if ( box->band->band_num == 2 ) {
        return rgb[x].rgbtBlue;
    } else {
    	return 0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::MI_H_ABOUTClick(TObject *Sender)
{
	AboutForm->ShowModal();
}
//---------------------------------------------------------------------------


void __fastcall TSatViewMainForm::HistReduceMaxCheckBoxClick(
      TObject *Sender)
{
	/* �q�X�g�O�����X�V */
    int i;

    for ( i = 0; i < list_band->Count; i++ ) {
    	DrawHist( (struct REMOS_FRONT_BAND *)list_band->Items[i] );
    }
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::UpDown3MouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	Application->MessageBoxA( "", "", MB_OK );	
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::FormResize(TObject *Sender)
{
	if ( b_open ) {
		PleaseClick();
    }
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::TBScrClick(TObject *Sender)
{
	/* �X�N���[���𓯊� */
	if ( b_sync ) {
        b_sync = false;
    } else {
        b_sync = true;
    }
}
//---------------------------------------------------------------------------





void __fastcall TSatViewMainForm::TBLtClick(TObject *Sender)
{
	MI_W_LTClick( Sender );
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::TBRtClick(TObject *Sender)
{
	MI_W_RTClick( Sender );

}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::TBLbClick(TObject *Sender)
{
	MI_W_LBClick( Sender );

}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::TBRbClick(TObject *Sender)
{
	MI_W_RBClick( Sender );

}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_W_TClick(TObject *Sender)
{
	/* �E�C���h�E���ړ� */
    HWND h_wnd_desktop;
	TSize desktop;
    TRect rect;

    /* �f�X�N�g�b�v�T�C�Y�擾 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

	h_wnd_desktop = GetDesktopWindow();
	GetWindowRect( h_wnd_desktop, &rect );

    /* �ړ� */
    SatViewMainForm->Left = 0;
    SatViewMainForm->Top  = 0;

    /* �T�C�Y�ύX */
    SatViewMainForm->Width  = desktop.cx;
    SatViewMainForm->Height = desktop.cy / 2;
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_W_BClick(TObject *Sender)
{
	/* �E�C���h�E���ړ� */
	TSize desktop;

    /* �f�X�N�g�b�v�T�C�Y�擾 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

    /* �ړ� */
    SatViewMainForm->Left = 0;
    SatViewMainForm->Top  = desktop.cy / 2;

    /* �T�C�Y�ύX */
    SatViewMainForm->Width  = desktop.cx;
    SatViewMainForm->Height = desktop.cy / 2;
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_W_LClick(TObject *Sender)
{
	/* �E�C���h�E���ړ� */
	TSize desktop;

    /* �f�X�N�g�b�v�T�C�Y�擾 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

    /* �ړ� */
    SatViewMainForm->Left = 0;
    SatViewMainForm->Top  = 0;

    /* �T�C�Y�ύX */
    SatViewMainForm->Width  = desktop.cx / 2;
    SatViewMainForm->Height = desktop.cy;
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_W_RClick(TObject *Sender)
{
	/* �E�C���h�E���ړ� */
	TSize desktop;

    /* �f�X�N�g�b�v�T�C�Y�擾 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

    /* �ړ� */
    SatViewMainForm->Left = desktop.cx / 2;
    SatViewMainForm->Top  = 0;

    /* �T�C�Y�ύX */
    SatViewMainForm->Width  = desktop.cx / 2;
    SatViewMainForm->Height = desktop.cy;
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ToolButton3Click(TObject *Sender)
{
	MI_W_TClick( Sender );	
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ToolButton5Click(TObject *Sender)
{
	MI_W_BClick( Sender );	
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ToolButton6Click(TObject *Sender)
{
	MI_W_LClick( Sender );	
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ToolButton9Click(TObject *Sender)
{
	MI_W_RClick( Sender );
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::BtnColorClick(TObject *Sender)
{
    TPoint pos;
    pos.x = 0;
    pos.y = 0;
    pos = BtnColor->ClientToScreen( pos );
    pos.x += BtnColor->Width;
    ColorMenu->Popup( pos.x, pos.y );
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::TBUpdateClick(TObject *Sender)
{
	/* �`�拖�� */
	if ( !TBUpdate->Down ) {
    	b_draw = false;
    } else {
    	b_draw = true;
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::MI_C_NDVIClick(TObject *Sender)
{
	/* NDVI�v���Z�b�g��\�� */
    int i;
    struct REMOS_FRONT_BAND *box;

    /* �t�@�C�����J����Ė�����ΊJ���Ȃ� */
    if ( !b_open ) {
    	return;
    }

    /* �����ݒ� */
    PresetFormNDVI->NIRComboBox->Clear();
    PresetFormNDVI->RedComboBox->Clear();

    /* �o���h�o�^ */
    for ( i = 0; i < list_band->Count; i++ ) {
    	box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

        PresetFormNDVI->NIRComboBox->Items->Add( box->label_name->Caption );
        PresetFormNDVI->RedComboBox->Items->Add( box->label_name->Caption );
    }

    PresetFormNDVI->NIRComboBox->ItemIndex = 0;
    PresetFormNDVI->RedComboBox->ItemIndex = 0;

    PresetFormNDVI->Left = SatViewMainForm->Left + SatViewMainForm->Width / 2 - PresetFormNDVI->Width / 2;
    PresetFormNDVI->Top = SatViewMainForm->Top + SatViewMainForm->Height / 2 - PresetFormNDVI->Height / 2;

	PresetFormNDVI->Show();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_C_TMClick(TObject *Sender)
{
	/* NDVI�v���Z�b�g��\�� */
    int i;
    struct REMOS_FRONT_BAND *box;

    /* �t�@�C�����J����Ė�����ΊJ���Ȃ� */
    if ( !b_open ) {
    	return;
    }

    /* �����ݒ� */
    PresetFormTHM->BandComboBox->Clear();
    PresetFormTHM->ExpComboBox->Clear();

    /* �o���h�o�^ */
    for ( i = 0; i < list_band->Count; i++ ) {
    	box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

        PresetFormTHM->BandComboBox->Items->Add( box->label_name->Caption );
    }

    /* ���o�^ */
    PresetFormTHM->ExpComboBox->Items->Add( ExpEditR->Text );
    PresetFormTHM->ExpComboBox->Items->Add( ExpEditG->Text );
    PresetFormTHM->ExpComboBox->Items->Add( ExpEditB->Text );

    PresetFormTHM->BandComboBox->ItemIndex = 0;
    PresetFormTHM->ExpComboBox->ItemIndex = 0;

    PresetFormTHM->Left = SatViewMainForm->Left + SatViewMainForm->Width / 2 - PresetFormTHM->Width / 2;
    PresetFormTHM->Top = SatViewMainForm->Top + SatViewMainForm->Height / 2 - PresetFormTHM->Height / 2;

	PresetFormTHM->Show();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_C_LS_THMClick(TObject *Sender)
{
	/* LANDSAT_THM�v���Z�b�g��\�� */
    int i;
    struct REMOS_FRONT_BAND *box;

    /* �t�@�C�����J����Ė�����ΊJ���Ȃ� */
    if ( !b_open ) {
    	return;
    }

    /* �����ݒ� */
    PresetFormLS_THM->BandComboBox->Clear();

    /* �o���h�o�^ */
    for ( i = 0; i < list_band->Count; i++ ) {
    	box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

        PresetFormLS_THM->BandComboBox->Items->Add( box->label_name->Caption );
    }

    PresetFormLS_THM->BandComboBox->ItemIndex = 0;

    // LMAX��LMIN���ǂݍ��߂�ΐݒ�A�ǂݍ��߂Ȃ���΃f�t�H���g�l
    if ( b_config_land ) {
    	PresetFormLS_THM->lmax_61 = land_lmax;
        PresetFormLS_THM->lmin_61 = land_lmin;

    	PresetFormLS_THM->lmax_62 = land_lmax_2;
        PresetFormLS_THM->lmin_62 = land_lmin_2;
    } else {
    	// �f�t�H���g�l
        PresetFormLS_THM->lmax_61 = 17.04;
        PresetFormLS_THM->lmin_61 = 0;

        PresetFormLS_THM->lmax_62 = 12.65;
        PresetFormLS_THM->lmin_62 = 3.2;
    }

    // �ݒ��ǂ݂��܂���
    PresetFormLS_THM->readSetting();

    // �x�����o��
    if ( b_config_ls7 == false ) {
    	AnsiString msg;

        msg = "Landsat7�̐ݒ�f�[�^�[���ǂݍ��܂�Ă��܂���B\n���x�\���𐳊m�ɍs���Ȃ��\��������܂��B\n�e�ݒ�l�̓f�t�H���g�l���g�p����܂��B";

    	Application->MessageBoxA( msg.c_str(), "���", MB_ICONINFORMATION|MB_OK );
    }

    // �\��
    PresetFormLS_THM->Left = SatViewMainForm->Left + SatViewMainForm->Width / 2 - PresetFormTHM->Width / 2;
    PresetFormLS_THM->Top = SatViewMainForm->Top + SatViewMainForm->Height / 2 - PresetFormTHM->Height / 2;

	PresetFormLS_THM->Show();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::OnDropFiles( TWMDropFiles msg )
{
	/* �h���b�v�C�x���g�󂯎�� */
    char filename[256];												/* �t�@�C�����o�b�t�@ */
    int files;														/* �󂯎�萔 */
    int i;
    TStringList *sl;

    sl = new TStringList();

    files = DragQueryFile( (HDROP)msg.Drop, 0xFFFFFFFF, NULL, 0 );	/* �t�@�C�������󂯎�� */

    /* OpenDialog�𗘗p */
    OpenDialog->Files->Clear();

	for ( i = 0; i < files; i++ ) {
    	DragQueryFile( (HDROP)msg.Drop, i, filename, 255 );			/* �t�@�C�����󂯎�� */
        sl->Add( filename );
    }

    DragFinish( (HDROP)msg.Drop );									/* �h���b�v���� */

    sl->Sort();
    OpenDialog->Files->Clear();

    for ( i = 0; i < sl->Count; i++ ) {
    	OpenDialog->Files->Add( sl->Strings[i] );
    }

    /* �J�� */
    OpenFiles();

    delete sl;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::AppMessage( tagMSG &msg, bool &handled )
{
	/* D&D�p, ���b�Z�[�W���� */
    AnsiString dbg;

    if ( msg.message == WM_DROPFILES ) {
    	/* �h���b�v�w�߂������� */
        TWMDropFiles msg_drop;

        msg_drop.Drop = msg.wParam;
        OnDropFiles( msg_drop );
        handled = msg_drop.Result;
    } else if ( b_sync ) {
    	// �����������[�h�Ȃ�
        if ( msg.message == umsg_x ) {
            msg_buf_x = msg.wParam;
            msg_buf_mode = 0;
            // DrawImg();
        } else if ( msg.message == umsg_y ) {
            msg_buf_y = msg.wParam;
            msg_buf_mode = 0;
            // DrawImg();
        } else if ( msg.message == umsg_mag ) {
            zoom_pos = msg.wParam;
            DrawZoomBox( 0, 0 );
            DrawImg();
        } else if ( msg.message == umsg_lat ) {
            /* �ܓx �o�x�ł͕`�悹�� */
            msg_buf_mode = 1;
            msg_buf_lat = msg.wParam / 10000.0;
        } else if ( msg.message == umsg_lon ) {
            /* �o�x */
            msg_buf_mode = 1;
            msg_buf_lon = msg.wParam / 10000.0;
        } else if ( msg.message == umsg_draw ) {
            /* �`��w�� */
            /* DEBUG : ������������������ msg_buf_mode = 1 */
            if ( msg_buf_mode == 1 ) {
                /* �o�x�ܓx���[�h������ */
                // dbg = "lat " + FloatToStr( msg_buf_lat ) + " lon " + FloatToStr( msg_buf_lon );
                // Application->MessageBoxA( dbg.c_str(), "", MB_OK );

                /* �o�x�ܓx���[�h�ɑΉ��o����Όo�x�ܓx���[�h�Ń����N */
                if ( b_config ) {
                	// UTM���[�h�Ȃ�UTM�Ŏ擾
                    if ( b_config_utm ) {
                    	msg_buf_x = config_utm.getXY( Point2D( msg_buf_lon, msg_buf_lat ) ).x;
                        msg_buf_y = config_utm.getXY( Point2D( msg_buf_lon, msg_buf_lat ) ).y;
                    } else {
                    	// �������Ȃ炾�����p�̐ݒ���g��
                        if ( b_config_alos ) {
                        	msg_buf_x = conf_alos.getI( msg_buf_lat, msg_buf_lon );
                            msg_buf_y = conf_alos.getJ( msg_buf_lat, msg_buf_lon );
                    	} else {
                        	remos_latlon_to_xy( &msg_buf_x, &msg_buf_y, img_w, img_h, msg_buf_lat, msg_buf_lon, conf_lt_lon, conf_lt_lat, conf_lb_lon, conf_lb_lat, conf_rt_lon, conf_rt_lat, conf_rb_lon, conf_rb_lat );
                        }
                	}
                } else {
                    /* �Ή����Ă��Ȃ��ꍇ�͉������Ȃ� */
                    msg_buf_x = ScrImgHor->Position;
                    msg_buf_y = ScrImgVert->Position;
                }
            }

            // dbg = "x " + IntToStr( msg_buf_x ) + " y " + IntToStr( msg_buf_y );
            // Application->MessageBoxA( dbg.c_str(), "", MB_OK );

            ScrImgHor->Position  = msg_buf_x;
            ScrImgVert->Position = msg_buf_y;

            DrawImg();
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::OpenFiles( void )
{
	/* �t�@�C���J���֐� */
	/* �t�@�C�����J�� */
    struct REMOS_FILE_CONTAINER *remos;
    struct REMOS_FRONT_BAND *box;
    struct ECONF_FILE_CONTAINER ef;
    bool b_hist_calc;
    bool b_new_open = false;
    Graphics::TBitmap *img_bmp;
    TJPEGImage *img_jpg;
    int ret;
    AnsiString msg, fln, str_buf;
    AnsiString fln_0, fln_1, fln_2;
    char num[2];
    TSearchRec sr;
    TSearchRec sr_conf, sr_conf_2;

    /* ���b�Z�[�W�\�� */
    StatusForm->MessageLabel->Caption = "�摜��ǂݍ���ł��܂�";
    StatusForm->Show();
    Application->ProcessMessages();

    for ( int i = 0; i < OpenDialog->Files->Count; i++ ) {
        /* �Ώۃt�@�C�����傫�����āA���q�X�g�O�������K�v�ȏꍇ�Ɍx�� */
        FindFirst( OpenDialog->Files->Strings[i], faAnyFile, sr );

        if ( HistCheckBox->Checked ) {
            b_hist_calc = true;
        } else {
            b_hist_calc = false;
        }

        if ( ( sr.Size > 100000000 ) && ( b_hist_calc == true ) ) {
            if ( Application->MessageBoxA( "�t�@�C���T�C�Y��100MB�𒴂��Ă��邽�߃q�X�g�O�����쐬�Ɏ��Ԃ�������\��������܂��B\n�q�X�g�O�������쐬���܂����H", "���", MB_ICONINFORMATION|MB_YESNO ) == IDNO ) {
                b_hist_calc = false;
            }
        }

        FindClose( sr );

        /* rmeos�m�� */
        remos = (struct REMOS_FILE_CONTAINER *)malloc( sizeof(struct REMOS_FILE_CONTAINER) );

        /* BMP,JPG���`�F�b�N */
        if ( ExtractFileExt( OpenDialog->Files->Strings[i] ).UpperCase() == ".BMP" || ExtractFileExt( OpenDialog->Files->Strings[i] ).UpperCase() == ".JPG" ) {
            /* BMP, JPG���[�h */

            /* Canvas���[�h�ł�remos�����܂��g���Ȃ��̂ŁA�t�@�C�������R�s�[��������A�����I�ɕ��� */
            remos_open( remos, OpenDialog->Files->Strings[i].c_str(), REMOS_FILE_TYPE_NOT_RECOG );

            /* �t�@�C���A�o���h�ݒ���蓮�� */
            remos->band_count = 3;
            remos->bands = (struct REMOS_BAND*)malloc( sizeof(struct REMOS_BAND) * 3 );

            /* �o���h��ݒ肳���� */
            for ( int j = 0; j < 3; j++ ) {
                remos->bands[j].band_num      = j;
                remos->bands[j].band_count    = 3;
                remos->bands[j].bits          = 8;
                remos->bands[j].range_bottom  = 0;
                remos->bands[j].range_top     = 255;
                remos->bands[j].range_max     = 255;
                remos->bands[j].range_min     = 0;
                remos->bands[j].sample_format = REMOS_BAND_SAMPLE_FORMAT_UINT;		// DEBUG : 20130511���ꂪ�Ȃ����߂Ƀ����W�w�肪�@�\���Ȃ�����
            }

            /* �摜�����[�h */
            img_bmp = new Graphics::TBitmap();

            if ( ExtractFileExt( OpenDialog->Files->Strings[i] ).UpperCase() == ".BMP" ) {
                /* bmp�̏ꍇ */
                img_bmp->LoadFromFile( OpenDialog->Files->Strings[i].c_str() );
            } else {
                /* jpg�̏ꍇ */
                img_jpg = new TJPEGImage();
                img_jpg->LoadFromFile( OpenDialog->Files->Strings[i].c_str() );
                img_bmp->Assign( img_jpg );
                delete img_jpg;
            }

            /* ���ɊJ����Ă����ꍇ�ŁA�T�C�Y���ȑO�̂ƈႤ�ƃG���[ */
            if ( b_open ) {
                if ( ( img_w != img_bmp->Width ) || ( img_h != img_bmp->Height ) ) {
                    Application->MessageBoxA( "�摜�̑傫�����Ⴄ�o���h���m�͓����ɊJ���܂���", "���s", MB_OK|MB_ICONINFORMATION );

                    remos_close( remos );
                    free( remos );
                    delete img_bmp;

                    /* ���b�Z�[�W������ */
                    StatusForm->Close();

                    return ;
                }
            }

            /* ���� */
            b_open = true;
            b_new_open = true;

            /* �o�^ */
            list_file->Add( remos );

            /* �ݒ� */
            img_w = img_bmp->Width;
            img_h = img_bmp->Height;

            ScrImgHor->Max  = img_w - 1;
            ScrImgVert->Max = img_h - 1;

            StatusBar->Panels->Items[1]->Text = "�c : " + IntToStr( img_h ) + " �� : " + IntToStr( img_w );

            /* �o���h���o�^ */
            for ( int j = 0; j < remos->band_count; j++ ) {
                /* �o���h�{�b�N�X�̐����Ɠo�^ */
                remos->bands[j].line_count     = img_h;
                remos->bands[j].line_img_width = img_w;
                remos->bands[j].line_width     = img_w;
                remos->bands[j].line_footer    = 0;
                remos->bands[j].line_header    = 0;

                remos->bands[j].hist_max                  = 1;	/* 0���Ɗ댯�Ȃ̂� */
                remos->bands[j].hist_max_reduce_topbottom = 1;  /* 0���Ɗ댯�Ȃ̂� */

                remos->bands[j].hist_max = 255;

                // �r�b�g���Ȃǂ��w��
                remos->bands[j].byte_per_sample = 1;
                remos->bands[j].sample_per_pix  = 1;
                remos->bands[j].bits = 8;

                if ( !b_hist_calc ) {
                    for ( int k = 0; k < 256; k++ ) {
                        remos->bands[j].hist[k] = 0;
                    }
                }

                /* �o���h�{�b�N�X����� */
                list_band->Add( MakeBandBox( &remos->bands[j], ExtractFileName( remos->file_name ), list_band->Count ) );

                box = (struct REMOS_FRONT_BAND *)list_band->Items[list_band->Count - 1];

                /* Canvas���[�h�� */
                box->canvas_mode = true;
                box->canvas_color = box->index;
                box->img_canvas  = img_bmp;

                /* �q�X�g�O�����쐬�t���O */
                if ( b_hist_calc ) {
                    box->hist_maked = true;
                    MakeHist( box );
                } else {
                    box->hist_maked = false;

                    /* �q�X�g�O�����N���A */
                    for ( int k = 0; k < 256; k++ ) {
                    	box->band->hist[i] = 0;
                    }
                }

                /* �X�V */
                UpdateBandBox( box );

                /* ���b�Z�[�W���� */
                Application->ProcessMessages();
            }
        } else {
            /* �q���f�[�^���J�� remos���[�h */
            if ( 0 != ( ret = (int)remos_open( remos, OpenDialog->Files->Strings[i].c_str(), REMOS_FILE_TYPE_AUTO ) ) ) {
                if ( ret == REMOS_RET_READ_FAILED ) {
                    /* �ǂ߂����ǎ��ʎ��s */
                    msg = "\"" + ExtractFileName( OpenDialog->Files->Strings[i] ) + "\"\n"
                        + "�͕s���ȉq���f�[�^�`���ł�";

                    Application->MessageBoxA( msg.c_str(), "���ʎ��s", MB_OK|MB_ICONINFORMATION );

                    remos_close ( remos );

                    free( remos );
                } else {
                    /* ���ɊJ����Ă����ꍇ�ŁA�T�C�Y���ȑO�̂ƈႤ�ƃG���[ */
                    if ( b_open ) {
                        if ( ( img_w != remos->img_width ) || ( img_h != remos->img_height ) ) {
                            Application->MessageBoxA( "�摜�̑傫�����Ⴄ�o���h���m�͓����ɊJ���܂���", "���s", MB_OK|MB_ICONINFORMATION );

                            remos_close( remos );
                            free( remos );

                            /* ���b�Z�[�W������ */
                            StatusForm->Close();

                            return ;
                        }
                    }

                    /* ���� */
                    b_open = true;
                    b_new_open = true;

                    /* �o�^ */
                    list_file->Add( remos );

                    /* �ݒ� */
                    img_w = remos->img_width;
                    img_h = remos->img_height;

                    ScrImgHor->Max  = img_w - 1;
                    ScrImgVert->Max = img_h - 1;

                    StatusBar->Panels->Items[1]->Text = "�c : " + IntToStr( img_h ) + " �� : " + IntToStr( img_w );

                    /* �o���h���o�^ */
                    for ( int j = 0; j < remos->band_count; j++ ) {
                        /* �o���h�{�b�N�X�̐����Ɠo�^ */

                        /* �o�^���A�q�X�g�O�������쐬 */
                        if ( b_hist_calc ) {
                            remos_make_hist( &remos->bands[j] );
                        } else {
                            /* 0���Z�h�~�Ȃǂ̂��ߓK���ȏ����l���� */
                            remos->bands[j].hist_max_reduce_topbottom = 255;
                            remos->bands[j].hist_max = 255;

                            /* �q�X�g�O�����N���A */
                            for ( int k = 0; k < 256; k++ ) {
                                remos->bands[j].hist[k] = 0;
                            }
                        }

                        list_band->Add( MakeBandBox( &remos->bands[j], ExtractFileName( remos->file_name ), list_band->Count ) );

                        box = (struct REMOS_FRONT_BAND *)list_band->Items[list_band->Count - 1];

                        /* �q�X�g�O�����쐬�t���O */
                        if ( b_hist_calc ) {
                            box->hist_maked = true;
                        } else {
                            box->hist_maked = false;
                        }

                        /* �L�����o�X���[�h�ł͂Ȃ� */
                        box->canvas_mode = false;

                        /* �X�V */
                        UpdateBandBox( box );

                        /* ���b�Z�[�W���� */
                        Application->ProcessMessages();
                    }
                }
            } else {
                /* �ǂݍ��ݎ��s */
                free( remos );

                // ����ɕ��Ă͂����Ȃ�
                // b_open = false;
            }
        }
    }

    /* ���b�Z�[�W������ */
    StatusForm->Close();

    /* �ݒ�t�@�C���̓ǂݍ��݂����݂� ( �ʓ|�������̂Ŗ��񂷂� ) */
    if ( b_new_open ) {
    	// �ݒ�ǂݍ��݊J�n

    	// �J�����̂ň�x�ݒ���N���A����
    	clearSetting();

        fln_0 = ExtractFilePath( OpenDialog->Files->Strings[0] ) + "summary.txt";
        fln_1 = ExtractFilePath( OpenDialog->Files->Strings[0] ) + "*.met";
        fln_2 = ExtractFilePath( OpenDialog->Files->Strings[0] ) + "*.txt";

        if ( econf_open( &ef, fln_0.c_str() ) ) {
            /* �������p�F�� */
            // �������̓��x��1B2R_U�������̂Ƃ��돈���ł��Ȃ��悤�ɂ��Ă���
            if ( econf_recog( &ef ) ) {
            	if ( econf_search( &ef, "Pds_PixelSpacing" ) != NULL ) {
                	b_config_land = false;
                	b_config_resolution = true;

                    conf_resolution_x = econf_as_double( econf_search( &ef, "Pds_PixelSpacing" ) );
                    conf_resolution_y = conf_resolution_x;
                } else {
                	b_config_land = false;
                	b_config_resolution = false;
                }

				// ���x��1B2�Ȃ�
                if ( econf_search( &ef, "Pds_ProductID" ) != NULL ) {
                	AnsiString plevel = econf_as_str( econf_search( &ef, "Pds_ProductID" ) );

                	if ( plevel.Pos( "O1B2R_U" ) != 0 || plevel.Pos( "O1B2G_U" ) != 0 ) {
						// ALOS��UTM���e1B2������
                        AnsiString fln_led;
                        TSearchRec sr_led;

                        // ���[�_�[�t�@�C���ǂݍ���
                        fln_led = ExtractFilePath( OpenDialog->Files->Strings[0] ) + "LED*";

                        if ( FindFirst( fln_led, faAnyFile, sr_led ) == 0 ) {
                        	// ���������̂ŊJ���Ă݂�
                            if ( conf_alos.loadLeader( sr_led.Name.c_str() ) ) {
                            	// ���������̂Őݒ�L��,���W���ꉞ�ۑ�
                                conf_lt_lat = conf_alos.getLat( 0, 0 );
                                conf_lt_lon = conf_alos.getLon( 0, 0 );
                                conf_rt_lat = conf_alos.getLat( img_w, 0 );
                                conf_rt_lon = conf_alos.getLon( img_w, 0 );
                                conf_lb_lat = conf_alos.getLat( 0, img_h );
                                conf_lb_lon = conf_alos.getLon( 0, img_h );
                                conf_rb_lat = conf_alos.getLat( img_w, img_h );
                                conf_rb_lon = conf_alos.getLon( img_w, img_h );

                                b_config = true;
                                b_config_alos = true;
                                b_config_land = false;
                                b_config_latlon_utm_proj = false;
                            }
                        }

                        // ���R�[�h�����
                        FindClose( sr_led );
                    }
                }

                // ���t�̐ݒ�
                if ( econf_search( &ef, "Lbi_ObservationDate" ) ) {
                	b_config_date = true;

                    config_date_str = econf_as_str( econf_search( &ef, "Lbi_ObservationDate" ) );
                } else {
                	b_config_date = false;
                }
            }

            econf_close( &ef );
        } else {
        	/* LANDSAT�p�F�� */
            int met = FindFirst( fln_1, faAnyFile, sr_conf );
            int txt = FindFirst( fln_2, faAnyFile, sr_conf_2 );
            AnsiString current_name;
            bool end_find = false;

            // ���ׂĎ��s����Ή������Ȃ�
            if ( met != 0 && txt != 0 ) {
            	end_find = true;
            }

            /* met�D���met���Ȃ����txt�Ŏ��s */
            while ( !end_find ) {
                if ( met == 0 ) {
                	current_name = sr_conf.Name;

                    if ( FindNext( sr_conf ) != 0 ) {
                    	end_find = true;
                    }
                } else {
                	current_name = sr_conf_2.Name;

                    if ( FindNext( sr_conf_2 ) != 0 ) {
                    	end_find = true;
                    }
                }

                if ( econf_open( &ef, current_name.c_str() ) ) {
                    /* �F�� */
                    if ( econf_recog( &ef ) ) {
                        if ( econf_search( &ef, "LMAX_BAND61" ) != NULL ) {
                            /* LANDSAT�p���� */
                            b_config_land = true;

                            // ���x�ݒ�ǂݍ���
                            land_lmax = econf_as_double( econf_search( &ef, "LMAX_BAND61" ) );
                            land_lmin = econf_as_double( econf_search( &ef, "LMIN_BAND61" ) );

                            land_qcal_min = econf_as_double( econf_search( &ef, "QCALMIN_BAND61" ) );
                            land_qcal_max = econf_as_double( econf_search( &ef, "QCALMAX_BAND61" ) );

                            land_lmax_2 = econf_as_double( econf_search( &ef, "LMAX_BAND62" ) );
    						land_lmin_2 = econf_as_double( econf_search( &ef, "LMIN_BAND62" ) );
                            land_qcal_min_2 = econf_as_double( econf_search( &ef, "QCALMIN_BAND62" ) );
                            land_qcal_max_2 = econf_as_double( econf_search( &ef, "QCALMAX_BAND62" ) );

                            AnsiString fn_8, fn_61, fn_62;
                            AnsiString fnbuf;

                            // �t�@�C�������擾
                            fn_8  = econf_as_str( econf_search( &ef, "BAND8_FILE_NAME" ) );
                            fn_61 = econf_as_str( econf_search( &ef, "BAND61_FILE_NAME" ) );
                            fn_62 = econf_as_str( econf_search( &ef, "BAND62_FILE_NAME" ) );

                            // �t�@�C������r
                            for ( int i = 0; i < OpenDialog->Files->Count; i++ ) {
                                if ( fn_8.Pos( ExtractFileName( OpenDialog->Files->Strings[i] ) ) ) {
                                    // 8�Ȃ�p���N��
                                    conf_resolution_x = econf_as_double( econf_search( &ef, "GRID_CELL_SIZE_PAN" ) );
                                    conf_resolution_y = conf_resolution_x;
                                    b_config_resolution = true;

                                    break;
                                } else if ( fn_61.Pos( ExtractFileName( OpenDialog->Files->Strings[i] ) ) || fn_62.Pos( ExtractFileName( OpenDialog->Files->Strings[i] ) ) ) {
                                    // 6162�Ȃ牷�x
                                    conf_resolution_x = econf_as_double( econf_search( &ef, "GRID_CELL_SIZE_THM" ) );
                                    conf_resolution_y = conf_resolution_x;
                                    b_config_resolution = true;

                                    break;
                                } else {
                                    conf_resolution_x = econf_as_double( econf_search( &ef, "GRID_CELL_SIZE_REF" ) );
                                    conf_resolution_y = conf_resolution_x;
                                    b_config_resolution = true;

                                    break;
                                }
                            }
                        } else {
                            b_config_land = false;
                            b_config_resolution = false;
                        }

                        // Landsat7���ǂ���
                        if ( AnsiString( econf_as_str( econf_search( &ef, "SPACECRAFT_ID" ) ) ) == "\"Landsat7\"" ) {
                        	b_config_ls7 = true;
                        } else {
                        	b_config_ls7 = false;
                        }

                        // ���t������
                        if ( econf_search( &ef, "ACQUISITION_DATE" ) ) {
                        	b_config_date = true;

                            config_date_str = econf_as_str( econf_search( &ef, "ACQUISITION_DATE" ) );
                        } else {
                        	b_config_date = false;
                        }


                        /* SCENE��PRODUCT�̈Ӗ��͈Ⴄ�C������ */
                        if ( econf_search( &ef, "PRODUCT_UL_CORNER_LAT" ) != NULL ) {
                            b_config = true;

                            conf_lt_lat = econf_as_double( econf_search( &ef, "PRODUCT_UL_CORNER_LAT" ) );
                            conf_lt_lon = econf_as_double( econf_search( &ef, "PRODUCT_UL_CORNER_LON" ) );
                            conf_rt_lat = econf_as_double( econf_search( &ef, "PRODUCT_UR_CORNER_LAT" ) );
                            conf_rt_lon = econf_as_double( econf_search( &ef, "PRODUCT_UR_CORNER_LON" ) );
                            conf_lb_lat = econf_as_double( econf_search( &ef, "PRODUCT_LL_CORNER_LAT" ) );
                            conf_lb_lon = econf_as_double( econf_search( &ef, "PRODUCT_LL_CORNER_LON" ) );
                            conf_rb_lat = econf_as_double( econf_search( &ef, "PRODUCT_LR_CORNER_LAT" ) );
                            conf_rb_lon = econf_as_double( econf_search( &ef, "PRODUCT_LR_CORNER_LON" ) );
                        } else {
                            b_config = false;
                        }
                    }

                    econf_close( &ef );
                }
            }

            FindClose( sr_conf );
            FindClose( sr_conf_2 );
        }

        // �������łȂ���Ή摜�f�[�^�[������W��ǂݍ��� ( ���܂������΂�����Ɉڍs )
        if ( ( b_config == true && b_config_land == true ) || b_config == false ) {
        	// TIFF�̔F�������݂�
            GeotiffRec gt;
            int i;

            // ��ł��ǂ߂��܂Ŏ���
            for ( i = 0; i < OpenDialog->Files->Count; i++ ) {
            	if ( gt.open( OpenDialog->Files->Strings[i].c_str() ) ) {
                	break;
                }
            }

            // �������o����Ώ����J�n
            if ( gt.initialize() ) {
            	// �������ł����̂ŕ���
                gt.close();

                // �l�Ӎ��W�ݒ�
                conf_lt_lat = gt.getLatLon( Point2D( 0, 0 ) ).y;
                conf_lt_lon = gt.getLatLon( Point2D( 0, 0 ) ).x;
                conf_rt_lat = gt.getLatLon( Point2D( img_w, 0 ) ).y;
                conf_rt_lon = gt.getLatLon( Point2D( img_w, 0 ) ).x;
                conf_lb_lat = gt.getLatLon( Point2D( 0, img_h ) ).y;
                conf_lb_lon = gt.getLatLon( Point2D( 0, img_h ) ).x;
                conf_rb_lat = gt.getLatLon( Point2D( img_w, img_h ) ).y;
                conf_rb_lon = gt.getLatLon( Point2D( img_w, img_h ) ).x;

                // �ݒ�L��
                b_config = true;

                // UTM���[�h�Ȃ�UTM�ݒ�L��
                b_config_utm = gt.isUTM();

                config_utm = gt;

                // �𑜓x�ݒ� ( LatLon�ō��W�ݒ�����Ă�����̂őΉ��ł��Ȃ� UTM�ɕϊ����Ă���Ƃ����肪���� : TODO )
                if ( gt.isUTM() ) {
                	b_config_resolution = true;

                	conf_resolution_x = gt.getScale().x;
                	conf_resolution_y = gt.getScale().y;
				}
            }
        }

        // �ݒ蕶����X�V
        setSettingStr();

        // �E�B���h�E�^�C�g���ݒ�
        setWindowTitle( OpenDialog->Files->Strings[OpenDialog->Files->Count - 1] );
    }

    /* �`�� */
    DrawImg();
}
//---------------------------------------------------------------------------
void TSatViewMainForm::setWindowTitle( AnsiString filename )
{
	// �E�B���h�E�^�C�g���ݒ�
    AnsiString path;

    // ��������ꍇ
    if ( filename != "" ) {
        path = ExtractFileDir( filename );

        while ( path.Pos( "\\" ) ) {
            path.Delete( 1, path.Pos( "\\" ) );
        }

        SatViewMainForm->Caption = path + " - " + AnsiString( "SatelliteEye" );
        Application->Title = path + " - " + AnsiString( "SatelliteEye" );
    } else {
        SatViewMainForm->Caption = AnsiString( "SatelliteEye" );
        Application->Title = AnsiString( "SatelliteEye" );
    }
}
//---------------------------------------------------------------------------
void TSatViewMainForm::setSettingStr( void )
{
	// �ݒ蕶�����ݒ�
    if ( b_config || ( b_config == false && b_config_land == true ) ) {
    	// �q�����
    	if ( b_config_land ) {
        	SettingStrPanel->Caption = "Landsat";

            if ( b_config_ls7 ) {
            	SettingStrPanel->Caption = SettingStrPanel->Caption + "7";
            }
        } else if ( b_config_alos ) {
        	SettingStrPanel->Caption = "������";
        } else {
            SettingStrPanel->Caption = "�s��(���W�ݒ�̂ݗL��)";
        }

        // UTM�Ȃ�utm��ǉ�
        if ( b_config_utm ) {
        	// SettingStrPanel->Caption = SettingStrPanel->Caption + "(UTM)";
        }

        // ���t
        if ( b_config_date ) {
        	SettingStrPanel->Caption = SettingStrPanel->Caption + " : " + config_date_str;
        }
    } else {
    	SettingStrPanel->Caption = "�ݒ�t�@�C������";
    }

    /*
    if ( b_config ) {
        StatusBar->Panels->Items[3]->Text = "���W�ݒ�L��";

        if ( b_config_land ) {
            StatusBar->Panels->Items[3]->Text = "���W�ݒ�L�� LANDSAT���x�ݒ�L��";
        }
    } else if ( b_config_land ) {
        StatusBar->Panels->Items[3]->Text = "LANDSAT���x�ݒ�L��";
    } else {
    	StatusBar->Panels->Items[3]->Text = "�ݒ�t�@�C������";
    }
    */
}
//---------------------------------------------------------------------------
void TSatViewMainForm::clearSetting( void )
{
	// �ݒ���N���A����
    b_config = false;
    b_config_land = false;
    b_config_resolution = false;
    b_config_utm = false;
    b_config_ls7 = false;
    b_config_date = false;
    b_config_alos = false;
    b_config_latlon_utm_proj = false;

    // �ݒ蕶����ݒ�
    setSettingStr();

    // �E�B���h�E�^�C�g���N���A
    setWindowTitle( "" );
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::Button1Click(TObject *Sender)
{
	ColorBarForm->Show();
    ColorBarForm->draw();

    ColorMapConfigForm->setColorMap( color_map );
    ColorMapConfigForm->updateView();
    ColorMapConfigForm->Show();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::setColorBarExp( AnsiString exp )
{
	// �������ݒ�
    color_bar_exp = exp;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::TBMonosasiClick(TObject *Sender)
{
	if ( b_mode_length ) {
    	b_mode_length = false;
        TBMonosasi->Down = false;
    } else {
        // �ݒ肪�L���ł���Β�K���[�h�Ɉڂ�
        if ( b_config_resolution ) {
            b_mode_length = true;
            TBMonosasi->Down = true;
        } else {
            Application->MessageBoxA( "�𑜓x�̐ݒ��ǂݍ��ނ��Ƃ��ł��܂���ł����B\n�����𑪒�ł��܂���B", "��_�Ԃ̋����𑪒�", MB_OK | MB_ICONINFORMATION );
            TBMonosasi->Down = false;

            return;
        }

        // �X�^���v���[�h�L���Ȃ�I�t��
        if ( b_mode_stamp_cb ) {
        	TBStamp->Click();
        }
    }
}
//---------------------------------------------------------------------------


void __fastcall TSatViewMainForm::TBStampClick(TObject *Sender)
{
	// �J���[�o�[�X�^���v���[�h�؂�ւ�
	if ( b_mode_stamp_cb ) {
    	b_mode_stamp_cb = false;
        TBStamp->Down = false;

        // �o�b�t�@����R�s�[
        SatImage->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), img_back->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

    } else {
    	// �����[�h�ł͎��s�ł��Ȃ�
        if ( b_draw_mode_exp ) {
        	Application->MessageBoxA( "�u���ŕ`���v���[�h�ł͐F�̌��{��\��t�����܂���B", "���", MB_ICONINFORMATION | MB_OK );
            TBStamp->Down = false;

            return;
        }

    	b_mode_stamp_cb = true;
        TBStamp->Down = true;

		// �o�b�t�@�֓]��
    	img_back->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), SatImage->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );


        // ��K���[�h�L���Ȃ�I�t��
        if ( b_mode_length ) {
        	TBMonosasi->Click();
        }
    }
}
//---------------------------------------------------------------------------



void __fastcall TSatViewMainForm::ExpDrawRadioButtonClick(TObject *Sender)
{
	// ���`�惂�[�h
    b_draw_mode_exp = true;
    ExpEditR->Enabled = true;
	ExpEditG->Enabled = true;
    ExpEditB->Enabled = true;

    ShowLevelColorBarButton->Enabled = false;
    ConfigLevelButton->Enabled = false;

    // �J���[�o�[������
    ColorBarForm->Close();

    // �J���[�o�[�ݒ� ( �ꉞ )
    ColorBarForm->setExpMode( true );

    // �ĕ`��
    DrawImg();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::LevelDrawRadioButtonClick(
      TObject *Sender)
{
	// ���x���X���C�X�`�惂�[�h
    b_draw_mode_exp = false;
    ExpEditR->Enabled = false;
	ExpEditG->Enabled = false;
    ExpEditB->Enabled = false;

    ShowLevelColorBarButton->Enabled = true;
    ConfigLevelButton->Enabled = true;

    // �J���[�o�[�ݒ�
    ColorBarForm->setExpMode( false );
    ColorBarForm->setColorMap( color_map );

    // �ĕ`��
    DrawImg();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ConfigLevelButtonClick(TObject *Sender)
{
	// ���x���X���C�X�̐ݒ���N��
    ColorMapConfigForm->Show();
    ColorMapConfigForm->setColorMap( color_map );
    ColorMapConfigForm->updateView();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ShowLevelColorBarButtonClick(
      TObject *Sender)
{
    // �J���[�o�[�\��
    ColorBarForm->viewNowValue( true );
	ColorBarForm->setColorMap( color_map );
    ColorBarForm->Show();
    ColorBarForm->draw();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_C_GDEMClick(TObject *Sender)
{
	// ASTERGDEM�̃v���Z�b�g
    int i;
    struct REMOS_FRONT_BAND *box;

    /* �t�@�C�����J����Ė�����ΊJ���Ȃ� */
    if ( !b_open ) {
    	return;
    }

    /* �����ݒ� */
    PresetFormAGDEM->BandComboBox->Clear();

    /* �o���h�o�^ */
    for ( i = 0; i < list_band->Count; i++ ) {
    	box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

        PresetFormAGDEM->BandComboBox->Items->Add( box->label_name->Caption );
    }

    PresetFormAGDEM->BandComboBox->ItemIndex = 0;

	PresetFormAGDEM->Show();
}
//---------------------------------------------------------------------------





void __fastcall TSatViewMainForm::TBCurposClick(TObject *Sender)
{
	/* �s�N�Z�����\�� */
    if ( !PixForm->Visible ) {
		PixForm->Show();
        PixForm->Caption = "���W�l ( �ŐV )";
    }
}
//---------------------------------------------------------------------------

