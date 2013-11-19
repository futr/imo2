//---------------------------------------------------------------------------

#ifndef ColorBarH
#define ColorBarH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include "colormap.h"
//---------------------------------------------------------------------------
#include "ecalc.h"
#include <math.h>
//---------------------------------------------------------------------------
class TColorBarForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TImage *ColorBarImage;
	void __fastcall FormResize(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
private:	// ユーザー宣言
	double bottom_value;
    double top_value;
    double now_value;

    AnsiString r_str;
    AnsiString g_str;
    AnsiString b_str;
    AnsiString unit_str;

    // 電卓
    struct ECALC_TOKEN *tok_r;
    struct ECALC_TOKEN *tok_g;
    struct ECALC_TOKEN *tok_b;
    struct ECALC_TOKEN *tok_unit;
    struct ECALC_TOKEN *tok_cm_unit;

    AnsiString unit;

    bool write_now_value;
    bool b_exp_mode;

    ColorMap *color_map;

public:		// ユーザー宣言
	void __fastcall setExpMode( bool mode = true );
    bool __fastcall getExpMode( void );
    void __fastcall setColorMap( ColorMap *map );
	void __fastcall viewNowValue( bool view = true );
    void __fastcall setColorString( AnsiString r, AnsiString g, AnsiString b );
    void __fastcall setUnitString( AnsiString exp, AnsiString unit );
    void __fastcall setValueRange( double bottom, double top );
    void __fastcall setNowValue( double now );
    void __fastcall draw( void );

	__fastcall TColorBarForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TColorBarForm *ColorBarForm;
//---------------------------------------------------------------------------
#endif
