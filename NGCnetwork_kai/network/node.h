#ifndef __NODE_H__
#define __NODE_H__

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iterator>

#include "factor.h" // bdd?f?B???N?g???????

#include "utility.h"
#include "network.h"
#include "general.h"

using namespace std;

class Network;

class Node {
public:
    /*------ name?Cid?Ctype, parent ------*/
    // todo: c_str()?????????????????????
    Node(node_types tpe); 
    inline const char* name() { return nameRep.c_str(); } // ???O
    inline std::string getName() { return nameRep; }
    inline void        clearName() { nameRep.resize(0); }
    inline int         getId() { return id; }             // Network?N???X????????????
    inline int         getUniqueId() { return uniqueId; } // ???????
    inline node_types  getType() { return typeRep; }
    inline void        setType(node_types tpe) { typeRep = tpe; }
    inline Network*    parentNet() { return parent; } // ???????? Network

    /*------ input, output ???? ------*/
    inline int nIn() { return input.size(); }  // ?????
    inline int nOut() { return output.size(); }; // ?o???

    inline vector<Node*>& getInput() { return input; }     // ?????z?????
    inline vector<Node*>& getOutput() { return output; }     // ?????z?????
    inline vector<int>&      getLev2idx() { return lev2idx; } // lev2idx????

    /*------ ???_????? ------*/
    inline Bdd  getFunc() const { return func; }    // ???_????????
    // todo: what is cset
    inline Sop  getCset() const { return cset; }    // cube set???
    inline int  getPolarity() const { return polarity; } // polarity???

    /*------ ?f?[?^?? print ------*/
    // todo: for debug
    void dprint() { print(1); }
    void print(int rec_flag);
    void printBlif(FILE *fp);
    void printName(FILE *fp);

    // node????_????blif ?`????\??????B
    // lev2index ?? ???? ???????? fan-in ????W????A
    // fanin ??1????blif?o???1????????????????B
    // ??func ?? ??????compress?????????
    // ??????Alevel2index ?????????????????
    void printIntFunc(FILE * sout, Node * node);

    //?o??m?[?h??1??O?????????????????????
    void printBlif_before_output(FILE *fp);

    //??  ?t?@?C????o?????? ?????X??????o?[?W????
    void printBlif_before_output_no_change(FILE *fp);
    void printBlif_no_change(FILE *fp);

    friend class Network;
    inline Node* index2fanin(int index) { return input[index]; }
    inline Node* newNode(node_types tpe) {return new Node(tpe);} 
    inline unordered_map<Node*, Bdd> makeFuncIfNot_All(vector<Node*>& list);
    Bdd  makeFuncIfNot(); 
    inline void addInput(Node* fanin) { input.push_back(fanin); }
    void changeName(string str){ nameRep = str;}
    void  connect(Node* fanin);     // OUTPUT, XOR, AND???
    
private:
    enum node_types typeRep;  // type of Node
    int             id;       // id number
    int             uniqueId; // guarantees uniqueness (from 1)
    Network*        parent;   // ???????? Network
    std::string     nameRep;  // name

    vector<Node*>   input;    // inputs  connect to
    vector<Node*>   output;   // outputs connect to

    Bdd             func;     // ???_?????
    Sop             cset;     // cube set
    Factor          fac;      // factored form
    int             polarity; // ?o?????[0???],1??????(default)]
    // cset??fac????e???Cfunc????????f
    // ?????????????????????Clev2idx??\??
    vector<int>        lev2idx;

    /*------ ?????C?? ------*/
    //Node(node_types tpe); // ????
    Node(const Node& op); // ?R?s?[
    virtual ~Node();      // ??Node??????CNetwork???o?^??C?????????
    static int nObj;      // ???????I?u?W?F?N?g???
    static int nBorn;     // ?????????I?u?W?F?N?g??? -> uniqueId????f

