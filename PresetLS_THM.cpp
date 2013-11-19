//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "PresetLS_THM.h"
#include "Main.h"
#include "ColorBar.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPresetFormLS_THM *PresetFormLS_THM;
//---------------------------------------------------------------------------
__fastcall TPresetFormLS_THM::TPresetFormLS_THM(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormLS_THM::FormShow(TObject *Sender)
{
	BtnOk->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormLS_THM::BtnCancelClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void TPresetFormLS_THM::readSetting( void )
{
	// �ݒ��ǂݍ���
    if ( TypeRadioButton61->Checked ) {
    	LMINEdit->Text = FloatToStr( lmin_61 );
        LMAXEdit->Text = FloatToStr( lmax_61 );
    } else {
    	LMINEdit->Text = FloatToStr( lmin_62 );
        LMAXEdit->Text = FloatToStr( lmax_62 );
    }
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormLS_THM::BtnOkClick(TObject *Sender)
{
	/* ���x���z�p�J���[�}�b�v���쐬 */
    ColorMap *map;

    AnsiString unit_str;
    AnsiString cb;

    char *fmt_r;
    char *fmt_g;
    char *fmt_b;
    char *fmt_thm;
    char *fmt_cb_r;
    char *fmt_cb_g;
	char *fmt_cb_b;


    char band6;
    char band;

    int temp_width = TempUpDown->Position - TempMinUpDown->Position;
    int max_temp = TempUpDown->Position;
    int min_temp = TempMinUpDown->Position;

    float lmax = StrToFloat( LMAXEdit->Text );
    float lmin = StrToFloat( LMINEdit->Text );

    map = SatViewMainForm->color_map;

    /* �R���{�{�b�N�X���L�����N�^�� */
    band6 = BandComboBox->ItemIndex + 0x61;
    band = 'w';

    // ���x�v�Z��
    fmt_thm = "( v = %c - 1 ), ( v < 0 )if( v = 0 ), ( %c = 1282.71 / ln( 666.09 / ( %c * ( %f - %f ) / 254.0 + %f ) + 1 ) - 273.15 )";
    unit_str = unit_str.sprintf( fmt_thm, band6, band, 'v', lmax, lmin, lmin );

    // ����o�^
    map->setExpression( unit_str );
    map->setUnitString( "��" );
    map->setRange( min_temp, max_temp );

    // �F���쐬
    map->deleteAllColorLevel();

    map->addColorLevel( new ColorLevel( max_temp, 255, 0, 0 ) );
    map->addColorLevel( new ColorLevel( ( max_temp - min_temp ) * ( 4 / 5.0 ) + min_temp, 255, 255,   0 ) );
    map->addColorLevel( new ColorLevel( ( max_temp - min_temp ) * ( 3 / 5.0 ) + min_temp,   0, 255,   0 ) );
    map->addColorLevel( new ColorLevel( ( max_temp - min_temp ) * ( 2 / 5.0 ) + min_temp,   0, 255, 255 ) );
    map->addColorLevel( new ColorLevel( ( max_temp - min_temp ) * ( 1 / 5.0 ) + min_temp,   0,   0, 255 ) );
    map->addColorLevel( new ColorLevel( ( max_temp - min_temp ) * ( 0 / 5.0 ) + min_temp,   0,   0,   0 ) );

    map->setSmooth();

    // ���C���t�H�[�����J���[�}�b�v���[�h��
    SatViewMainForm->LevelDrawRadioButton->Checked = true;

    /* �q�X�g�O�����ݒ���� */
    ( (struct REMOS_FRONT_BAND *)SatViewMainForm->list_band->Items[BandComboBox->ItemIndex] )->updown_bottom->Position = 0;
    ( (struct REMOS_FRONT_BAND *)SatViewMainForm->list_band->Items[BandComboBox->ItemIndex] )->updown_top->Position = 255;

    SatViewMainForm->UpdateBandBox( (struct REMOS_FRONT_BAND *)SatViewMainForm->list_band->Items[BandComboBox->ItemIndex] );


	// �{�^������
    //this->Visible = false;
    this->BtnOk->Enabled = false;

    /* ���łɏ������� */
    SatViewMainForm->DrawImg();

    // �J���[�o�[�\��
    // ColorBarForm->Caption = "���x";
    ColorBarForm->viewNowValue( true );
    ColorBarForm->setColorMap( SatViewMainForm->color_map );
    ColorBarForm->setExpMode( false );
    ColorBarForm->setUnitString( "a", "��" );
    ColorBarForm->Show();
    ColorBarForm->draw();

    /* �{�^�����A */
    this->BtnOk->Enabled = true;

    //Close();

// �Â���
#if 0
	/* ���C���t�H�[���̃o���h�G�f�B�b�g���������� */
    AnsiString r;
    AnsiString g;
    AnsiString b;

    AnsiString cb_r;
    AnsiString cb_g;
    AnsiString cb_b;

    AnsiString unit_exp;
    AnsiString cb;

    char *fmt_r;
    char *fmt_g;
    char *fmt_b;
    char *fmt_thm;
    char *fmt_cb_r;
    char *fmt_cb_g;
	char *fmt_cb_b;


    char band6;
    char band;

    int temp_width = TempUpDown->Position - TempMinUpDown->Position;
    int max_temp = TempUpDown->Position;
    int min_temp = TempMinUpDown->Position;

    float lmax = StrToFloat( LMAXEdit->Text );
    float lmin = StrToFloat( LMINEdit->Text );

    /* �R���{�{�b�N�X���L�����N�^�� */
    band6 = BandComboBox->ItemIndex + 0x61;
    band = 'w';

    /* �o���h�����l */
    // fmt_r = "( v = %c - 1 ), ( v < 0 )if( v = 0 ), ( %c = 1282.71 / ( ln( 666.09 / ( ( %c ) * ( %f - %f ) / ( 255.0 - 1.0 ) + %f ) + 1 ) ) - 273.15 - %d ), ( %c < ( %d / 5 * 4 ) )if( x = 0 ), ( %c >= ( %d / 5 * 4 ) )if( x = 255 ), ( ( %c < ( %d / 5 * 4 ) ) * ( %c >= ( %d / 5 * 3 ) ) )if( x = 255 * ( ( %c - ( %d / 5 * 3 ) ) / ( %d / 5 ) ) ), x";
    // fmt_g = "( v = %c - 1 ), ( v < 0 )if( v = 0 ), ( %c = 1282.71 / ( ln( 666.09 / ( ( %c ) * ( %f - %f ) / ( 255.0 - 1.0 ) + %f ) + 1 ) ) - 273.15 - %d ), ( %c < ( %d / 5 * 2 ) )if( y = 0 ), ( %c >= ( %d / 5 * 2 ) )if( y = 255 ), ( ( %c >= ( %d / 5 ) ) * ( %c < ( %d / 5 * 2 ) ) )if( y = 255 * ( ( %c - %d / 5 ) / ( %d / 5 ) ) ), ( ( %c >= ( %d / 5 * 4 ) ) )if( y = 255 - 255 * ( ( %c - ( %d / 5 * 4 ) ) / ( %d / 5 ) ) ), y";
    // fmt_b = "( v = %c - 1 ), ( v < 0 )if( v = 0 ), ( %c = 1282.71 / ( ln( 666.09 / ( ( %c ) * ( %f - %f ) / ( 255.0 - 1.0 ) + %f ) + 1 ) ) - 273.15 - %d ), ( %c < ( %d / 5 * 3 ) )if( z = 255 ), ( %c >= ( %d / 5 * 3 ) )if( z = 0 ), ( %c < ( %d / 5 ) )if( z = 255 * ( %c / ( %d / 5 ) ) ), ( ( %c >= ( %d / 5 * 2 ) ) * ( %c < ( %d / 5 * 3 ) ) )if( z = 255 - 255 * ( ( %c - ( %d / 5 * 2 ) ) / ( %d / 5 ) ) ), z";

    fmt_r = "( v = %c - 1 ), ( v < 0 )if( v = 0 ), ( %c = 1282.71 / ( ln( 666.09 / ( ( %c ) * ( %f - %f ) / ( 255.0 - 1.0 ) + %f ) + 1 ) ) - 273.15 - %d ), ( %c < ( %d / 5 * 4 ) )if( x = 0 ), ( %c >= ( %d / 5 * 4 ) )if( x = 255 ), ( ( %c < ( %d / 5 * 4 ) ) * ( %c >= ( %d / 5 * 3 ) ) )if( x = 255 * ( ( %c - ( %d / 5 * 3 ) ) / ( %d / 5 ) ) ), x";
    fmt_g = "( v = %c - 1 ), ( v < 0 )if( v = 0 ), ( %c = 1282.71 / ( ln( 666.09 / ( ( %c ) * ( %f - %f ) / ( 255.0 - 1.0 ) + %f ) + 1 ) ) - 273.15 - %d ), ( ( %c < 0 ) + ( %c > %d ) )if( %c = 0 ), ( %c < ( %d / 5 * 2 ) )if( y = 0 ), ( %c >= ( %d / 5 * 2 ) )if( y = 255 ), ( ( %c >= ( %d / 5 ) ) * ( %c < ( %d / 5 * 2 ) ) )if( y = 255 * ( ( %c - %d / 5 ) / ( %d / 5 ) ) ), ( ( %c >= ( %d / 5 * 4 ) ) )if( y = 255 - 255 * ( ( %c - ( %d / 5 * 4 ) ) / ( %d / 5 ) ) ), y";
    fmt_b = "( v = %c - 1 ), ( v < 0 )if( v = 0 ), ( %c = 1282.71 / ( ln( 666.09 / ( ( %c ) * ( %f - %f ) / ( 255.0 - 1.0 ) + %f ) + 1 ) ) - 273.15 - %d ), ( ( %c < 0 ) + ( %c > %d ) )if( %c = 0 ), ( %c < ( %d / 5 * 3 ) )if( z = 255 ), ( %c >= ( %d / 5 * 3 ) )if( z = 0 ), ( %c < ( %d / 5 ) )if( z = 255 * ( %c / ( %d / 5 ) ) ), ( ( %c >= ( %d / 5 * 2 ) ) * ( %c < ( %d / 5 * 3 ) ) )if( z = 255 - 255 * ( ( %c - ( %d / 5 * 2 ) ) / ( %d / 5 ) ) ), z";

    fmt_thm = "( v = %c - 1 ), ( v < 0 )if( v = 0 ), ( %c = 1282.71 / ( ln( 666.09 / ( ( %c ) * ( %f - %f ) / ( 255.0 - 1.0 ) + %f ) + 1 ) ) - 273.15 )";

    fmt_cb_r = "( w = a - %d ), ( %c < ( %d / 5 * 4 ) )if( x = 0 ), ( %c >= ( %d / 5 * 4 ) )if( x = 255 ), ( ( %c < ( %d / 5 * 4 ) ) * ( %c >= ( %d / 5 * 3 ) ) )if( x = 255 * ( ( %c - ( %d / 5 * 3 ) ) / ( %d / 5 ) ) ), x";
    fmt_cb_g = "( w = a - %d ), ( ( %c < 0 ) + ( %c > %d ) )if( %c = 0 ), ( %c < ( %d / 5 * 2 ) )if( y = 0 ), ( %c >= ( %d / 5 * 2 ) )if( y = 255 ), ( ( %c >= ( %d / 5 ) ) * ( %c < ( %d / 5 * 2 ) ) )if( y = 255 * ( ( %c - %d / 5 ) / ( %d / 5 ) ) ), ( ( %c >= ( %d / 5 * 4 ) ) )if( y = 255 - 255 * ( ( %c - ( %d / 5 * 4 ) ) / ( %d / 5 ) ) ), y";
    fmt_cb_b = "( w = a - %d ), ( ( %c < 0 ) + ( %c > %d ) )if( %c = 0 ), ( %c < ( %d / 5 * 3 ) )if( z = 255 ), ( %c >= ( %d / 5 * 3 ) )if( z = 0 ), ( %c < ( %d / 5 ) )if( z = 255 * ( %c / ( %d / 5 ) ) ), ( ( %c >= ( %d / 5 * 2 ) ) * ( %c < ( %d / 5 * 3 ) ) )if( z = 255 - 255 * ( ( %c - ( %d / 5 * 2 ) ) / ( %d / 5 ) ) ), z";

    /* sprintf�Ŏ������ */
    // r = r.sprintf( fmt_r, band6, band, 'v', lmax, lmin, lmin, min_temp, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, temp_width );
    // g = g.sprintf( fmt_g, band6, band, 'v', lmax, lmin, lmin, min_temp, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, temp_width, band, temp_width, band, temp_width, temp_width );
    // b = b.sprintf( fmt_b, band6, band, 'v', lmax, lmin, lmin, min_temp, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, temp_width );

    r = r.sprintf( fmt_r, band6, band, 'v', lmax, lmin, lmin, min_temp, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, temp_width );
    g = g.sprintf( fmt_g, band6, band, 'v', lmax, lmin, lmin, min_temp, band, band, temp_width, band, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, temp_width, band, temp_width, band, temp_width, temp_width );
    b = b.sprintf( fmt_b, band6, band, 'v', lmax, lmin, lmin, min_temp, band, band, temp_width, band, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, temp_width );

    cb = cb.sprintf( fmt_thm, 'a', band, 'v', lmax, lmin, lmin );

    // �J���[�o�[�p�̎�
    cb_r = cb_r.sprintf( fmt_cb_r, min_temp, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, temp_width );
    cb_g = cb_g.sprintf( fmt_cb_g, min_temp, band, band, temp_width, band, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, temp_width, band, temp_width, band, temp_width, temp_width );
    cb_b = cb_b.sprintf( fmt_cb_b, min_temp, band, band, temp_width, band, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, band, temp_width, temp_width );

    /* ���C���� */
    SatViewMainForm->ExpEditR->Text = r;
    SatViewMainForm->ExpEditG->Text = g;
    SatViewMainForm->ExpEditB->Text = b;

    SatViewMainForm->setColorBarExp( cb );

    /* �q�X�g�O�����ݒ���� */
    ( (struct REMOS_FRONT_BAND *)SatViewMainForm->list_band->Items[BandComboBox->ItemIndex] )->updown_bottom->Position = 0;
    ( (struct REMOS_FRONT_BAND *)SatViewMainForm->list_band->Items[BandComboBox->ItemIndex] )->updown_top->Position = 255;

    SatViewMainForm->UpdateBandBox( (struct REMOS_FRONT_BAND *)SatViewMainForm->list_band->Items[BandComboBox->ItemIndex] );

    unit_exp = unit_exp.sprintf( "a" );

	// �{�^������
    //this->Visible = false;
    this->BtnOk->Enabled = false;

    /* ���łɏ������� */
    SatViewMainForm->DrawImg();

    // �J���[�o�[�\��
    ColorBarForm->Caption = "���x";
    ColorBarForm->viewNowValue( true );
    ColorBarForm->setColorString( cb_r, cb_g, cb_b );
    ColorBarForm->setValueRange( min_temp, max_temp );
    ColorBarForm->setUnitString( unit_exp, "��" );
    ColorBarForm->Show();
    ColorBarForm->draw();

    /* �{�^�����A */
    this->BtnOk->Enabled = true;

    //Close();
#endif
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormLS_THM::MaxTempEditChange(TObject *Sender)
{
	int max;
    int min;

    try {
    	max = TempUpDown->Position;
        min = TempMinUpDown->Position;
    } catch (...) {
    	max = 0;
        min = 0;
    }

    LabelRed->Caption = IntToStr( max );
    LabelYellow->Caption = IntToStr( ( max - min ) / 5 * 4 + min );
    LabelGreen->Caption = IntToStr( ( max - min ) / 5 * 3 + min );
    LabelWater->Caption = IntToStr( ( max - min ) / 5 * 2 + min );
    LabelBlue->Caption = IntToStr( ( max - min ) / 5 * 1 + min );
    LabelBlack->Caption = IntToStr( min );
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormLS_THM::FormClose(TObject *Sender,
      TCloseAction &Action)
{
	// �J���[�o�[�������Ă���Ώ���
	if ( ColorBarForm->Visible ) {
    	// ColorBarForm->Close();
    }
}
//---------------------------------------------------------------------------


void __fastcall TPresetFormLS_THM::TypeRadioButton61Click(TObject *Sender)
{
	readSetting();	
}
//---------------------------------------------------------------------------

void __fastcall TPresetFormLS_THM::TypeRadioButton62Click(TObject *Sender)
{
	readSetting();
}
//---------------------------------------------------------------------------

