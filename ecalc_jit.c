#include "ecalc_jit.h"

#ifdef _WIN32
#include <windows.h>
#endif

// ecalcjit�G���W�� 32bit��Windows�p
// �o�͂���o�C�i����86�p�A�Ăяo���K���86�pcdecl�ł�
// ���߃Z�b�g��i686�ȍ~�i���Ǝv���܂��j

ECALC_JIT_TREE *ecalc_create_jit_tree( struct ECALC_TOKEN *token )
{
	// JIT�G���W���쐬
	size_t size;
	ECALC_JIT_TREE *tree;

	// token��NULL�Ȃ牽�����Ȃ�
	if ( token == NULL ) {
        return NULL;
    }

	// �\���̊m��
	tree = (ECALC_JIT_TREE *)malloc( sizeof(ECALC_JIT_TREE) );
	ecalc_bin_printer_reset_tree( tree );

	// JIT�̈�̗�
	size = ecalc_get_jit_tree_size( tree, token );

	// ���s�\��������Ԋm��
	tree->size = size;
	tree->pos  = 0;
	tree->data = (unsigned char *)ecalc_allocate_jit_memory( size );

	// �֐��o�C�i���o��
	ecalc_bin_printer( tree, token );

	return tree;
}

void ecalc_free_jit_tree( ECALC_JIT_TREE *tree )
{
	// JIT�G���W���j��

    // NULL�Ȃ牽�����Ȃ�
    if ( tree == NULL ) {
        return;
    }

	// ���s�\��������Ԕj��
	ecalc_free_jit_memory( tree->data );

	free( tree );
}

void *ecalc_allocate_jit_memory( size_t size )
{
	// JIT�p�o�C�i����Ԋm��
#ifdef _WIN32
	return VirtualAlloc( NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE );
#else
	return NULL;
#endif
}

void ecalc_free_jit_memory( void *data )
{
	// JIT�p�o�C�i����Ԕj��
#ifdef _WIN32
	VirtualFree( data, 0, MEM_RELEASE );
#else
#endif
}

double ecalc_get_jit_tree_value( ECALC_JIT_TREE *tree, double **vars, double ans )
{
	// JIT�؂̒l���擾
	double ret;
	double ( *func )( double **vars, double ans );
	
    // NULL�Ȃ牽�����Ȃ�
    if ( tree == NULL ) {
        return 0;
    }

	// �֐��|�C���^�Z�b�g
	func = ( double (*)( double **, double ) )tree->data;

	// ���s
	ret = func( vars, ans );

	return ret;
}

size_t ecalc_get_jit_tree_size( ECALC_JIT_TREE *tree, struct ECALC_TOKEN *token )
{
	// �K�v�ȃ������ʂ�(���߂ɎG��)�v�Z
	size_t size = 0;

	// tree->data��NULL�̏�Ԃň�����邱�Ƃ�tree->pos����T�C�Y��������
	ecalc_bin_printer( tree, token );

	// �֐��o�C�i���o�͈ʒu����T�C�Y���擾
	size = tree->pos;

	return size;
}

void ecalc_bin_printer( ECALC_JIT_TREE *tree, struct ECALC_TOKEN *token )
{
	// �o�C�i���o�͊J�n

	// �֐��J�n
	ecalc_bin_printer_opening( tree );

	// �؂�JIT�ɓW�J
	ecalc_bin_printer_tree( tree, token );

	// �֐��I��
	ecalc_bin_printer_closing( tree );
}

