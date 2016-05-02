//---------------------------------------------------------------------------
/*
1.41->1.42
全画面保存モードで赤チャネルしか保存されなくなってたのを修正

1.40->1.41
JPGを閉じるときのエラーを修正
ファイルアクセス関数をWin32APIにして大きなファイルを開けるようにした（い）
PALSAR2 L1.1対応

1.39->1.40
ecalcにJITを付けた

1.38->1.39
拡大縮小の予測画像作成を関数化
ダブルクリック拡大時にも予測画像を表示するようにした

1.37->1.38
remos内の関数を整理した
画像取得関数のバンド分離方法をより正確にした
浮動小数点演算を高速にした
命令セットをPentium Proにした
描画関数を整理した（保存と描画用はまだ分離してる）
ミニマップの描画を統合した
倍率の管理をzoom_posのみで行うようにした
拡大縮小の予測描画を高速に、より自然な表示になるようにした
ダブルクリックするとその地点を固定して一段階拡大するようにした

1.35->1.36
コミットログ参照

1.34->1.35
ALOS2のL2.1を表示可能にした。
band.img_widthの意味を変更した(バイト数)影響でTIFFが表示できなくなっていた問題を修正。
(まだ新たな問題が生じているかもしれない)
可能な限り描画中を表示するようにした。
撮影時刻も分かれば表示するようにした。
econf.cでダブルクォートで囲まれた文字列を読めるようにした。
ヒストグラム作成中にメッセージを表示するようにした。

1.33->1.34 PALSARL1.5対応、1.1対応できたかもしれないがうまく表示できない

1.32->1.33
ドラッグドロップ時にカレントディレクトリが切り替わらないので
設定ファイルが見つからない問題を修正

1.31->1.32
ヒストグラム高速化
温度分布UIをシンプルに
ホイールで拡大されない問題を修正

1.28->1.31
リポジトリのgit化
Landsat8対応
その他修正多数
多少の動作の仕様変更あり

1.27->1.28
色見本でのカーソル上の値をunsigned intで扱っていた問題の修正

1.26->1.27
負の値があった場合でも読み込めるようにした
座標値ウィンドウの動作を変更
PALSARでのメモリーリークを一つ解消

1.25->1.26
スタンプで経度緯度を表示するようにした
初期状態でレベルスライスに白黒がプリセットされるようにした
レベルスライス設定の表示を多少綺麗に

1.23->1.24
だいちで座標計算を行うとき、経度緯度->UTM->経度緯度としていたが
UTMゾーンをまたぐ場合があったため、リーダーファイルを使うように変更した
そのため、座標計算を行なっている関数が変更されている
remos::AlosConfigを追加

1.11->1.12
拡大モード時の描画高速化
連携のバグ
保存機能強化

一回読んでからファイルを閉じれば多少開放してくれないだろうか
定数式の最適化をかければ少しは早くなる？
*/


#include <vcl.h>
#pragma hdrstop

#include "About.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAboutForm *AboutForm;
//---------------------------------------------------------------------------
__fastcall TAboutForm::TAboutForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TAboutForm::ButtonOkClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------




