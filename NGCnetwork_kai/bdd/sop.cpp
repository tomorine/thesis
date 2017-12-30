#include "sop.h"
#include "isf.h"
#include "cube.h"
#include "ger-base.h"

int Sop::objnum = 0;

Sop Sop::operator = (const Sop& orig)
{
    if ( *this != orig ) {
	rep = orig.rep;
    }
    return *this;
}

// lev	1  2  3  4  5  6  ...
// var	1' 1  2' 2  3' 3  ...

Sop Sop::and_lit(int lit) const
{
    int var = lev2var(lit);
    if ( var <= topvar() ) { // var�����Ɋ܂�ł�����
	int lev1 = var2lev1(var);
	int lev0 = var2lev0(var);
	Bdd fd = rep.subset0(lev1).subset0(lev0);// �������e�������܂܂Ȃ�����
	Bdd flit = rep.subset1(lit); 		// ���e�����Ŋ���������
	return (flit + fd).change(lit);
    }
    else return rep.change(lit);
}

Sop Sop::factor0(int var) const
{
    int lev0 = var2lev0(var);
    return rep.subset1(lev0);	 // �����e�����Ŋ���������
}

Sop Sop::factor1(int var) const
{
    int lev1 = var2lev1(var);
    return rep.subset1(lev1);	 // �����e�����Ŋ���������
}

Sop Sop::factorR(int var) const
{
    int lev0 = var2lev0(var);
    int lev1 = var2lev1(var);
    return rep.subset0(lev1).subset0(lev0);	// �������e�������܂܂Ȃ�����
}

int Sop::topvar() const
{
    int lev = rep.top();
    return lev2var(lev);
}

int Sop::toplit() const
{
    return rep.top();
}

Bdd Sop::getFunc(int* lev2idx) const
{
    if ( lev2idx ) {
    // todo:  vector<int> tmparray(lev2idx[0]+1, lev2idx);
    // todo: ��������
    vector<int> tmparray;
    for (int i=0; i < lev2idx[0]+1; i++) {
        tmparray.push_back(lev2idx[i]);
    }

	tmparray[0] = 0;
	return changeOrder(inverse(tmparray)).rep.sop2func();
    }
    else return rep.sop2func();
}

/* �^����ꂽpermutation�ɏ]���ĕϐ�����ύX����D
   permutation��lev2idx�ƌ��āC����ɍ��킹��悤�ɕϐ�����ύX����̂ł͂Ȃ��D
   �ϐ�����lev2idx�ɂ��邽�߂ɂ́Cpermutation��lev2idx�̋t�ʑ���^����D
   �t�ɁC�ϐ�����lev2idx�ł�����̂�normalize����ɂ́C
   permutation��lev2idx���̂��̂�^����΂悢�D*/
Sop Sop::changeOrder(const vector<int>& permutation) const
{
    int n = permutation.size();
    vector<int> localpermu(permutation);

    Sop result = *this;
    // �傫�������猈�߂Ă���
    for ( int dest=n; --dest>0; ) {
	if ( localpermu[dest] == dest ) continue;
	for ( int idx=dest; --idx>0; ) {
	    if ( localpermu[idx] == dest ) {
// 		printf("jumpup( %d, %d )\n", idx, dest);
		result = result.jumpup(idx, dest);
        jumpUD<vector<int>>(localpermu, idx, dest);
		break;
	    }
	}
    }
//     for ( int idx=n; --idx>0; ) assert( localpermu[idx] == idx );
    return result;
}

Sop Sop::varshift(int var, int degree) const
{
    Sop result;
    // lev0�̕����������D2�{shift�D
    result.rep = rep.zlevshift(var2lev0(var), (degree<<1));
    return result;
}

Sop Sop::jumpdown(int i, int j) const
{
    assert( i >= j );
    Sop result = *this;
    for ( int k = i; k > j; k-- ) {
	result = result.jumpup(k-1, k);
    }
    return result;
}

Sop Sop::jumpup(int i, int j) const
{
    assert( i <= j );
    Bdd result = rep.zjumpup(var2lev1(i), var2lev1(j));
    result = result.zjumpup(var2lev0(i), var2lev0(j));
    return result;
}

