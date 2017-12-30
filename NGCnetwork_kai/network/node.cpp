#include "node.h"


  
static void print_buffer(FILE *fp, const char * in, const char * out);
// todo: ---------node-ger.cpp

// static void checkFunc(const Bdd& f, const Bdd& fd); // todo ���g�p
static void printFunc(Node* fn);


int Node::nObj = 0;
int Node::nBorn = 0;

// �R���X�g���N�^�CNode��type�������w�肷��D
Node::Node(enum node_types tpe) 
    : typeRep(tpe), id(0), uniqueId(++nBorn), parent(0), polarity(1)
{
    nObj++;
}

// �R�s�[�R���X�g���N�^
// input,typeRep,func,level2index,cset,polarity ���R�s�[�����D
// Network�ɓo�^����Ȃ����߁C
// output,id,parent �̓R�s�[����Ȃ��D
Node::Node(const Node& op)
    : typeRep(op.typeRep), id(0), uniqueId(++nBorn), parent(0), input(op.input),
      func(op.func), cset(op.cset), fac(op.fac), polarity(op.polarity),
      lev2idx(op.lev2idx)
{
    nObj++;
    nameRep = op.nameRep;
}

// ��Node�Ƃ̐ڑ��CNetwork�ւ̓o�^���C�ɂ�������
Node::~Node() {
    typeRep = FREED;
    nObj--;
}

/*	---- Node���Ȃ����� ----
 �o�^���ꂽNode: reg
 �o�^����Ă��Ȃ�Node: unreg
 input -> output
 unreg -> reg:	input�����Ă��邾���Coutput�͒m��Ȃ�
 unreg -> unreg: input�����Ă��邾���Coutput�͒m��Ȃ�
 reg -> reg:	input, output���Ɍ��Ă���
 reg -> unreg:	�֎~				*/

// replaced��this�Œu��������D
void Node::replaceNode(Node* replaced, int alsoName)
{
    if ( this == replaced ) return; // �������g

    assert( parentNet() == replaced->parentNet() );

    //// �o�͑��Q�[�g�̓��͂�ύX
    //// ���͑��Q�[�g�̏o�͂�ύX
    for (auto&& outn : output) {
        replace(outn->input.begin(), outn->input.end(), replaced, this);
        addOutput(outn);
    }

    replaced->output.clear();
    replaced->output.shrink_to_fit();

    // 1999.5.28 �K�v�ɂ��CalsoName�ŕς����悤�ɂ����D
    if ( alsoName ) nameRep = replaced->nameRep; // ���O�̃R�s�[
}

// ���͂Ɋ܂܂��before��after�ɑウ��
void Node::changeInput(Node* before, Node* after)
{
    replace(input.begin(), input.end(), before, after);
    addLink(after); // add
    //    if ( before->outContain(this) == true) before->delOutput(this);
    if ( after->outContain(before) == true) after->delOutput(before);
}

bool Node::outContain(Node* fanout)
{
    for (const auto& no : output) if ( no == fanout ) return true;
    return false;
}

int Node::fanin2level(Node* fanin)
{
    // fanin -> index
    int index = fanin2index(fanin);

    // index -> level
    return index2level(index);	// �����Ƃ���0
}

Node* Node::level2fanin(int level)
{
    int index = lev2idx[level];
    if ( index < 0 ) return 0;
    else if ( index >= nIn() ) return 0;
    return index2fanin( index );		// �����Ƃ���0
}

int Node::fanin2index(Node* fanin)
{
    int index;
    for ( index = nIn(); --index >= 0; )
        if ( index2fanin(index) == fanin ) break;
    return index;    // �����Ƃ���-1
}
int Node::index2level(int index)
{
    // index -> level
    int level;
    for ( level = lev2idx.size(); --level > 0; ) {
        if ( lev2idx[level] == index ) break;
    }
    return level; // �����Ƃ���0
}
int Node::level2index(int level)
{
    assert( (unsigned int)level < lev2idx.size() );
    return lev2idx[level];
}
// level2index[level]�ԖڂɊi�[����Ă���ϐ����݂�B�z��͈̔͊O�̈�����
// �^������ -1 ��Ԃ��B

// ���͂Ɋ܂܂��fanin��ے肷��
void Node::invInput(Node* fanin)
{
    int level = fanin2level(fanin);

    if ( func != Bdd::null )
        func = func.compose( level, ~(Bdd::var(level)) );
    if ( cset != Sop::null ) {
        cset = cset.compose(level, Sop::lit0(level), Sop::lit1(level));
        fac.clear();
    }
}

// �����֐��𔽓]����
void Node::invFunc()
{
    if ( cset != Sop::null ) {
        // 	Bdd cset_f = makeFuncIfNot();
        // 	cset = IsfLU(~cset_f).makeISOP();
        // 	fac.clear();
        polarity = polarity ^ 1;
    }
    if ( func != Bdd::null ) {
        func = ~func;	// ���]
    }
}

Bdd Node::makeFuncIfNot() {
    if ( (typeRep != INPUT) && (func == Bdd::null) ) {
      //cout << "a" << endl;
        assert( cset != Sop::null ); // �ǂ��炩�͂���͂�
	//cout << "b" << endl;
        func = cset.getFunc();
	//cout << "c" << endl;
        if ( polarity == 0 ) {
            func = ~func;
        }
	//cout << "d" << endl;
    }
    return func;
}

// ���̏��Cfunc���璼�ڍ��̂ŁCpolarity��1
Sop Node::makeCsetIfNot() {
    if ( (typeRep != INPUT) && (cset == Sop::null) ) {
        assert( func != Bdd::null ); // �ǂ��炩�͂���͂�
        cset = IsfLU(func).makeISOP();
        polarity = 1;
        fac.clear();
    }
    return cset;
}

// cubeOrder()���g���ĕϐ�����ς���D�ϐ����̕ύX��Ԃ��Dcset�̑��݂��O��D
vector<int> Node::reorderLev2idx()
{
    int numIn = cset.topvar();
    vector<int> permu;
    cubeOrder(cset, permu, numIn); 

    // cset
    if ( cset != Sop::null ) {
        cset = cset.changeOrder(inverse(permu));
        fac.clear();
    }
    // func
    if ( func != Bdd::null ) {
        func = func.changeOrder(inverse(permu));
    }
    // lev2idx
    changeOrder<vector<int>>(lev2idx, inverse(permu));
    return permu;
}

void Node::delDupInput() // �������͂�����΍폜
{
    int change = 0;
    int n = lev2idx.size()-1;
    for ( int lev1=n; lev1>0; lev1-- ) {
        int varIndex1 = lev2idx[lev1];
        if ( varIndex1 < 1 ) continue;
        for ( int lev2 = lev1-1 ; lev2>0; lev2-- ) {
            int varIndex2 = lev2idx[lev2];
            if ( varIndex2 < 1 ) continue;
            if ( input[ varIndex1 ] == input[ varIndex2 ] ) {
                // 		fprintf(stderr, "same input\n");
                change = 1;
                if ( func != Bdd::null ) 
                    func = func.compose( lev2, Bdd::var(lev1) );
                if ( cset != Sop::null ) {
                    cset = cset.compose( lev2, Sop::lit1(lev1),
                                         Sop::lit0(lev1) );
                    fac.clear();
                }

                lev2idx[lev2] = -1;
            }
        }
    }

    if ( change ) {	// ���̒i�K�ł́Cinput���������Ă���
        int n = trimLev2idx();
        if ( n == 0 ) {	// func��constant�̏ꍇ
            Bdd func = makeFuncIfNot();
            makeConstant(func);
        }
        else {
            Bdd sup = (func!=Bdd::null) ? func.varSet() : cset.varSet();
            this->print(1);
            connectInputSub(sup, input, lev2idx.data());
            this->print(1);
        }
    }
}

/*------
 Node�̓����_����f�Ƃ��Cf�ɂ�����e�ϐ��̃��x�� lev �ɑ΂��āC
 ���͂� varlist[ level2index[ lev ] ] ���Ȃ��D
 varlist �� index ���� fanin ��Ԃ��D
 level2index �� varlist �ɂ����� index��Ԃ��D
 (level2index==0�̎���level��index)
 ------*/
