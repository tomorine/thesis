#ifndef _bddc_h
#define _bddc_h

#include <stdio.h>

typedef int bddid;
typedef unsigned int uint;

//// ����

extern const int BDDNULL; // null
extern const int MAXLEV; // 2^14 -1 = 0011..1 == 16384-1

extern void     bddalloc(int init_p, int max_p); // ������
extern void     bddstatus(); // ���Z���ʃe�[�u���̎g�p����\��
extern void     bddgc(); // �K�[�x�b�W�R���N�V����
extern int      bddused(); // �g�p�m�[�h��

extern bddid    bddcopy(bddid f); // �R�s�[
extern bddid    bddfree(bddid f); // �J��

extern bddid    bddvar(int lev); // �ϐ�x_lev��Ԃ�, 1 <= lev <= MAXLEV

extern bddid    bddvarset(bddid f); // �ˑ��ϐ�

extern int      bddtop(int f); // �ŏ�ʂ̕ϐ��̃��x��
extern int      bddsize(bddid f, int i, int j); // �m�[�h��
extern int      bddsize(bddid f, int lev); // �m�[�h��
extern int      bddsize(bddid f); // �m�[�h��

extern bddid    bddreadcache(int op, bddid f, bddid g); // ���Z�e�[�u���ǂ�
extern void     bddwritecache(int op, bddid f, bddid g, bddid h); //   ����

//// BDD

extern const int BDDF; // false, �^���l0
extern const int BDDT; // true, �^���l1

inline bddid    bddnot(bddid f) { return bddcopy(~f); } // NOT
extern bddid    bddand(bddid f, bddid g); // AND
inline bddid    bddor(bddid f, bddid g) { return ~bddand(~f, ~g); } // OR
extern bddid   	bddxor(bddid f, bddid g); // XOR

extern bddid    bddsmooth(bddid f, int lev); // f_{lev=0} + f_{lev=1}
inline bddid    bddconsensus(bddid f, int lev) { return ~bddsmooth(~f, lev); }

extern bddid   	bddrstr(bddid f, int lev); // lev: BDDF, ~lev: BDDT����
extern bddid    bddrstrtop0(bddid f); // �ŏ�ʂ̕ϐ��ɑ��
extern bddid    bddrstrtop1(bddid f); // �ŏ�ʂ̕ϐ��ɑ��

extern bddid	bddcompress(bddid f); // ���x�����l�߂�
extern bddid   	bddlevshift(bddid f, int lev, int degree); // ���x�����V�t�g
extern bddid   	bddjumpud(bddid f, int i, int j); // ���x����ύX

extern bddid    bddcofact(bddid f, bddid g); // g���P�A�Z�b�g�Ƃ���f���ȒP��
extern int      bddinter(bddid f, bddid g); // ����肪���邩�ǂ���
extern bddid    bddvarite(int lev, bddid f1, bddid f0);// x_lev f1 + ~x_lev f0
extern bddid    bddcompose(bddid f, int lev, bddid g); // f(x_lev = g)
extern uint     bdddense(bddid f); // �^���l�\���x

extern void     bddprintbdl(bddid f, FILE* bdlout); // BDL�`���ŏo��

//// ZBDD

extern const int ZBDDE; // empty  { }, ��W��
extern const int ZBDDB; // base {0..0}, 0..0�݂̂�v�f�Ƃ���W��

extern bddid    zbddchange(bddid f, int lev); // f�̑S�v�f��,lev�̗L���𔽓]
extern bddid    zbddsubset(bddid f, int lev); // lev: subset0, ~lev: subset1

extern bddid    zbddintersec(bddid f, bddid g); // �ϏW��
extern bddid    zbddunion(bddid f, bddid g); // �a�W��
extern bddid    zbdddiff(bddid f, bddid g); // ���W��
extern bddid    zbddproduct(bddid f, bddid g); // ���ϏW��
extern bddid    zbdddivide(bddid f, bddid g);  // f / g :  algebraic-division

extern uint     zbddcard(bddid f); // �W���̗v�f��
extern uint     zbddcard(bddid f, int lev); // lev���܂ޏW���̗v�f��
extern int      zbddmulti(bddid f); // zbddcard�̌��ʂ�0��1��2�ȏォ
extern int      zbddmulti(bddid f, int lev); // zbddcard�̌��ʂ�0��1��2�ȏォ
extern int      zbddlit(bddid f); // �W���̑S���e������
extern void     zbddlitcount(bddid f, uint* literals); // �e���e�����̏o����
extern void     zbddlitcount(bddid f, uint* literals, int max); // max�ȉ��̂�
extern void     zbddmultilit(bddid f, char* literals);

extern bddid    zbddcommonlit(bddid f);  // ���ׂĂ�cube�ɏo�Ă���lit�̘a��Ԃ�
extern bddid    zbddsum2product(bddid f); // �a�`��ό`��, a + b + c -> abc
extern bddid    zbddDelLitCube(bddid f); // 1���e������cube������

extern bddid    zbddsop2func(bddid f); // Sop�\���� f �ł���֐���Ԃ�

extern bddid    zbddjumpup(bddid f, int i, int j); // ���x����ύX
extern bddid    zbddlevshift(bddid f, int lev, int degree); // ���x�����V�t�g

#endif