void ecalc_bin_printer_tree( ECALC_JIT_TREE *tree, struct ECALC_TOKEN *token )
{
	// �؂��o�C�i����
	/*
	 * eax�̓|�C���^���W�X�^�Aecx��eax�̃o�b�t�@�Aedx�͒萔���W�X�^�Ƃ���
	 * ebx�͕ۑ����Ȃ���΂Ȃ�Ȃ��̂Ŏg���Ă��Ȃ�
	 * �߂�l��st(0)�ɓ����
	 * EBP�͏�ɃR�[�����ꂽ���_�ł�ESP���w���Ă���
	 * �܂�avrs,ans�ɂ�EBP�o�R�ŃA�N�Z�X�\
	 * ecalc_bin_printer_tree���l�X�g���ăR�[������ꍇcdecl�Ɠ��l�Ƀ��W�X�^��j�󂷂�̂�
	 * �Ăяo�����ŃX�^�b�N�ʒu����A���W�X�^�ޔ��Ȃǂ��K�v
	 */
	const int left  = -8;
	const int right = -16;
	const int dbuf  = -24;
	const int arg2  = -32;
	const int arg1  = -40;
	int depth = 40;
	size_t pos1, pos2, pos3, pos4, apos1, apos2, apos3;

	// EAX�����݂̃X�^�b�N�g�b�v��
	ecalc_bin_printer_load_var_ptr_to_eax( tree, 0 );

	// EXP�ȊO�Ȃ狭���[��
	if ( token->type != ECALC_TOKEN_EXP ) {
		ecalc_bin_printer_store_double_val( tree, dbuf, 0 );
		ecalc_bin_printer_fld( tree, dbuf );

		return;
	}

	// left, right��0�N���A
	ecalc_bin_printer_store_double_val( tree, left, 0 );
	ecalc_bin_printer_store_double_val( tree, right, 0 );

	// ���Ӓl����
	if ( token->left != NULL ) {
		if ( token->left->type == ECALC_TOKEN_LITE ) {
			// ���Ӓl�萔���e���������ɃZ�b�g
			ecalc_bin_printer_store_double_val( tree, left, token->left->value );
		} else if ( token->left->type == ECALC_TOKEN_VAR ) {
			// ���Ӓl�ϐ��l�����ɃZ�b�g
			ecalc_bin_printer_load_exp_var_ptr_to_eax( tree, token->left->value );
			ecalc_bin_printer_fld( tree, 0 );
			ecalc_bin_printer_load_var_ptr_to_eax( tree, 0 );
			ecalc_bin_printer_fstp( tree, left );
		} else if ( token->left->type == ECALC_TOKEN_EXP ) {
			// ���ӂ����������ꍇ�����JIT�W�J���߂�nst(0)�����ɃZ�b�g
			ecalc_bin_printer_add_esp_i8( tree, -depth );
			ecalc_bin_printer_tree( tree, token->left );
			ecalc_bin_printer_add_esp_i8( tree, +depth );
			ecalc_bin_printer_load_var_ptr_to_eax( tree, 0 );
			ecalc_bin_printer_fstp( tree, left );
		} else {
			// ����0��
			ecalc_bin_printer_store_double_val( tree, left, 0 );
		}
	}

	// �E�ӎ擾
	if ( token->right != NULL ) {
		if ( token->right->type == ECALC_TOKEN_LITE ) {
			// �E�Ӓl�萔���e�������E�ɃZ�b�g
			ecalc_bin_printer_store_double_val( tree, right, token->right->value );
		} else if ( token->right->type == ECALC_TOKEN_VAR ) {
			// �E�Ӓl�ϐ��l���E�ɃZ�b�g
			ecalc_bin_printer_load_exp_var_ptr_to_eax( tree, token->right->value );
			ecalc_bin_printer_fld( tree, 0 );
			ecalc_bin_printer_load_var_ptr_to_eax( tree, 0 );
			ecalc_bin_printer_fstp( tree, right );
		} else if ( token->right->type == ECALC_TOKEN_EXP ) {
			// �E�ӂ����Ƃ��Ĉ����ꍇ�A�����ŏ���
			if ( token->value == ECALC_OPE_LOOP ) {
				// ���[�v���� (@��)
				// st(0) : �J�E���^(�����l0)
				// st(1) : 1
				// st(2) : left(���[�v��)
				// �Ƃ��Ĕ�r���s

				// for ( i = 0; i != left; i++ ) {

				// PUSH
				ecalc_bin_printer_fld( tree, left );
				ecalc_bin_printer_fld1( tree );
				ecalc_bin_printer_fldz( tree );

				// ���݈ʒu�ۑ�
				pos1 = ecalc_bin_printer_get_pos( tree );

				// st(0)��st(2)���r
				ecalc_bin_printer_fcomi( tree, 2 );

				// ZF(C3)��1�Ȃ�st(0)==st(2)�Ȃ̂�pos3�܂ŃW�����v
				apos1 = ecalc_bin_printer_je( tree, 0 );

				// ���݈ʒu�ۑ�
				pos2 = ecalc_bin_printer_get_pos( tree );

				// POP
				ecalc_bin_printer_fstp( tree, dbuf );
				ecalc_bin_printer_fstp_st0( tree );
				ecalc_bin_printer_fstp_st0( tree );

				// ���s
				ecalc_bin_printer_add_esp_i8( tree, -depth );
				ecalc_bin_printer_tree( tree, token->right );
				ecalc_bin_printer_add_esp_i8( tree, +depth );
				ecalc_bin_printer_load_var_ptr_to_eax( tree, 0 );
				ecalc_bin_printer_fstp( tree, right );

				// PUSH
				ecalc_bin_printer_fld( tree, left );
				ecalc_bin_printer_fld1( tree );
				ecalc_bin_printer_fld( tree, dbuf );

				// �C���N�������g
				ecalc_bin_printer_fadd( tree, 1 );

				// pos1�̔�r�܂Ŗ߂�
				apos2 = ecalc_bin_printer_jmp( tree, 0 );

				// ���݈ʒu�ۑ�
				pos3 = ecalc_bin_printer_get_pos( tree );

				// �W�����v�A�h���X���ߍ���
				ecalc_bin_printer_set_address( tree, apos1, pos3 - pos2 );
				ecalc_bin_printer_set_address( tree, apos2, pos1 - pos3 );

				// FPU�X�^�b�N�N���A
				ecalc_bin_printer_fstp_st0( tree );
				ecalc_bin_printer_fstp_st0( tree );
				ecalc_bin_printer_fstp_st0( tree );

				// }
			} else if ( token->value == ECALC_FUNC_IF ) {
				// if��
				// st(0) : 0
				// st(1) : left
				// �Ƃ��Ĕ�r

				// PUSH
				ecalc_bin_printer_fldz( tree );
				ecalc_bin_printer_fld( tree, left );

				// st(0)��st(1)���r
				ecalc_bin_printer_fcomi( tree, 1 );

				// ZF(FPU��C3)��1�Ȃ�st(0)==st(1)�Ȃ̂�pos2�܂ŃW�����v
				apos1 = ecalc_bin_printer_je( tree, 0 );

				// ���݈ʒu�ۑ�
				pos1 = ecalc_bin_printer_get_pos( tree );

				// POP
				ecalc_bin_printer_fstp_st0( tree );
				ecalc_bin_printer_fstp_st0( tree );

				// ���s ( �E�̒l�͎g��Ȃ��̂ōŌ��stp�͂�����POP�ł��悢 )
				ecalc_bin_printer_add_esp_i8( tree, -depth );
				ecalc_bin_printer_tree( tree, token->right );
				ecalc_bin_printer_add_esp_i8( tree, +depth );
				ecalc_bin_printer_load_var_ptr_to_eax( tree, 0 );
				ecalc_bin_printer_fstp( tree, right );

				// �_�~�[PUSH
				ecalc_bin_printer_fldz( tree );
				ecalc_bin_printer_fldz( tree );

				// ���݈ʒu�ۑ�
				pos2 = ecalc_bin_printer_get_pos( tree );

				// �W�����v�A�h���X���ߍ���
				ecalc_bin_printer_set_address( tree, apos1, pos2 - pos1 );

				// FPU�X�^�b�N�N���A
				ecalc_bin_printer_fstp_st0( tree );
				ecalc_bin_printer_fstp_st0( tree );

				// right = left
				ecalc_bin_printer_fld( tree, left );
				ecalc_bin_printer_fstp( tree, right );
			} else {
				// �E�ӂ̎�����
				ecalc_bin_printer_add_esp_i8( tree, -depth );
				ecalc_bin_printer_tree( tree, token->right );
				ecalc_bin_printer_add_esp_i8( tree, +depth );
				ecalc_bin_printer_load_var_ptr_to_eax( tree, 0 );
				ecalc_bin_printer_fstp( tree, right );
			}
		} else {
			// �E��0��
			ecalc_bin_printer_store_double_val( tree, right, 0 );
		}
	}

	// st(1) = left, st(0) = right
	ecalc_bin_printer_fld( tree, left );
	ecalc_bin_printer_fld( tree, right );

	// �v�Z ���ʂ̓X�^�b�N���x����2�̏�Ԃ�st(0)�ɓ����Ă�����̂Ƃ���
	// ���ꂼ�ꎩ����case����st(0)�Ɍ��ʂ����ăX�^�b�N���x����1�ɂ���
	switch ( (int)token->value ) {
	case ECALC_OPE_ADD:
		// �����Z
		ecalc_bin_printer_faddp( tree );
		break;
	case ECALC_OPE_SUB:
		// �����Z
		ecalc_bin_printer_fsubp( tree );
		break;
	case ECALC_OPE_MUL:
		// �|���Z
		ecalc_bin_printer_fmulp( tree );
		break;
	case ECALC_OPE_DIV:
		// ����Z
		// / 0�̓G���[�𔭐������Ȃ�
		// st(0) = 0�Ƃ���st(1) : right�Ɣ�r
		ecalc_bin_printer_fldz( tree );
		ecalc_bin_printer_fcomi( tree, 1 );

		// right == 0�Ȃ�pos2�֔��
		apos1 = ecalc_bin_printer_je( tree, 0 );

		// ���݈ʒu�ۑ�
		pos1 = ecalc_bin_printer_get_pos( tree );

		// right != 0�Ȃ̂Ŋ���Z
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fdivp( tree );

		// ��������pos3�̏I���܂ŃW�����v
		apos2 = ecalc_bin_printer_jmp( tree, 0 );

		// ���݈ʒu�ۑ�
		pos2 = ecalc_bin_printer_get_pos( tree );

		// right == 0�Ȃ̂�st(0)=0�Ƃ���
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fldz( tree );

		// ���݈ʒu�ۑ�
		pos3 = ecalc_bin_printer_get_pos( tree );

		// �W�����v�A�h���X����
		ecalc_bin_printer_set_address( tree, apos1, pos2 - pos1 );
		ecalc_bin_printer_set_address( tree, apos2, pos3 - pos2 );

		break;
	case ECALC_OPE_MOD:
		// �]��
		// % 0�̓G���[�𔭐������Ȃ�
		// st(0) = 0�Ƃ���st(1) : right�Ɣ�r
		ecalc_bin_printer_fldz( tree );
		ecalc_bin_printer_fcomi( tree, 1 );

		// right == 0�Ȃ�pos2�֔��
		apos1 = ecalc_bin_printer_je( tree, 0 );

		// ���݈ʒu�ۑ�
		pos1 = ecalc_bin_printer_get_pos( tree );

		// right != 0�Ȃ̂ŗ]����v�Z
		// ����POP and PUSH
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fstp( tree, arg2 );
		ecalc_bin_printer_fstp( tree, arg1 );

		// �֐��|�C���^��eax�Ƀ��[�h�E�R�[��
		ecalc_bin_printer_load_function_ptr_to_eax( tree, ecalc_get_func_addr( token->value ) );
		ecalc_bin_printer_add_esp_i8( tree, -depth );
		ecalc_bin_printer_call( tree );
		ecalc_bin_printer_add_esp_i8( tree, +depth );
		ecalc_bin_printer_load_var_ptr_to_eax( tree, 0 );

		// ��������pos3�̏I���܂ŃW�����v
		apos2 = ecalc_bin_printer_jmp( tree, 0 );

		// ���݈ʒu�ۑ�
		pos2 = ecalc_bin_printer_get_pos( tree );

		// right == 0�Ȃ̂�st(0)=0, st(1)=right�Ƃ���
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fldz( tree );

		// ���݈ʒu�ۑ�
		pos3 = ecalc_bin_printer_get_pos( tree );

		// �W�����v�A�h���X����
		ecalc_bin_printer_set_address( tree, apos1, pos2 - pos1 );
		ecalc_bin_printer_set_address( tree, apos2, pos3 - pos2 );

		break;
	case ECALC_OPE_STI:
		// ���
		// ���ӕϐ��|�C���^��EAX�Ƀ��[�h
		ecalc_bin_printer_load_exp_var_ptr_to_eax( tree, token->left->value );

		// ���
		ecalc_bin_printer_fstp( tree, 0 );
		ecalc_bin_printer_fstp_st0( tree );

		// st(0)�Z�b�g
		ecalc_bin_printer_fld( tree, 0 );

		// EAX���A
		ecalc_bin_printer_load_var_ptr_to_eax( tree, 0 );

		break;
	case ECALC_OPE_SEPA:
		// ��؂�
		ecalc_bin_printer_fxch_st1( tree );
		ecalc_bin_printer_fstp_st0( tree );
		break;
	case ECALC_OPE_LOOP:
		// �J��Ԃ�
		ecalc_bin_printer_fxch_st1( tree );
		ecalc_bin_printer_fstp_st0( tree );
		break;
	case ECALC_OPE_LBIG:
		// �����傫��

		// st(0)��st(1)���r
		ecalc_bin_printer_fcomi( tree, 1 );

		// POP
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fstp_st0( tree );

		// ZF(C3)��1�Ȃ�st(0)==st(1)�Ȃ̂�pos2�܂ŃW�����v
		apos1 = ecalc_bin_printer_je( tree, 0 );

		// ���݈ʒu�ۑ�
		pos1 = ecalc_bin_printer_get_pos( tree );

		// CF(FPU��C0)��1�Ȃ�st(0) < st(1)�Ȃ̂�pos3�܂ŃW�����v
		apos2 = ecalc_bin_printer_jb( tree, 0 );

		// ���݈ʒu�ۑ�
		pos2 = ecalc_bin_printer_get_pos( tree );

		// left <= right �Ȃ̂�0��PUSH
		ecalc_bin_printer_fldz( tree );

		// ��������pos3�̏I���܂ŃW�����v
		apos3 = ecalc_bin_printer_jmp( tree, 0 );

		// ���݈ʒu�ۑ�
		pos3 = ecalc_bin_printer_get_pos( tree );

		// right < left �Ȃ̂�1��PUSH
		ecalc_bin_printer_fld1( tree );

		// ���݈ʒu�ۑ�
		pos4 = ecalc_bin_printer_get_pos( tree );

		// �W�����v�A�h���X���ߍ���
		ecalc_bin_printer_set_address( tree, apos1, pos2 - pos1 );
		ecalc_bin_printer_set_address( tree, apos2, pos3 - pos2 );
		ecalc_bin_printer_set_address( tree, apos3, pos4 - pos3 );

		break;
	case ECALC_OPE_RBIG:
		// �E���傫��

		// st(0)��st(1)���r
		ecalc_bin_printer_fcomi( tree, 1 );

		// POP
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fstp_st0( tree );

		// ZF(C3)��1�Ȃ�st(0)==st(1)�Ȃ̂�pos3�܂ŃW�����v
		apos1 = ecalc_bin_printer_je( tree, 0 );

		// ���݈ʒu�ۑ�
		pos1 = ecalc_bin_printer_get_pos( tree );

		// CF(FPU��C0)��1�Ȃ�st(0) < st(1)�Ȃ̂�pos3�܂ŃW�����v
		apos2 = ecalc_bin_printer_jb( tree, 0 );

		// ���݈ʒu�ۑ�
		pos2 = ecalc_bin_printer_get_pos( tree );

		// right > left �Ȃ̂�1��PUSH
		ecalc_bin_printer_fld1( tree );

		// ��������pos3�̏I���܂ŃW�����v
		apos3 = ecalc_bin_printer_jmp( tree, 0 );

		// ���݈ʒu�ۑ�
		pos3 = ecalc_bin_printer_get_pos( tree );

		// right <= left �Ȃ̂�0��PUSH
		ecalc_bin_printer_fldz( tree );

		// ���݈ʒu�ۑ�
		pos4 = ecalc_bin_printer_get_pos( tree );

		// �W�����v�A�h���X���ߍ���
		ecalc_bin_printer_set_address( tree, apos1, pos3 - pos1 );
		ecalc_bin_printer_set_address( tree, apos2, pos3 - pos2 );
		ecalc_bin_printer_set_address( tree, apos3, pos4 - pos3 );

		break;
	case ECALC_OPE_EQU:
		// ����

		// st(0)��st(1)���r
		ecalc_bin_printer_fcomi( tree, 1 );

		// POP
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fstp_st0( tree );

		// ZF(C3)��1�Ȃ�st(0)==st(1)�Ȃ̂�pos2�܂ŃW�����v
		apos1 = ecalc_bin_printer_je( tree, 0 );

		// ���݈ʒu�ۑ�
		pos1 = ecalc_bin_printer_get_pos( tree );

		// right != left �Ȃ̂�0��PUSH
		ecalc_bin_printer_fldz( tree );

		// ��������pos3�̏I���܂ŃW�����v
		apos2 = ecalc_bin_printer_jmp( tree, 0 );

		// ���݈ʒu�ۑ�
		pos2 = ecalc_bin_printer_get_pos( tree );

		// right == left �Ȃ̂�1��PUSH
		ecalc_bin_printer_fld1( tree );

		// ���݈ʒu�ۑ�
		pos3 = ecalc_bin_printer_get_pos( tree );

		// �W�����v�A�h���X���ߍ���
		ecalc_bin_printer_set_address( tree, apos1, pos2 - pos1 );
		ecalc_bin_printer_set_address( tree, apos2, pos3 - pos2 );

		break;
	case ECALC_FUNC_SIN:
		// sin
		ecalc_bin_printer_fsin( tree );
		ecalc_bin_printer_fxch_st1( tree );
		ecalc_bin_printer_fstp_st0( tree );

		break;
	case ECALC_FUNC_COS:
		// cos
		ecalc_bin_printer_fcos( tree );
		ecalc_bin_printer_fxch_st1( tree );
		ecalc_bin_printer_fstp_st0( tree );

		break;
	case ECALC_FUNC_TAN:                /* tan */
	case ECALC_FUNC_ASIN:               /* asin */
	case ECALC_FUNC_ACOS:               /* acos */
	case ECALC_FUNC_ATAN:               /* atan */
	case ECALC_FUNC_LOG10:              /* log10 */
	case ECALC_FUNC_LOGN:               /* logn */
		// �����Ƃ���st(0) = right���Z�b�g
		ecalc_bin_printer_fstp( tree, arg1 );
		ecalc_bin_printer_fstp_st0( tree );

		// �֐��|�C���^��eax�Ƀ��[�h�E�R�[��
		ecalc_bin_printer_load_function_ptr_to_eax( tree, ecalc_get_func_addr( token->value ) );
		ecalc_bin_printer_add_esp_i8( tree, -depth );
		ecalc_bin_printer_call( tree );
		ecalc_bin_printer_add_esp_i8( tree, +depth );
		ecalc_bin_printer_load_var_ptr_to_eax( tree, 0 );

		break;
	case ECALC_FUNC_SQRT:
		// sqrt
		ecalc_bin_printer_fsqrt( tree );
		ecalc_bin_printer_fxch_st1( tree );
		ecalc_bin_printer_fstp_st0( tree );

		break;
	case ECALC_FUNC_POW:
	case ECALC_FUNC_ATAN2:
		// POWER, ATAN2

		// �����Ƃ���left : arg1, right : arg2���Z�b�g
		ecalc_bin_printer_fstp( tree, arg2 );
		ecalc_bin_printer_fstp( tree, arg1 );

		// �֐��|�C���^��eax�Ƀ��[�h�E�R�[��
		ecalc_bin_printer_load_function_ptr_to_eax( tree, ecalc_get_func_addr( token->value ) );
		ecalc_bin_printer_add_esp_i8( tree, -depth );
		ecalc_bin_printer_call( tree );
		ecalc_bin_printer_add_esp_i8( tree, +depth );
		ecalc_bin_printer_load_var_ptr_to_eax( tree, 0 );

		break;
	case ECALC_FUNC_RAD:
		// rad

		// ���j��
		ecalc_bin_printer_fxch_st1( tree );
		ecalc_bin_printer_fstp_st0( tree );

		// 180���
		ecalc_bin_printer_store_double_val( tree, dbuf, 180 );
		ecalc_bin_printer_fld( tree, dbuf );
		ecalc_bin_printer_fdivp( tree );

		// PI������
		ecalc_bin_printer_fldpi( tree );
		ecalc_bin_printer_fmulp( tree );

		break;
	case ECALC_FUNC_DEG:
		// deg

		// ���j��
		ecalc_bin_printer_fxch_st1( tree );
		ecalc_bin_printer_fstp_st0( tree );

		// PI���
		ecalc_bin_printer_fldpi( tree );
		ecalc_bin_printer_fdivp( tree );

		// 180������
		ecalc_bin_printer_store_double_val( tree, dbuf, 180 );
		ecalc_bin_printer_fld( tree, dbuf );
		ecalc_bin_printer_fmulp( tree );

		break;
	case ECALC_FUNC_PI:
		// ��
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fldpi( tree );
		// ecalc_bin_printer_store_double_val( tree, dbuf, M_PI );
		// ecalc_bin_printer_fld( tree, dbuf );

		break;
	case ECALC_FUNC_EPS0:
		// ��0
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_store_double_val( tree, dbuf, 8.85418782e-12 );
		ecalc_bin_printer_fld( tree, dbuf );

		break;
	case ECALC_FUNC_ANS:
		// ans
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_load_exp_ans_ptr_to_eax( tree );
		ecalc_bin_printer_fld( tree, 0 );
		ecalc_bin_printer_load_var_ptr_to_eax( tree, 0 );

		break;
	case ECALC_FUNC_IF:
		// if��

		// ���j�����ĉE��Ԃ�
		ecalc_bin_printer_fxch_st1( tree );
		ecalc_bin_printer_fstp_st0( tree );

		break;
	default:
		// �f�t�H���g�̓[��
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fstp_st0( tree );
		ecalc_bin_printer_fldz( tree );
	}

	// ��̃m�[�h�o�͊���
	// ���̎��_��FPU�X�^�b�N���x����1��st(0)�ɓ���������ׂ�
}