//void Node::connectInput(Bdd f, Arrayw<Node*>& varlist, int* level2index=0)
void Node::connectInput(Bdd f, vector<Node*>& varlist, int* level2index)
{
    func = f;
    cset = Sop::null;
    fac.clear();

    // f��constant�̏ꍇ
    Bdd sup = f.varSet();
    if ( sup == Bdd::empty ) {
        makeConstant(f);
        return;
    }
    else typeRep = LUT;

    connectInputSub(sup, varlist, level2index);
}

void Node::makeConstant(const Bdd& f)
{
    assert( (f == Bdd::zero) || (f == Bdd::one) );
    typeRep = CONSTANT;
    delAllInput();
    func = f;
}

void Node::makeLUT(const Bdd& f, const vector<Node*>& level2node)
{
    func = f;
    cset = Sop::null;
    fac.clear();

    // f��constant�̏ꍇ
    Bdd sup = f.varSet();
    if ( sup == Bdd::empty ) {
        makeConstant(f);
        return;
    }
    else typeRep = LUT;

    connectInputSub(sup, level2node);
}

void Node::makeSOP(const Sop& cs, int pola, const vector<Node*>& level2node)
{
    cset = cs;
    polarity = pola;
    fac.clear();

    // cs��constant�̏ꍇ
    Bdd sup = cs.varSet();
    if ( sup == Bdd::empty ) {
        Bdd func = makeFuncIfNot();
        makeConstant(func);
    }
    else {
        typeRep = SOP;
        func = Bdd::null;
        connectInputSub(sup, level2node);
    }
}

void Node::makeLUT(const Bdd& f, Node* base_n)
{
    func = f;
    cset = Sop::null;
    fac.clear();

    // f��constant�̏ꍇ
    Bdd sup = f.varSet();
    if ( sup == Bdd::empty ) {
        makeConstant(f);
        return;
    }
    else typeRep = LUT;

    connectInputSub(sup, base_n->input, base_n->lev2idx.data());
}

// todo: node-ger.cpp

//�����̎d�l�������Ⴄ
void Node::makeLUT(const Bdd& f, const vector<Node*> varlist, int* level2index)
{
    func = f;
    cset = Sop::null;
    fac.clear();

    // f��constant�̏ꍇ
    Bdd sup = f.varSet();
    if ( sup == Bdd::empty ) {
        makeConstant(f);
        return;
    }
    else typeRep = LUT;

    connectInputSub(sup, varlist, level2index);
}

// todo: ---------------node-ger.cpp

void Node::makeSOP(const Sop& cs, int pola, Node* base_n)
{
    cset = cs;
    polarity = pola;
    fac.clear();

    // cs��constant�̏ꍇ
    Bdd sup = cs.varSet();
    if ( sup == Bdd::empty ) {
        Bdd func = makeFuncIfNot();
        makeConstant(func);
    }
    else {
        typeRep = SOP;
        func = Bdd::null;
        connectInputSub(sup, base_n->input, base_n->lev2idx.data());
    }
}

void Node::updateLUT(const Bdd& f, Node* new_n, int new_n_lev)
{
    func = f;
    cset = Sop::null;
    fac.clear();

    // f��constant�̏ꍇ
    Bdd sup = f.varSet();
    if ( sup == Bdd::empty ) {
        makeConstant(f);
        return;
    }
    else typeRep = LUT;

    vector<Node*> newInput(getInput());
    newInput.push_back(new_n);

    vector<int> newLev2idx( getLev2idx() );
    if ( (unsigned int)new_n_lev+1 > newLev2idx.size() ) {
        // newLev2idx.reserve( new_n_lev+1 ); // todo
        newLev2idx.resize(new_n_lev+1);
    }

    newLev2idx[new_n_lev] = newInput.size()-1;

    connectInputSub(sup, newInput, newLev2idx.data());
}

void Node::updateSOP(const Sop& cs, int pola, Node* new_n, int new_n_lev)
{
    cset = cs;
    polarity = pola;
    fac.clear();

    // cs��constant�̏ꍇ
    Bdd sup = cs.varSet();
    if ( sup == Bdd::empty ) {
        Bdd func = makeFuncIfNot();
        makeConstant(func);
        return;
    }

    typeRep = SOP;
    func = Bdd::null;

    vector<Node*> newInput(getInput());
    newInput.push_back(new_n);

    vector<int> newLev2idx(getLev2idx());
    if ( (unsigned int)new_n_lev+1 > newLev2idx.size() )
        // newLev2idx.reserve( new_n_lev+1 ); // todo
        newLev2idx.resize( new_n_lev+1 );
    newLev2idx[new_n_lev] = newInput.size()-1;

    connectInputSub(sup, newInput, newLev2idx.data());
}

void Node::connect(Node* fanin)
{
    assert( this != fanin );
    if ( getType() == OUTPUT ) {
        delAllInput();
    
        // input �̏���
        input.resize(1);
        input[0] = fanin;

        // �o�^����Ă�����fanin����̃����N�����炤
        addLink(fanin); // add

        // func, lev2idx
        // 	Network* p = fanin->parentNet();
        // 	int fanin_lev = ( p && p->funcGlobal() ) ? p->nodeLev(fanin) : 1;
        int fanin_lev = 1;

        func = Bdd::var( fanin_lev );

        // lev2idx.reserve(fanin_lev+1); // todo
        lev2idx.resize(fanin_lev+1);
        for ( int i = fanin_lev; --i>=0; ) lev2idx[i] = -1;
        lev2idx[fanin_lev] = 0;
    }
    else if ( getType() == AND || getType() == NOT || getType() == OR) {
        int newIdx = input.size();
        input.push_back(fanin);
        addLink(fanin); // add

        if ( func == Bdd::null ){
            if(getType() == AND){
                func = Bdd::one;
            }else if(getType() == OR){  
                func = Bdd::zero;
            }else if(getType() == NOT){
                func = Bdd::one;
            }
        }
        
            int newLev = func.top() + 1;
        if ( getType() == AND )
            func = Bdd::varIte( newLev, func, Bdd::zero );
        else if(getType() == OR )
            func = Bdd::varIte( newLev, Bdd::one, func );
        else if(getType() == NOT)
            func = Bdd::varIte( newLev, Bdd::zero, func );               
        else 
            func = Bdd::varIte( newLev, ~func, func );       
        // lev2idx.reserve(newLev+1); // todo
        lev2idx.resize(newLev+1);
        lev2idx[newLev] = newIdx;
    }
    else assert(0);
}

void Node::disconnect(Node* fanin)
{
    assert( this != fanin );
    if ( getType() == OUTPUT ) {
        delAllInput(); // 1���͂̂͂��Ȃ̂�

        // func, lev2idx
        func = Bdd::null;
        lev2idx.clear();
    }
    else if ( getType() == AND || getType() == XOR ) {
        fprintf(stderr, "sorry, disconnect() for AND, XOR is not ready\n");
        abort();
    }
    else assert(0);
}

void Node::compressFunc()
{
    // func��cset�̂ǂ��炩�͑��݂���Dvarset��ZBDD�`�������COR�Ƃ������D
    Bdd varset;
    if ( func != Bdd::null ) varset = func.varSet();
    else if ( cset != Sop::null ) varset = cset.varSet();
    else assert(0);

    // 1997.9.1 getPermuForCompress���g���悤�ɂ����D
    vector<int> permu = varset.getPermuForCompress();

    if ( cset != Sop::null ) {
        cset = cset.changeOrder(permu);
        fac.clear();
    }
    if ( func != Bdd::null ) {
        func = func.changeOrder(permu);
    }

    changeOrder<vector<int>>(lev2idx, permu);
    trimLev2idx();
}

