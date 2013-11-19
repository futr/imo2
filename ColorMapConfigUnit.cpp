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
	// ������
    selectLevelIndex = -1;

    // ���x�����X�g������
    levelLabels = new TList;

    updateEdit = false;
    updatePreset = true;

    // �|�C���^������
    map = &m_map;

    m_map.deleteAllColorLevel();

    // �J���[�}�b�v�̃v���Z�b�g�����

    // ���F
    psRainbow.deleteAllColorLevel();
    psRainbow.addColorLevel( new ColorLevel( 5, 255, 0, 0 ) );
    psRainbow.addColorLevel( new ColorLevel( 4, 255, 255,   0 ) );
    psRainbow.addColorLevel( new ColorLevel( 3,   0, 255,   0 ) );
    psRainbow.addColorLevel( new ColorLevel( 2,   0, 255, 255 ) );
    psRainbow.addColorLevel( new ColorLevel( 1,   0,   0, 255 ) );
    psRainbow.addColorLevel( new ColorLevel( 0,   0,   0,   0 ) );
    psRainbow.setSmooth();

    // ����
    psBlackWhite.deleteAllColorLevel();
    psBlackWhite.addColorLevel( new ColorLevel( 0,   0,   0,   0 ) );
    psBlackWhite.addColorLevel( new ColorLevel( 1, 255, 255, 255 ) );
    psBlackWhite.setSmooth();

    // ���ˋP�x�݂����Ȃ��
    psRadio.deleteAllColorLevel();
    psRadio.addColorLevel( new ColorLevel( 3, 255, 255, 255 ) );
    psRadio.addColorLevel( new ColorLevel( 2, 255, 255,   0 ) );
    psRadio.addColorLevel( new ColorLevel( 1, 255,   0,   0 ) );
    psRadio.addColorLevel( new ColorLevel( 0,   0,   0,   0 ) );
    psRadio.setSmooth();

    // ����
    psWhiteBlack.deleteAllColorLevel();
    psWhiteBlack.addColorLevel( new ColorLevel( 1,   0,   0,   0 ) );
    psWhiteBlack.addColorLevel( new ColorLevel( 0, 255, 255, 255 ) );
    psWhiteBlack.setSmooth();

    // ���F2
    psRainbow2.deleteAllColorLevel();
    psRainbow2.addColorLevel( new ColorLevel( 4, 255,   0,   0 ) );
    psRainbow2.addColorLevel( new ColorLevel( 3, 255, 255,   0 ) );
    psRainbow2.addColorLevel( new ColorLevel( 2,   0, 255,   0 ) );
    psRainbow2.addColorLevel( new ColorLevel( 1,   0, 255, 255 ) );
    psRainbow2.addColorLevel( new ColorLevel( 0,   0,   0, 255 ) );
    psRainbow2.setSmooth();

    // GNU���ۂ���
    psGnu.deleteAllColorLevel();
    psGnu.addColorLevel( new ColorLevel( 3, 255, 255,   0 ) );
    psGnu.addColorLevel( new ColorLevel( 2, 255,   0,   0 ) );
    psGnu.addColorLevel( new ColorLevel( 1, 128,   0, 255 ) );
    psGnu.addColorLevel( new ColorLevel( 0,   0,   0,   0 ) );
    psGnu.setSmooth();

    // ���X�g�ɓo�^ ( c-style��cast���g���Ă����ɁA���Ȃ若�� )
    PreSetComboBox->Items->AddObject( "����", (TObject *)&psBlackWhite );
    PreSetComboBox->Items->AddObject( "����", (TObject *)&psWhiteBlack );
    PreSetComboBox->Items->AddObject( "���F", (TObject *)&psRainbow );
    PreSetComboBox->Items->AddObject( "���F�Q", (TObject *)&psRainbow2 );
    PreSetComboBox->Items->AddObject( "���ԉ���", (TObject *)&psRadio );
    PreSetComboBox->Items->AddObject( "GNU", (TObject *)&psGnu );

    // 0�ԖڑI��
    PreSetComboBox->ItemIndex = 0;
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::FormDestroy(TObject *Sender)
{
	// ���
    int i;

    // ���x�����X�g���
    for ( i = 0; i < levelLabels->Count; i++ ) {
    	delete (TLabel *)levelLabels->Items[i];
    }

    delete levelLabels;
}
//---------------------------------------------------------------------------
void TColorMapConfigForm::setColorMap( ColorMap *map )
{
	// �J���[�}�b�v��ݒ�

    // �X�V�֎~
    updatePreset = false;

    m_map = *map;
    back_map = map;

    // �ő�l�ݒ�
    if ( m_map.getColorLevelCount() > 0 ) {
    	MinLevelEdit->Text = FloatToStr( m_map.getColorLevel( 0 )->getLevel() );
    	MaxLevelEdit->Text = FloatToStr( m_map.getColorLevel( m_map.getColorLevelCount() - 1 )->getLevel() );
    }

    // �X�V����
    updatePreset = true;

    // �\���X�V
    updateView();
}
//---------------------------------------------------------------------------
void TColorMapConfigForm::updateView( void )
{
	// �\�����X�V����
    int i;
    TLabel *newLabel;
    ColorLevel *clevel;

    // ���x�����X�g�j��
    for ( i = 0; i < levelLabels->Count; i++ ) {
    	delete (TLabel *)levelLabels->Items[i];
    }

    levelLabels->Clear();

    // ���x�����X�g�ǉ�
    for ( i = 0; i < map->getColorLevelCount(); i++ ) {
    	// �Ή��J���[���x���擾
        clevel = map->getColorLevel( i );

    	// ���x���쐬
    	newLabel = new TLabel( this );

        // ���x��������
    	newLabel->Parent = LevelImageBox;
        newLabel->Tag = i;
        newLabel->OnClick = onSelectLevelLabel;

        // ���x���ݒ�
        newLabel->Caption = FloatToStr( clevel->getLevel() );
        newLabel->Color = clWhite;
        newLabel->Font->Size = 13;
        newLabel->Width = 60;
        newLabel->Height = 20;

        // ���x���ʒu�v�Z
        newLabel->Left = LevelImage->Left + LevelImage->Width + 5;
        newLabel->Top  = LevelImage->Top - newLabel->Height / 2 + ( (double)LevelImage->Height / map->getColorLevelCount() * ( map->getColorLevelCount() - i - 1 ) );

    	// ���x�����X�g�ɓo�^
        levelLabels->Add( newLabel );
    }

    // ���Ȃǂ�ǂݍ���
    ExpEdit->Text = map->getExpression();
    UnitStrEdit->Text = map->getUnitString();

    // �Ȃ߂炩���[�h�ǂݍ���
    SmoothCheckBox->Checked = map->getSmooth();

    // ���x����I������
    if ( map->getColorLevelCount() == 0 ) {
    	// �ЂƂ��Ȃ���Ζ����ȑI��
    	selectColorLevel( -1 );
    } else {
    	// ���݂̂��̂�I��
        selectColorLevel( selectLevelIndex );
    }

    // ���x���C���[�W�ĕ`��
    drawLevelImage();

    // �v���r���[�ĕ`��
    drawPreview();
}
//---------------------------------------------------------------------------
void TColorMapConfigForm::drawLevelImage( void )
{
	// ���x���C���[�W������
    int i;
    int index;
    int under_index;
    ColorLevel *upper_l;
    ColorLevel *under_l;
    TColor level_color;
    double ratio;
    double level;

    // �o�^��0�Ȃ�^�����ɂ���
    if ( map->getColorLevelCount() == 0 ) {
    	LevelImage->Canvas->Brush->Color = clWhite;
        LevelImage->Canvas->FillRect( Rect( 0, 0, LevelImage->Width, LevelImage->Height ) );

        return;
    }

    // ���ׂĂ̈ʒu�ɂ���
    for ( i = 0; i < LevelImage->Height; i++ ) {
    	// ���̈ʒu�ł̑ΏۃC���f�b�N�X���擾
        index = map->getColorLevelCount() - 1 - (int)( i / ( (double)LevelImage->Height / map->getColorLevelCount() ) );

        // ���̃C���f�b�N�X���擾
        under_index = map->getColorLevelCount() - 2 - (int)( i / ( (double)LevelImage->Height / map->getColorLevelCount() ) );

        // ���݈ʒu�̕����ɑ΂���� ( *1000���邱�ƂŐ��x���グ�Ă��� )
        ratio = ( ( i * 1000 ) % (int)( 1000.0 * LevelImage->Height / map->getColorLevelCount() ) ) / ( 1000 * (double)LevelImage->Height / map->getColorLevelCount() );

        // �㉺�̐F���x���擾
        upper_l = map->getColorLevel( index );
        under_l = map->getColorLevel( under_index );

        // �摜��̃��x�����擾
        if ( map->getSmooth() ) {
        	// �X���[�X����
            if ( under_l != NULL ) {
            	// �����x��������ꍇ
                level = upper_l->getLevel() - ( upper_l->getLevel() - under_l->getLevel() ) * ratio;
            } else {
            	// �����x�����Ȃ��ꍇ
                level = upper_l->getLevel();
            }
        } else {
        	// �X���[�X�Ȃ�
            level = upper_l->getLevel();
        }

        // �F�擾
        map->setValue( level );
        map->makeColor();

        // �`��
        LevelImage->Canvas->Pen->Color = (TColor)RGB( map->getR(), map->getG(), map->getB() );
        LevelImage->Canvas->Pen->Width = 1;

        LevelImage->Canvas->MoveTo( 0, i );
        LevelImage->Canvas->LineTo( LevelImage->Width, i );
    }

    // �����ʒu�̐�������
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
	// �v���r���[��`��
    int zoom;

    double ratio;

    // �œK�ȃY�[����T��
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

    // �֐��̋��߂�`�ɍ��킹��
    zoom = ZOOM_MAX / 2 + zoom;

    if ( zoom > ZOOM_MAX ) {
    	zoom = ZOOM_MAX;
    }

    SatViewMainForm->DrawImg( PreviewImage, NULL, false, zoom, SatViewMainForm->img_w / 2, SatViewMainForm->img_h / 2, map );
}
//---------------------------------------------------------------------------
void TColorMapConfigForm::selectColorLevel( int index )
{
	// �J���[���x����I��
    int i;
    ColorLevel *clevel;

    // ���x��������΂��ׂĖ���
    for ( i = 0; i < levelLabels->Count; i++ ) {
    	// ���x���擾
    	TLabel *llabel = (TLabel *)levelLabels->Items[i];

        // ���x���̐F�𔒂�
        llabel->Color = clWhite;
    }

    // �����C���f�b�N�X�Ȃ瑀��s�\
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

        // ���x��������ΑI��
        if ( index < levelLabels->Count ) {
        	TLabel *llabel = (TLabel *)levelLabels->Items[selectLevelIndex];

            // ���x���̐F��ς���
            llabel->Color = clAqua;
        }
    }

    // �J���[���x���擾
    clevel = map->getColorLevel( selectLevelIndex );

    // �G�f�B�^�X�V�J�n
    updateEdit = true;

    // �R���|�[�l���g��ݒ�
    LevelEdit->Text = FloatToStr( clevel->getLevel() );
    LevelEditUpDown->Position = clevel->getLevel();

    // �G�f�B�^�X�V���~
    updateEdit = false;

    LevelColorPanel->Color = (TColor)RGB( clevel->getR(), clevel->getG(), clevel->getB() );
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::onSelectLevelLabel( TObject *Sender )
{
	// ���x�����x�����N���b�N���ꂽ���̃C�x���g
    TLabel *llabel = (TLabel *)(Sender);

    selectColorLevel( llabel->Tag );
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::ExpEditChange(TObject *Sender)
{
	// ���������ݒ�
    map->setExpression( ExpEdit->Text );
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::UnitStrEditChange(TObject *Sender)
{
	// �P�ʕ������ݒ�
    map->setUnitString( UnitStrEdit->Text );
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::SmoothCheckBoxClick(TObject *Sender)
{
	// �Ȃ߂炩���[�h�ݒ�
    if ( SmoothCheckBox->Checked ) {
    	map->setSmooth( true );
    } else {
    	map->setSmooth( false );
    }

    // �X�V
    updateView();
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::SelectColorButtonClick(
      TObject *Sender)
{
	// �F�I��
    if ( LevelColorDialog->Execute() ) {
    	// ���ݑI������Ă��郌�x���擾
    	ColorLevel *clevel;
        TColor color = LevelColorDialog->Color;
        unsigned char *cpcolor = (unsigned char *)&color;

        clevel = map->getColorLevel( selectLevelIndex );

        clevel->setColor( cpcolor[0], cpcolor[1], cpcolor[2] );
        LevelColorPanel->Color = color;
    }

    // �X�V
    updateView();
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::DeleteLevelButtonClick(
      TObject *Sender)
{
	// ���ݑI������Ă��郌�x�����폜����
    map->deleteColorLevel( selectLevelIndex );

    // �X�V
    updateView();
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::AddLevelButtonClick(TObject *Sender)
{
	// �V���ȃ��x����ǉ�
    double max_level;

    // �ő�̃��x�����쐬
    if ( map->getColorLevelCount() != 0 ) {
    	// ���łɂ���ꍇ
        max_level = map->getColorLevel( map->getColorLevelCount() - 1 )->getLevel() + 1;
    } else {
    	// �Ȃ��ꍇ
        max_level = 0;
    }

    // �ǉ�
    map->addColorLevel( new ColorLevel( max_level, 0, 0, 0 ) );

    // �ő��I��
    selectLevelIndex = map->getColorLevelCount() - 1;

    // �X�V
    updateView();
}
//---------------------------------------------------------------------------




void __fastcall TColorMapConfigForm::LevelColorPanelClick(TObject *Sender)
{
	// �F�I��
    if ( SelectColorButton->Enabled ) {
    	SelectColorButton->Click();
    }
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::CancelButtonClick(TObject *Sender)
{
	// �ݒ�
    *back_map = m_map;

	// �ĕ`�悳���ĕ���
    SatViewMainForm->DrawImg();

	Close();	
}
//---------------------------------------------------------------------------
void __fastcall TColorMapConfigForm::SetButtonClick(TObject *Sender)
{
	// ���x���ύX
    double level;
    AnsiString before_str;

	// �X�V���Ȃ牽�����Ȃ�
    /*
    if ( updateEdit ) {
    	return;
    }
    */

    // �s���Ȓl�Ȃ牽�����Ȃ�
    try {
    	level = LevelEdit->Text.ToDouble();
    } catch (...) {
        return;
    }

	// ���x���l�̕ύX
    ColorLevel *clevel;

    // �J���[���x���擾
    clevel = map->getColorLevel( selectLevelIndex );

    // NULL�Ȃ牽�����Ȃ�
    if ( clevel == NULL ) {
    	return;
    }

    // �K�v�Ȃ�䗦��ۂ�
    if ( StaticRatioCheckBox->Checked ) {
    	if ( selectLevelIndex == 0 ) {
        	map->setBottomLevel( level );
        } else if ( selectLevelIndex == map->getColorLevelCount() - 1 ) {
        	map->setTopLevel( level );
        } else {
        	clevel->setLevel( level );
        }
    } else {
    	// �l�ݒ�
    	clevel->setLevel( level );
    }

    // �A�b�v�_�E���ݒ�
    // LevelEditUpDown->Position = clevel->getLevel();

    // �ă\�[�g
    map->sortLevel();

    // �C���f�b�N�X���X�V
    selectLevelIndex = map->getColorLevelIndex( clevel );

    // �X�V
    updateView();
}
//---------------------------------------------------------------------------


void __fastcall TColorMapConfigForm::LevelEditUpDownClick(TObject *Sender,
      TUDBtnType Button)
{
	// �G�f�B�^�̒l���X�V
	LevelEdit->Text = LevelEditUpDown->Position;

    Application->ProcessMessages();
    //Application->MessageBoxA( AnsiString( LevelEditUpDown->Position ).c_str(), "", MB_OK );
	SetButton->Click();
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::ClearButtonClick(TObject *Sender)
{
	// ���x����S�č폜
    while ( map->getColorLevelCount() ) {
    	map->deleteColorLevel( 0 );
    }

    // ������Đݒ�
    ExpEdit->Text = "a";
    UnitStrEdit->Text = "�P��";

    // �X�V
    this->updateView();
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::PreSetComboBoxDrawItem(
      TWinControl *Control, int Index, TRect &Rect, TOwnerDrawState State)
{
	// �J���[�o�[��`�悷��
    ColorMap *map;
    int i;
    int index;
    int under_index;
    ColorLevel *upper_l;
    ColorLevel *under_l;
    TColor level_color;
    double ratio;
    double level;

    // �J���[�o�[�擾
    map = (ColorMap *)PreSetComboBox->Items->Objects[Index];

    // �o�^��0�Ȃ�^�����ɂ���
    if ( map->getColorLevelCount() == 0 ) {
    	PreSetComboBox->Canvas->Brush->Color = clWhite;
        PreSetComboBox->Canvas->FillRect( Rect );

        return;
    }

    // ���ׂĂ̈ʒu�ɂ���
    for ( i = 0; i < Rect.Width(); i++ ) {
		// ���x���쐬
    	level = i / (double)Rect.Width() * ( map->getColorLevel( map->getColorLevelCount() - 1 )->getLevel() - map->getColorLevel( 0 )->getLevel() );

        // �F�擾
        map->setValue( level );
        map->makeColor();

        // �`��
        PreSetComboBox->Canvas->Pen->Color = (TColor)RGB( map->getR(), map->getG(), map->getB() );
        PreSetComboBox->Canvas->Pen->Width = 1;

        PreSetComboBox->Canvas->MoveTo( i, Rect.Bottom );
        PreSetComboBox->Canvas->LineTo( i, Rect.Top );
    }
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::SetPresetButtonClick(TObject *Sender)
{
	// ���݂̃v���Z�b�g���̗p
    double lvb;
    double lvt;

    // �X�V�����Ȃ���Ή������Ȃ�
    if ( !updatePreset ) {
    	return;
    }

    // �J���[�}�b�v���㏑��
    *map = *(ColorMap *)PreSetComboBox->Items->Objects[PreSetComboBox->ItemIndex];

    // �J���[�}�b�v�ɕK�v�Ȑݒ��
    map->setUnitString( UnitStrEdit->Text );
    map->setExpression( ExpEdit->Text );

    map->setSmooth();

    // ���x���擾
    try {
    	lvb = MinLevelEdit->Text.ToDouble();
        lvt = MaxLevelEdit->Text.ToDouble();
    } catch ( ... ) {
    	// ���s
        lvb = MinLevelUpDown->Position;
        lvt = MaxLevelUpDown->Position;
    }

    map->setTopLevel( lvt );
    map->setBottomLevel( lvb );

    // �X�V
	updateView();
}
//---------------------------------------------------------------------------




void __fastcall TColorMapConfigForm::MinLevelEditChange(TObject *Sender)
{
	// �l���ς���Ă��X�V
    try {
    	StrToFloat( MinLevelEdit->Text );
        SetPresetButton->Click();
    } catch( ... ) {
    }
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::MaxLevelEditChange(TObject *Sender)
{
	// �l���ς���Ă��X�V
    try {
    	StrToFloat( MaxLevelEdit->Text );
        SetPresetButton->Click();
    } catch( ... ) {
    }
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::PreSetComboBoxSelect(TObject *Sender)
{
	// �A�C�e�����ύX���ꂽ
    SetPresetButton->Click();	
}
//---------------------------------------------------------------------------

void __fastcall TColorMapConfigForm::CloseButtonClick(TObject *Sender)
{
	// �L�����Z��
    Close();	
}
//---------------------------------------------------------------------------

