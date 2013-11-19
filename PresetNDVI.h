//---------------------------------------------------------------------------

#ifndef PresetNDVIH
#define PresetNDVIH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TPresetFormNDVI : public TForm
{
__published:	// IDE 管理のコンポーネント
	TButton *BtnOk;
	TButton *BtnCancel;
	TLabel *TitleLabel;
	TMemo *DetailMemo;
	TLabel *Label1;
	TComboBox *RedComboBox;
	TLabel *Label2;
	TComboBox *NIRComboBox;
	void __fastcall BtnCancelClick(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
private:	// ユーザー宣言
public:		// ユーザー宣言
	__fastcall TPresetFormNDVI(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TPresetFormNDVI *PresetFormNDVI;
//---------------------------------------------------------------------------
#endif
