//---------------------------------------------------------------------------

#ifndef SaveFormUnitH
#define SaveFormUnitH
//---------------------------------------------------------------------------
#include "ebmp.h"
#include "ecalc.h"
#include "ecalc_jit.h"
#include "Main.h"
#include <math.h>
#include <Math.hpp>
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TSaveForm : public TForm
{
__published:	// IDE �Ǘ��̃R���|�[�l���g
	TRadioGroup *SaveModeRadioGroup;
	TButton *CancelButton;
	TButton *SaveButton;
	TProgressBar *SaveProgressBar;
	TSaveDialog *ImgSaveDialog;
	TLabel *TimeLabel;
	void __fastcall CancelButtonClick(TObject *Sender);
	void __fastcall SaveButtonClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
private:	// ���[�U�[�錾
    bool __fastcall SaveToBmp( AnsiString filename, int zpos );
public:		// ���[�U�[�錾
	__fastcall TSaveForm(TComponent* Owner);

    bool saving;
};
//---------------------------------------------------------------------------
extern PACKAGE TSaveForm *SaveForm;
//---------------------------------------------------------------------------
#endif
