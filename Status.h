//---------------------------------------------------------------------------

#ifndef StatusH
#define StatusH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TStatusForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TLabel *MessageLabel;
	void __fastcall FormShow(TObject *Sender);
private:	// ユーザー宣言
public:		// ユーザー宣言
	__fastcall TStatusForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TStatusForm *StatusForm;
//---------------------------------------------------------------------------
#endif