void ecalc_bin_printer_print( ECALC_JIT_TREE *tree, unsigned char *buf, size_t size )
{
	// ��������o�C�g�����������
	if ( tree->data != NULL ) {
		memcpy( tree->data + tree->pos, buf, size );
	}

	tree->pos += size;
}

size_t ecalc_bin_printer_get_pos( ECALC_JIT_TREE *tree )
{
	// ���݈ʒu�擾
	return tree->pos;
}

void ecalc_bin_printer_reset_tree( ECALC_JIT_TREE *tree )
{
	// �\���̂��N���A����i�T�C�Y����ȂǂɎg���j
	tree->pos  = 0;
	tree->size = 0;
	tree->data = NULL;
	tree->pos  = 0;
}

void ecalc_bin_printer_set_address( ECALC_JIT_TREE *tree, size_t pos, int32_t address )
{
	// �A�h���X����œ���邽�߂̕⏕�֐�
	if ( tree->data != NULL ) {
		memcpy( tree->data + pos, &address, sizeof( address ) );
	}
}

void ecalc_bin_printer_opening( ECALC_JIT_TREE *tree )
{
	// �I�[�v�j���O
	/*
	push ebp			55
	mov ebp, esp		8BEC �������� 89E5
	*/
	unsigned char bin[] = {0x55, 0x89, 0xE5};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_closing( ECALC_JIT_TREE *tree )
{
	// �N���[�W���O
	/*
	pop ebp				5D
	ret					C3
	*/

	unsigned char bin[] = {0x5D, 0xC3};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_load_arg_ptr_to_eax( ECALC_JIT_TREE *tree, int8_t val )
{
	// EBP��擪�Ƃ�������̃|�C���^��EAX�ɓǂݍ���
	/*
	 * lea eax, [ebp+val]	8D45vv
	 */
	unsigned char bin[] = {0x8D, 0x45, 0x00};

	bin[2] = val;

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_load_var_ptr_to_eax( ECALC_JIT_TREE *tree, int8_t val )
{
	// ESP����Ƃ����|�C���^��EAX�ɓǂݍ���
	/*
	 * lea eax, [esp+val]	8D4424vv
	 */

	unsigned char bin[] = {0x8D, 0x44, 0x24, 0x00};

	bin[3] = val;

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_load_eax_pointed_to_eax( ECALC_JIT_TREE *tree, int8_t val )
{
	// EAX�̎w����������ԏ�ɂ���l��EAX�ɃR�s�[
	/*
	 * mov eax, [eax+val]	8B40vv
	 */
	unsigned char bin[] = {0x8B, 0x40, 0x00};

	bin[2] = val;

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_push_dword_val(ECALC_JIT_TREE *tree, uint32_t val)
{
	// DWORD�l���X�^�b�N�ɐς�
	/*
	 * push 0xvvvvvvvv	68vvvvvvvv
	 */

	unsigned char bin[] = {0x68, 0x00, 0x00, 0x00, 0x00};

	memcpy( bin + 1, &val, sizeof(uint32_t) );

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_add_eax_i8(ECALC_JIT_TREE *tree, int8_t val)
{
	// eax��signed 8bit������
	/*
	 * add eax, v	83C0vv
	 */

	unsigned char bin[] = {0x83, 0xC0, 0x00};

	bin[2] = val;

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_store_dword_val(ECALC_JIT_TREE *tree, int8_t pos, uint32_t val)
{
	// eax+pos�̎w���ꏊ��dword�l���Z�b�g
	/*
	 * mov dword ptr [eax+pos], val		C740pp vvvvvvvv
	 */

	unsigned char bin[] = {0xC7, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};

	bin[2] = pos;
	memcpy( bin + 3, &val, sizeof(uint32_t) );

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_store_double_val(ECALC_JIT_TREE *tree, int8_t pos, double val)
{
	// double��eax+pos�̎w���ʒu�ɃX�g�A
	ecalc_bin_printer_store_dword_val( tree, pos,     *(uint32_t *)( ( (unsigned char *)&val ) + 0 ) );
	ecalc_bin_printer_store_dword_val( tree, pos + 4, *(uint32_t *)( ( (unsigned char *)&val ) + 4 ) );
}

void ecalc_bin_printer_push_dword(ECALC_JIT_TREE *tree, int8_t val)
{
	// eax+val�̎w������X�^�b�N�Ƀv�b�V��
	/*
	 * push dword ptr [eax+val]	FF70vv
	 */

	unsigned char bin[] = {0xFF, 0x70, 0x00};

	bin[2] = val;

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_push_qword(ECALC_JIT_TREE *tree, int8_t val)
{
	// eax+val�̎w������X�^�b�N��8�o�C�g�v�b�V��(�����A�h���X����)
	ecalc_bin_printer_push_dword( tree, val + 4 );
	ecalc_bin_printer_push_dword( tree, val );
}

void ecalc_bin_printer_pop_dword(ECALC_JIT_TREE *tree, int8_t val)
{
	// eax+val�̎w����ɃX�^�b�N����|�b�v
	/*
	 * pop dword ptr [eax+val]	8F40vv
	 */

	unsigned char bin[] = {0x8F, 0x40, 0x00};

	bin[2] = val;

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_pop_qword(ECALC_JIT_TREE *tree, int8_t val)
{
	// eax+val�̎w����ɃX�^�b�N����8�o�C�g�|�b�v(PUSH�Ƌt��)
	ecalc_bin_printer_pop_dword( tree, val );
	ecalc_bin_printer_pop_dword( tree, val + 4 );
}

void ecalc_bin_printer_fld(ECALC_JIT_TREE *tree, int8_t val)
{
	// eax+val�̎w���ʒu�ɂ���double��FPU��PUSH
	/*
	 * fld qword ptr [eax+val]	DD40vv
	 */

	unsigned char bin[] = {0xDD, 0x40, 0x00};

	bin[2] = val;

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fstp(ECALC_JIT_TREE *tree, int8_t val)
{
	// eax+val��FPU�̃g�b�v���X�g�A���ă|�b�v
	/*
	 * fstp qword ptr [eax+val]	DD58vv
	 */

	unsigned char bin[] = {0xDD, 0x58, 0x00};

	bin[2] = val;

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fistp( ECALC_JIT_TREE *tree, int8_t val )
{
	// eax+val��FPU�̃g�b�v��32bit�����t�����Ƃ��ăX�g�A���ă|�b�v
	/*
	 * fistp qword ptr [eax+val]	DF78vv
	 */

	unsigned char bin[] = {0xDF, 0x78, 0x00};

	bin[2] = val;

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fstp_st0( ECALC_JIT_TREE *tree )
{
	// FPU�X�^�b�N��POP���ăf�[�^�]���Ȃ�
	/*
	 * fstp st(0)	DDD8
	 */
	unsigned char bin[] = {0xDD, 0xD8};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fldz(ECALC_JIT_TREE *tree)
{
	// FPU��0���v�b�V��
	/*
	 * fldz	D9EE
	 */
	unsigned char bin[] = {0xD9, 0xEE};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fld1( ECALC_JIT_TREE *tree )
{
	// FPU��1���v�b�V��
	/*
	 * fld1 D9E8
	 */
	unsigned char bin[] = {0xD9, 0xE8};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fldpi(ECALC_JIT_TREE *tree)
{
	// FPU��pi���v�b�V��
	/*
	 * fldpi	D9EB
	 */
	unsigned char bin[] = {0xD9, 0xEB};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_faddp(ECALC_JIT_TREE *tree)
{
	// st(1) <- st(1) + st(0), POP
	/*
	 * faddp	DEC1
	 */
	unsigned char bin[] = {0xDE, 0xC1};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fadd( ECALC_JIT_TREE *tree, uint8_t st )
{
	// st(0) <- st(1) + st(0)
	/*
	 * fadd st(st)	D8Cs
	 */
	unsigned char bin[] = {0xD8, 0xC0};

	bin[1] = bin[1] | st;

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fsubp(ECALC_JIT_TREE *tree)
{
	// st(1) <- st(1) - st(0)
	/*
	 * fsubp	DEE9
	 */
	unsigned char bin[] = {0xDE, 0xE9};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fmulp(ECALC_JIT_TREE *tree)
{
	// st(1) <- st(1) * st(0)
	/*
	 * fmulp	DEC9
	 */
	unsigned char bin[] = {0xDE, 0xC9};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fdivp(ECALC_JIT_TREE *tree)
{
	// st(1) <- st(1) / st(0)
	/*
	 * fdivp	DEF9
	 */
	unsigned char bin[] = {0xDE, 0xF9};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fabs(ECALC_JIT_TREE *tree)
{
	// st(0) <- abs( st(0) )
	/*
	 * fabs	D9E1
	 */
	unsigned char bin[] = {0xD9, 0xE1};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fxch_st1(ECALC_JIT_TREE *tree)
{
	// st(0) <> st(1)
	/*
	 * fxch st(1)	D9C9
	 */
	unsigned char bin[] = {0xD9, 0xC9};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fchs( ECALC_JIT_TREE *tree )
{
	// st(0) <= -st(0)
	/*
	 * fchs	D9E0
	 */
	unsigned char bin[] = {0xD9, 0xE0};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fsin(ECALC_JIT_TREE *tree)
{
	// st(0) <- sin( st(0) )
	/*
	 * fsin	D9FE
	 */
	unsigned char bin[] = {0xD9, 0xFE};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fsqrt(ECALC_JIT_TREE *tree)
{
	// st(0) <- sqrt( st(0) )
	/*
	 * fsqrt	D9FA
	 */
	unsigned char bin[] = {0xD9, 0xFA};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fcos(ECALC_JIT_TREE *tree)
{
	// st(0) <- cos( st(0) )
	/*
	 * fsin	D9FF
	 */
	unsigned char bin[] = {0xD9, 0xFF};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_fcomi(ECALC_JIT_TREE *tree, uint8_t st)
{
	// CPU�̔�r�t���b�O���Z�b�g������ st(0)��st(st)�̔�r
	/*
	 * fcomi st(st)	DBFs
	 */
	unsigned char bin[] = {0xDB, 0xF0};

	bin[1] = bin[1] | st;

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_load_exp_ans_ptr_to_eax(ECALC_JIT_TREE *tree)
{
	// eax��ans�̃|�C���^�Z�b�g
	ecalc_bin_printer_load_arg_ptr_to_eax( tree, 12 );
}

void ecalc_bin_printer_load_exp_var_ptr_to_eax( ECALC_JIT_TREE *tree, int index )
{
	// eax��var[index]�̃|�C���^���Z�b�g
	unsigned char bin[] = {0x8D, 0x04, 0x90};

	// EAX��double**�̏ꏊ�����[�h
	ecalc_bin_printer_load_arg_ptr_to_eax( tree, 8 );

	// EAX��double**�̒l�i�����̒l�j�����[�h
	ecalc_bin_printer_load_eax_pointed_to_eax( tree, 0 );

	// index��edx�Ƀ��[�h
	ecalc_bin_printer_load_val_to_edx( tree, index );

	// �ϐ��̃A�h���X�v�Z
	// lea eax, [eax+edx*4]	8D0490
	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );

	// EAX��double*�̒l�����[�h
	ecalc_bin_printer_load_eax_pointed_to_eax( tree, 0 );
}

void ecalc_bin_printer_load_function_ptr_to_eax( ECALC_JIT_TREE *tree, void ( *func )( void ) )
{
	// eax�Ɋ֐��|�C���^�i���l�j��ǂݍ���
	/*
	 * mov eax, val	B8vvvvvvvv
	 */
	unsigned char bin[] = {0xB8, 0x00, 0x00, 0x00, 0x00};

	memcpy( bin + 1, &func, sizeof( void ( * )( void ) ) );

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_mov_eax_ecx(ECALC_JIT_TREE *tree)
{
	// ECX��EAX�ɃR�s�[
	/*
	 * mov eax, ecx	89C8
	 */
	unsigned char bin[] = {0x89, 0xC8};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_mov_ecx_eax(ECALC_JIT_TREE *tree)
{
	// EAX��ECX�ɃR�s�[
	/*
	 * mov ecx, eax	89C1
	 */
	unsigned char bin[] = {0x89, 0xC1};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_mov_eax_edx(ECALC_JIT_TREE *tree)
{
	// EDX��EAX�ɃR�s�[
	/*
	 * mov eax, edx	89D0
	 */
	unsigned char bin[] = {0x89, 0xD0};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_mov_edx_eax(ECALC_JIT_TREE *tree)
{
	// EAX��EDX�ɃR�s�[
	/*
	 * mov edx, eax	89C2
	 */
	unsigned char bin[] = {0x89, 0xC2};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_call( ECALC_JIT_TREE *tree )
{
	// EAX�̎w���֐����R�[��
	/*
	 * call eax	FFD0
	 */
	unsigned char bin[] = {0xFF, 0xD0};

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_load_val_to_edx(ECALC_JIT_TREE *tree, int32_t val)
{
	// EDX��32�r�b�g����val�����[�h
	/*
	 * mov edx, val	BAvvvvvvvv
	 */
	unsigned char bin[] = {0xBA, 0x00, 0x00, 0x00, 0x00};

	memcpy( bin + 1, &val, sizeof(int32_t) );

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

void ecalc_bin_printer_add_esp_i8(ECALC_JIT_TREE *tree, int8_t val)
{
	// ESP��val������
	/*
	 * add esp, val	83C4vv
	 */
	unsigned char bin[] = {0x83, 0xC4, 0x00};

	bin[2] = val;

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );
}

size_t ecalc_bin_printer_je(ECALC_JIT_TREE *tree, int32_t val)
{
	// Z=1�Ȃ�32bit���΃W�����v
	// �A�h���X����œ��͂��邽�߂ɃA�h���X�������ʒu��Ԃ�
	/*
	 * je �w���	0F84vvvvvvvv
	 */
	unsigned char bin[] = {0x0F, 0x84, 0x00, 0x00, 0x00, 0x00};

	memcpy( bin + 2, &val, sizeof(int32_t) );

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );

	return tree->pos - 4;
}

size_t ecalc_bin_printer_jmp(ECALC_JIT_TREE *tree, int32_t val)
{
	// 32bit���΃W�����v
	// �A�h���X����œ��͂��邽�߂ɃA�h���X�������ʒu��Ԃ�
	/*
	 * jmp �w���	E9vvvvvvvv
	 */
	unsigned char bin[] = {0xE9, 0x00, 0x00, 0x00, 0x00};

	memcpy( bin + 1, &val, sizeof(int32_t) );

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );

	return tree->pos - 4;
}

size_t ecalc_bin_printer_jb(ECALC_JIT_TREE *tree, int32_t val)
{
	// CF=1�Ȃ�32bit���΃W�����v
	// �A�h���X����œ��͂��邽�߂ɃA�h���X�������ʒu��Ԃ�
	/*
	 * je �w���	0F82vvvvvvvv
	 */
	unsigned char bin[] = {0x0F, 0x82, 0x00, 0x00, 0x00, 0x00};

	memcpy( bin + 2, &val, sizeof(int32_t) );

	ecalc_bin_printer_print( tree, bin, sizeof( bin ) );

	return tree->pos - 4;
}