// �o�͐���0�ł���Node���폜���C�ċA�I�ɕs�K�v��Node���폜����
void Node::recLetNouse()
{
    if ( nOut() ) return;
    //     assert( parentNet() );

    vector<Node*> inputReserve;
    int oldNin = nIn();

    for ( int i = nIn()-1; i>=0; i-- ) inputReserve.push_back(index2fanin(i));
    delAllInput();
    for ( int i = oldNin-1; i>=0; i-- ) inputReserve[i]->recLetNouse();

    assert( nIn() == 0 && nOut() == 0 );
}

// ���͐���1�ȉ��ł�����̂ɑ΂��C���̏o�͑���ύX����D
// �������C�o�͑���OUTPUT�̎��͍s��Ȃ��D
int Node::del_1input()
{
    int change = 0;
    //////// 1���͈ȉ� ////////
    if ( nIn() <= 1 ) {
        for (auto&& out : output) {
            // *this ���o�b�t�@�̎� or out��OUTPUT�łȂ��Ƃ� collapse����
            if ( ((nIn() == 1) && (getFunc().rstrtop1() == Bdd::one)) ||
                 (out->getType() != OUTPUT) ) {
                out->collapse(this);
                change = 1;
            }
        }
    }
    return change;
}

// �����ɐݒ�
static int uniqueIdCmp(const void* lhs, const void* rhs)
{
    Node* lp = *((Node**)(lhs));
    Node* rp = *((Node**)(rhs));
    int lid = lp->getUniqueId();
    int rid = rp->getUniqueId();
    return lid - rid;
}

// ni �� nj �̓��͏W�����ׂ�D
// ni �̂� nj �̂��܂ނƂ�(�����Ƃ���)�C1��Ԃ��D
// ����ȊO�̂Ƃ��́C0��Ԃ��D
// �܂��C1��Ԃ��Ƃ��Cni �� ���͂Ɋւ��� node2level ���Ԃ��D
int Node::compareInputSet(Node* ni, Node* nj, unordered_map<void*,int>& node2level)
{
    int ni_nin = ni->input.size();
    int nj_nin = nj->input.size();

    if ( ni_nin < nj_nin ) return 0;

    vector<Node*> ni_input = ni->input;
    vector<Node*> nj_input = nj->input;

    sort(ni_input.begin(), ni_input.end(), uniqueIdCmp);
    sort(nj_input.begin(), nj_input.end(), uniqueIdCmp);

    int i = ni_nin-1;
    int j = nj_nin-1;
    while ( (i >= 0) && (j >= 0) ) {
        // nj_input�ɂ�����̂͂��ׂ�ni_input�ɂȂ���΂Ȃ�Ȃ�
        if ( nj_input[j] == ni_input[i] ) {
            i--;
            j--;
        }
        else if ( i > 0 ) {
            i--;
        }
        else {
            return 0;
        }
    }
    if ( j >= 0 ) return 0; // ��v���Ȃ��������̂��������D

    // ni �� ���͂Ɋւ��� node2level �����D
    // node2level.clear();
    unordered_map<void*,int>().swap(node2level);
    for ( int lev = ni->lev2idx.size(); --lev >= 0 ; ) {
        int idx = ni->lev2idx[lev];
        if ( idx >= 0 ) {
            Node* no = ni->input[idx];
            node2level[no] = lev;
        }
    }
    return 1;
}

// ni��nj�̓��͐���������1�ȏ�ŁC
// �����֐��������Ƃ�1��Ԃ��C�ے�̊֌W�̂Ƃ�-1��Ԃ��D
int Node::isSameFunc(Node* ni, Node* nj)
{
    Bdd func_i = ni->getFunc();
    Bdd func_j;

    //     Network* pi = ni->parentNet();
    //     Network* pj = nj->parentNet();
    //     if ( pi && pj && (pi == pi) && pi->funcGlobal() ) {
    // 	func_j = nj->getFunc();
    //     }
    //     else {

    int ni_nin = ni->nIn();
    int nj_nin = nj->nIn();

    if ( ni_nin != nj_nin ) return 0;
    else if ( ni_nin == 0 ) return 0;
    else { // sup_i.size() == sup_j.size() >= 1
        unordered_map<void*,int> node2level;
        int same = compareInputSet(ni, nj, node2level);
        if ( same == 0 ) return 0;

        func_j = nj->calcFunc(node2level);
    }

    //     }

    if ( func_i == func_j ) return 1;
    else if ( func_i == ~func_j ) return -1;
    else return 0;
}

int Node::nLitFac()
{
    makeCsetIfNot();

    if ( fac.isEmpty() ) {
        fac = Factor(cset);
        fac.makeCubeFree();
    }
    return fac.nLit();
}

int Node::nLitSop()
{
    makeCsetIfNot();
    return cset.nLit();
}

// node2level�̋t�ʑ������߂�D
static void reverse_map_node2level(unordered_map<void*,int>& node2level,
                                   vector<Node*>& level2node)
{
    for(const auto& it : node2level) {
        int level = it.second;
        Node* no = (Node*)it.first;
        level2node[level] = no;
    }
}

void Node::collapse(Node* fanin)
{
    const int debug = 0;

    if ( debug ) {
        this->print(1);
        printFunc(this);
        fanin->print(1);
        printFunc(fanin);
    }

    // node2level�����߂�D
    unordered_map<void*, int> node2level; // todo
    assignLevel(this, fanin, node2level);

    if ( debug ) {
        for(const auto& it : node2level) {
            int level = it.second;
            Node* no = (Node*)it.first;
            fprintf(stderr, "level = %d", level);
            no->print(0);
        }
    }

    Bdd varset;     // �V�����ϐ��W���D
    // func������ꍇ��func����C����ȊO��cset���狁�߂�D

    // func��cset�̂��閳���͕ۑ������D
    if ( func != Bdd::null ) {	// func������ꍇ
        func = calcFunc(node2level);
        // cset������ꍇ�͍X�V�Dfunc������D
        if ( cset != Sop::null ) {
            cset = IsfLU(func).makeISOP();
            fac.clear();
        }
        varset = func.varSet();
    }
    else { // func������cset�݂̂̏ꍇ
        // 	fprintf(stderr, "collapse(), only sop\n");
        assert( cset != Sop::null );
        cset = calcSop(node2level); // Sop�̏�Ōv�Z
        fac.clear();
        varset = cset.varSet();
    }

    // type�����߂�D
    if ( varset == Bdd::empty ) {
        Bdd func = makeFuncIfNot();
        makeConstant(func);
        return;
    }
    else if ( getType() == OUTPUT ) {
        assert( varset.size() == 1 );
    }
    else if ( cset != Sop::null ) { // cset�������SOP�Ƃ���D
        typeRep = SOP;
    }
    else {
        typeRep = LUT;
    }

    // node2level����level2node�����D
    vector<Node*> level2node;
    reverse_map_node2level(node2level, level2node);

    connectInputSub(varset, level2node);

    if ( debug ) {
        fprintf(stderr, "after collapse():\n");
        this->print(1);
        printFunc(this);
        fanin->print(1);
        printFunc(fanin);
    }
}

int Node::collapseTest(Node* fanin)
{
    int nLitFacSelf = nLitFac();
    int nLitFacFanin = fanin->nLitFac();

    // node2level�����߂�D
    // Cmap<void*,int>	node2level(0, ptrHash, 10); // todo
    unordered_map<void*,int> node2level; // todo
    assignLevel(this, fanin, node2level);

    Bdd newfunc;
    Sop newcset;

    if ( func != Bdd::null ) {	// func������ꍇ
        newfunc = calcFunc(node2level);
        newcset = IsfLU(newfunc).makeISOP();
    }
    else { // func������cset�݂̂̏ꍇ
        newcset = calcSop(node2level); // Sop�̏�Ōv�Z
    }

    Factor newfac = Factor(newcset);
    newfac.makeCubeFree();

    int nLitFacAfter = newfac.nLit();
    int gain = (nLitFacSelf + nLitFacFanin) - nLitFacAfter;

    //      fprintf(stderr, "collapseTest(), nLitFac [%d %d] -> [%d], gain(%d)\n",
    //  	    nLitFacSelf, nLitFacFanin, nLitFacAfter, gain);

    return gain;
}