Bdd Sop::varSet() const	// �ˑ�����ϐ��W����ZBdd�ŕԂ��D
{
    int toplev = rep.top();
    int topvar = lev2var(toplev);

    Bdd pnsup = rep.varSet();
    int n_lev = topvar<<1;
    int* lev_list = new int[ n_lev +1 ]; // level�̃��X�g
    for ( int l = n_lev; l>=0; l-- ) lev_list[l] = 0;

    while ( pnsup != Bdd::empty ) {
	int lev = pnsup.top();
	lev_list[lev] = 1;
	pnsup = pnsup.subset0(lev);
    }

    Bdd result = Bdd::empty;
    for ( int i=1; i<=topvar; i++ ) {
	if ( lev_list[var2lev1(i)] | lev_list[var2lev0(i)] )
	    result = result + (Bdd::base).change(i);
    }
    delete [] lev_list;
    return result;
}

void Sop::varSet(Arrayint& varset) const // �ˑ�����ϐ��W����z��ŕԂ��D
{                                        // varset��clear()����Ă��邱��
    Bdd pnsup = rep.varSet();
    while ( pnsup != Bdd::empty ) {
	int lev = pnsup.top();
	int var = lev2var(lev);
	int size = varset.size();
	if ( (size == 0) || (varset[ size-1 ] != var) ) varset.add(var);
	pnsup = pnsup.subset0(lev);
    }
}

void Sop::litSet(Arrayint& litset) const // �ˑ�����ϐ��W����z��ŕԂ��D
{                                        // varset��clear()����Ă��邱��
    Bdd pnsup = rep.varSet();
    while ( pnsup != Bdd::empty ) {
	int lit = pnsup.top();
	litset.add(lit);
	pnsup = pnsup.subset0(lit);
    }
}

Sop Sop::rstrMinterm(const Bdd& minterm) const
{
    int lev = minterm.top();
    if ( lev == 0 ) {
	assert(minterm == Bdd::one);
	return *this;
    }

    Bdd f1 = minterm.rstrtop1();
    Bdd f0 = minterm.rstrtop0();
    if ( f1 != Bdd::zero ) {
	return ( factor1(lev) + factorR(lev) ).rstrMinterm(f1);
    }
    else if ( f0 != Bdd::zero ) {
	return ( factor0(lev) + factorR(lev) ).rstrMinterm(f0);
    }
    else {
	assert(0);
	return Sop::null;
    }
}

// * �����́CZBdd��Sop�ň���Ă���Da ~a�̈����D
Sop sopProduct(const Sop& f, const Sop& g)
{
    /*------ terminal cases ------*/
    if ( f == Sop::zero || g == Sop::zero ) return Sop::zero;
    else if ( f == Sop::one ) return g;
    else if ( g == Sop::one ) return f;

    /*------ normalize (swap f and g if needed) ------*/
    int flev = f.topvar();
    int glev = g.topvar();
    if ( flev < glev ) return sopProduct(g, f);

    /*------ read the result table ------*/
    Bdd cache = Bdd::readCache(Bdd::SOP_PRODUCT, f.rep, g.rep);
    if ( cache != Bdd::null ) return Sop(cache);

    /*-------- recursive computation --------*/
    Sop f0 = f.factor0(flev);
    Sop f1 = f.factor1(flev);
    Sop fR = f.factorR(flev);
    Sop h0, h1, hR;

    if ( flev == glev ) {
	Sop g0 = g.factor0(glev);
	Sop g1 = g.factor1(glev);
	Sop gR = g.factorR(glev);

	h0 = sopProduct(f0, g0) + sopProduct(f0, gR) + sopProduct(g0, fR);
	h1 = sopProduct(f1, g1) + sopProduct(f1, gR) + sopProduct(g1, fR);
	hR = sopProduct(fR, gR);
    }
    else {
	h0 = sopProduct(f0, g);
	h1 = sopProduct(f1, g);
	hR = sopProduct(fR, g);
    }
    Sop h = h0.and0(flev) + h1.and1(flev) + hR;
    /*------ write to the result table ------*/
    Bdd::writeCache(Bdd::SOP_PRODUCT, f.rep, g.rep, h.rep);
    return h;
}

