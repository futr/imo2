#include "colormap.h"

ColorLevel::ColorLevel(double level, int r, int g, int b)
{
    // �J���[���x���̃R���X�g���N�^�[
    this->level = level;

    this->r = r;
    this->g = g;
    this->b = b;
}


void ColorLevel::setLevel(double level)
{
    // ���x����ݒ�
    this->level = level;
}


void ColorLevel::setColor(int r, int g, int b)
{
    // �F��ݒ�
    this->r = r;
    this->g = g;
    this->b = b;
}


double ColorLevel::getLevel()
{
    // ���x�����擾
    return level;
}


int ColorLevel::getR()
{
    // �Ԏ擾
    return r;
}

int ColorLevel::getG()
{
    // �Ύ擾
    return g;
}


int ColorLevel::getB()
{
    // �擾
    return b;
}

ColorMap::ColorMap()
{
    // �J���[�}�b�v�̃R���X�g���N�^�[

    setRange( 0, 255 );
    setSmooth( false );

    // �J���[���X�g������
    levels = new TList;

    // ��������
    tok_exp = NULL;

    // ���̏����l
    setExpression( "a" );
    unit_str = "";
}

ColorMap::~ColorMap()
{
    // �J���[�}�b�v�̃f�X�g���N�^�[
    int i;

    // �ǉ��ς݂̃J���[���x�����폜
    for ( i = 0; i < levels->Count; i++ ) {
        delete (ColorLevel *)getColorLevel( i );
    }

    // �g�[�N���폜
    ecalc_free_token( tok_exp );

    delete levels;
}

void ColorMap::setExpression(AnsiString exp)
{
    // ��������ݒ�
    this->exp = exp;

    // ���쐬
    ecalc_free_token( tok_exp );
    tok_exp = ecalc_make_token( exp.c_str() );
    tok_exp = ecalc_make_tree( tok_exp );
}


void ColorMap::evalExpression(double **args, double ans)
{
    // ����]�����ĐF������

    // ���]��
    value = ecalc_get_tree_value( tok_exp, args, ans );
}


void ColorMap::setRange(double bottom, double top)
{
    // �͈͂�����
    this->bottom = bottom;
    this->top = top;
}


void ColorMap::setSmooth(bool smooth)
{
    // �Ȃ߂炩�ɂ���
    this->smooth = smooth;
}


double ColorMap::getR()
{
    // �ԐF�擾
    return r;
}


double ColorMap::getG()
{
    // �ΐF�擾
    return g;
}


double ColorMap::getB()
{
    // �F�擾
    return b;
}


void ColorMap::makeColor()
{
    // ���݂̕]���l�ƃ��x������F������
    ColorLevel *upper;
    ColorLevel *under;
    double ratio;
    int set_r;
    int set_g;
    int set_b;


    // �㉺�̃��x�����擾
    upper = getUpperColorLevel( value );
    under = getUnderColorLevel( value );

    // �Е��̒[���Ȃ���Έ�ԋ߂��[�̐F���Ƃ�
    if ( upper == NULL && under != NULL ) {
        setColor( under->getR(), under->getG(), under->getB() );

        return;
    }

    if ( upper != NULL && under == NULL ) {
        setColor( upper->getR(), upper->getG(), upper->getB() );

        return;
    }

    // �ǂ�����Ȃ���΍�
    if ( upper == NULL && under == NULL ) {
        setColor( 0, 0, 0 );
        return;
    }

    // �㉺�Ƃ��L���Ȓl�������̂ŐF������

    if ( smooth ) {
        // �X���[�X����ꍇ

        // �䗦������
        ratio = ( value - under->getLevel() ) / ( upper->getLevel() - under->getLevel() );

        // �䗦����F������
        set_r = ( upper->getR() - under->getR() ) * ratio + under->getR();
        set_g = ( upper->getG() - under->getG() ) * ratio + under->getG();
        set_b = ( upper->getB() - under->getB() ) * ratio + under->getB();

        // �F��ݒ�
        setColor( set_r, set_g, set_b );
    } else {
        // ���x���X���C�X�̏ꍇ
        setColor( upper->getR(), upper->getG(), upper->getB() );
    }
}