// ������fanout��1�ŁCfanout��ł̎����ɑ΂��郊�e�������C
// factored form��1�ł����merge�D
// SIS �� eliminate -1 �ɑ�������D
int Node::eliminate()
{
    if ( nOut() != 1 ) return 0;
    Node* fanout = output[0];
    if ( fanout->getType() == OUTPUT ) return 0;
    //      if ( fanout->nIn() > 16 ) return 0; // �傫������ꍇ�͂��Ȃ�

    fanout->makeCsetIfNot(); // fanout��cset�����
    if ( fanout->fac.isEmpty() ) { // fanout��fac�����
        fanout->fac = Factor(fanout->cset);
        fanout->fac.makeCubeFree();
    }

    int var_to_me = fanout->fanin2level(this);
    int n_lit = fanout->fac.n_appear(var_to_me);

    if ( n_lit <= 1 ) {
        assert( n_lit == 1 );
        if ( fanout->collapseTest(this) >= 0 ) { // ���e������������0�ł�OK
            fanout->collapse(this);

            Network* parent = parentNet();
            assert( parent ); // eliminate()���l����Ƃ������Ƃ�,parent������
            parent->delNode(this); // �����Ă����ɏ�����
            return 1;
        }
    }
    return 0;
}

// ������fanout��1�ŁCfanout��ł̎����ɑ΂��郊�e������unate�ł����merge
int Node::mergedIfUsedUnate()
{
    if ( nOut() >= 2 ) return 0;
    if ( cset == Sop::null ) return 0;	// cset�������Ă��邱��

    Node* fanout = output[0];
    if ( fanout->cset == Sop::null ) return 0;	// cset�������Ă��邱��

    Sop fo_cs = fanout->cset;
    int var_to_me = fanout->fanin2level(this);

    int nlit = fo_cs.factor0(var_to_me).nCube(); // �����e�����������cube��
    int plit = fo_cs.factor1(var_to_me).nCube(); // �����e�����������cube��

    if ( (nlit == 0) || (plit == 0) ) { // �ǂ��炩��0

        // 	int this_cube_free = (cset.maxLit() == Sop::null);
        // 	int fanout_cube_free = (fanout->cset.maxLit() == Sop::null);
        // 	if ( this_cube_free || fanout_cube_free ) {
        // 	if ( this_cube_free ) {
        fanout->collapse(this);
        return 1;
        // 	}
    }
    return 0;
}

Node* Node::mergeSameType()	// merge���ꂽNode��Ԃ��D�Ȃ����0��Ԃ��D
{
    Bdd constant;
    if ( getType() == AND ) {
        for (auto&& no : input) {
            if ( no->nOut() > 1 ) continue;
            if ( no->getType() == AND ) {
                assert( no->nIn() > 1 );
                constant = no->func.rstrtop0();
                if ( constant.top() > 0 ) constant = no->func.rstrtop1();
                assert( constant.top() == 0 );

                int lev = fanin2level(no);
                constant = func.compose(lev, constant);
                if ( constant.top() > 0 ) continue;

                collapse(no);
                setType(AND);
                return no;
            }
        }
    }
    if ( getType() == XOR ) {
        for (auto&& no : input) {
            if ( no->nOut() > 1 ) continue;
            if ( no->getType() == XOR ) {
                collapse(no);
                setType(XOR);
                return no;
            }
        }
    }

    return 0;
}

//////////////////////////////////////////////////////////////////

// node2level�ɏ]���Ċ֐����v�Z�D
Bdd Node::calcFunc(const unordered_map<void*,int>& node2level)
{
    // node2level����node2outf�����߂�
    // Cmap<void*,Bdd> node2outf(Bdd::null, ptrHash, 10); //todo
    unordered_map<void*,Bdd> node2outf;
    for(const auto& it : node2level) {
        Node* no = (Node*)it.first;
        int lev = it.second;
        node2outf[no] = Bdd::var(lev);
    }

    return rec_calcFunc(node2outf);
}

// node2outf�ɏ]���Ċ֐����v�Z�D
// node2outf�ɂ́C���ʂ����܂��Ă����D
Bdd Node::rec_calcFunc(unordered_map<void*,Bdd>& node2outf)
{
    if ( getType() == CONSTANT ) return makeFuncIfNot();

    /* terminal case */
    // Bdd f = node2outf.get(this); // todo
    Bdd f = node2outf[this];
    if ( f != Bdd::null ) return f;

    /* recursive */
    // unordered_map<void*,Bdd> fanin2func(Bdd::null, ptrHash, 10); //todo
    unordered_map<void*,Bdd> fanin2func;
    for (const auto& no1 : input) {
        fanin2func[no1] = no1->rec_calcFunc(node2outf);
    }
    return node2outf[this] = calcFunc_sub(fanin2func);
}

Sop Node::calcSop_first(const unordered_map<void*,int>& node2level)
{
    if ( getType() == CONSTANT ) return makeCsetIfNot();

    /* recursive */
    // Cmap<void*,Sop> node2sop(Sop::null, ptrHash, 10);
    unordered_map<void*,Sop> node2sop;

    for (const auto& no1 : input) {
        node2sop[no1] = no1->calcSop(node2level);
    }

    return calcSop_sub(node2sop);
}

Sop Node::calcSop(const unordered_map<void*,int>& node2level)
{
    if ( getType() == CONSTANT ) return makeCsetIfNot();

    /* terminal case */
    // int level = node2level.get(this); // todo
    int level = node2level.find(this)->second; // ������ const �̂���find����
    if ( level > 0 ) {
        // polarity�ɒ��ӁD0�̎��́C�ϐ��𔽓]���Ă����Ȃ���΂Ȃ�Ȃ��D
        if ( polarity == 0 ) return Sop::lit0(level);
        else return Sop::lit1(level);
    }

    /* recursive */
    // Cmap<void*,Sop> node2sop(Sop::null, ptrHash, 10); // todo
    unordered_map<void*,Sop> node2sop;
    for (const auto& no1 : input) {
        node2sop[no1] = no1->calcSop(node2level);
    }

    return calcSop_sub(node2sop);
}

void Node::print(int rec_flag)
{
    int i;
    int id = getUniqueId();
    switch (getType()) {
    case INPUT:
        fprintf(stderr, "INPUT[%2d](%s) O:%d\n",
                id, name(), nOut());
        break;
    case OUTPUT:
        fprintf(stderr, "OUTPUT[%2d](%s) I:%d[", id, name(), nIn());
        break;
    case LUT:
        fprintf(stderr, "LUT[%3d](%s) I:%d[", id, name(), nIn());
        break;
    case SOP:
        fprintf(stderr, "SOP[%3d](%s) I:%d[", id, name(), nIn());
        break;
    case AND:
        fprintf(stderr, "AND[%3d](%s) I:%d[", id, name(), nIn());
        break;
    case XOR:
        fprintf(stderr, "XOR[%3d](%s) I:%d[", id, name(), nIn());
        break;
    case SYM:
        fprintf(stderr, "SYM[%3d](%s) I:%d[", id, name(), nIn());
        break;
    case CONSTANT:
        fprintf(stderr, "CONSTANT[%2d](%s)\n", id, name());
        break;
    case UNDEFINED:
        fprintf(stderr, "UNDEFINED[%2d](%s)\n", id, name());
        break;
    default:
        fprintf(stderr, "???[%3d](%s)\n", id, name());
        assert(0);
        break;
    }

    switch (getType()) {
    case OUTPUT:
    case LUT:
    case SOP:
    case AND:
    case XOR:
    case SYM:
        // 	for ( i=0; i<nIn(); i++ ) fprintf(stderr, "%d", ( getInv(i) ) ? 0 : 1);
        fprintf(stderr, "] O:%d\n", nOut());
        break;
    default:
        break;
    }

    if ( rec_flag == 0 ) return;

    int n = nIn();
    for ( i=0; i<n; i++ ) {
        fprintf(stderr, "\tI: ");
        index2fanin(i)->print(0);
    }

    n = nOut();
    for ( i=0; i<n; i++ ) {
        fprintf(stderr, "\tO: ");
        output[i]->print(0);
    }
}

