#ifndef _cube_h
#define _cube_h

#include <stdio.h>
#include <string.h>
#include "IsvList.H"
#include "arrayint.h"
#include "bddop.h"
class Sop;

#define isPosiLit(lit) ((lit) >= 0)
#define isNegaLit(lit) ((lit) < 0)
#define rmNegaLit(lit) (((lit) < 0) ? ~(lit) : (lit))

class Cube : public ILNode {    // 1�o�͂�����C�Y����0�����Node.input�Ɠ����D
public:
	Cube(int n_in) : size(n_in), rep( new char[n_in+1] ) {
	    for ( int i = n_in; --i>=0; ) rep[i] = 'x';
	    rep[n_in] = '\0';
	}
	Cube(int n_in, const char* val)
    	: size(n_in), rep( new char[n_in+1] ) { strcpy(rep, val); }
    	Cube(const Cube& orig) 
    	: size(orig.size), rep( new char[orig.size +1] ) {
	    strcpy(rep, orig.rep);
	}
    	virtual ~Cube() { delete [] rep; }

    int operator == (const Cube& obj);

    void	setValue(const char* val) { strcpy(rep, val); }
    char	value(int idx) { return rep[idx]; }
    void	setValue(int idx, char value) { rep[idx] = value; }
    void	rstr(int varIdx);	// varIdx: 1���̂܂� 0�ے�~
    void	jumpup(int i, int j); // �C���f�b�N�Xi��j�ɂ܂�jumpup
    int		empty() { return size <= 0 ; }
    void	print(FILE* sout=stdout, char* postString=(char*)"",
		      int* permutation=0);
	// rep[i] �� permutation[i]�̏ꏊ�ɕ\�������D0�̏ꍇ�� i �̏ꏊ�ɁD
    Bdd		getBdd(Arrayint& lev2idx);

private:
    int		size;	// �z��̒���(�ˑ�����ϐ��̐��ł͂Ȃ�)
    char*	rep;	// in[0]����͂��܂�D
			// �Ō��'0'������̂ŁCsize+1�̒����D
};

class CubeSet {
public:
    	CubeSet(int nIn) : numIn(nIn) {}
    	CubeSet(const CubeSet& orig) : numIn(orig.numIn), f1(orig.f1) {};
    	CubeSet(int nIn, const Sop& origSop);
    	~CubeSet() {}

    void	rstr(int varIdx);	// varIdx: 1���̂܂� 0�ے�~
    // scope���ōł������o������ϐ����CfreqLit�ɓ����D
    // ���e�����̐�/���́C��:freqLit�C��:~freqLit
    // freqLit�ϐ��̏o���񐔂�Ԃ��D
    void	compress();	// �֌W�̂Ȃ��ϐ�������
    void	jumpup(int i, int j); // �C���f�b�N�Xi��j�ɂ܂�jumpup
    int		nIn() { return numIn; }
    int		nCube() { return f1.size(); }
    void	print(FILE* sout=stdout, char* postString=(char*)"",
		      int* permutation=0);
    Bdd		getBdd(Arrayint& lev2idx);

    static void	printSop(Sop f, FILE* sout=stderr,
                             char* postString = (char*)"", int* var2idx=0);
    // var2idx: Sop��var(1����n�܂�)����C�\������idx(0����n�܂�)�ւ̎ʑ�
    static void printSop(const Bdd& f, FILE* sout=stderr, int* var2idx=0);


  // -------- adding by ger 1998 06 29  ------------
  // for ( Cube* ptr = a.first(); ptr; a.next(ptr) ) { use ptr; }
  Cube*          first() const { return (Cube*)f1.first(); }
  void        next(Cube*& ptr) const {
    f1.next(ptr);
  }
  // -------- adding by ger 1998 06 29  ------------



private:
    void	makeCubeSop(int nin, const Sop& f, Cube* nowCube, int nowno);

    int			numIn;
    IsvList<Cube>	f1;
};

#endif
