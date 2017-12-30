#include "cube.h"
#include "arrayint.h"
#include "isf.h"
#include "sop.h"

int Cube::operator == (const Cube& obj)
{
    if ( size != obj.size ) return 0;
    else if ( strcmp(rep, obj.rep) != 0 ) return 0;
    else return 1;
}

void Cube::rstr(int varIdx)	// BDDF: varIdx, BDDT: ~varIdx
{
    int value = ( varIdx < 0 ) ? 1 : 0;
    int idx = value ? ~varIdx : varIdx;

    if ( rep[idx] == '0' ) {
        if ( value == 0 ) rep[idx] = '-';
        else if ( value == 1 ) size = 0;
    }
    else if ( rep[idx] == '1' ) {
        if ( value == 0 ) size = 0;
        else if ( value == 1 ) rep[idx] = '-';
    }
}

void Cube::jumpup(int i, int j)
{
    int in_i = rep[i];
    for ( int k = i; k < j; k++ ) rep[k] = rep[k+1];
    rep[j] = in_i;
}

Bdd Cube::getBdd(Arrayint& lev2idx)
{
    Bdd result = Bdd::one;

    for ( int lev = lev2idx.size(); --lev > 0; ) {
        int idx = lev2idx[ lev ];
        if ( idx < 0 || idx >= size ) continue;

        if ( rep[idx] == '0' ) result = result & ~Bdd::var(lev);
        else if ( rep[idx] == '1' ) result = result & Bdd::var(lev);
    }
    return result;
}

void Cube::print(FILE* sout, char* postString, int* permutation)
{
    if ( permutation ) {
        /* newline �� permutation ������� cube �\�������D
         permutation �̓��e�����āC�K�v�ŏ����̒����ɂ��� */
        int maxp = -1;
        for ( int i=size; --i>=0; ) {
            int p = permutation[i];
            if ( p < 0 ) continue;
            if ( p > maxp ) maxp = p;
        }

        char* newline = new char[maxp+2];
        newline[maxp+1] = '\0';
        for ( int i=maxp; --i>=0; ) newline[i] = '-';

        for ( int i=size; --i>=0; ) {
            int p = permutation[i];
            if ( p < 0 ) continue;
            newline[p] = rep[i];
        }

        fprintf(sout, "%s%s\n", newline, postString);
        delete [] newline;
    }
    else {
        /* permutation ���Ȃ���΁C�P�ɕ\�� */
        fprintf(sout, "%s%s\n", rep, postString);
    }
}

////////////////////

void CubeSet::compress()
{
    int i, j;
    int n = nIn();
    Arrayint depend( n );
    for ( i = n; --i>=0; ) depend[i] = 0;

    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        for ( i = n; --i>=0; )
            if ( c->value(i) == '0' || c->value(i) == '1' ) depend[i] = 1;
    }

    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        for ( i=0, j=0; i<n; i++ ) {
            if ( depend[i] != 0 ) c->setValue( j++, c->value(i) );
        }
    }

    numIn = j;

    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        c->setValue( numIn, '\0' );
    }
}

void CubeSet::rstr(int varIdx)	// varIdx: 1���̂܂� 0�ے�~
{
    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        c->rstr(varIdx);
    }	

    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        if ( c->empty() ) {
            Cube* delc = f1.remove(c);
            delete delc;
        }
    }
}

void CubeSet::print(FILE* sout, char* postString, int* permutation)
{
    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        c->print(sout, postString, permutation);
    }
}

void CubeSet::jumpup(int i, int j) // �C���f�b�N�Xi��j�ɂ܂�jumpup
{
    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        c->jumpup(i,j);
    }
}

Bdd CubeSet::getBdd(Arrayint& lev2idx)
{
    Bdd result = Bdd::zero;
    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        result = result | c->getBdd(lev2idx);
    }
    return result;
}

// ����CubeSet�̃R���X�g���N�^�Ƃ��̎���makeCubeSop�̓y�A�ɂȂ��Ă���D
// �����̒��� Cube() �� new ���Ă���Ƃ��낪3�ӏ����邪�C
// ������ Cube() �́CmakeCubeSop�̏I�[�P�[�X�ŁC
// f1.append��f1�ɕt�����邩�C�����Ȃ����delete �����D

CubeSet::CubeSet(int nIn, const Sop& origSop) : numIn(nIn)
{
    Cube* nowCube = new Cube(nIn);
    makeCubeSop(nIn, origSop, nowCube, 0);
}

void CubeSet::makeCubeSop(int nin, const Sop& f, Cube* nowCube, int nowno)
{
    int check = nin - nowno;  // ���̔ԍ��̔z��܂ŁA�����Ƃ͂����Ă�B

    if ( f == Sop::one ) {// ���̎��̂ݕt��������DNowCube�ɍ��܂ł̂��ݒ�D
        for ( int i=0; i<check; i++ ) //�����艺�́ANowCube���ݒ肳��ĂȂ��B
            nowCube->setValue(i, '-');
        f1.append( nowCube );
        return;
    }
    else if ( f == Sop::zero ) {
        delete nowCube;
        return; // ���̎��͕t����������return
    }

    // 2�R�s�[����3�ɂ���D
    Cube* newCube0 = new Cube( *nowCube );
    Cube* newCube1 = new Cube( *nowCube );
    Cube* newCubeR = nowCube;

    // Cube�̓Y����0����n-1
    // var��1����n
    int var = f.topvar();

    //ISOP�̕ϐ��ԍ������ł邱�Ƃ�����̂�
    for ( int i = var; i<check; i++ ) {
        newCube0->setValue(i, '-');
        newCube1->setValue(i, '-');
        newCubeR->setValue(i, '-'); 
        nowno++;
    }

    newCube0->setValue(var-1, '0');
    newCube1->setValue(var-1, '1');
    newCubeR->setValue(var-1, '-');

    makeCubeSop(nin, f.factor0(var), newCube0, nowno+1);
    makeCubeSop(nin, f.factor1(var), newCube1, nowno+1);
    makeCubeSop(nin, f.factorR(var), newCubeR, nowno+1);
}
    
void CubeSet::printSop(Sop f, FILE* sout,
                       char* postString, int* var2idx)
{    // var2idx: Sop��var(1����n�܂�)����C�\������idx(0����n�܂�)�ւ̎ʑ�
    if ( f == Sop::null ) return;
    int dummyN = f.topvar();
    CubeSet cs = CubeSet(dummyN, f); // �Y��: Sop��1����CCubeSet��0����
    if ( var2idx ) {
        cs.print(sout, postString, var2idx+1);
    	// CubeSet��print�̈����ł���permutation�ɕϊ����邽�߁D
    	// ���ʓI��var��1�����炷���ƂɂȂ�D
    }
    else {
        cs.print(sout, postString, 0);
    }
}

void CubeSet::printSop(const Bdd& f, FILE* sout, int* var2idx)
{
    if ( f == Bdd::one ) {
        fprintf(sout, "1\n");
        return;
    }
    else if ( f == Bdd::null || f == Bdd::zero ) return;

    Sop isop = IsfLU(f).makeISOP();

    Sop NOTisop = IsfLU(~f).makeISOP(); 

    if( NOTisop.nCube() < isop.nCube())
        CubeSet::printSop(NOTisop, sout, (char*)" 0", var2idx);
    else
        CubeSet::printSop(isop, sout, (char*)" 1", var2idx);
}
