//---------------------------------------------------------------------------

#ifndef PresetLS8_THMH
#define PresetLS8_THMH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
//---------------------------------------------------------------------------
class TPresetFormLS8_THM : public TForm
{
__published:	// IDE 管理のコンポーネント
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TImage *Image1;
	TLabel *LabelRed;
	TLabel *Label4;
	TLabel *LabelYellow;
	TLabel *LabelGreen;
	TLabel *LabelWater;
	TLabel *LabelBlue;
	TLabel *LabelBlack;
	TLabel *Label5;
	TLabel *Label6;
	TButton *BtnOk;
	TButton *BtnCancel;
	TComboBox *BandComboBox;
	TEdit *k2Edit;
	TEdit *k1Edit;
	TEdit *MaxTempEdit;
	TUpDown *TempUpDown;
	TEdit *MinTempEdit;
	TUpDown *TempMinUpDown;
	TRadioButton *TypeRadioButton10;
	TRadioButton *TypeRadioButton11;
	TEdit *MLEdit;
	TEdit *ALEdit;
	TLabel *Label7;
	TLabel *Label8;
	void __fastcall BtnCancelClick(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall TypeRadioButton10Click(TObject *Sender);
	void __fastcall TypeRadioButton11Click(TObject *Sender);
private:	// ユーザー宣言
public:		// ユーザー宣言
	__fastcall TPresetFormLS8_THM(TComponent* Owner);

	void readSetting( void );

    double k1_10;
    double k1_11;
    double k2_10;
    double k2_11;
    double rad_add_10;
    double rad_add_11;
    double rad_mul_10;
    double rad_mul_11;
};
//---------------------------------------------------------------------------
extern PACKAGE TPresetFormLS8_THM *PresetFormLS8_THM;
//---------------------------------------------------------------------------
#endif