void Node::printName(FILE* fp)
{
    fprintf(fp, " %s", name());
}

// node�̓����_����blif �`���ŕ\������B
// lev2index �� ���� �ϐ��ԍ��� fan-in �Ƃ̊֌W������ׁA
// fanin ��1�Ԗڂ�blif�o�͂�1�ԂɑΉ�����悤�ɂ��Ă�B
// ��lutfunc �� compress����Ă��Ȃ��Ă��悢�D
// ��level2index �́C�����Ɛݒ肳��Ă��邱��

void Node::printIntFunc(FILE * sout, Node * node)
{
    //  Bdd lf = node->getFunc();
    Bdd lf = node->makeFuncIfNot();

    if(lf == Bdd::null){ 
        //m    cerr << "printIntFunc Warning!! Bdd::null cannot display \n";
        return;
    }
    if(lf == Bdd::zero){ //�����\�����Ȃ�
        return;
    }
    else if(lf == Bdd::one){ //1 �����\��
        fprintf(stderr, "1\n");
        return;
    }
    //  ISF isf(lf, lf);
    //  Sop isop =  isf.makeISOPnew(); 
    //  ISF NOTisf(~lf, ~lf);
    //  Sop NOTisop = NOTisf.makeISOPnew(); 
    //  12/25 �V���� IsfLU  �̕��Ɋ������D
    IsfLU isf = IsfLU(lf, lf);
    Sop isop =  isf.makeISOP();
    IsfLU notisf = IsfLU(~lf, ~lf);
    Sop NOTisop =  notisf.makeISOP();

    int* var2idx = node->getLev2idx().data();

    if( NOTisop.nCube() < isop.nCube()){
        CubeSet::printSop(NOTisop, sout, (char*)" 0", var2idx);
    }
    else{
        CubeSet::printSop(isop, sout, (char*)" 1", var2idx);
    }
}

void Node::check(int numRecursive)
{
    printf("check(): node->id = %d\n", id);
    switch ( getType() ) {
    case INPUT:
        assert( input.size() == 0 );
        break;
    case OUTPUT:
        assert( output.size() == 0 );
        assert( input.size() == 1 );
        break;
    case LUT:
    case SOP:
    case AND:
    case XOR:
    case SYM:
    case CONSTANT:
    case UNDEFINED:
        break;
    default:
        assert(!(int)"invalid type");
        break;
    }
    if ( numRecursive > 0 ) {
        for (auto&& fin : input) fin->check( numRecursive-1 );
        for (auto&& fout : output) fout->check( numRecursive-1 );
    }
}

int Node::trimLev2idx() // �K�v�Ȃ����̒����ɂ��āC�ő��level��Ԃ�
{
    int n = lev2idx.size();
    // n��0�̎��͉������Ȃ�
    if ( n > 0 ) {
        while ( (n > 0) && lev2idx[--n] < 0 ); // n��0�܂ł�������Ȃ�
        // lev2idx.reserve(n+1); // todo
        lev2idx.resize(n+1);
    }
    return n;
}

void Node::printBlif(FILE *fp)
{
    fprintf(fp, ".names");
    int n = nIn();
    for ( int i=0; i<n; i++ )
        index2fanin(i)->printName(fp);
    printName(fp);
    fprintf(fp, "\n");
    switch (getType()) {
    case LUT:
    case AND:
    case XOR:
    case SYM:
        if ( cset == Sop::null ) {
            compressFunc();
            printIntFunc(fp, this);
        }
        else {
            trimLev2idx();
            assert( lev2idx.size() == (unsigned int)cset.topvar()+1 );
            if ( polarity == 0 )
                CubeSet::printSop(cset, fp, (char*)" 0", lev2idx.data());
            else
                CubeSet::printSop(cset, fp, (char*)" 1", lev2idx.data());
        }
        break;
    case SOP:
        trimLev2idx();
        assert( lev2idx.size() == (unsigned int)cset.topvar()+1 );
        if ( polarity == 0 )
            CubeSet::printSop(cset, fp, (char*)" 0", lev2idx.data());
        else
            CubeSet::printSop(cset, fp, (char*)" 1", lev2idx.data());
        break;
    case CONSTANT: 
        if ( func == Bdd::zero ) break;
        else fprintf(fp, "1\n");
        break;
    default: // INPUT, OUTPUT������
        fprintf(fp, "error\n");
        assert(0);
    }
}

// todo: node-ger.cpp

// in -> out �ɂȂ����Ă���, out�̘_�����Ȃ�(�܂�buffer)�ł������
// blif�`���ŏo��
static void print_buffer(FILE *fp, const char * in, const char * out)
{
    fprintf(fp, ".names");
    fprintf(fp, " %s", in);
    fprintf(fp, " %s", out);
    fprintf(fp, "\n");
    fprintf(fp, "1 1\n");
}

void Node::printBlif_before_output(FILE *fp)
{
    assert(checkBeforeOutput() == true);
    assert(getType() != OUTPUT);
    //���o�͂����ړ��͒[�q�ɂȂ����Ă���ꍇ (�����͓��͒[�q) �[ �p�^�[��1
    if(getType() == INPUT){
        for (const auto& outn : output) {
            if(outn->getType() == OUTPUT){
                print_buffer(fp, this->name(), outn->name());
            }
        }
        return;
    }

    //�ȉ��̓p�^�[��2�̏���
    for (const auto& outn : output) {
        if(outn->getType() == OUTPUT){
            if (this->getName() != outn->getName() ) { //���O������
                print_buffer(fp, this->name(), outn->name());
                this->printBlif(fp);
            }
            else{  //���O�������Ȃ̂ŁCbuffer �͏Ȃ�
                this->printBlif(fp);
            }
        }
    }
}


///////////////////// 1998 08 20 add
//��  �t�@�C���ɏo�͂��Ă� �����ύX���Ȃ��o�[�W����
void Node::printBlif_before_output_no_change(FILE *fp)
{
    assert(checkBeforeOutput() == true);
    assert(getType() != OUTPUT);
    //���o�͂����ړ��͒[�q�ɂȂ����Ă���ꍇ (�����͓��͒[�q) �[ �p�^�[��1
    if(getType() == INPUT){
        for (const auto& outn : output) {
            if(outn->getType() == OUTPUT){
                print_buffer(fp, this->name(), outn->name()); //����͉����ύX���Ȃ��D
            }
        }
        return;
    }
    //�ȉ��̓p�^�[��2�̏���
    for (const auto& outn : output) {
        if(outn->getType() == OUTPUT){
            if (this->getName() != outn->getName() ) { //���O������
                print_buffer(fp, this->name(), outn->name());
                this->printBlif_no_change(fp);
            }
            else{  //���O�������Ȃ̂ŁCbuffer �͏Ȃ�
                this->printBlif_no_change(fp);
            }
        }
    }
}


//��  �t�@�C���ɏo�͂��Ă� �����ύX���Ȃ��o�[�W����
//   name ���Ȃ��Ƃ�����D
void Node::printBlif_no_change(FILE *fp)
{
    assert( getName().length() > 0 );  //name����`����ĂȂ��̂͂��������Ƃ���D
    //  assert( (*(parent->name2node))[getName()] == this ); // ���ꂪ�����Ȃ�OK

    Bdd         	func_back = func;	// �����_���֐�
    Sop		cset_back = cset;		// cube set
    vector<Node*> level2node_back;

    //CONSTANT �̏ꍇ ����� �܂���  by sawada����̎w�E 09/04
    if(this->getType() != CONSTANT){
        int OldSize = lev2idx.size();
        level2node_back.resize(OldSize);
        level2node_back[0]=0;
        for(int i=1; i<OldSize; i++){
            level2node_back[i]=level2fanin(i);
        }
        printBlif(fp);

        //���̏��ɂ��邽�߂ɂȂ������D
        func = func_back;
        cset = cset_back;
        Bdd sup;
        if (getType() == SOP) {
            sup = cset.varSet();
        }
        else{
            sup = func.varSet();
        }
        //���ꂢ�邩��?
        if ( sup == Bdd::empty ) {
            makeConstant(func);
            return;
        }
        connectInputSub(sup, level2node_back);
    }
    else{  //CONSTANT �̏ꍇ
        printBlif(fp);
    }
}

