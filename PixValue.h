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
__published:	// IDE �Ǘ��̃R���|�[�l���g
	TLabel *ValueLabel;
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// ���[�U�[�錾
public:		// ���[�U�[�錾
	bool do_delete;

	__fastcall TPixForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TPixForm *PixForm;
//---------------------------------------------------------------------------
#endif