/* Sop����Cg�ƌ����̂���cube�݂̂����o�� */
Sop Sop::intersected(const Bdd& g) const
{
    /*------ terminal cases ------*/
    if ( g == Bdd::one ) return *this;
    else if ( g == Bdd::zero ) return Sop::zero;

    int top = topvar();
    if ( top == 0 ) return *this; // g��zero�ł͂Ȃ�

    /*------ read the result table ------*/
    Bdd cache = Bdd::readCache(Bdd::INTERSECTED, rep, g);
    if ( cache != Bdd::null ) return Sop(cache);

    /*-------- recursive computation --------*/
    int glev = g.top();
    Sop h;
    if ( glev >= top ) { // �����𖳂ɂ��Ă����炾���ł��ǂ���������Ȃ�
	Sop f0 = factor0(glev);
	Sop f1 = factor1(glev);
	Sop fR = factorR(glev);
	Bdd g0 = g.rstrtop0();
	Bdd g1 = g.rstrtop1();
	Sop h0 = ( f0 != Sop::zero ) ? f0.intersected(g0) : Sop::zero;
	Sop h1 = ( f1 != Sop::zero ) ? f1.intersected(g1) : Sop::zero;
	Sop hR = ( fR != Sop::zero ) ? fR.intersected(g0 | g1) : Sop::zero;
	h = h0.and0(glev) + h1.and1(glev) + hR;
    }
    else { // glev < top, g��top�Ɉˑ����Ȃ�
	Sop f0 = factor0(top);
	Sop f1 = factor1(top);
	Sop fR = factorR(top);
	Sop h0 = ( f0 != Sop::zero ) ? f0.intersected(g) : Sop::zero;
	Sop h1 = ( f1 != Sop::zero ) ? f1.intersected(g) : Sop::zero;
	h = h0.and0(top) + h1.and1(top) + fR.intersected(g);
    }

    /*------ write to the result table ------*/
    Bdd::writeCache(Bdd::INTERSECTED, rep, g, h.rep);
    return h;
}

Sop Sop::expandCubes(const Bdd& offset) const
{
//     Sop tmpc = *this;
//     assert( tmpc.getFunc() <= ~offset ); // debug�p�C���Ԃ�������D

    /*------ terminal cases ------*/
    int top = topvar();
    if ( top == 0 ) return *this;
    else if ( offset == Bdd::zero ) return Sop::one;

    /*------ read the result table ------*/
    Bdd cache = Bdd::readCache(Bdd::EXPAND_CUBE, rep, offset);
    if ( cache != Bdd::null ) return Sop(cache);

    /*-------- recursive computation --------*/
    // lev�̑傫���ϐ�����expand����Ă���
    Sop f0 = factor0(top);
    Sop f1 = factor1(top);
    Sop fR = factorR(top);

    Sop f0n = f0.intersected(offset);		// top��expand�ł��Ȃ�
    Sop f0e = f0 - f0n;				// top��expand�ł���
    Sop f1n = f1.intersected(offset);		// top��expand�ł��Ȃ�
    Sop f1e = f1 - f1n;				// top��expand�ł���

    Bdd offset0 = offset.rstr0(top);
    Bdd offset1 = offset.rstr1(top);
    Sop h0 = f0n.expandCubes(offset0);
    Sop h1 = f1n.expandCubes(offset1);
    Sop hR = (fR + f0e + f1e).expandCubes( offset0 | offset1 );
              //top�Ɉˑ����Ȃ�cube����

    Sop h = h0.and0(top) + h1.and1(top) + hR;

    /*------ write to the result table ------*/
    Bdd::writeCache(Bdd::EXPAND_CUBE, rep, offset, h.rep);

//     tmpc = *this;
//     assert( tmpc.getFunc() <= ~offset ); // debug�p�C���Ԃ�������D
    return h;
}    

