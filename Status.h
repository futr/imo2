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
__published:	// IDE �Ǘ��̃R���|�[�l���g
	TLabel *MessageLabel;
	void __fastcall FormShow(TObject *Sender);
private:	// ���[�U�[�錾
public:		// ���[�U�[�錾
	__fastcall TStatusForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TStatusForm *StatusForm;
//---------------------------------------------------------------------------
#endif