// ���͑���Node�̃��X�g
void Node::getFaninCorn(vector<Node*>& result)
{
    for (auto&& no : input) no->getFaninCorn(result);
    result.push_back(this);
}

// this == isNode �Ȃ���܂Ń`�F�b�N����false��Ԃ��B
bool Node::isSuccessor(Node* isNode){
    if (nOut() == 0) return false; //is �� �Ȃ��Ȃ瓖�Rfalse
    else if( isImmediateSuccessor(isNode) == true) return true; // IS �̎�

    bool flag = false;
    for (const auto& no : output) {
        flag = no->isSuccessor(isNode);
        if (flag) break;
    }
    return flag;
}

bool Node::isPredecessor(Node* isNode){
    int nin = nIn();
    if(nin == 0) {         //ip �� �Ȃ��Ȃ瓖�Rfalse
        return false;
    }
    else if ( isImmediatePredecessor(isNode) == true) {     // IP �̎�
        return true;
    }
    bool flag = false;
    for (int i=0;  (i < nin) && (flag==false);i++) {
        flag = index2fanin(i)->isPredecessor(isNode);
    }
    return flag;
}

//mode = 1��lev2idx�֌W�\�����Ȃ��B
// mode2 = 0,1,2 UniquID/ID/name �ŁC fanin fanout�m�[�h�̕\�����@������D
//void  Node::printInfo(FILE *fp, int mode=0) //node�̏����o��(For Debug)
//void  Node::printInfo(FILE *fp, int mode=0, int mode2=0) //node�̏����o��(For Debug)
void  Node::printInfo(FILE *fp, int mode, int mode2) //node�̏����o��(For Debug)
{
    if(this == 0){
        fprintf(fp,"\n****************************************\n");
        fprintf(fp,"*  THIS Node is Null Pointer - Something Wrong ??????\n");
        fprintf(fp,"****************************************\n");
        return;
    }
    fprintf(fp,"\n****************************************\n");
    fprintf(fp,"**  Printing Info. of Node No. = %d **\n", id);
    fprintf(stderr,"* This Node Unique ID = %d \n", this->getUniqueId());
    fprintf(fp, "*Node Name = %s \n", nameRep.c_str() );
    //   fprintf(fp, "*Node Name = %s \n", (const char*) nameRep );
    fprintf(fp, "**NODE TYPE  =  ");
    switch(typeRep){
    case INPUT:
        fprintf(fp, "INPUT \n");
        break;
    case OUTPUT:
        fprintf(fp, "OUTPUT \n");
        break;
    case AND:
        fprintf(fp, "AND \n");
        break;
    case CONSTANT:
        fprintf(fp, "CONSTANT \n");
        break;
    case XOR:
        fprintf(fp, "XOR \n");
        break;
    case LUT:
        fprintf(fp, "LUT \n");
        break;
    case SOP:
        fprintf(stderr, "SOP \n");
        break;
    case SYM:
        fprintf(stderr, "SYM \n");
        break;
    case FREED:
        fprintf(fp, "FREED \n");
        break;
    case UNDEFINED:
        fprintf(fp, "UNDEFINED \n");
        break;
    default: 
        fprintf(fp, "UNKNOWN - NOT SPECIFIED - something wrong ***** ?? \n");
        break;   
    }
    fprintf(fp, "** ilev  = %d \n", this->parentNet()->calcIdepth(this) );

    fprintf(fp, "** olev  = %d \n", this->parentNet()->calcOdepth(this) );
    fprintf(fp, "** nIn  = %d \n", nIn());

    switch(mode2){
    case 0:
        fprintf(fp, " --- Fanin's UniqueId's are (from index 0)  = ");
        for(int i = 0; i< nIn(); i++){
            fprintf(fp, " %2d", index2fanin(i)->getUniqueId() );
        }
        break;
    case 1:
        fprintf(fp, " --- Fanin's Id's are (from index 0)  = ");
        for(int i = 0; i< nIn(); i++){
            fprintf(fp, " %2d", index2fanin(i)->id); 
        }
        break;
    case 2:
        fprintf(fp, " --- Fanin's NAME's are (from index 0)  = ");
        for(int i = 0; i< nIn(); i++){
            fprintf(fp, " %s", index2fanin(i)->name() );
        }
        break;
    }
    fprintf(fp, "\n");
    if(mode == 0){  //���͂̃��x���֌W
        fprintf(fp, " --- Fanin's level are (from index 0) = ");
        for(int i = 0; i< nIn(); i++){
            fprintf(fp, " %2d", index2level(i));
        }
        fprintf(fp, "\n");
        fprintf(fp, " ---** lev2idx[] = \n");
        for(unsigned int i = 0; i< lev2idx.size(); i++){
            fprintf(fp, " lev2idx[%d] = %2d\n", i, lev2idx[i]);
        }
        fprintf(fp, "\n");
    }

    fprintf(fp, "** nOut()  = %d \n", nOut());

    switch(mode2){
    case 0:
        fprintf(fp, " --- Fanout's UniqueId's are (from index 0) = ");
        for (const auto& temp : output) fprintf(fp, " %d ", temp->getUniqueId());
        break;
    case 1:
        fprintf(fp, " --- Fanout's id's are (from index 0) = ");
        for (const auto& temp : output) fprintf(fp, " %d ", temp->id);
        break;
    case 2:
        fprintf(fp, " --- Fanout's Names are (from index 0) = ");
        for (const auto& temp : output) fprintf(fp, " %s ", temp->name());
        break;
    }


    fprintf(fp, "\n");
    fprintf(fp,"****************************************\n");
}

void Node::compressFuncAndIntDC()
{
    // func��cset�̂ǂ��炩�͑��݂���Dvarset��ZBDD�`�������COR�Ƃ������D
    Bdd varset;
    if ( func != Bdd::null ) varset = func.varSet();
    else if ( cset != Sop::null ) varset = cset.varSet();
    else assert(0);

    // 1997.9.1 getPermuForCompress���g���悤�ɂ����D
    vector<int> permu = varset.getPermuForCompress();

    if ( cset != Sop::null ) {
        cset = cset.changeOrder(permu);
        fac.clear();
    }
    if ( func != Bdd::null ) {
        func = func.changeOrder(permu);
    }

    /////////// ����ȍ~�� IntDC�̂��߂̒ǉ�
    Bdd  tempDC= getIntDC();
    tempDC = tempDC.changeOrder(permu);
    SetIntDC(tempDC);
    ///////////////////

    changeOrder<vector<int>>(lev2idx, permu);
    trimLev2idx();
}

// todo: ---------node-ger.cpp


//////////////////////////////////////////////////////////////////

/* private: */

// ���͂�S�č폜
void Node::delAllInput()
{
    /* ���͐悩��̎Q�Ƃ��폜 */
    /* �o�^����Ă�����C���͐���o�^����Ă��ĎQ�Ƃ������Ă��� */
    if ( parentNet() ) {
        for (auto&& in : input) in->delOutput(this);
    }
    /* �o�^����Ă��Ȃ�������C���͐�͎Q�Ƃ������Ă��Ȃ� */

    /* ���͐�ւ̎Q�Ƃ��폜 */
    clearInput();
}

// �������o�^����Ă��Ȃ������牽�����Ȃ��D
// �o�^����Ă�����, fanin���o�^����Ă���͂��ŁC��������link�����炤�D
void Node::addLink(Node* fanin)
{
    assert(fanin);
    if ( parentNet() ) {
        if ( fanin->parentNet() ) {
            fanin->addOutput(this);
        }
        else {
            assert( !(int)"registered Node has input to unregistered Node" );
        }
    }
}

// �������o�^����Ă����牽�����Ȃ��D
// �o�^����ĂȂ�������, fanin�����link���͂����D
void Node::delLink(Node* fanin)
{
    assert(fanin);
    if ( !parentNet() ) {
        fanin->delOutput(this);
    }
}