// onset��cover����͈͂ŁC�璷��cube�폜����D
Sop Sop::irredundant(const Bdd& onset) const
{
    /*------ terminal cases ------*/
    if ( onset == Bdd::zero ) return Sop::zero;
    int top = topvar();
    if ( top == 0 ) return *this;

    /*------ read the result table ------*/
    Bdd cache = Bdd::readCache(Bdd::IRREDUNDANT, rep, onset);
    if ( cache != Bdd::null ) return Sop(cache);

    /*-------- recursive computation --------*/
    Sop f0 = factor0(top);
    Sop f1 = factor1(top);
    Sop fR = factorR(top);

    // lev�̑傫���ϐ��قǋ`�������Ȃ��̂ŁC������܂�cube����폜����₷��
    // lev�̑傫���ϐ��قǋ`���𑽂����邱�Ƃ��\�ł���Ǝv��
    Bdd coverR = fR.getFunc();
    Sop h0 = ( f0 != Sop::zero ) ? f0.irredundant(onset.rstr0(top) & ~coverR)
	                         : Sop::zero;
    Sop h1 = ( f1 != Sop::zero ) ? f1.irredundant(onset.rstr1(top) & ~coverR)
	                         : Sop::zero;
    Sop hR = ( fR != Sop::zero ) ? fR.irredundant( onset &
			~(Bdd::varIte( top, h1.getFunc(), h0.getFunc() )) )
	                         : Sop::zero;
    Sop h = h0.and0(top) + h1.and1(top) + hR;

    /*------ write to the result table ------*/
    Bdd::writeCache(Bdd::IRREDUNDANT, rep, onset, h.rep);
    return h;
}

Sop Sop::distance1merge() // ab' + ab -> a �̏������s��
{
    /*------ terminal cases ------*/
    int top = topvar();
    if ( top == 0 ) return *this;

    /*------ read the result table ------*/
//      Bdd cache = Bdd::readCache(Bdd::EXPAND_CUBE, rep, offset);
//      if ( cache != Bdd::null ) return Sop(cache);

    /*-------- recursive computation --------*/
    // lev�̑傫���ϐ�����expand����Ă���
    Sop f0 = factor0(top);
    Sop f1 = factor1(top);
    Sop fR = factorR(top);

    Sop common = intersec(f0, f1);

    Sop h0, h1, hR;
    if ( common != Sop::zero ) {
	Sop f0nocom = f0 - common;
	Sop f1nocom = f1 - common;
	h0 = f0nocom.distance1merge();
	h1 = f1nocom.distance1merge();
	hR = (fR + common).distance1merge();
    }
    else {
	h0 = f0.distance1merge();
	h1 = f1.distance1merge();
	hR = fR.distance1merge();
    }

    Sop h = h0.and0(top) + h1.and1(top) + hR;
    /*------ write to the result table ------*/
//      Bdd::writeCache(Bdd::EXPAND_CUBE, rep, offset, h.rep);

    return h;
}

// ab' + a -> a �̏������s���D�ł�����ł͕s���S�D
// kernel�𐶐����Ă�����肵�Ȃ��Ɗ��S�ɂ͂Ȃ�Ȃ��D
Sop Sop::sccMin()
{
    /*------ terminal cases ------*/
    int top = topvar();
    if ( top == 0 ) return *this;

    if ( this->value() < 0 ) return Sop::one;
    
    /*------ read the result table ------*/
//      Bdd cache = Bdd::readCache(Bdd::EXPAND_CUBE, rep, offset);
//      if ( cache != Bdd::null ) return Sop(cache);

    /*-------- recursive computation --------*/
    // lev�̑傫���ϐ�����expand����Ă���
    Sop f0 = factor0(top);
    Sop f1 = factor1(top);
    Sop fR = factorR(top);

    Sop common0 = intersec(f0, fR);
    Sop common1 = intersec(f1, fR);

    Sop h0, h1, hR;
    if ( common0 != Sop::zero ) {
	Sop f0nocom = f0 - common0;
	h0 = f0nocom.sccMin();
	h1 = f1.sccMin();
    }
    else if ( common1 != Sop::zero ) {
	Sop f1nocom = f1 - common1;
	h1 = f1nocom.sccMin();
	h0 = f0.sccMin();
    }
    else {
	h0 = f0.sccMin();
	h1 = f1.sccMin();
    }
    hR = fR.sccMin();
	
    Sop h = h0.and0(top) + h1.and1(top) + hR;
    /*------ write to the result table ------*/
//      Bdd::writeCache(Bdd::EXPAND_CUBE, rep, offset, h.rep);

    return h;
}


