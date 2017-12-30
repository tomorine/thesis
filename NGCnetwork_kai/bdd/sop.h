#ifndef _sop_h
#define _sop_h

#include <stdio.h>
#include <vector>

#include "bddop.h"
#include "utility.h"

using namespace std;

class Sop {	// sum of product
public:
    static const Sop	null;
    static const Sop	zero;
    static const Sop	one;

    Sop() : rep( Bdd::null ) { ++objnum; }
    Sop(const Bdd& r) : rep(r) { ++objnum; }
    Sop(const Sop& orig) : rep( orig.rep ) { ++objnum; }
    ~Sop() { objnum--; }
    static int objnum;

    Sop		operator = (const Sop& orig);
    friend int  operator == (const Sop& lhs, const Sop& rhs)
    	{ return lhs.rep == rhs.rep; }
    friend int  operator != (const Sop& lhs, const Sop& rhs)
    	{ return lhs.rep != rhs.rep; }

    /*------ var �� 1����n�܂� ------*/
    Sop	and0(int var) const { return and_lit( var2lev0(var) ); }
    Sop	and1(int var) const { return and_lit( var2lev1(var) ); }
    Sop and_lit(int lit) const;
    Sop	factor0(int var) const;
    Sop	factor1(int var) const;
    Sop	factorR(int var) const;
    Sop factor_lit(int lit) const { return rep.subset1(lit); }

    static Sop lit0(int var) { return Bdd::var( var2lev0(var) ); }
    static Sop lit1(int var) { return Bdd::var( var2lev1(var) ); }
    static Sop litFromLev(int lev) { return Bdd::var(lev); }

    friend Sop	operator + (const Sop& f, const Sop& g)
	{ return f.rep + g.rep; }
    friend Sop	operator - (const Sop& f, const Sop& g)
    	{ return f.rep - g.rep; }
    friend Sop	operator / (const Sop& f, const Sop& g)
	{ return f.rep / g.rep; }
    friend Sop	operator * (const Sop& f, const Sop& g)
    	{ return f.rep * g.rep; } // a ~a�̈��������Ȃ�(���̂���葬��)

    friend Sop	sopProduct(const Sop& f, const Sop& g);
    				  // a ~a -> 0 �̈���������

    static Sop	intersec(const Sop& f, const Sop& g) {
	return Bdd::intersec(f.rep, g.rep);
    }

    Sop	intersected(const Bdd& g) const; // g�ƌ����̂���cube�W��
    Sop	included(const Bdd& g) const // g �Ɋ܂܂��cube�W��
    	{ return *this - (*this).intersected(~g); }
    Sop	expandCubes(const Bdd& offset) const;
    // offset�ɏd�Ȃ�Ȃ��悤�Ɋecube��傫������Dredundant��������Ȃ�
    Sop irredundant(const Bdd& onset) const;
    // onset��cover����͈͂ŁC�璷��cube�폜����D
    Sop compose(int level, Sop posi, Sop nega);

    Sop distance1merge(); // ab' + ab -> a �̏������s��
    Sop sccMin(); // ab' + a -> a �̏������s���D�ł��s���S�D

    Sop maxLit() const; // �ł���������郊�e������Ԃ��D
    Sop dupLit() const { return rep.dupLit(); } // 2��ȏ㌻��邠�郊�e����

    Sop getKernel() const;
    int getSdivisor(int wdmax, int max, Sop& result); // double cube divisor

    Sop most_common_literal(Sop cube);
    Sop commonCube() const;

    int		topvar() const;
    int		toplit() const;
    int		isTopNeg() const { return rep.top() & 1; }
    		// top�̕ϐ��������e�������ǂ���
    void	printX() const { rep.printX(); }
    void        fprint(FILE* fp) const;
    void        sparsePrint() const;

    int		nCube() const { return rep.card(); }
    int		nLit() const { return rep.lit(); }
    Bdd		getFunc(int* lev2idx=0) const;
    // lev2idx�̒ʂ�ɕϐ��������ւ���Dlev2idx[0]�ɔz��̒��������邱�ƁD
    // lev2idx=0�̂Ƃ���var��level�������D
    Sop		changeOrder(const vector<int>& permutation) const;
    Bdd		varSet() const;	// �ˑ�����ϐ��W����ZBdd�ŕԂ��D
    void        varSet(Arrayint& varset) const; // �ˑ�����ϐ��W����z��ŕԂ�
                                               // varset��clear()����Ă��邱��
    void        litSet(Arrayint& litset) const; // �ˑ�����ϐ��W����z��ŕԂ�
                                               // litset��clear()����Ă��邱��
    int		nIn() const { return varSet().size(); } // �ˑ�����ϐ��̐��D

    int		value() const { return rep.value(); }
    Bdd		getRep() const { return rep; }
    Sop		getAcube() const { return rep.getAelement(); }

    Sop		varshift(int var, int degree) const;
    Sop		jumpup(int i, int j) const;
    Sop		jumpdown(int i, int j) const; // jumpup �Ŏ���
    Sop		rstr0(int var) const { return factor0(var) + factorR(var); }
    Sop		rstr1(int var) const { return factor1(var) + factorR(var); }
    Sop		rstrMinterm(const Bdd& minterm) const;

    static int  getComplementedLit(int lit);

protected:
    Bdd		rep;

    static int	var2lev0(int var) { return (var<<1) -1; }
    static int	var2lev1(int var) { return var<<1; }
    static int	lev2var(int lev) { return (lev+1) >> 1; }
};

inline int sopHash(const Sop& key) { return key.value(); }

class SopM {	// sum of product for multi-output (and don't care)
    enum { DEF_NIN = 256 };
public:
    SopM() : cset(Sop::null), numIn(0) {}
    SopM(const Sop& s, int nin = DEF_NIN) : cset(s), numIn(nin) {}
    						// ���͐����w��
    SopM(const SopM& orig) : cset( orig.cset ), numIn(orig.numIn) {}
    ~SopM() {}

    SopM	operator = (const SopM& orig);
    friend int   operator == (const SopM& lhs, const SopM& rhs) {
	return (lhs.numIn == rhs.numIn) && (lhs.cset == rhs.cset);
    }
    friend int   operator != (const SopM& lhs, const SopM& rhs) {
	return (lhs.numIn != rhs.numIn) || (lhs.cset != rhs.cset);
    }

    /*------ var �� 1����n�܂� ------*/
    /*------ oidx �� 1����n�܂� ------*/
    SopM	operator += (const SopM& rhs) {
	assert( numIn == rhs.numIn );
	cset = cset + rhs.cset;
	return *this;
    }

    // onset �� dcset �Ŏw�肷��Doffset�͎g��Ȃ�
    SopM	putOnset(int oidx)    // ���g��oidx��onset�Ƃ���
    	{ cset = cset.and1(oidx+numIn); return *this; }
    SopM	putDcset(int oidx)    // ���g��oidx��dcset�Ƃ���
    	{ cset = cset.and0(oidx+numIn); return *this; }
    // lev     1 2 3 4 5 6   7   8   9   10  11  12
    // oidx    (numIn = 3)  1dc 1on 2dc 2on 3dc 3on ...	
    // ���e����0 �� dcset����C���e����1 �� onset������

    Sop		getOnset(int oidx);
    Sop		getDcset(int oidx);
    Sop		elimOidx();
    	// �o�͂̃��e���������������́C�S�o�͂�Cube�̘a��Ԃ��D

    void	printX() { cset.printX(); }
    int		nCube() { return cset.nCube(); }
    Sop		getSop() { return cset; }
private:
    Sop		cset;
    int		numIn;
};

#endif