// level -> Node �����w��Didx�̏�����level�̏���
void Node::connectInputSub(const Bdd& varset, const vector<Node*>& level2node)
{
    Bdd sup = varset;
    int nin = varset.size();
    int lev = sup.top();

    vector<Node*> node_list(lev+1);
    vector<int> level_list(lev+1);

    // node_list,level_list�ɂ́C����level���珇�ɕ��ԁDcompress���ꂽ�����D
    int idx = nin;
    while ( lev != 0 ) {

        idx--;
        level_list[ idx ] = lev;
        node_list[ idx ] = level2node[lev];

        sup = sup.subset0(lev);
        lev = sup.top();
    }

    connectInputCore(node_list, level_list);
}

// level -> idx -> Node ���w��D
// varlist�͂����ŃR�s�[�����
void Node::connectInputSub(const Bdd& varset, vector<Node*> varlist,
                            int* level2index)
{
    int varlistSize = varlist.size();
    //  tmpidx2lev �̗p��
    vector<int> tmpidx2lev( varlistSize );
    for ( int i = varlistSize-1; i>=0; i-- ) {
        tmpidx2lev[i] = 0;
    }

    // input �Ɏg�p���� Node* �� level �� tmpidx2lev �ɏ�������
    Bdd sup = varset;
    int lev = sup.top();
    while ( lev != 0 ) {
        int varIndex = level2index ? level2index[lev] : lev;
        tmpidx2lev[ varIndex ] = lev;
        sup = sup.subset0(lev);
        lev = sup.top();
    }

    connectInputCore(varlist, tmpidx2lev);
}

// node_list, level_list�ɏ]���āCinput, lev2idx���X�V
//   node_list:  idx -> Node*
//   level_list: idx -> level
void Node::connectInputCore(vector<Node*> node_list, vector<int> level_list)
{
    delAllInput();

    // node_list, level_list�̑傫��
    int list_size = level_list.size();

    // maxlevel�����߂�D
    int maxlevel = 0;
    for ( int i = list_size; --i>=0; ) {
        if ( maxlevel < level_list[i] ) maxlevel = level_list[i];
    }

    // lev2idx �̏�����
    lev2idx.resize( maxlevel+1 );
    for ( int i = 0; i<=maxlevel; i++ ) lev2idx[i] = -1;

    // input, lev2idx������D
    for ( int i=0; i<list_size; i++ ) {
        if ( level_list[i] != 0 ) {
            input.push_back( node_list[i] );
            lev2idx[ level_list[i] ] = input.size()-1;
        }
    }

    // output ���Ȃ�
    for (auto&& fin : input) addLink(fin);
}

// fanin2outf�ɏ]���Ċ֐����v�Z�D���Ȃ킿�CNode�̓��͂���o�͂̊֐����v�Z�D
Bdd Node::calcFunc_sub(unordered_map<void*,Bdd>& fanin2outf)
{
    Bdd result;

    if ( (getType() == LUT) || (getType() == SOP) || (getType() == SYM) ) {
        func = makeFuncIfNot(); // ������������
        /*------
         result���v�Z���邽�߂ɁCfunc �� fanin2outf[no] ��level�����炷�D
         �Efunc�����炷�ƁC�ȉ��̂悤�ɔߎS�Ȃ��ƂɂȂ�ꍇ������D
         top�̕ϐ����珇�ɑ�����Ă����̂ɁC������ꂽ��C
         ���̕��̃��x���ɂȂ��Ă��܂����Ƃ�����D
         �Efanin2outf[no] �����炷�̂�robust�����Clevshift�̉񐔂������Ȃ�D
         ------*/
        if ( nIn() <= 5 ) { // ���ƂȂ� 5���͈ȉ��� func�����炷���Ƃɂ����D
            //// topvarFanin �����߂�
            int topvarFanin = 0;
            for (const auto& no : input) {
                Bdd f = fanin2outf[no];
                assert( f != Bdd::null );
                if ( topvarFanin < f.top() ) topvarFanin = f.top();
            }
            //// �e���̂Ȃ����x���܂ŃV�t�g�Dtopvar�����炷
            result = func.levshift(1, topvarFanin);
            //// top�̕ϐ����珇�ɑ�����Ă����D
            Bdd sup = func.varSet();
            while ( sup != Bdd::empty ) {
                int lev = sup.top();
                Bdd f = fanin2outf[ level2fanin(lev) ];
                assert( f != Bdd::null );
                result = result.compose(topvarFanin + lev, f);
                assert(sup != Bdd::empty);
                sup = sup.subset0(lev);
            }
            assert(sup == Bdd::empty);
        }
        else {
            //// topvarSelf �����߂�
            int topvarSelf = func.top();

            result = func;     // result �Ɍ��ʁD
            //// top�̕ϐ����珇�ɑ�����Ă����D
            Bdd sup = func.varSet();
            while ( sup != Bdd::empty ) {
                int lev = sup.top();
                Bdd tmpf = fanin2outf[ level2fanin(lev) ];
                assert( tmpf != Bdd::null );
                // fanin2outf ��topvar�������炵�đ��
                result = result.compose(lev, tmpf.levshift(1, topvarSelf));
                assert(sup != Bdd::empty);
                sup = sup.subset0(lev);
            }
            assert(sup == Bdd::empty);
            result = result.levshift(1, -topvarSelf); // ���ɖ߂��D
        }
    }
    else if ( getType() == AND ) {
        // �o�͂̔ے�����߂�
        Bdd easyConstant;
        Bdd funcRstr0 = func.rstrtop0();
        Bdd funcRstr1 = func.rstrtop1();
        if ( funcRstr0.top() == 0 ) easyConstant = funcRstr0;
        else if ( funcRstr1.top() == 0 ) easyConstant = funcRstr1;
        else assert(0);
        int invout = (easyConstant == Bdd::zero) ? 0 : 1;

        // ���͂̔ے�����߂�
        vector<int> inv(nIn()); // index -> inverted or not
        result = func; // temporary �� result ��p����
        int lev = result.top();
        while ( lev ) {
            funcRstr0 = result.rstrtop0();
            funcRstr1 = result.rstrtop1();
            if ( funcRstr0 == easyConstant ) {
                inv[ level2index(lev) ] = 0;
                result = result.rstrtop1();
            }
            else if ( funcRstr1 == easyConstant ) {
                inv[ level2index(lev) ] = 1;
                result = result.rstrtop0();
            }
            else assert(0);
            lev = result.top();
        }

        // result�̌v�Z
        result = Bdd::one; // result ��{���̖ړI�Ŏg��
        for ( int i=nIn(); --i>=0; ) {
            Bdd infunc = fanin2outf[ index2fanin(i) ];
            assert( infunc != Bdd::null );
            if ( inv[i] ) infunc = ~infunc;
            result = result & infunc;
        }
        if (invout) result = ~result;
    }
    else if ( getType() == XOR ) {
        result = func;
        int lev = func.top();
        while ( lev ) {
            result = result.rstr0(lev);
            lev = result.top();
        }    // func�̓��e�ɂ��������āC�K����result�̏����l�ƂȂ�
        assert( result == Bdd::zero || result == Bdd::one );
        for (const auto& no : input) {
            Bdd infunc = fanin2outf[no];
            assert( infunc != Bdd::null );
            result = result ^ infunc;
        }
    }
    else if ( getType() == OUTPUT ) {
        result = fanin2outf[input[0]];
        assert( result != Bdd::null );
        if ( func.rstrtop1() == Bdd::zero ) result = ~result;
    }
    else if ( getType() == CONSTANT ) {
        result = makeFuncIfNot();
    }
    else {
        fprintf(stderr, "calcFunc_sub(), unknown type");
        abort();
    }
    return result;
}

