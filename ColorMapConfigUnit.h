//---------------------------------------------------------------------------

#ifndef ColorMapConfigUnitH
#define ColorMapConfigUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
#include "colormap.h"
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TColorMapConfigForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TImage *LevelImage;
	TButton *AddLevelButton;
	TButton *DeleteLevelButton;
	TEdit *LevelEdit;
	TUpDown *LevelEditUpDown;
	TPanel *LevelColorPanel;
	TLabel *Label1;
	TButton *SelectColorButton;
	TGroupBox *LevelImageBox;
	TGroupBox *LevelEditorGroupBox;
	TBevel *Bevel1;
	TGroupBox *PreViewGroupBox;
	TCheckBox *SmoothCheckBox;
	TLabel *Label2;
	TGroupBox *ExpGroupBox;
	TLabel *Label3;
	TEdit *ExpEdit;
	TLabel *Label4;
	TEdit *UnitStrEdit;
	TButton *CancelButton;
	TImage *PreviewImage;
	TColorDialog *LevelColorDialog;
	TCheckBox *StaticRatioCheckBox;
	TButton *SetButton;
	TButton *ClearButton;
	TGroupBox *GroupBox1;
	TComboBox *PreSetComboBox;
	TEdit *MinLevelEdit;
	TLabel *Label5;
	TLabel *Label6;
	TEdit *MaxLevelEdit;
	TLabel *Label7;
	TButton *SetPresetButton;
	TUpDown *MaxLevelUpDown;
	TUpDown *MinLevelUpDown;
	TLabel *Label8;
	TButton *CloseButton;
	TLabel *Label9;
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall ExpEditChange(TObject *Sender);
	void __fastcall UnitStrEditChange(TObject *Sender);
	void __fastcall SmoothCheckBoxClick(TObject *Sender);
	void __fastcall SelectColorButtonClick(TObject *Sender);
	void __fastcall DeleteLevelButtonClick(TObject *Sender);
	void __fastcall AddLevelButtonClick(TObject *Sender);
	void __fastcall LevelColorPanelClick(TObject *Sender);
	void __fastcall CancelButtonClick(TObject *Sender);
	void __fastcall SetButtonClick(TObject *Sender);
	void __fastcall LevelEditUpDownClick(TObject *Sender, TUDBtnType Button);
	void __fastcall ClearButtonClick(TObject *Sender);
	void __fastcall PreSetComboBoxDrawItem(TWinControl *Control, int Index,
          TRect &Rect, TOwnerDrawState State);
	void __fastcall SetPresetButtonClick(TObject *Sender);
	void __fastcall MinLevelEditChange(TObject *Sender);
	void __fastcall MaxLevelEditChange(TObject *Sender);
	void __fastcall PreSetComboBoxSelect(TObject *Sender);
	void __fastcall CloseButtonClick(TObject *Sender);
private:	// ユーザー宣言
	ColorMap *map;
    ColorMap *back_map;
    ColorMap m_map;

public:		// ユーザー宣言
	__fastcall TColorMapConfigForm(TComponent* Owner);

    void setColorMap( ColorMap *map );
    void drawLevelImage( void );
    void drawPreview( void );
    void updateView( void );
	void __fastcall onSelectLevelLabel( TObject *Sender );
    void selectColorLevel( int index );

    int selectLevelIndex;

    TList *levelLabels;

    ColorMap psBlackWhite;
    ColorMap psRainbow;
    ColorMap psHeight;
    ColorMap psRadio;
    ColorMap psWhiteBlack;
    ColorMap psRainbow2;
    ColorMap psGnu;

    bool updateEdit;
    bool updatePreset;
};
//---------------------------------------------------------------------------
extern PACKAGE TColorMapConfigForm *ColorMapConfigForm;
//---------------------------------------------------------------------------
#endif
