//---------------------------------------------------------------------------

#ifndef PresetHTMH
#define PresetHTMH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TPresetFormTHM : public TForm
{
__published:	// IDE �Ǘ��̃R���|�[�l���g
	TButton *BtnOk;
	TButton *BtnCancel;
	TLabel *TitleLabel;
	TMemo *DetailMemo;
	TLabel *Label1;
	TComboBox *BandComboBox;
	TRadioButton *BandRadioButton;
	TRadioButton *ExpRadioButton;
	TComboBox *ExpComboBox;
	TLabel *Label2;
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnCancelClick(TObject *Sender);
	void __fastcall BandRadioButtonClick(TObject *Sender);
	void __fastcall ExpRadioButtonClick(TObject *Sender);
private:	// ���[�U�[�錾
public:		// ���[�U�[�錾
	__fastcall TPresetFormTHM(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TPresetFormTHM *PresetFormTHM;
//---------------------------------------------------------------------------
#endif