// fanin2sop�ɏ]���Ċ֐����v�Z�D���Ȃ킿�CNode�̓��͂���o�͂�Sop���v�Z�D
Sop Node::calcSop_sub(unordered_map<void*,Sop>& fanin2sop)
{
    if ( getType() == CONSTANT ) {
        return makeCsetIfNot();
    }

    makeCsetIfNot(); // assert( cset != Sop::null );

    Sop result;
    /*------
     result���v�Z���邽�߂ɁCcset �� fanin2sop[no] ��level�����炷�D
     �Ecset�����炷�ƁC�ȉ��̂悤�ɔߎS�Ȃ��ƂɂȂ�ꍇ������D
     top�̕ϐ����珇�ɑ�����Ă����̂ɁC������ꂽ��C
     ���̕��̃��x���ɂȂ��Ă��܂����Ƃ�����D
     �Efanin2sop[no] �����炷�̂�robust�����Clevshift�̉񐔂������Ȃ�D
     ------*/
    if ( nIn() <= 5 ) { // ���ƂȂ� 5���͈ȉ��� cset�����炷���Ƃɂ����D
        //// topvarFanin �����߂�
        int topvarFanin = 0;
        for (const auto& no : input) {
            Sop f = fanin2sop[no];
            assert( f != Sop::null );
            if ( topvarFanin < f.topvar() ) topvarFanin = f.topvar();
        }
        //// �e���̂Ȃ����x���܂ŃV�t�g�Dtopvar�����炷
        result = cset.varshift(1, topvarFanin);
        //// top�̕ϐ����珇�ɑ�����Ă����D
        Bdd sup = cset.varSet();
        while ( sup != Bdd::empty ) {
            int lev = sup.top();
            Node* fanin = level2fanin(lev);
            Sop fc = fanin2sop[ fanin ];
            assert( fc != Sop::null );

            if ( fanin->getPolarity() == 0 )
                result = result.compose(topvarFanin + lev, Sop::null, fc);
            else
                result = result.compose(topvarFanin + lev, fc, Sop::null);

            assert(sup != Bdd::empty);
            sup = sup.subset0(lev);
        }
        assert(sup == Bdd::empty);
    }
    else {
        //// topvarSelf �����߂�
        int topvarSelf = cset.topvar();

        result = cset;     // result �Ɍ��ʁD
        //// top�̕ϐ����珇�ɑ�����Ă����D
        Bdd sup = cset.varSet();
        while ( sup != Bdd::empty ) {
            int lev = sup.top();
            Node* fanin = level2fanin(lev);
            Sop fc = fanin2sop[ fanin ];
            assert( fc != Sop::null );
            // fanin2sop ��topvar�������炵�đ��
            fc = fc.varshift(1, topvarSelf);

            if ( fanin->getPolarity() == 0 )
                result = result.compose(lev, Sop::null, fc);
            else
                result = result.compose(lev, fc, Sop::null);

            assert(sup != Bdd::empty);
            sup = sup.subset0(lev);
        }
        assert(sup == Bdd::empty);
        result = result.varshift(1, -topvarSelf); // ���ɖ߂��D
    }

    // �璷�ȕϐ���Cprime�łȂ�cube���c���Ă��邩������Ȃ��D

    //     // makeISOP�ō�蒼��
    //     Bdd f = result.getFunc();
    //     result = IsfLU(f).makeISOP();

    return result;
}


// gn��fn�̓��͂Ƃ���D
// gn�̓��͂�fn�̓���(gn�ȊO)�ɑ΂��Cunique��level�����蓖�Ă�D
void Node::assignLevel(Node* fn, Node* gn, unordered_map<void*,int>& node2level)
{
    //     Network* p = fn->parentNet();
    //     if ( p && p->funcGlobal() ) {
    // 	for ( Node* no1 = fn->firstIn(); no1; no1 = fn->nextIn() ) {
    // 	    if ( no1 != gn ) {
    // 		if ( node2level.seek(no1) == 0 )
    // 		    node2level[no1] = p->nodeLev(no1);
    // 	    }
    // 	}
    // 	for ( Node* no1 = gn->firstIn(); no1; no1 = gn->nextIn() ) {
    // 	    if ( node2level.seek(no1) == 0 )
    // 		node2level[no1] = p->nodeLev(no1);
    // 	}
    //     }
    //     else { // funcGlobal()�łȂ��ꍇ
	// level:  1  2  .................................  n
	// Node:  f1 f2  .. f[gi-1] g1 g2 .. gm f[gi+1] .. fk
	// ����Node�������񌻂ꂽ�Ƃ��́C���level�ɂ���D

	int fntop = fn->trimLev2idx();
	int gntop = gn->trimLev2idx();
	int gnlev = fn->fanin2level(gn);

	int level = 0;
	for ( int i = 1; i < gnlev; i++ ) {
	    Node* no = fn->level2fanin(i);
	    if ( no ) node2level[no] = ++level;
	}
	for ( int i = 1; i <= gntop; i++ ) {
	    Node* no = gn->level2fanin(i);
	    if ( no ) node2level[no] = ++level;
	}
	for ( int i = gnlev+1; i <= fntop; i++ ) {
	    Node* no = fn->level2fanin(i);
	    if ( no ) node2level[no] = ++level;
	}
    //     }
}

// todo ���g�p
// void checkFunc(const Bdd& f, const Bdd& fd)
// {
//     Bdd f_onset = fd.rstrtop1();
//     Bdd f_offset = fd.rstrtop0();
//     if ( !( f_onset <= f ) ) {
//         fprintf(stderr, "!( f_onset <= f ),  f:\n");
//         CubeSet::printSop(f);
//         fprintf(stderr, "              f_onset:\n");
//         CubeSet::printSop(f_onset);
//         abort();
//     }
//     if ( !( f <= ~f_offset) ) {
//         fprintf(stderr, "!( f <= ~f_offset), f:\n");
//         CubeSet::printSop(f);
//         fprintf(stderr, "            ~f_offset:\n");
//         CubeSet::printSop(~f_offset);
//         abort();
//     }
// }

void printFunc(Node* fn)
{
    printArrayint(fn->getLev2idx());

    Sop fc = fn->getCset();
    Bdd f = fn->getFunc();

    if ( fc != Bdd::null ) {
        Factor fac = Factor(fc);
        fac.makeCubeFree();
        fprintf(stderr, "fc: #cubelit=%d, #faclit=%d\n",
                fc.nLit(), fac.nLit());
        CubeSet::printSop(fc);
        fprintf(stderr, "fc.getFunc():\n"); CubeSet::printSop(fc.getFunc());
    }
    if ( f != Bdd::null ) {
        fprintf(stderr, "f:\n"); CubeSet::printSop(f);
    }
    if ( (fc != Bdd::null) && (f != Bdd::null) ) {
        assert( f == fc.getFunc() );
    }
}

// todo: node-ger.cpp

//�����̃m�[�h�̏o�͂̂����P�ł��o�͒[�q�ɂȂ����Ă����true
bool Node::checkBeforeOutput()
{
    if(nOut() == 0) return false;
    bool flag = false;
    for (const auto outn : output) if(outn->getType() == OUTPUT) flag = true;
    return flag;
}
//�o�̓m�[�h��1�O�ɂ������Ă������ꂪ�Ă΂�� (����ȊO�G���[)
// �����p�^�[���́C
//   1  ���������͒[�q (�Q�ȏ�̏o�͒[�q�ɂȂ����Ă��Ă��悢)
//   2  �����͓����m�[�h�ŁC(�Q�ȏ�̏o�͒[�q�ɂȂ����Ă��Ă��悢)
//  ���ǂ���̏ꍇ���C���̃m�[�h����o�͒[�q�ȊO�̂��̂ɂȂ���Ă��Ă����D
//  ���������C���ׂĂ̏o�͂��o�͒[�q�ɂȂ����Ă��Ȃ����assert�ł�����

// todo: ----------- node-ger.cpp


//���ׂẴm�[�h��func���v�Z
unordered_map<Node*, Bdd> Node::makeFuncIfNot_All(vector<Node*>& list){
  unordered_map<Node*, Bdd> funclist;
  
  for(auto i : list){
    Bdd func = i->makeFuncIfNot();
    funclist.insert(make_pair(i, func));
  }
  return funclist;
}