    /*------ input, output ???X (?_?u???????Nsafe????) ------*/
    void  collapse(Node* fanin);     // fanin??g????\??
    int   collapseTest(Node* fanin); // Factor????e???????????????
    void  delDupInput();             // ???????????????
    void  changeInput(Node* before, Node* after);  //?????u????????
    void  replaceNode(Node* replaced, int alsoName=1); // replaced??this??u????????DalsoName??name??R?s?[
    //void  connect(Node* fanin);     // OUTPUT, XOR, AND???
    void  disconnect(Node* fanin);  // OUTPUT???D?????I??XOR, AND??D

    void  recLetNouse(); // ?o?????0?????Node??????C??A?I??s?K?v??Node???????
    int   del_1input();  // ???????1??????????????C????o??????X????D

    Node* mergeSameType();     // ????????Type(XOR,AND)??Node?????????????
    int   mergedIfUsedUnate(); // fanout??1???Cfanout?????????????x?e??????unate??????merge
    int   eliminate();
    // ??????fanout??1???Cfanout?????????????x?e???????C
    // factored form??1???????merge

    /*------ ???_????? ------*/
    Sop  makeCsetIfNot(); // cset?????D????????D
    //Bdd  makeFuncIfNot(); // func?????D????????D

    int  trimLev2idx();   // ?K?v???????????????C????level????
    void compressFunc();  // ??????? compress ???? lev2idx ???X

    vector<int> reorderLev2idx(); // cubeOrder()??g??????????????D????????X?????D

    // todo: ?g????
    void invFunc();             // ???????]????
    void invInput(Node* fanin); // fanin?????????]

    // node2level?????????
    Bdd  calcFunc(const unordered_map<void*,int>& node2level);
    Sop  calcSop(const unordered_map<void*,int>& node2level);
    Sop  calcSop_first(const unordered_map<void*,int>& node2level);

    static int isSameFunc(Node* ni, Node* nj);

    int  nLitFac(); // factored form????e??????????
    int  nLitSop(); // sum-of-products????e??????????

    Bdd  calcFunc_sub(unordered_map<void*,Bdd>& fanin2func);
    // fanin2func??fanin??????????????????

    /*------ ???????????????X ------*/
    void makeConstant(const Bdd& f); // CONSTANT?????

    // level2node ?? f or cs ??]????C?????????C???????X?V
    // todo: marimo: makeMAJ????
    void makeLUT(const Bdd& f, const vector<Node*>& level2node);
    void makeSOP(const Sop& cs, int pola, const vector<Node*>& level2node);

    // base_n ???????X?g?? f or cs ??]????C?????????C???????X?V
    void makeLUT(const Bdd& f, Node* base_n);
    void makeSOP(const Sop& cs, int pola, Node* base_n);

    // todo: node-ger.h
    //??????d?l????????
    // level2index ?? 0 ???????C?K???^??????
    void makeLUT(const Bdd& f, const vector<Node*> varlist, int* level2index);
    // todo: ----------node-ger.h

    // f or cs ????????X?V?D
    // new_n?????????????Cnew_n_lev?????level?D
    // todo: ????H
    void updateLUT(const Bdd& f, Node* new_n, int new_n_lev);
    void updateSOP(const Sop& cs, int pola, Node* new_n, int new_n_lev);

    void getFaninCorn(vector<Node*>& result); // ???????Node????X?g

    // todo: ----------node-ger.h

    /*------------------------------------*/
    /*------ ?????obsolete??????? ------*/
    /*------------------------------------*/

    //    void    connectInput(Bdd f, Arrayw<Node*>& varlist, int* lev2idx=0);
    void connectInput(Bdd f, vector<Node*>& varlist, int* lev2idx);
    // Node????_????f????Cf???????e???????x?? lev ??????C
    // ????? varlist[ lev2idx[ lev ] ] ?????
    //   varlist ?? index ???? fanin ????
    //   lev2idx ?? varlist ??????? index???? (lev2idx==0?????level??index)

    // -> makeLUT()??makeSOP????D


    // todo: node-ger.h

