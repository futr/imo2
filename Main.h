//---------------------------------------------------------------------------

#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <ToolWin.hpp>
//---------------------------------------------------------------------------
#include "ecalc.h"
#include "remos.h"
#include "colormap.h"
#include "geotiffrec.h"
#include "remos_alos_config.h"
#include <Dialogs.hpp>
#include <ImgList.hpp>
#include <Buttons.hpp>
#include <Menus.hpp>
#include <Graphics.hpp>
#include <AppEvnts.hpp>
#include "CSPIN.h"
//---------------------------------------------------------------------------
#define ZOOM_MAX 10
#define WPARAM_DIV 1000000.0
//---------------------------------------------------------------------------
struct REMOS_FRONT_BAND {					/* ビルダーでのバンドコンテナ */
	TImage *img_hist;
    TLabel *label_name;
    TLabel *label_bottom;
    TLabel *label_top;
    TEdit *edit_bottom;
    TEdit *edit_top;
    TUpDown *updown_bottom;
    TUpDown *updown_top;
    TPanel *panel_cont;

    TBitBtn *btn_close;
    TBitBtn *btn_blue;
    TBitBtn *btn_red;
    TBitBtn *btn_green;
    TBitBtn *btn_auto;
    TBitBtn *btn_wb;
    TBitBtn *btn_max;

    TPanel *panel_red;
    TPanel *panel_green;
    TPanel *panel_blue;

    unsigned char *line_buf;

    AnsiString *fln;

    int index;

    bool hist_click;
    bool hist_maked;

    bool canvas_mode;
    int canvas_color;

    Graphics::TBitmap *img_canvas;	/* canvasモード描画用 */

    TMouseButton hist_button;

