//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "PixValue.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPixForm *PixForm;
//---------------------------------------------------------------------------
__fastcall TPixForm::TPixForm(TComponent* Owner)
	: TForm(Owner)
{
	do_delete = false;
}
//---------------------------------------------------------------------------
void __fastcall TPixForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	// ïKóvÇ»ÇÁçÌèú
    if ( do_delete ) {
    	Action = caFree;
    }
}
//---------------------------------------------------------------------------

