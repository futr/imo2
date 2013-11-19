//---------------------------------------------------------------------------

#ifndef PixValueH
#define PixValueH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TPixForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TLabel *ValueLabel;
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// ユーザー宣言
public:		// ユーザー宣言
	bool do_delete;

	__fastcall TPixForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TPixForm *PixForm;
//---------------------------------------------------------------------------
#endif