//  // factor0(level) * ginv + factor1(level) * g + factorR(level) ���v�Z
//  // �璷�ȕ\���������Ă��C���ɉ������Ȃ��D
//  // Sop::one �������Ă��鎞�����͍l���D
//  // ginv��Sop::null�̎��́Cg����getFunc����makeISOP�ŋ��߂�D
//  Sop Sop::compose(int level, const Sop& g, const Sop& ginv)
//  {
//      Sop result;
//      Sop f0 = factor0(level);
//      Sop f1 = factor1(level);
//      Sop fR = factorR(level);
//      if ( ginv == Sop::null ) {
//  	Sop local_ginv = Sop::zero;
//  	if (f0 != Sop::zero) {
//  	    Bdd gf = g.getFunc();
//  	    local_ginv = IsfLU(~gf).makeISOP();
//  	}
//  	result = sopProduct(f0, local_ginv) + sopProduct(f1, g) + fR;
//      }
//      else result = sopProduct(f0, ginv) + sopProduct(f1, g) + fR;

//      if ( Sop::intersec(result, Sop::one) == Sop::one ) result = Sop::one;
//      return result;
//  }

// factor0(level) * ginv + factor1(level) * g + factorR(level) ���v�Z
// �璷�ȕ\���������Ă��C���ɉ������Ȃ��D
// Sop::one �������Ă��鎞�����͍l���D
// posi��nega��Sop::null�̎��́C�����������g����getFunc����makeISOP�ŋ��߂�
Sop Sop::compose(int level, Sop posi, Sop nega)
{
    Sop result;
    Sop f0 = factor0(level);
    Sop f1 = factor1(level);
    Sop fR = factorR(level);

    if ( (f0 != Sop::zero) && (nega == Sop::null) ) {
	assert( posi != Sop::null );
	Bdd gf = posi.getFunc();
	nega = IsfLU(~gf).makeISOP();
    }
    if ( (f1 != Sop::zero) && (posi == Sop::null) ) {
	assert( nega != Sop::null );
	Bdd gf = nega.getFunc();
	posi = IsfLU(~gf).makeISOP();
    }

    result = sopProduct(f0, nega) + sopProduct(f1, posi) + fR;
    if ( Sop::intersec(result, Sop::one) == Sop::one ) result = Sop::one;
    return result;
}

Sop Sop::maxLit() const
{
    Sop max_lit = Sop::null;
    int max_i = 1;	// �Q��ȏ㌻��Ȃ��Ƃ����Ȃ�

    int n = rep.top();
    uint* literals = (uint*)calloc(n+1, sizeof(int));
    rep.litCount(literals);

    for ( int lev = n ; lev>=1; lev-- ) {
	int nlit = literals[ lev ];

	if ( max_i < nlit ) {
	    max_i = nlit;
	    int var = lev2var(lev);
	    int neglit = lev & 1;
	    max_lit = (neglit) ? Sop::lit0(var) : Sop::lit1(var);
	}
    }
    free(literals);
    return max_lit;
}

Sop Sop::getKernel() const
{
    Sop comCube = commonCube();
    Sop k = (*this) / comCube;

    Sop max_lit = k.maxLit();
    if ( max_lit != Sop::null ) {
	k = ( k / max_lit ).getKernel();
    }

    return k;
}
 
