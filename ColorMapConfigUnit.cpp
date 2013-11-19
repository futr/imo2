//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ColorMapConfigUnit.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TColorMapConfigForm *ColorMapConfigForm;
//---------------------------------------------------------------------------
__fastcall TColorMapConfigForm::TColorMapConfigForm(TComponent* Owner)
	: TForm(Owner)
{
	// 初期化
    selectLevelIndex = -1;

    // ラベルリスト初期化
    levelLabels = new TList;

    updateEdit = false;
    updatePreset = true;

    // ポインタ初期化
    map = &m_map;

    m_map.deleteAllColorLevel();

    // カラーマップのプリセットを作る

    // 虹色
    psRainbow.deleteAllColorLevel();
    psRainbow.addColorLevel( new ColorLevel( 5, 255, 0, 0 ) );
    psRainbow.addColorLevel( new ColorLevel( 4, 255, 255,   0 ) );
    psRainbow.addColorLevel( new ColorLevel( 3,   0, 255,   0 ) );
    psRainbow.addColorLevel( new ColorLevel( 2,   0, 255, 255 ) );
    psRainbow.addColorLevel( new ColorLevel( 1,   0,   0, 255 ) );
    psRainbow.addColorLevel( new ColorLevel( 0,   0,   0,   0 ) );
    psRainbow.setSmooth();

    // 白黒
    psBlackWhite.deleteAllColorLevel();
    psBlackWhite.addColorLevel( new ColorLevel( 0,   0,   0,   0 ) );
    psBlackWhite.addColorLevel( new ColorLevel( 1, 255, 255, 255 ) );
    psBlackWhite.setSmooth();

    // 放射輝度みたいなやつ
    psRadio.deleteAllColorLevel();
    psRadio.addColorLevel( new ColorLevel( 3, 255, 255, 255 ) );
    psRadio.addColorLevel( new ColorLevel( 2, 255, 255,   0 ) );
    psRadio.addColorLevel( new ColorLevel( 1, 255,   0,   0 ) );
    psRadio.addColorLevel( new ColorLevel( 0,   0,   0,   0 ) );
    psRadio.setSmooth();

    // 黒白
    psWhiteBlack.deleteAllColorLevel();
    psWhiteBlack.addColorLevel( new ColorLevel( 1,   0,   0,   0 ) );
    psWhiteBlack.addColorLevel( new ColorLevel( 0, 255, 255, 255 ) );
    psWhiteBlack.setSmooth();

    // 虹色2
    psRainbow2.deleteAllColorLevel();
    psRainbow2.addColorLevel( new ColorLevel( 4, 255,   0,   0 ) );
    psRainbow2.addColorLevel( new ColorLevel( 3, 255, 255,   0 ) );
    psRainbow2.addColorLevel( new ColorLevel( 2,   0, 255,   0 ) );
    psRainbow2.addColorLevel( new ColorLevel( 1,   0, 255, 255 ) );
    psRainbow2.addColorLevel( new ColorLevel( 0,   0,   0, 255 ) );
    psRainbow2.setSmooth();

    // GNUっぽいの
    psGnu.deleteAllColorLevel();
    psGnu.addColorLevel( new ColorLevel( 3, 255, 255,   0 ) );
    psGnu.addColorLevel( new ColorLevel( 2, 255,   0,   0 ) );
    psGnu.addColorLevel( new ColorLevel( 1, 128,   0, 255 ) );
    psGnu.addColorLevel( new ColorLevel( 0,   0,   0,   0 ) );
    psGnu.setSmooth();

    // リストに登録 ( c-styleのcastを使っている上に、かなり凶悪 )
    PreSetComboBox->Items->AddObject( "黒白", (TObject *)&psBlackWhite );
    PreSetComboBox->Items->AddObject( "白黒", (TObject *)&psWhiteBlack );
    PreSetComboBox->Items->AddObject( "虹色", (TObject *)&psRainbow );
    PreSetComboBox->Items->AddObject( "虹色２", (TObject *)&psRainbow2 );
    PreSetComboBox->Items->AddObject( "黒赤黄白", (TObject *)&psRadio );
    PreSetComboBox->Items->AddObject( "GNU", (TObject *)&psGnu );

    // 0番目選択
    PreSetComboBox->ItemIndex = 0;
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::FormDestroy(TObject *Sender)
{
	// 解放
    int i;

    // ラベルリスト解放
    for ( i = 0; i < levelLabels->Count; i++ ) {
    	delete (TLabel *)levelLabels->Items[i];
    }

    delete levelLabels;
}
//---------------------------------------------------------------------------
void TColorMapConfigForm::setColorMap( ColorMap *map )
{
	// カラーマップを設定

    // 更新禁止
    updatePreset = false;

    m_map = *map;
    back_map = map;

    // 最大値設定
    if ( m_map.getColorLevelCount() > 0 ) {
    	MinLevelEdit->Text = FloatToStr( m_map.getColorLevel( 0 )->getLevel() );
    	MaxLevelEdit->Text = FloatToStr( m_map.getColorLevel( m_map.getColorLevelCount() - 1 )->getLevel() );
    }

    // 更新許可
    updatePreset = true;

    // 表示更新
    updateView();
}
//---------------------------------------------------------------------------
void TColorMapConfigForm::updateView( void )
{
	// 表示を更新する
    int i;
    TLabel *newLabel;
    ColorLevel *clevel;

    // ラベルリスト破壊
    for ( i = 0; i < levelLabels->Count; i++ ) {
    	delete (TLabel *)levelLabels->Items[i];
    }

    levelLabels->Clear();

    // ラベルリスト追加
    for ( i = 0; i < map->getColorLevelCount(); i++ ) {
    	// 対応カラーレベル取得
        clevel = map->getColorLevel( i );

    	// ラベル作成
    	newLabel = new TLabel( this );

        // ラベル初期化
    	newLabel->Parent = LevelImageBox;
        newLabel->Tag = i;
        newLabel->OnClick = onSelectLevelLabel;

        // ラベル設定
        newLabel->Caption = FloatToStr( clevel->getLevel() );
        newLabel->Color = clWhite;
        newLabel->Font->Size = 13;
        newLabel->Width = 60;
        newLabel->Height = 20;

        // ラベル位置計算
        newLabel->Left = LevelImage->Left + LevelImage->Width + 5;
        newLabel->Top  = LevelImage->Top - newLabel->Height / 2 + ( (double)LevelImage->Height / map->getColorLevelCount() * ( map->getColorLevelCount() - i - 1 ) );

    	// ラベルリストに登録
        levelLabels->Add( newLabel );
    }

    // 式などを読み込み
    ExpEdit->Text = map->getExpression();
    UnitStrEdit->Text = map->getUnitString();

    // なめらかモード読み込み
    SmoothCheckBox->Checked = map->getSmooth();

    // レベルを選択する
    if ( map->getColorLevelCount() == 0 ) {
    	// ひとつもなければ無効な選択
    	selectColorLevel( -1 );
    } else {
    	// 現在のものを選択
        selectColorLevel( selectLevelIndex );
    }

    // レベルイメージ再描画
    drawLevelImage();

    // プレビュー再描画
    drawPreview();
}
//---------------------------------------------------------------------------
void TColorMapConfigForm::drawLevelImage( void )
{
	// レベルイメージを書く
    int i;
    int index;
    int under_index;
    ColorLevel *upper_l;
    ColorLevel *under_l;
    TColor level_color;
    double ratio;
    double level;

    // 登録数0なら真っ白にする
    if ( map->getColorLevelCount() == 0 ) {
    	LevelImage->Canvas->Brush->Color = clWhite;
        LevelImage->Canvas->FillRect( Rect( 0, 0, LevelImage->Width, LevelImage->Height ) );

        return;
    }

    // すべての位置について
    for ( i = 0; i < LevelImage->Height; i++ ) {
    	// この位置での対象インデックスを取得
        index = map->getColorLevelCount() - 1 - (int)( i / ( (double)LevelImage->Height / map->getColorLevelCount() ) );

        // 下のインデックスを取得
        under_index = map->getColorLevelCount() - 2 - (int)( i / ( (double)LevelImage->Height / map->getColorLevelCount() ) );

        // 現在位置の分割に対する比 ( *1000することで精度を上げている )
        ratio = ( ( i * 1000 ) % (int)( 1000.0 * LevelImage->Height / map->getColorLevelCount() ) ) / ( 1000 * (double)LevelImage->Height / map->getColorLevelCount() );

        // 上下の色レベル取得
        upper_l = map->getColorLevel( index );
        under_l = map->getColorLevel( under_index );

        // 画像上のレベルを取得
        if ( map->getSmooth() ) {
        	// スムースあり
            if ( under_l != NULL ) {
            	// 下レベルがある場合
                level = upper_l->getLevel() - ( upper_l->getLevel() - under_l->getLevel() ) * ratio;
            } else {
            	// 下レベルがない場合
                level = upper_l->getLevel();
            }
        } else {
        	// スムースなし
            level = upper_l->getLevel();
        }

        // 色取得
        map->setValue( level );
        map->makeColor();

        // 描画
        LevelImage->Canvas->Pen->Color = (TColor)RGB( map->getR(), map->getG(), map->getB() );
        LevelImage->Canvas->Pen->Width = 1;

        LevelImage->Canvas->MoveTo( 0, i );
        LevelImage->Canvas->LineTo( LevelImage->Width, i );
    }

    // 分割位置の線を書く
    for ( i = 0; i < map->getColorLevelCount() - 1; i++ ) {
    	int ypos;

        ypos = LevelImage->Height / map->getColorLevelCount() * ( i + 1 );

        LevelImage->Canvas->Pen->Color = clRed;
        LevelImage->Canvas->Pen->Width = 3;

        LevelImage->Canvas->MoveTo( 0, ypos );
        LevelImage->Canvas->LineTo( LevelImage->Width, ypos );
    }
}
//---------------------------------------------------------------------------
void TColorMapConfigForm::drawPreview( void )
{
	// プレビューを描画
    int zoom;

    double ratio;

    // 最適なズームを探索
    ratio = SatViewMainForm->img_w / (double)PreviewImage->Width;

    if ( ratio > 1 ) {
    	zoom = 2;

    	while ( zoom < ratio ) {
    		zoom *= 2;
    	}

        zoom /= 2;
    } else {
    	zoom = 0;
    }

    // 関数の求める形に合わせる
    zoom = ZOOM_MAX / 2 + zoom;

    if ( zoom > ZOOM_MAX ) {
    	zoom = ZOOM_MAX;
    }

    SatViewMainForm->DrawImg( PreviewImage, NULL, false, zoom, SatViewMainForm->img_w / 2, SatViewMainForm->img_h / 2, map );
}
//---------------------------------------------------------------------------
void TColorMapConfigForm::selectColorLevel( int index )
{
	// カラーレベルを選ぶ
    int i;
    ColorLevel *clevel;

    // ラベルがあればすべて無効
    for ( i = 0; i < levelLabels->Count; i++ ) {
    	// ラベル取得
    	TLabel *llabel = (TLabel *)levelLabels->Items[i];

        // ラベルの色を白に
        llabel->Color = clWhite;
    }

    // 無効インデックスなら操作不能
    if ( index < 0 || index >= map->getColorLevelCount() ) {
    	LevelEdit->Enabled = false;
        LevelEditUpDown->Enabled = false;
        SelectColorButton->Enabled = false;
        DeleteLevelButton->Enabled = false;
        SetButton->Enabled = false;
        AddLevelButton->Enabled = true;

        selectLevelIndex = -1;

        return;
    } else {
    	LevelEdit->Enabled = true;
        LevelEditUpDown->Enabled = true;
        SelectColorButton->Enabled = true;
        DeleteLevelButton->Enabled = true;
        SetButton->Enabled = true;
        AddLevelButton->Enabled = true;

        selectLevelIndex = index;

        // ラベルがあれば選択
        if ( index < levelLabels->Count ) {
        	TLabel *llabel = (TLabel *)levelLabels->Items[selectLevelIndex];

            // ラベルの色を変える
            llabel->Color = clAqua;
        }
    }

    // カラーレベル取得
    clevel = map->getColorLevel( selectLevelIndex );

    // エディタ更新開始
    updateEdit = true;

    // コンポーネントを設定
    LevelEdit->Text = FloatToStr( clevel->getLevel() );
    LevelEditUpDown->Position = clevel->getLevel();

    // エディタ更新中止
    updateEdit = false;

    LevelColorPanel->Color = (TColor)RGB( clevel->getR(), clevel->getG(), clevel->getB() );
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::onSelectLevelLabel( TObject *Sender )
{
	// レベルラベルがクリックされた時のイベント
    TLabel *llabel = (TLabel *)(Sender);

    selectColorLevel( llabel->Tag );
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::ExpEditChange(TObject *Sender)
{
	// 式文字列を設定
    map->setExpression( ExpEdit->Text );
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::UnitStrEditChange(TObject *Sender)
{
	// 単位文字列を設定
    map->setUnitString( UnitStrEdit->Text );
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::SmoothCheckBoxClick(TObject *Sender)
{
	// なめらかモード設定
    if ( SmoothCheckBox->Checked ) {
    	map->setSmooth( true );
    } else {
    	map->setSmooth( false );
    }

    // 更新
    updateView();
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::SelectColorButtonClick(
      TObject *Sender)
{
	// 色選択
    if ( LevelColorDialog->Execute() ) {
    	// 現在選択されているレベル取得
    	ColorLevel *clevel;
        TColor color = LevelColorDialog->Color;
        unsigned char *cpcolor = (unsigned char *)&color;

        clevel = map->getColorLevel( selectLevelIndex );

        clevel->setColor( cpcolor[0], cpcolor[1], cpcolor[2] );
        LevelColorPanel->Color = color;
    }

    // 更新
    updateView();
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::DeleteLevelButtonClick(
      TObject *Sender)
{
	// 現在選択されているレベルを削除する
    map->deleteColorLevel( selectLevelIndex );

    // 更新
    updateView();
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::AddLevelButtonClick(TObject *Sender)
{
	// 新たなレベルを追加
    double max_level;

    // 最大のレベルを作成
    if ( map->getColorLevelCount() != 0 ) {
    	// すでにある場合
        max_level = map->getColorLevel( map->getColorLevelCount() - 1 )->getLevel() + 1;
    } else {
    	// ない場合
        max_level = 0;
    }

    // 追加
    map->addColorLevel( new ColorLevel( max_level, 0, 0, 0 ) );

    // 最大を選択
    selectLevelIndex = map->getColorLevelCount() - 1;

    // 更新
    updateView();
}
//---------------------------------------------------------------------------




void __fastcall TColorMapConfigForm::LevelColorPanelClick(TObject *Sender)
{
	// 色選択
    if ( SelectColorButton->Enabled ) {
    	SelectColorButton->Click();
    }
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::CancelButtonClick(TObject *Sender)
{
	// 設定
    *back_map = m_map;

	// 再描画させて閉じる
    SatViewMainForm->DrawImg();

	Close();	
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::SetButtonClick(TObject *Sender)
{
	// レベル変更
    double level;
    AnsiString before_str;

	// 更新中なら何もしない
    /*
    if ( updateEdit ) {
    	return;
    }
    */

    // 不正な値なら何もしない
    try {
    	level = LevelEdit->Text.ToDouble();
    } catch (...) {
        return;
    }

	// レベル値の変更
    ColorLevel *clevel;

    // カラーレベル取得
    clevel = map->getColorLevel( selectLevelIndex );

    // NULLなら何もしない
    if ( clevel == NULL ) {
    	return;
    }

    // 必要なら比率を保つ
    if ( StaticRatioCheckBox->Checked ) {
    	if ( selectLevelIndex == 0 ) {
        	map->setBottomLevel( level );
        } else if ( selectLevelIndex == map->getColorLevelCount() - 1 ) {
        	map->setTopLevel( level );
        } else {
        	clevel->setLevel( level );
        }
    } else {
    	// 値設定
    	clevel->setLevel( level );
    }

    // アップダウン設定
    // LevelEditUpDown->Position = clevel->getLevel();

    // 再ソート
    map->sortLevel();

    // インデックスを更新
    selectLevelIndex = map->getColorLevelIndex( clevel );

    // 更新
    updateView();
}
//---------------------------------------------------------------------------


void __fastcall TColorMapConfigForm::LevelEditUpDownClick(TObject *Sender,
      TUDBtnType Button)
{
	// エディタの値を更新
	LevelEdit->Text = LevelEditUpDown->Position;

    Application->ProcessMessages();
    //Application->MessageBoxA( AnsiString( LevelEditUpDown->Position ).c_str(), "", MB_OK );
	SetButton->Click();
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::ClearButtonClick(TObject *Sender)
{
	// レベルを全て削除
    while ( map->getColorLevelCount() ) {
    	map->deleteColorLevel( 0 );
    }

    // 文字列再設定
    ExpEdit->Text = "a";
    UnitStrEdit->Text = "単位";

    // 更新
    this->updateView();
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::PreSetComboBoxDrawItem(
      TWinControl *Control, int Index, TRect &Rect, TOwnerDrawState State)
{
	// カラーバーを描画する
    ColorMap *map;
    int i;
    int index;
    int under_index;
    ColorLevel *upper_l;
    ColorLevel *under_l;
    TColor level_color;
    double ratio;
    double level;

    // カラーバー取得
    map = (ColorMap *)PreSetComboBox->Items->Objects[Index];

    // 登録数0なら真っ白にする
    if ( map->getColorLevelCount() == 0 ) {
    	PreSetComboBox->Canvas->Brush->Color = clWhite;
        PreSetComboBox->Canvas->FillRect( Rect );

        return;
    }

    // すべての位置について
    for ( i = 0; i < Rect.Width(); i++ ) {
		// レベル作成
    	level = i / (double)Rect.Width() * ( map->getColorLevel( map->getColorLevelCount() - 1 )->getLevel() - map->getColorLevel( 0 )->getLevel() );

        // 色取得
        map->setValue( level );
        map->makeColor();

        // 描画
        PreSetComboBox->Canvas->Pen->Color = (TColor)RGB( map->getR(), map->getG(), map->getB() );
        PreSetComboBox->Canvas->Pen->Width = 1;

        PreSetComboBox->Canvas->MoveTo( i, Rect.Bottom );
        PreSetComboBox->Canvas->LineTo( i, Rect.Top );
    }
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::SetPresetButtonClick(TObject *Sender)
{
	// 現在のプリセットを採用
    double lvb;
    double lvt;

    // 更新許可がなければ何もしない
    if ( !updatePreset ) {
    	return;
    }

    // カラーマップを上書き
    *map = *(ColorMap *)PreSetComboBox->Items->Objects[PreSetComboBox->ItemIndex];

    // カラーマップに必要な設定を
    map->setUnitString( UnitStrEdit->Text );
    map->setExpression( ExpEdit->Text );

    map->setSmooth();

    // レベル取得
    try {
    	lvb = MinLevelEdit->Text.ToDouble();
        lvt = MaxLevelEdit->Text.ToDouble();
    } catch ( ... ) {
    	// 失敗
        lvb = MinLevelUpDown->Position;
        lvt = MaxLevelUpDown->Position;
    }

    map->setTopLevel( lvt );
    map->setBottomLevel( lvb );

    // 更新
	updateView();
}
//---------------------------------------------------------------------------




void __fastcall TColorMapConfigForm::MinLevelEditChange(TObject *Sender)
{
	// 値が変わっても更新
    try {
    	StrToFloat( MinLevelEdit->Text );
        SetPresetButton->Click();
    } catch( ... ) {
    }
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::MaxLevelEditChange(TObject *Sender)
{
	// 値が変わっても更新
    try {
    	StrToFloat( MaxLevelEdit->Text );
        SetPresetButton->Click();
    } catch( ... ) {
    }
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::PreSetComboBoxSelect(TObject *Sender)
{
	// アイテムが変更された
    SetPresetButton->Click();	
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::CloseButtonClick(TObject *Sender)
{
	// キャンセル
    Close();	
}
//---------------------------------------------------------------------------

