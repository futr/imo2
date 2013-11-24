//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "PresetNDVI.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPresetFormNDVI *PresetFormNDVI;
//---------------------------------------------------------------------------
__fastcall TPresetFormNDVI::TPresetFormNDVI(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormNDVI::BtnCancelClick(TObject *Sender)
{
	Close();	
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormNDVI::BtnOkClick(TObject *Sender)
{
	/* ���C���t�H�[���̃o���h�G�f�B�b�g���������� */
    AnsiString r;
    AnsiString g;
    AnsiString b;

    ColorMap *map;

    char *fmt_r;

    char band_red;
    char band_ired;

    // map�ݒ�
    map = SatViewMainForm->color_map;

    /* �o���h�����l */
    // fmt_r = "( %c - %c ) / ( %c + %c ) * 128 + 128";
    fmt_r = "( %c - %c ) / ( %c + %c ) * 100";

    /* �R���{�{�b�N�X���L�����N�^�� */
    band_red  = RedComboBox->ItemIndex + 0x61;
    band_ired = NIRComboBox->ItemIndex + 0x61;

    /* sprintf�Ŏ������ */
    r = r.sprintf( fmt_r, band_ired, band_red, band_ired, band_red );
    g = r;
    b = r;

    /* ���C���� */
    /*
    SatViewMainForm->ExpEditR->Text = r;
    SatViewMainForm->ExpEditG->Text = g;
    SatViewMainForm->ExpEditB->Text = b;
    */

    // ����o�^
    map->setExpression( r );
    map->setUnitString( "[NDVI�~100]" );
    map->setRange( -100, 100 );

    // �F���쐬
    map->deleteAllColorLevel();

    map->addColorLevel( new ColorLevel(  100, 255, 255, 255 ) );
    map->addColorLevel( new ColorLevel( -100,   0,   0,   0 ) );

    map->setSmooth();

    // ���C���t�H�[�����J���[�}�b�v���[�h��
    SatViewMainForm->LevelDrawRadioButton->Checked = true;

    /* �q�X�g�O�����ݒ���� */
    SatViewMainForm->disableBandRange( (struct REMOS_FRONT_BAND *)SatViewMainForm->list_band->Items[RedComboBox->ItemIndex] );
    SatViewMainForm->disableBandRange( (struct REMOS_FRONT_BAND *)SatViewMainForm->list_band->Items[NIRComboBox->ItemIndex] );

    /* �����Ȃ��Ȃ� */
    this->Visible = false;
    this->BtnOk->Enabled = false;

    // ���C���t�H�[���������[�h��
    // SatViewMainForm->ExpDrawRadioButton->Checked = true;

    /* ���łɏ������� */
    SatViewMainForm->DrawImg();

    Close();
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormNDVI::FormShow(TObject *Sender)
{
	BtnOk->Enabled = true;
}
//---------------------------------------------------------------------------
