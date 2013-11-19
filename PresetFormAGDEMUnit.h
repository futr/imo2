//---------------------------------------------------------------------------

#ifndef PresetFormAGDEMUnitH
#define PresetFormAGDEMUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TPresetFormAGDEM : public TForm
{
__published:	// IDE �Ǘ��̃R���|�[�l���g
	TLabel *TitleLabel;
	TMemo *DetailMemo;
	TComboBox *BandComboBox;
	TLabel *Label1;
	TButton *BtnCancel;
	TButton *BtnOk;
	TEdit *MaxEdit;
	TLabel *Label2;
	TUpDown *MaxUpDown;
	TEdit *MinEdit;
	TLabel *Label3;
	TUpDown *MinUpDown;
	void __fastcall BtnCancelClick(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
private:	// ���[�U�[�錾
public:		// ���[�U�[�錾
	__fastcall TPresetFormAGDEM(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TPresetFormAGDEM *PresetFormAGDEM;
//---------------------------------------------------------------------------
#endif
