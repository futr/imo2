//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Status.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TStatusForm *StatusForm;
//---------------------------------------------------------------------------
__fastcall TStatusForm::TStatusForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TStatusForm::FormShow(TObject *Sender)
{
	/* メインフォームの真ん中へ */
    this->Left = SatViewMainForm->Left + SatViewMainForm->Width  / 2 - this->Width  / 2;
    this->Top  = SatViewMainForm->Top  + SatViewMainForm->Height / 2 - this->Height / 2;
}
//---------------------------------------------------------------------------