void ColorMap::sortLevel()
{
    // ���x�����\�[�g����
    levels->Sort( sortLevelFunction );
}


ColorLevel *ColorMap::getUnderColorLevel(double level)
{
    // val�̉����̐F���x����Ԃ�
    ColorLevel *clevel;
    ColorLevel *ret;
    int i;

    // �߂�l������
    ret = NULL;

    for ( i = 0; i < levels->Count; i++ ) {
        // ���x���擾
        clevel = getColorLevel( i );

        // ��������΍X�V
        if ( clevel->getLevel() < level ) {
            ret = clevel;
        } else {
            // �������Ȃ������̂ŏI��
            break;
        }
    }

    return ret;
}


ColorLevel *ColorMap::getUpperColorLevel(double level)
{
    // val�̏㑤�̐F���x����Ԃ�
    ColorLevel *clevel;
    ColorLevel *ret;
    int i;

    // �߂�l������
    ret = NULL;

    for ( i = levels->Count - 1; i >= 0; i-- ) {
        // ���x���擾
        clevel = getColorLevel( i );

        // �����ȏ�ɂȂ�Δ���
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
    // �F��ݒ�
    this->r = r;
    this->g = g;
    this->b = b;
}


void ColorMap::addColorLevel(ColorLevel *level)
{
    // �J���[���x����ǉ�
    levels->Add( level );

    // �ă\�[�g
    sortLevel();
}


void ColorMap::deleteColorLevel( int index )
{
    // �J���[���x�����폜
    ColorLevel *clevel;

    clevel = (ColorLevel *)levels->Items[index];

    levels->Delete( index );

    delete clevel;

    // �ă\�[�g
    sortLevel();
}


int ColorMap::getColorLevelCount()
{
    // �����x����
    return levels->Count;
}


ColorLevel *ColorMap::getColorLevel(int index)
{
    // �w��C���f�b�N�X��̃J���[���x�����擾

    // �ςȃC���f�b�N�X�Ȃ�NULL
    if ( index < 0 || index >= levels->Count ) {
        return NULL;
    }

    return (ColorLevel *)levels->Items[index];
}


int __fastcall sortLevelFunction(void *item1, void *item2)
{
    // TList�p�̔�r�֐�
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
    // ��������Ԃ�
    return exp;
}


void ColorMap::setUnitString(AnsiString unit)
{
    // �P�ʕ������ݒ�
    unit_str = unit;
}


AnsiString ColorMap::getUnitString()
{
    // �P�ʕ�������擾
    return unit_str;
}


bool ColorMap::getSmooth()
{
    // �Ȃ߂炩�ݒ�擾
    return smooth;
}


void ColorMap::setValue(double value)
{
    // ���l���ڐݒ�
    this->value = value;
}


double ColorMap::getValue()
{
    // ���l�擾
    return value;
}


int ColorMap::getColorLevelIndex(ColorLevel *level)
{
    // �J���[���x���|�C���^�[����C���f�b�N�X�擾
    return levels->IndexOf( level );
}


void ColorMap::deleteAllColorLevel()
{
    // ���ׂẴ��x�����폜
    while ( getColorLevelCount() ) {
        deleteColorLevel( 0 );
    }
}

void ColorMap::setTopLevel( double level )
{
	// �ŏ�ʃ��x����䗦������ĕύX
    ColorLevel *top;
    ColorLevel *bottom;
    ColorLevel *now;
    double width;
    double new_width;
    double top_level;
    double bottom_level;
    double ratio;
    int i;

    // �o�^����0�Ȃ牽�����Ȃ�
    if ( getColorLevelCount() == 0 ) {
    	return;
    }

    // �ŏ�ʂƍŉ��ʂ��擾
    top    = getColorLevel( getColorLevelCount() - 1 );
    bottom = getColorLevel( 0 );

    // �ŏ�ʂƍŉ��ʂ������Ȃ炽���P�ɐݒ�
    if ( bottom == top ) {
    	bottom->setLevel( level );

        return;
    }

	// �l�擾
    top_level = top->getLevel();
    bottom_level = bottom->getLevel();

    width = top_level - bottom_level;

    // ���Ƃ��Ƃ̍����O�Ȃ牽�����Ȃ�
    if ( width == 0 ) {
    	return;
    }

    // �V������
    new_width = level - bottom_level;

    // �オ����������Ă��ꍇ�͉������Ȃ�
    if ( new_width <= 0 ) {
    	return;
    }

    // ���ׂĂ̒l��䗦��ۂ��ĕύX
    for ( i = 1; i < getColorLevelCount() - 1; i++ ) {
    	// ���x���擾
        now = getColorLevel( i );

        // �䗦����
        ratio = ( now->getLevel() - bottom_level ) / width;

        // �V�����l��ݒ�
        now->setLevel( ratio * new_width + bottom_level );
    }

    // �g�b�v���x���ύX
    top->setLevel( level );
}

void ColorMap::setBottomLevel( double level )
{
	// �ŉ��ʃ��x����䗦������ĕύX
    ColorLevel *top;
    ColorLevel *bottom;
    ColorLevel *now;
    double width;
    double new_width;
    double top_level;
    double bottom_level;
    double ratio;
    int i;

    // �o�^����0�Ȃ牽�����Ȃ�
    if ( getColorLevelCount() == 0 ) {
    	return;
    }

    // �ŏ�ʂƍŉ��ʂ��擾
    top    = getColorLevel( getColorLevelCount() - 1 );
    bottom = getColorLevel( 0 );

    // �ŏ�ʂƍŉ��ʂ������Ȃ炽���P�ɐݒ�
    if ( bottom == top ) {
    	bottom->setLevel( level );

        return;
    }

	// �l�擾
    top_level = top->getLevel();
    bottom_level = bottom->getLevel();

    width = top_level - bottom_level;

    // ���Ƃ��Ƃ̍����O�Ȃ牽�����Ȃ�
    if ( width == 0 ) {
    	return;
    }

    // �V������
    new_width = top_level - level;

    // �オ����������Ă��ꍇ�͉������Ȃ�
    if ( new_width <= 0 ) {
    	return;
    }

    // ���ׂĂ̒l��䗦��ۂ��ĕύX
    for ( i = 1; i < getColorLevelCount() - 1; i++ ) {
    	// ���x���擾
        now = getColorLevel( i );

        // �䗦����
        ratio = ( now->getLevel() - bottom_level ) / width;

        // �V�����l��ݒ�
        now->setLevel( ratio * new_width + level );
    }

    // �ŉ��ʃ��x���ύX
    bottom->setLevel( level );
}

ColorMap &ColorMap::operator =( const ColorMap &r_cm )
{
	// �ݒ���R�s�[���邽�߂̑�����Z�q

    // ���ׂẴ��������N���A
    deleteAllColorLevel();

    // ���x�����R�s�[
    for ( int i = 0; i < r_cm.getColorLevelCount(); i++ ) {
    	addColorLevel( new ColorLevel( r_cm.getColorLevel( i )->getLevel(), r_cm.getColorLevel( i )->getR(), r_cm.getColorLevel( i )->getG(), r_cm.getColorLevel( i )->getB() ) );
    }

    // ���̑��̐ݒ���R�s�[
    setColor( r_cm.getR(), r_cm.getG(), r_cm.getB() );
    setValue( r_cm.getValue() );
    setExpression( r_cm.getExpression() );
    setUnitString( r_cm.getUnitString() );
    setSmooth( r_cm.getSmooth() );

    return *this;
}

