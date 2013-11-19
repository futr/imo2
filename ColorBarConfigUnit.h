//---------------------------------------------------------------------------

#ifndef ColorBarConfigUnitH
#define ColorBarConfigUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TColorBarConfigForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TLabeledEdit *REdit;
	TLabeledEdit *GEdit;
	TLabeledEdit *BEdit;
	TLabeledEdit *UnitExpEdit;
	TLabeledEdit *UnitNameEdit;
	TLabeledEdit *BottomEdit;
	TLabeledEdit *TopEdit;
	TCheckBox *ViewNowCheckBox;
	TButton *CancelButton;
	TButton *Button1;
private:	// ユーザー宣言
public:		// ユーザー宣言
	__fastcall TColorBarConfigForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TColorBarConfigForm *ColorBarConfigForm;
//---------------------------------------------------------------------------
#endif