    // todo: what is IntDC
    inline Bdd getIntDC() { return IntDC; }
    inline void SetIntDC(Bdd set) { IntDC = set; }

    void compressFuncAndIntDC();

    /*------ successor, predecessor ------*/
    //  is Node ?? immediate successor??? true ??????B
    // ?o?????????IS
    inline bool isImmediateSuccessor(Node * isNode) {
        if (outContain(isNode) == true) return true;
        else return false;
    }
    //  is Node ?? successor??? true ??????B
    bool isSuccessor(Node* isNode);
    //  is Node ?? immediate predecessor??? true ??????B
    inline bool isImmediatePredecessor(Node * ipNode) {
        if(fanin2index(ipNode) == -1) return false;
        else return true;
    }
    //  is Node ?? predecessor??? true ??????B
    bool isPredecessor(Node* ipNode);

    //    void	printInfo(FILE *sout, int mode=0);//node?????o??(For Debug)
    //    void	printInfo(FILE *sout, int mode=0, int mode2=0);//node?????o??(For Debug)
    void printInfo(FILE *sout, int mode, int mode2);//node?????o??(For Debug)

    // ----------node-ger.h

    void check(int numRecursive); // ???o?????????type????`?F?b?N
    void delAllInput();           // ?????????C?o???????

    /*------ input, output ???? ------*/
    //  level: func???????Bdd????x?? int 1????
    //     index: ?????????????       int 0????nIn()-1
    //     fanin: ?????Node          Node*
    //     lev2idx:     level -> index
    //     input:       index -> fanin
    // todo: ???x????BDD?????????
    int   fanin2level(Node* fanin); // fanin??level?D ?????????0
    int   fanin2index(Node* fanin); // fanin??index?D?????????-1
    Node* level2fanin(int level);   // ???x????level?????fanin?C?????????0
    int   level2index(int level);   // ???x????level?????fanin??index
    int   index2level(int index);   // in(index)??level?D?????????0
    //inline Node* index2fanin(int index) { return input[index]; }

    static int compareInputSet(Node* ni, Node* nj, unordered_map<void*,int>& node2level);
    // ni ?? nj ?????W???????D
    // ni ??? nj ???????(?????????)?C1?????D

    bool outContain(Node* fanout); // fanout ???o????????

    // input, output ???X (??????????N??????X)
   
    inline void addOutput(Node* fanout) { output.push_back(fanout); }
    inline void delOutput(Node* fanout) { output.erase(find(output.begin(), output.end(), fanout)); }
    inline void clearInput() { input.clear();input.shrink_to_fit(); }    // ????????
    inline void clearOutput() { output.clear(); }  // ?o??????

    // fanin????this???????N?C?o?^????????????C????????
    void addLink(Node* fanin); // ?????N??????
    void delLink(Node* fanin); // ?????N???????

    void connectInputSub(const Bdd& varset,
                          vector<Node*> varlist, int* level2index);
    void connectInputSub(const Bdd& varset, const vector<Node*>& level2node);
    void connectInputCore(vector<Node*> node_list, vector<int> level_list);

    Bdd  rec_calcFunc(unordered_map<void*,Bdd>& node2func);
    Sop  calcSop_sub(unordered_map<void*,Sop>& node2func);

    static void assignLevel(Node* fn, Node* gn, unordered_map<void*,int>& node2level);

    //     void    delOneInput(); // 1?????????????????????o??????

    //    friend class Network;
    // friend void Network::regNode(Node* node);
    // friend void Network::regPI(Node* node);
    // friend void Network::regPO(Node* node);

    // todo: node-ger.h

    //??????m?[?h??o???????P????o??[?q?????????????true
    bool checkBeforeOutput();
    Bdd IntDC; //???????don't care  -> NetDecompSDD ??g?p(?????public???)
    //???? ?f?t?H???g??  Bdd::NULL ???? ?????I????????K?v

    // todo: ----------node-ger.h
};

#endif // #ifndef __NODE_H__
