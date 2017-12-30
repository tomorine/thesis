#ifndef _ordbdd_h
#define _ordbdd_h

#include <memory.h>
#include "bddop.h"

class OrdBdd {
private:
    Bdd			f;
    int*		lev2idx; // lev2idx[0]: �z��̍ő�Y�� == �ŏ���f.top()
public:
		OrdBdd() : f(Bdd::null), lev2idx(0) {}
    		OrdBdd(Bdd g);
    		OrdBdd(Bdd g, int* l2i);
    		OrdBdd(const OrdBdd& op) 
    		: f(op.f), lev2idx( new int[op.lev2idx[0] + 1] ) {
		    memcpy(lev2idx, op.lev2idx, sizeof(int)*(op.lev2idx[0]+1));
		}
    		~OrdBdd() { delete [] lev2idx; }
    OrdBdd&	operator = (const OrdBdd& oprand);
    void	jumpup(int i, int j);
    void	jumpdown(int i, int j);
    void	compress();
    void	normalize();
    void	levshift(int lev, int degree);

    Bdd&	getFunc() { return f; }
    void	setFunc(Bdd func) { f = func; }
    int		maxLevel() { return lev2idx[0]; }
    int		nVar() { return lev2idx[0]; } // compress����Ă���Ƃ��̂ݗL��
    int		nSup() { return f.varSet().size(); }

    int*	getLev2idx() { return lev2idx; }
    int		level2index(int lev) { assert(lev>0); return lev2idx[lev]; }
    int		index2level(int idx);

    void	symReorder(short* pairVar, VarSymm* symRel); 
		// �Ώ̂ȕϐ���ׂ荇�킹�ɂ���
};

#endif
