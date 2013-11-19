//---------------------------------------------------------------------------

#ifndef PresetLS_THMH
#define PresetLS_THMH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TPresetFormLS_THM : public TForm
{
__published:	// IDE 管理のコンポーネント
	TLabel *TitleLabel;
	TLabel *Label1;
	TButton *BtnOk;
	TButton *BtnCancel;
	TMemo *DetailMemo;
	TComboBox *BandComboBox;
	TEdit *LMAXEdit;
	TEdit *LMINEdit;
	TLabel *Label2;
	TLabel *Label3;
	TImage *Image1;
	TLabel *LabelRed;
	TEdit *MaxTempEdit;
	TLabel *Label4;
	TLabel *LabelYellow;
	TLabel *LabelGreen;
	TLabel *LabelWater;
	TLabel *LabelBlue;
	TLabel *LabelBlack;
	TUpDown *TempUpDown;
	TEdit *MinTempEdit;
	TUpDown *TempMinUpDown;
	TLabel *Label5;
	TRadioButton *TypeRadioButton61;
	TLabel *Label6;
	TRadioButton *TypeRadioButton62;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnCancelClick(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall MaxTempEditChange(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall TypeRadioButton61Click(TObject *Sender);
	void __fastcall TypeRadioButton62Click(TObject *Sender);
private:	// ユーザー宣言
public:		// ユーザー宣言
	void readSetting( void );

	double lmin_61;
    double lmax_61;
    double lmin_62;
    double lmax_62;

	__fastcall TPresetFormLS_THM(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TPresetFormLS_THM *PresetFormLS_THM;
//---------------------------------------------------------------------------
#endif
