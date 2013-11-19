//---------------------------------------------------------------------------

#ifndef HelpFormCodeH
#define HelpFormCodeH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class THelpForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TPanel *Panel1;
	TPanel *Panel2;
	TMemo *Memo1;
	TButton *Button1;
	void __fastcall Button1Click(TObject *Sender);
private:	// ユーザー宣言
public:		// ユーザー宣言
	__fastcall THelpForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE THelpForm *HelpForm;
//---------------------------------------------------------------------------
#endif