    struct REMOS_BAND *band;
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class TSatViewMainForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TStatusBar *StatusBar;
	TPanel *BandPanel;
	TPanel *ImagePanel;
	TToolBar *ToolBar;
	TToolButton *TBOpen;
	TImageList *ButtonImageList;
	TOpenDialog *OpenDialog;
	TPanel *CalcPanel;
	TToolButton *TBSave;
	TToolButton *TBClose;
	TImage *SatImage;
	TScrollBox *BandList;
	TPanel *Panel1;
	TLabel *Label1;
	TImage *Image1;
	TPanel *Panel2;
	TPanel *Panel3;
	TPanel *Panel4;
	TEdit *ExpEditR;
	TEdit *ExpEditG;
	TEdit *ExpEditB;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TEdit *Edit1;
	TEdit *Edit2;
	TLabel *Label5;
	TLabel *Label6;
	TUpDown *UpDown1;
	TUpDown *UpDown2;
	TPanel *ZoomPanel;
	TToolButton *ToolButton1;
	TPanel *PanelImage;
	TPanel *PanelHorScr;
	TScrollBar *ScrImgVert;
	TScrollBar *ScrImgHor;
	TPanel *PanelSepa;
	TBitBtn *BitBtn1;
	TBitBtn *BitBtn2;
	TBitBtn *BitBtn3;
	TBitBtn *BitBtn4;
	TBitBtn *BitBtn5;
	TPanel *ButtonPanel;
	TPanel *ConfigPanel;
	TCheckBox *HistCheckBox;
	TCheckBox *ScrCheckBox;
	TMainMenu *MainMenu1;
	TMenuItem *F1;
	TBitBtn *BitBtn6;
	TBitBtn *BitBtn7;
	TLabel *Label7;
	TEdit *AutoRangeEdit;
	TImage *ZoomImage;
	TImage *ZoomUpImage;
	TPanel *ZoomCharPanel;
	TToolButton *TBSide;
	TMenuItem *MI_QUIT;
	TMenuItem *MI_OPEN;
	TMenuItem *MI_SAVE;
	TMenuItem *N2;
	TMenuItem *MI_CLOSE;
	TMenuItem *N1;
	TButton *BtnDraw;
	TPanel *Panel5;
	TImage *Image3;
	TApplicationEvents *ApplicationEvents;
	TMenuItem *V1;
	TMenuItem *MI_V_BAND;
	TMenuItem *MI_V_TOOL;
	TMenuItem *MI_V_ZOOM;
	TMenuItem *MI_V_STATUS;
	TMenuItem *N3;
	TMenuItem *N4;
	TMenuItem *W1;
	TMenuItem *MI_W_LT;
	TMenuItem *MI_W_RT;
	TMenuItem *MI_W_LB;
	TMenuItem *MI_W_RB;
	TMenuItem *MI_V_CONF;
	TMenuItem *N5;
	TCheckBox *AutoRangeCheckBox;
	TMenuItem *H1;
	TMenuItem *MI_H_ABOUT;
	TCheckBox *HistReduceMaxCheckBox;
	TToolButton *TBScr;
	TToolButton *TBLt;
	TToolButton *TBRt;
	TToolButton *TBLb;
	TToolButton *TBRb;
	TToolButton *ToolButton7;
	TToolButton *ToolButton8;
	TToolButton *ToolButton2;
	TToolButton *ToolButton3;
	TToolButton *ToolButton5;
	TToolButton *ToolButton6;
	TToolButton *ToolButton9;
	TToolButton *ToolButton10;
	TMenuItem *N6;
	TMenuItem *MI_W_T;
	TMenuItem *MI_W_B;
	TMenuItem *MI_W_L;
	TMenuItem *MI_W_R;
	TMenuItem *N11;
	TToolButton *TBUpdate;
	TBitBtn *BtnColor;
	TPopupMenu *ColorMenu;
	TMenuItem *MI_C_NDVI;
	TMenuItem *MI_C_TM;
	TMenuItem *MI_C_LS_THM;
	TToolButton *ToolButton4;
	TToolButton *TBMonosasi;
	TToolButton *TBStamp;
	TRadioButton *ExpDrawRadioButton;
	TRadioButton *LevelDrawRadioButton;
	TButton *ConfigLevelButton;
	TButton *ShowLevelColorBarButton;
	TMenuItem *MI_C_GDEM;
	TPanel *DrawModePanel;
	TPanel *DrawButtonPanel;
	TPanel *SettingStrPanel;
	TToolButton *TBCurpos;
	TMenuItem *MI_C_LS8_THM;
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall TBOpenClick(TObject *Sender);
	void __fastcall TBSaveClick(TObject *Sender);
	void __fastcall TBCloseClick(TObject *Sender);
	void __fastcall SatImageMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SatImageMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SatImageMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
	void __fastcall ScrImgHorScroll(TObject *Sender, TScrollCode ScrollCode,
          int &ScrollPos);
	void __fastcall ScrImgVertScroll(TObject *Sender, TScrollCode ScrollCode,
          int &ScrollPos);
	void __fastcall ExpEditRChange(TObject *Sender);
	void __fastcall ExpEditBChange(TObject *Sender);
	void __fastcall ExpEditGChange(TObject *Sender);
	void __fastcall ZoomImageMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
	void __fastcall ZoomImageMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall ZoomImageMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall TBSideClick(TObject *Sender);
	void __fastcall MI_QUITClick(TObject *Sender);
	void __fastcall MI_OPENClick(TObject *Sender);
	void __fastcall MI_SAVEClick(TObject *Sender);
	void __fastcall MI_CLOSEClick(TObject *Sender);
	void __fastcall BtnDrawClick(TObject *Sender);
	void __fastcall MI_V_BANDClick(TObject *Sender);
	void __fastcall MI_V_TOOLClick(TObject *Sender);
	void __fastcall V1Click(TObject *Sender);
	void __fastcall MI_V_ZOOMClick(TObject *Sender);
	void __fastcall MI_V_STATUSClick(TObject *Sender);
	void __fastcall ZoomUpImageMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall Image3MouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall MI_W_LTClick(TObject *Sender);
	void __fastcall MI_W_RTClick(TObject *Sender);
	void __fastcall MI_W_LBClick(TObject *Sender);
	void __fastcall MI_W_RBClick(TObject *Sender);
	void __fastcall ZoomPanelResize(TObject *Sender);
	void __fastcall MI_V_CONFClick(TObject *Sender);
	void __fastcall MI_H_ABOUTClick(TObject *Sender);
	void __fastcall HistReduceMaxCheckBoxClick(TObject *Sender);
	void __fastcall UpDown3MouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall TBScrClick(TObject *Sender);
	void __fastcall TBLtClick(TObject *Sender);
	void __fastcall TBRtClick(TObject *Sender);
	void __fastcall TBLbClick(TObject *Sender);
	void __fastcall TBRbClick(TObject *Sender);
	void __fastcall MI_W_TClick(TObject *Sender);
	void __fastcall MI_W_BClick(TObject *Sender);
	void __fastcall MI_W_LClick(TObject *Sender);
	void __fastcall MI_W_RClick(TObject *Sender);
	void __fastcall ToolButton3Click(TObject *Sender);
	void __fastcall ToolButton5Click(TObject *Sender);
	void __fastcall ToolButton6Click(TObject *Sender);
	void __fastcall ToolButton9Click(TObject *Sender);
	void __fastcall BtnColorClick(TObject *Sender);
	void __fastcall TBUpdateClick(TObject *Sender);
	void __fastcall MI_C_NDVIClick(TObject *Sender);
	void __fastcall MI_C_TMClick(TObject *Sender);
	void __fastcall MI_C_LS_THMClick(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall TBMonosasiClick(TObject *Sender);
	void __fastcall TBStampClick(TObject *Sender);
	void __fastcall ExpDrawRadioButtonClick(TObject *Sender);
	void __fastcall LevelDrawRadioButtonClick(TObject *Sender);
	void __fastcall ConfigLevelButtonClick(TObject *Sender);
	void __fastcall ShowLevelColorBarButtonClick(TObject *Sender);
	void __fastcall MI_C_GDEMClick(TObject *Sender);
	void __fastcall TBCurposClick(TObject *Sender);
	void __fastcall MI_C_LS8_THMClick(TObject *Sender);
private:	// ユーザー宣言
public:
	TList *list_file;				// ファイル用リスト
    TList *list_band;				// バンド用リスト

    // レベルスライス用
    ColorMap *color_map;

    /* 電卓ユニット */
    struct ECALC_TOKEN *tok_r;
    struct ECALC_TOKEN *tok_g;
    struct ECALC_TOKEN *tok_b;
    struct ECALC_TOKEN *tok_color_bar;

    AnsiString color_bar_exp;

    /* 画像 */
    Graphics::TBitmap *img_back;	/* バックバッファ */

    /* 各値 */
    int img_w;						/* 画像はば */
    int img_h;						/* 画像高さ */

    /* フラグ */
    bool b_open;					/* 開かれている */
    bool b_click;					/* クリックされた */
    bool b_move;					/* カーソル動いた */
    bool b_drawing;					/* 描画中 */
    bool b_zoom_click;  			/* ズームクリック中 */
    bool b_canvas;					/* Canvasモード使用中 */
    bool b_sync;					/* スクロールシンクロ用 */
    bool b_draw;					/* 描画許可 */

    bool b_mode_length;				// ものさしモード
    bool b_mode_stamp_cb; 			// スタンプモード
    bool b_draw_mode_exp;			// 式描画モード


    bool b_config;     				/* 設定ファイル読み込み済み */
    bool b_config_land;				/* LANDSAT用 */
    bool b_config_resolution;		// 解像度設定
    bool b_config_utm;				// UTMモードの座標
    bool b_config_ls7;				// Landsat7である
    bool b_config_ls8;				// Landsat8である
    bool b_config_alos;				// だいちである。
    bool b_config_date;				// 日付の設定
    bool b_config_latlon_utm_proj;	// 座標指定はlatlonだけどUTM投影

    GeotiffRec config_utm;			// UTM用の設定
    AnsiString config_date_str;		// 日付文字列

    UINT umsg_x;					/* プロセス通信用 */
    UINT umsg_y;
    UINT umsg_lat;
    UINT umsg_lon;
    UINT umsg_mag;
    UINT umsg_connect;
    UINT umsg_draw;

    int msg_buf_x;
    int msg_buf_y;
    double msg_buf_lat;
    double msg_buf_lon;
    int msg_buf_mode;				/* MODE0がXYモード */

    int cp_vert;
    int cp_hor;
    int cp_x;
    int cp_y;

    int zoom_pos;
    int zoom_pos_click;
    int zoom_pos_bef;

    int img_start_x_line;
    int img_start_y_line;
    int img_cent_x_line;
    int img_cent_y_line;
    int line_add_x;
    int line_add_y;

    double land_lmax;
    double land_lmin;
    double land_qcal_min;
    double land_qcal_max;

    double land_lmax_2;
    double land_lmin_2;
    double land_qcal_min_2;
    double land_qcal_max_2;

    double land_8_k1_10;
    double land_8_k1_11;
    double land_8_k2_10;
    double land_8_k2_11;
    double land_8_rad_add_10;
    double land_8_rad_add_11;
    double land_8_rad_mul_10;
    double land_8_rad_mul_11;

    double conf_lt_lat;
    double conf_lt_lon;
    double conf_rt_lat;
    double conf_rt_lon;
    double conf_lb_lat;
    double conf_lb_lon;
    double conf_rb_lat;
    double conf_rb_lon;

    double conf_lt_e;
    double conf_lt_n;
    double conf_rt_e;
    double conf_rt_n;
    double conf_lb_e;
    double conf_lb_n;
    double conf_rb_e;
    double conf_rb_n;
    AnsiString conf_zone_str;

    remos::AlosConfig conf_alos;

    double conf_resolution_x;
    double conf_resolution_y;

    // 設定用
    void clearSetting( void );
    void setSettingStr( void );

    void setWindowTitle( AnsiString filename );

    /* 経度緯度を文字列でくれる */
    AnsiString GetLon( int x, int y );
    AnsiString GetLat( int x, int y );

    // カラーバー用
    void __fastcall setColorBarExp( AnsiString exp );

    /* ヒント処理 */
    void __fastcall ShowHint( TObject *sender );

    /* D&D用 */
    void __fastcall OnDropFiles( TWMDropFiles msg );

    /* メッセージ */
    void __fastcall AppMessage( tagMSG &msg, bool &handled );

    /* 指定バンドボックスのレンジ設定解除 */
    void __fastcall disableBandRange( struct REMOS_FRONT_BAND *box );

    /* バンドコンテナ */
    struct REMOS_FRONT_BAND *MakeBandBox( struct REMOS_BAND *band, AnsiString fln, int index );	/* バンドボックス作成 */
    void FreeBandBox( struct REMOS_FRONT_BAND *box );											/* バンドボックス開放 */
    void UpdateBandBox( struct REMOS_FRONT_BAND *box );
    void DrawHist( REMOS_FRONT_BAND *box );
    void MakeHist( struct REMOS_FRONT_BAND *box );
    unsigned char GetPixel( struct REMOS_FRONT_BAND *box, int pos );
    void GetLineData( struct REMOS_FRONT_BAND *box, unsigned char *buf, int line, int from, int count );
    unsigned char GetUCharValue( double value );
    void ReadBandLineBuf( struct REMOS_FRONT_BAND *band, int line, int pos, int count );
    double ZoomPosToMag( int zoom_pos );
    double GetBandPixel( struct REMOS_FRONT_BAND *band, int pos );

    void syncPos();

    void __fastcall OpenFiles( void );

    void __fastcall DisplayHint(TObject *Sender);
    void __fastcall PleaseClick( void );
    void __fastcall DrawZoomBox( int X, int Y );
    void __fastcall DrawMiniMap( int vert, int hor );

    void __fastcall BandCloseBtnClick( TObject *Sender );
    void __fastcall ImageMouseWheelEvent( TObject *Sender, TShiftState Shift,
    	int WheelDelta, const TPoint &MousePos, bool &Handled );
	void __fastcall TSatViewMainForm::HistMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall TSatViewMainForm::HistMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall TSatViewMainForm::HistMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y);
    void __fastcall TSatViewMainForm::BandEditUpChange(TObject *Sender);
    void __fastcall TSatViewMainForm::BandEditDownChange(TObject *Sender);
    void __fastcall TSatViewMainForm::BlueBtnClick(TObject *Sender);
    void __fastcall TSatViewMainForm::RedBtnClick(TObject *Sender);
    void __fastcall TSatViewMainForm::GreenBtnClick(TObject *Sender);
    void __fastcall TSatViewMainForm::AutoBtnClick(TObject *Sender);
    void __fastcall TSatViewMainForm::WBBtnClick(TObject *Sender);
    void __fastcall TSatViewMainForm::MaxBtnClick(TObject *Sender);

public:		// ユーザー宣言
	__fastcall TSatViewMainForm(TComponent* Owner);


    /* 描画 */
    void __fastcall DrawImg( void );
    void __fastcall DrawImg( TImage *screen, Graphics::TBitmap *back_screen, bool drawModeExp, int zoompos, int img_cent_x, int img_cent_y, ColorMap *cmap );
};
//---------------------------------------------------------------------------
extern PACKAGE TSatViewMainForm *SatViewMainForm;
//---------------------------------------------------------------------------
#endif
