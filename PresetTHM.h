//---------------------------------------------------------------------------

#ifndef PresetTHMH
#define PresetTHMH
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
	TComboBox *RedComboBox;
	TLabel *Label2;
	TComboBox *NIRComboBox;
	void __fastcall BtnCancelClick(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
private:	// ���[�U�[�錾
public:		// ���[�U�[�錾
	__fastcall TPresetFormTHM(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TPresetFormTHM *PresetFormTHM;
//---------------------------------------------------------------------------
#endif