// wdmax�𒴂���value��double cube divisor�����߂�
// maxVar�ȉ��̕ϐ��̂ݐ����đΏۂƂ���D
int Sop::getSdivisor(int wdmax, int maxVar, Sop& result)
{
    int n = rep.top();	// n: ���e�����̎�ނ̐��C�ϐ��̐���2�{
    int m2 = maxVar<<1; // maxVar�����e������
    if ( n > m2 ) n = m2; // ����ȉ��݂̂𐔂���

    uint* literals = (uint*)calloc(n+1, sizeof(int));
    rep.litCount(literals, n);
    int k = 0;
    for ( int lev = n ; lev>=1; lev-- ) {
	int nlit = literals[ lev ];
	if ( k < nlit ) k = nlit;
    }

    int threshold = 0;
    int max_ilev, max_jlev;
    Arrayint L;	// ���܂��Ă���
    Arrayint T; // ����N���A
    for ( ; (k-2) > wdmax; k-- ) {
// 	fprintf(stderr, "k = %d\n", k);
	
	T.clear();
	for ( int lev = 1; lev<=n; lev++ ) {
	    int nlit = literals[ lev ];
	    if ( nlit == k ) {
		T.add(lev);
	    }
	}

	for ( int i = 0; i < T.size(); i++ ) {
	    int ilev = T[i];
	    Bdd i_fac = rep.subset1(ilev);
	    for ( int j = i+1; j < T.size(); j++ ) {
		int jlev = T[j];

// 		Bdd ij_fac = i_fac.subset1(jlev);
// 		int ncube = ij_fac.card();
		int ncube = i_fac.card(jlev);
	    
		if ( ncube > threshold ) {
		    max_ilev = ilev;
		    max_jlev = jlev;
		    threshold = ncube;
		}
	    }
	    for ( int j = 0; j < L.size(); j++ ) {
		int jlev = L[j];

// 		Bdd ij_fac = i_fac.subset1(jlev);
// 		int ncube = ij_fac.card();
		int ncube = i_fac.card(jlev);
		
		if ( ncube > threshold ) {
		    max_ilev = ilev;
		    max_jlev = jlev;
		    threshold = ncube;
		}
	    }
	}

	if ( threshold >= k ) {
	    result = (Bdd::base).change(max_ilev).change(max_jlev);
	    delete literals;
	    return threshold - 2;
	}

	for ( int i = 0; i < T.size(); i++ ) {
	    int ilev = T[i];
	    L.add(ilev);
	}
    }

    result = Sop::null;
    delete literals;
    return 0;
}

// cube�Ɋ܂܂�Ă��郊�e�����̒��ŁC*this�ɍł������������̂�I�ԁD
Sop Sop::most_common_literal(Sop cube)
{
    Sop most_common = Sop::null;
    int max_n_cube = 0;
    for ( int top; (top = cube.topvar()); ) {
	Sop d;
	int neg = cube.isTopNeg();
	if ( neg ) {
	    cube = cube.factor0(top);
	    d = Sop::lit0(top);
	}
	else {
	    cube = cube.factor1(top);
	    d = Sop::lit1(top);
	}
	Sop q = (*this) / d;
	int q_lit = q.nCube();
	if ( q_lit > max_n_cube ) {
	    most_common = d;
	    max_n_cube = q_lit;
	}
    }
    return most_common;
}

// ���ׂĂ�cube�Ɋ܂܂�Ă��郊�e�����̐ς�Ԃ��D�����ꍇ��Sop::one;
Sop Sop::commonCube() const
{
    Bdd litset = rep.commonLit(); // ���e�����̘a
    return Sop( litset.sum2product() );
}

int Sop::getComplementedLit(int lit)
{
    int neglit = lit & 1;
    int var = lev2var(lit);
    if ( neglit ) return var2lev1(var);
    else return var2lev0(var);
}

//////////////////////////////////////////////////////////////////

SopM SopM::operator = (const SopM& orig)
{
    if ( *this != orig ) {
	cset = orig.cset;
	numIn = orig.numIn;
    }
    return *this;
}

Sop SopM::elimOidx()
{
    /*------ �o�͂̃��e���������� ------*/
    Sop result = cset;
    int top = result.topvar();
    while ( top > numIn ) {
	Sop f0 = result.factor0(top);
	Sop f1 = result.factor1(top);
	Sop fR = result.factorR(top);
	result = f0 + f1 + fR;
	top = result.topvar();
    }
    return result;
}

Sop SopM::getOnset(int oidx)
{
    SopM result = SopM( cset.factor1(oidx+numIn), numIn );
    return result.elimOidx();
}

Sop SopM::getDcset(int oidx)
{
    SopM result = SopM( cset.factor0(oidx+numIn), numIn );
    return result.elimOidx();
}

void Sop::fprint(FILE* fp) const
{
    if ( (*this) == Sop::null ) {
	fprintf(fp, "NULL");
    }
    else {
	sop_print(fp, (*this), 0);
    }
}

void Sop::sparsePrint() const
{
    Sop sop = *this;

    int n = 0;
    int shifted = 0;
    Arrayint vararray;
    sop.varSet(vararray);
    for ( int i = vararray.size(); --i>=0; ) {
	int lev = vararray[i];
 	fprintf(stderr, "%d ", lev);
 	lev -= shifted;
 	n++;
 	int diff = lev-n;
 	sop = sop.varshift(lev, -diff);
 	shifted += diff;
    }
    fprintf(stderr, "\n");
    CubeSet::printSop(sop);
}
