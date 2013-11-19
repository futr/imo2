#include "colormap.h"

ColorLevel::ColorLevel(double level, int r, int g, int b)
{
    // カラーレベルのコンストラクター
    this->level = level;

    this->r = r;
    this->g = g;
    this->b = b;
}


void ColorLevel::setLevel(double level)
{
    // レベルを設定
    this->level = level;
}


void ColorLevel::setColor(int r, int g, int b)
{
    // 色を設定
    this->r = r;
    this->g = g;
    this->b = b;
}


double ColorLevel::getLevel()
{
    // レベルを取得
    return level;
}


int ColorLevel::getR()
{
    // 赤取得
    return r;
}

int ColorLevel::getG()
{
    // 緑取得
    return g;
}


int ColorLevel::getB()
{
    // 青取得
    return b;
}

ColorMap::ColorMap()
{
    // カラーマップのコンストラクター

    setRange( 0, 255 );
    setSmooth( false );

    // カラーリストをつくる
    levels = new TList;

    // 式初期化
    tok_exp = NULL;

    // 式の初期値
    setExpression( "a" );
    unit_str = "";
}

ColorMap::~ColorMap()
{
    // カラーマップのデストラクター
    int i;

    // 追加済みのカラーレベルを削除
    for ( i = 0; i < levels->Count; i++ ) {
        delete (ColorLevel *)getColorLevel( i );
    }

    // トークン削除
    ecalc_free_token( tok_exp );

    delete levels;
}

void ColorMap::setExpression(AnsiString exp)
{
    // 式文字を設定
    this->exp = exp;

    // 式作成
    ecalc_free_token( tok_exp );
    tok_exp = ecalc_make_token( exp.c_str() );
    tok_exp = ecalc_make_tree( tok_exp );
}


void ColorMap::evalExpression(double **args, double ans)
{
    // 式を評価して色を決定

    // 式評価
    value = ecalc_get_tree_value( tok_exp, args, ans );
}


void ColorMap::setRange(double bottom, double top)
{
    // 範囲を決定
    this->bottom = bottom;
    this->top = top;
}


void ColorMap::setSmooth(bool smooth)
{
    // なめらかにする
    this->smooth = smooth;
}


double ColorMap::getR()
{
    // 赤色取得
    return r;
}


double ColorMap::getG()
{
    // 緑色取得
    return g;
}


double ColorMap::getB()
{
    // 青色取得
    return b;
}


void ColorMap::makeColor()
{
    // 現在の評価値とレベルから色をつくる
    ColorLevel *upper;
    ColorLevel *under;
    double ratio;
    int set_r;
    int set_g;
    int set_b;


    // 上下のレベルを取得
    upper = getUpperColorLevel( value );
    under = getUnderColorLevel( value );

    // 片方の端がなければ一番近い端の色をとる
    if ( upper == NULL && under != NULL ) {
        setColor( under->getR(), under->getG(), under->getB() );

        return;
    }

    if ( upper != NULL && under == NULL ) {
        setColor( upper->getR(), upper->getG(), upper->getB() );

        return;
    }

    // どちらもなければ黒
    if ( upper == NULL && under == NULL ) {
        setColor( 0, 0, 0 );
        return;
    }

    // 上下とも有効な値だったので色をつくる

    if ( smooth ) {
        // スムースする場合

        // 比率を決定
        ratio = ( value - under->getLevel() ) / ( upper->getLevel() - under->getLevel() );

        // 比率から色を決定
        set_r = ( upper->getR() - under->getR() ) * ratio + under->getR();
        set_g = ( upper->getG() - under->getG() ) * ratio + under->getG();
        set_b = ( upper->getB() - under->getB() ) * ratio + under->getB();

        // 色を設定
        setColor( set_r, set_g, set_b );
    } else {
        // レベルスライスの場合
        setColor( upper->getR(), upper->getG(), upper->getB() );
    }
}


void ColorMap::sortLevel()
{
    // レベルをソートする
    levels->Sort( sortLevelFunction );
}


ColorLevel *ColorMap::getUnderColorLevel(double level)
{
    // valの下側の色レベルを返す
    ColorLevel *clevel;
    ColorLevel *ret;
    int i;

    // 戻り値初期化
    ret = NULL;

    for ( i = 0; i < levels->Count; i++ ) {
        // レベル取得
        clevel = getColorLevel( i );

        // 小さければ更新
        if ( clevel->getLevel() < level ) {
            ret = clevel;
        } else {
            // 小さくなかったので終了
            break;
        }
    }

    return ret;
}


ColorLevel *ColorMap::getUpperColorLevel(double level)
{
    // valの上側の色レベルを返す
    ColorLevel *clevel;
    ColorLevel *ret;
    int i;

    // 戻り値初期化
    ret = NULL;

    for ( i = levels->Count - 1; i >= 0; i-- ) {
        // レベル取得
        clevel = getColorLevel( i );

        // 自分以上になれば発見
        if ( clevel->getLevel() >= level ) {
            ret = clevel;
        } else {
        	break;
        }
    }

    return ret;
}


void ColorMap::setColor(int r, int g, int b)
{
    // 色を設定
    this->r = r;
    this->g = g;
    this->b = b;
}


void ColorMap::addColorLevel(ColorLevel *level)
{
    // カラーレベルを追加
    levels->Add( level );

    // 再ソート
    sortLevel();
}


void ColorMap::deleteColorLevel( int index )
{
    // カラーレベルを削除
    ColorLevel *clevel;

    clevel = (ColorLevel *)levels->Items[index];

    levels->Delete( index );

    delete clevel;

    // 再ソート
    sortLevel();
}


int ColorMap::getColorLevelCount()
{
    // 総レベル数
    return levels->Count;
}


ColorLevel *ColorMap::getColorLevel(int index)
{
    // 指定インデックスるのカラーレベルを取得

    // 変なインデックスならNULL
    if ( index < 0 || index >= levels->Count ) {
        return NULL;
    }

    return (ColorLevel *)levels->Items[index];
}


int __fastcall sortLevelFunction(void *item1, void *item2)
{
    // TList用の比較関数
    ColorLevel *i1;
    ColorLevel *i2;

    i1 = (ColorLevel *)item1;
    i2 = (ColorLevel *)item2;

    if ( i1->getLevel() > i2->getLevel() ) {
    	return 1;
    } else if ( i1->getLevel() < i2->getLevel() ) {
    	return -1;
    } else {
    	return 0;
    }
}


AnsiString ColorMap::getExpression()
{
    // 式文字を返す
    return exp;
}


void ColorMap::setUnitString(AnsiString unit)
{
    // 単位文字列を設定
    unit_str = unit;
}


AnsiString ColorMap::getUnitString()
{
    // 単位文字列を取得
    return unit_str;
}


bool ColorMap::getSmooth()
{
    // なめらか設定取得
    return smooth;
}


void ColorMap::setValue(double value)
{
    // 式値直接設定
    this->value = value;
}


double ColorMap::getValue()
{
    // 式値取得
    return value;
}


int ColorMap::getColorLevelIndex(ColorLevel *level)
{
    // カラーレベルポインターからインデックス取得
    return levels->IndexOf( level );
}


void ColorMap::deleteAllColorLevel()
{
    // すべてのレベルを削除
    while ( getColorLevelCount() ) {
        deleteColorLevel( 0 );
    }
}

void ColorMap::setTopLevel( double level )
{
	// 最上位レベルを比率を守って変更
    ColorLevel *top;
    ColorLevel *bottom;
    ColorLevel *now;
    double width;
    double new_width;
    double top_level;
    double bottom_level;
    double ratio;
    int i;

    // 登録数が0なら何もしない
    if ( getColorLevelCount() == 0 ) {
    	return;
    }

    // 最上位と最下位を取得
    top    = getColorLevel( getColorLevelCount() - 1 );
    bottom = getColorLevel( 0 );

    // 最上位と最下位が同じならただ単に設定
    if ( bottom == top ) {
    	bottom->setLevel( level );

        return;
    }

	// 値取得
    top_level = top->getLevel();
    bottom_level = bottom->getLevel();

    width = top_level - bottom_level;

    // もともとの差が０なら何もしない
    if ( width == 0 ) {
    	return;
    }

    // 新しい幅
    new_width = level - bottom_level;

    // 上が下を下回ってた場合は何もしない
    if ( new_width <= 0 ) {
    	return;
    }

    // すべての値を比率を保って変更
    for ( i = 1; i < getColorLevelCount() - 1; i++ ) {
    	// レベル取得
        now = getColorLevel( i );

        // 比率決定
        ratio = ( now->getLevel() - bottom_level ) / width;

        // 新しい値を設定
        now->setLevel( ratio * new_width + bottom_level );
    }

    // トップレベル変更
    top->setLevel( level );
}

void ColorMap::setBottomLevel( double level )
{
	// 最下位レベルを比率を守って変更
    ColorLevel *top;
    ColorLevel *bottom;
    ColorLevel *now;
    double width;
    double new_width;
    double top_level;
    double bottom_level;
    double ratio;
    int i;

    // 登録数が0なら何もしない
    if ( getColorLevelCount() == 0 ) {
    	return;
    }

    // 最上位と最下位を取得
    top    = getColorLevel( getColorLevelCount() - 1 );
    bottom = getColorLevel( 0 );

    // 最上位と最下位が同じならただ単に設定
    if ( bottom == top ) {
    	bottom->setLevel( level );

        return;
    }

	// 値取得
    top_level = top->getLevel();
    bottom_level = bottom->getLevel();

    width = top_level - bottom_level;

    // もともとの差が０なら何もしない
    if ( width == 0 ) {
    	return;
    }

    // 新しい幅
    new_width = top_level - level;

    // 上が下を下回ってた場合は何もしない
    if ( new_width <= 0 ) {
    	return;
    }

    // すべての値を比率を保って変更
    for ( i = 1; i < getColorLevelCount() - 1; i++ ) {
    	// レベル取得
        now = getColorLevel( i );

        // 比率決定
        ratio = ( now->getLevel() - bottom_level ) / width;

        // 新しい値を設定
        now->setLevel( ratio * new_width + level );
    }

    // 最下位レベル変更
    bottom->setLevel( level );
}

ColorMap &ColorMap::operator =( const ColorMap &r_cm )
{
	// 設定をコピーするための代入演算子

    // すべてのメモリをクリア
    deleteAllColorLevel();

    // レベルをコピー
    for ( int i = 0; i < r_cm.getColorLevelCount(); i++ ) {
    	addColorLevel( new ColorLevel( r_cm.getColorLevel( i )->getLevel(), r_cm.getColorLevel( i )->getR(), r_cm.getColorLevel( i )->getG(), r_cm.getColorLevel( i )->getB() ) );
    }

    // その他の設定をコピー
    setColor( r_cm.getR(), r_cm.getG(), r_cm.getB() );
    setValue( r_cm.getValue() );
    setExpression( r_cm.getExpression() );
    setUnitString( r_cm.getUnitString() );
    setSmooth( r_cm.getSmooth() );

    return *this;
}

