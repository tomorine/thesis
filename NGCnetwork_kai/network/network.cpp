#include "network.h"

static int maxLevelCmp(const void* lhs, const void* rhs);
static void df_interleave(Node* g, Node* frm, Node*& last,
                          list<Node*>& vlist,
                          unordered_map<void*,int>& visited, unordered_map<void*,void*>& from);

// static	Cmap<void*,int> gateMaxLevels(0, ptrHash, 1000); // todo
static	unordered_map<void*,int> gateMaxLevels;
// PIに対する出力までの最大距離，各内部ノードに対しては，tfiの中で最大のもの

// static Cmap<void*, vector<Node*>* > nodeInput(0, ptrHash, 1000); // todo
static unordered_map<void*, vector<Node*>* > nodeInput;
static unordered_map<void*,int>* local_idepths;
/* ------------- dfsorder.cpp -------- */


using namespace std;

// Network::Network() : variables(0, ptrHash),
//                      outfuncs(Bdd::null, ptrHash), idepths(-1, ptrHash), odepths(-1, ptrHash),
//                      num_connect(0), num_level(0), num_node(0), PIorig_name(0), POorig_name(0),
//                      new_node_nameID(1) {
Network::Network() : num_connect(0), num_level(0), num_node(0), PIorig_name(0), POorig_name(0),
                     new_node_nameID(1) {
    // variables.insert(make_pair(, )); //todo
    // idepths.insert(make_pair(, ));
    // odepths.insert(make_pair(, ));
    // outfuncs.insert(make_pair(, ));
    primaryI.push_back(0);       // primaryI[0] を埋める
    primaryO.push_back(0);       // primaryO[0] を埋める
    intNode.push_back(0);        // intNode[0] を埋める
    specPO.push_back(Bdd::null); // specPO[0] を埋める
    sopPO.push_back(Sop::null);  // sopPO[0] を埋める
    constant0 = 0;         // 0を埋める
    constant1 = 0;         // 0を埋める

    //name2node = new Cmap<std::string, Node*>(0, hash_string, 1000); // todo
    name2node = new unordered_map<string, Node*>();
}

Network::~Network()
{
    for (int i=1; i<=nPI(); i++ ) delete getPI(i);
    for (int i=1; i<=nPO(); i++ ) delete getPO(i);
    for (auto no = ++intNode.begin(); no != intNode.end(); no++) delete *no;
    if ( constant0 ) delete constant0;
    if ( constant1 ) delete constant1;
    delete name2node;
}

void Network::bddChangeNet(Bdd f, vector<Node*>& intnode, vector<bool>& n_frg)
{
    if(f == Bdd::null){ 
        //cout << "return null" <<endl;
        return;
    }
    Bdd f_, f0, f1;                            
    if (f == Bdd::zero) {     
        //cout << "retrun zero" <<endl;                    
        return;
    }
    f_ = ~f;
    if (f_ == Bdd::zero) {
        //cout << "return one" <<endl;
        return;
    }
    //cout << "frg size is" << n_frg.size() <<endl;
    f0 = f.rstrtop0();
    f1 = f.rstrtop1();
    
if(f1 == Bdd::one){
      //  cout << "f1 == Bdd::one" <<endl;
        n_frg.push_back(true);
        //assert(n_frg.size()==3);        
        Node* tmp = new Node(AND);
        intnode.push_back(tmp); 
        int idx = intnode.size()-1; //intnode[0] == null
        int i=3;
        for(auto j: n_frg){
            if (j){
                intnode[idx]->connect(intnode[i]);
                --i;
            }else{
                Node* nt = new Node(NOT);
                nt -> connect(intnode[i]);
                intnode[idx] -> connect(nt);
                intnode.push_back(nt);
                --i;
            }
        }
        n_frg.pop_back();
    }
 
 //   cout << "f1 f0 is not constant" << endl;
        n_frg.push_back(true);
        bddChangeNet(f1,intnode,n_frg);
        n_frg.pop_back();


    if(f0 == Bdd::one){
        //cout << "f0 == Bdd::one" <<endl;
        n_frg.push_back(false);
        //assert(n_frg.size()==3);
        Node* tmp = new Node(AND);
        intnode.push_back(tmp); 
        int idx = intnode.size()-1; //intnode[0] == null
        int i=3;
        for(auto j: n_frg){
            if (j){
                //cout << "intnode size is " << intnode.size() << " idx is " << idx << " i is " <<i << endl;
                intnode[idx]->connect(intnode[i]);
          //      cout << "true" << endl;
                --i;
            }else{
            //    cout << "false" << endl;
                Node* nt = new Node(NOT);
                nt -> connect(intnode[i]);
                intnode[idx] -> connect(nt);
                intnode.push_back(nt);
                --i;
            }
        }
        n_frg.pop_back();
        //cout << " return p1" << endl;
        return;
    }

        n_frg.push_back(false);        
        bddChangeNet(f0,intnode,n_frg);
        n_frg.pop_back();
        //cout << " return p3" << endl;        
        return;
    
     assert(0);
}


void Network::calcOutFuncIm(Node* node)
{
    vector<Node*> fanin = node->getInput();
    Bdd tmp;

    if(node->getType() == AND){
        cout << "AND" <<endl;
        tmp = Bdd::one;
        for(auto i: fanin){
            tmp = tmp * outfuncs[i];
        }
        cout << "END" <<endl;     
    }else if(node->getType() == OR){
        cout << "OR" << endl;
        tmp = Bdd::zero;
        for(auto i: fanin){
            tmp = tmp | outfuncs[i];
        }
        cout << "END" <<endl;             
    }else if(node->getType() == NOT){
        cout << "NOT" << endl;
        tmp = ~outfuncs[fanin[0]];
        cout << "END" <<endl;             
    }else if(node->getType() == OUTPUT){
        cout << "OUTPUT" << endl;
        tmp = outfuncs[fanin[0]];
        cout << "END" <<endl;             
    }else if(node->getType() != INPUT){
        cout << "calcOutFuncIm() doesn`t its type" <<endl;
        assert(0);
    }
    
    outfuncs[node] = tmp;
}

void Network::calcOutFuncIm_All(Node* node)
{
    vector<Node*> fanin = node->getInput();
    for(auto i: fanin){
        if(outfuncs[i] == Bdd::null) {
            cout << "Recursion" << endl;
            calcOutFuncIm_All(i);
        }
    }
    calcOutFuncIm(node);
    return ;
}


void Network::regPI(Node* node)
{
    int id = primaryI.size();
    primaryI.push_back(node);
    node->id = id;
    node->parent = this;
    (*name2node)[node->getName()] = node;
    //     checkChangeName(constant0);
}

void Network::regPO(Node* node)
{
    int id = primaryO.size();
    primaryO.push_back(node);
    node->id = id;
    node->parent = this;

    // OUTPUTはname2nodeには登録しない
}

void Network::makeConstantNodes()
{
    if ( constant0 == 0 ) {
        constant0 = new Node(CONSTANT);
        constant0->makeConstant(Bdd::zero);
        constant0->parent = this;
        constant0->nameRep = "<0>";
    }
    if ( constant1 == 0 ) {
        constant1 = new Node(CONSTANT);
        constant1->makeConstant(Bdd::one);
        constant1->parent = this;
        constant1->nameRep = "<1>";
    }
    // 定数Nodeはname2nodeには登録しない
    // sweepClean() することで, 必ずBLIFには出なくなる．
}

// nodeを入力に向かって再帰的に登録する
void Network::regNode(Node* node)
{
    //cout << "regnode" <<endl;
    if ( node->parentNet() ) {
        assert( node->parentNet() == this );
        return;
    }
    else {
        for (auto&& fin : node->input) {
            if ( fin->parentNet() == 0 ) regNode( fin );
        }
        intNode.push_back(node);
        node->id = intNode.size()-1;
        node->parent = this;
        num_node++;

        // 名前
        if ( node->getName().length() > 0 ) {
            checkChangeName(node);
        }
        else {
            createNameIfNot(node);
        }
        (*name2node)[node->getName()] = node;

        for (auto&& fin : node->input) node->addLink( fin ); //add
    }
}

void Network::unregNode(Node* node)
{
    /* 削除されるNodeは，出力数が0でなければならない */
    assert( node->nOut() == 0 );

    assert( node->parentNet() == this );
    intNode[ node->id ] = 0;
    node->parent = 0;
    num_node--;

    // 削除されたNodeの名前は，新たに使われないようにしておく
    (*name2node)[node->getName()] = (Node*)Network::DELETED_NODE;

    for (auto&& fin : node->input) node->delLink( fin ); //del
}

Node*	Network::createNode(node_types tpe, const std::string& name) {
    //cout << "createNode" << endl;
    Node* node = new Node(tpe);
    node->nameRep = name;
    regNode( node );
    return node;
}

Node*	Network::createPI(const std::string& name) {
    //cout << "createPI" << endl;
    Node* node = new Node(INPUT);
    node->nameRep = name;
    regPI( node );
    return node;
}

Node*	Network::createPO(const std::string& name) {
    Node* node = new Node(OUTPUT);
    node->nameRep = name;
    regPO( node );
    return node;
}

void Network::delNode(Node* node)
{
    unregNode( node );
    delete node;
}

void Network::delPI(Node* node)
{
    assert( node->nOut() == 0 );
    primaryI.erase(find(primaryI.begin(), primaryI.end(), node));
    (*name2node)[node->getName()] = (Node*)Network::DELETED_NODE;
    delete node;
}

void Network::delPO(Node* node)
{
    node->disconnect(node->input[0]);
    primaryO.erase(find(primaryO.begin(), primaryO.end(), node)); // nodeを削除
    delete node;
}

// 内部ノードの入出力，PIの出力，POの入力をすべて消して，
// 内部ノードを全て削除
void Network::delAllNode()
{
    for (auto no = ++intNode.begin(); no != intNode.end(); no++) {
        (*no)->clearInput();
        (*no)->clearOutput();
    }
    int i;
    for ( i=1; i<=nPO(); i++ )
        if ( getPO(i)->nIn() ) getPO(i)->clearInput();
    for ( i=1; i<=nPI(); i++ )
        if ( getPI(i)->nOut() ) getPI(i)->clearOutput();

    clean();
}

/* 各Nodeにおいて，定数Node・1入力Nodeなど使わないようにする */
void Network::sweep()
{
    int change = 1;
    while ( change ) {
        change = 0;
        for (auto no = ++intNode.begin(); no != intNode.end(); no++) {
            if ( (*no)->del_1input() ) change = 1;
        }
    }
}

/* 出力数が0である Nodeを削除する */
void Network::clean()
{
    //////// clean unused gates ////////
    int change = 1;
    while ( change ) {
        change = 0;
        for (auto no = ++intNode.begin(); no != intNode.end(); no++) {
            if ( (*no)->nOut() == 0 ) {
                delNode(*no);
                // これで，出力数が0であるNodeが増えるかもしれない．
                change = 1;
            }
        }
    }

    //     compressIntNode(); // intNodeを詰める
}

/* 同じ入力，同じ内部関数のNodeを削除 */
void Network::elimDupNode()
{
    int change = 1;
    while ( change ) {
        change = 0;
        for (auto ni = ++intNode.begin(); ni != intNode.end(); ni++) {
            // 1入力以下はしない．いずれ sweepで無くなる．
            if ( (*ni)->nIn() <= 1 ) continue;
            // 	    printf("ni = %d\n", ni->getUniqueId());
            for (auto nj = ++ni; nj != intNode.end(); nj++) {
                // 		printf("  nj = %d\n", nj->getUniqueId());

                int replace = Node::isSameFunc(*ni, *nj);
                if ( replace == 1 ) {
                    (*ni)->replaceNode(*nj);
                    delNode(*nj);
                    change = 1;
                    break; // replaceが起こったら，niも進めなければならない．
                }
                else if ( replace == -1 ) {
                    // niの出力にNOTをつけたものをnjとする
                    vector<Node*> level2node(2);
                    level2node[1] = *ni;
                    Bdd f = ~(Bdd::var(1)); // NOT
                    (*nj)->makeLUT(f, level2node);

                    change = 1;
                    break; // replaceが起こったら，niも進めなければならない．
                }
            }
        }
        if ( change ) sweepClean();
    }
    printCost((char*)"elimDupNode()");
}

/* unateでしか使われていないNodeをcollapseする */
void Network::collapseUsedUnate()
{
    int changed = 1;
    while ( changed ) {
        changed = 0;
        for (auto no = ++intNode.begin(); no != intNode.end(); no++) {
            if ((*no)->mergedIfUsedUnate()) {
                assert( (*no)->nOut() == 0 );
                delNode(*no);
                changed = 1;
            }
        }
    }
    sweepClean();
    printCost((char*)"collapseUsedUnate()");
}

/* valueの小さいNodeをcollapseする */
void Network::collapseSmallValue()
{
    int changed = 1;
    /* mergeIfCubeFreeだけで，すべてcube freeになり，リテラル数も少なくなる．
     ただし，出力に否定がつく場合がある．*/
    while ( changed ) {
        changed = 0;
        for (auto no = ++intNode.begin(); no != intNode.end(); no++) {
            if ((*no)->eliminate()) {
                changed = 1;
            }
        }
    }
    sweepClean();
    printCost((char*)"collapseSmallValue()");
}

void Network::printAllNode()
{
    for (auto no = ++intNode.begin(); no != intNode.end(); no++) {
        printf("ptr=%p, id=%d\n", *no, (*no)->id);
        // no->print(1);
    }
}

// "#node = %3d, #lev = %2d, #lit(sop) = %4d, #lit(fac) = %4d, %s\n",
void Network::printCost(char *msg)
{
    // #lev の計算だけここで
    // unordered_map<void*,int> idepths(-1, ptrHash); // todo
    unordered_map<void*,int> idepths_temp;

    for (auto no = ++primaryO.begin(); no != primaryO.end(); no++) {
        calcIdepth(*no, idepths_temp);
    }

    int maxlev = 0;
    for(const auto& it : idepths_temp) {
        int dp = it.second;
        maxlev = (dp > maxlev) ? dp : maxlev;
    }

    fprintf(stderr,
            "#node = %3d, #lev = %2d, #lit(sop) = %4d, #lit(fac) = %4d, %s\n",
            nNode(), maxlev-1, literalCount(), literalCountFac(), msg);
}

int Network::verify()
{
    fprintf(stderr, "verifying\n");
    if ( specPO.size() != (unsigned int)nPO()+1 ) {
        my_abort((char*)"specPO[i]'s were not calculated.");
    }

    clearOutfuncAll();
    calcOutfuncAll();

    for ( int i = nPO(); i>=1; i-- ) {

        int ng = 0;
        Bdd f = outfuncs[ getPO(i) ];
        Bdd onset = specPO[i].rstrtop1();
        if ( !( onset <= f ) ) {
            getPO(i)->print(1);
            fprintf(stderr, "f:\n");
            CubeSet::printSop( f );
            fprintf(stderr, "onset:\n");
            CubeSet::printSop( onset );
            fprintf(stderr, "Not covered onset:\n");
            CubeSet::printSop( onset & ~f );
            ng = 1;
        }
        // 	assert( ng == 0 );
        if ( ng ) return 0;
        Bdd offset = specPO[i].rstrtop0();
        if ( !( f <= ~offset ) ) {
            fprintf(stderr, "Covered offset:\n");
            CubeSet::printSop( offset & f );
            ng = 1;
        }
        // 	assert( ng == 0 );
        if ( ng ) return 0;
    }
    return 1;
}
vector<int> Network::getLev2IdxFrom0() { // indexは0から始まる
    vector<int> from0(nPI()+1);	// 最大level <= nPI()
    for ( int i=nPI(); i>0; i-- ) from0[i] = lev2piidx[i] -1;
    return from0;
}

int Network::literalCount()
{
    int nLitSop=0;
    for (auto no = ++intNode.begin(); no != intNode.end(); no++) {
        nLitSop += (*no)->nLitSop();
    }
    return nLitSop;
}

int Network::literalCountFac()
{
    int nLitFac=0;
    for (auto no = ++intNode.begin(); no != intNode.end(); no++) {
        nLitFac += (*no)->nLitFac();
    }
    return nLitFac;
}

void Network::calcOutfuncAll()
{
    setOutfuncs();
    for (auto no = ++primaryO.begin(); no != primaryO.end(); no++)
        calcOutfunc(*no);
}

void Network::clearOutfuncAll()
{
    // unordered_map<void*, Bdd>().swap(outfuncs); // todo: メモリ切り詰めなくても開放される？検討
    outfuncs.clear();
}

void Network::calcIdepthAll()
{
    for (auto no = ++primaryO.begin(); no != primaryO.end(); no++) {
        calcIdepth(*no);
    }
}

void Network::clearIdepthAll()
{
    // unordered_map<void*, int>().swap(idepths); // todo: メモリ切り詰めなくても開放される？検討
    idepths.clear();
}

void Network::calcOdepthAll()
{
    for (auto no = ++primaryI.begin(); no != primaryI.end(); no++)
        calcOdepth(*no);
}

void Network::clearOdepthAll()
{
    odepths.clear();
}

// boundary: [Node* -> level] を
// level2node: [level -> Node*] に変換
// todo: 使用されていないので消す予定
// static void convert_Cmap_Arrayw(const Cmap<void*,int>& boundary,
//                                 Arrayw<Node*>& level2node)
// {
//     for ( Pix p = boundary.first(); p; boundary.next(p) ) {
//         Node* no = (Node*)boundary.key(p);
//         int level = boundary.value(p);

//         if ( level2node.size() < level+1 ) {
//             level2node.reserve( level+1 );
//         }
//         level2node[ level ] = no;
//     }
// }

// nodeの名前が空だったら，作る．
// ただし，INPUTとOUTPUTは名前を持っていること
void Network::createNameIfNot(Node* node)
{
    if ( node->getName().length() > 0 ) return;

    if ( node->getType() == INPUT ) {
        my_abort((char*)"INPUT should have name");
    }
    else if ( node->getType() == OUTPUT ) {
        my_abort((char*)"OUTPUT should have name");
    }

    // new_node_nameIDから，名前を生成する．
    // name2nodeを見て，重複しないようにしている．
    char tmp[16];
    std::string result;
    do {
        sprintf(tmp, "{%d}", new_node_nameID++  );
        result = std::string(tmp);
    } while ( (*name2node)[result] != 0  );

    node->nameRep = result;
}

// 自分の名前が，別のNodeで登録されていたら，自分の名前を変更する．
void Network::checkChangeName(Node* node)
{
    std::string result = node->getName();
    while ( (*name2node)[result] != 0 ) { // 同じものがあるとき
        if ( (*name2node)[result] == node ) break; // それが自分ならOK

        // 	result = "_" + result;

        // new_node_nameIDから，名前を生成する．
        // name2nodeを見て，重複しないようにしている．
        char tmp[16];
        do {
            sprintf(tmp, "{%d}", new_node_nameID++  );
            result = std::string
                (tmp);
        } while ( (*name2node)[result] != 0  );

    }
    node->nameRep = result;
}

// 外部出力OUTPUTに直結しているノードの名前をOUTPUTと同じにする
void Network::forceSameNameBeforePO()
{
    for (auto po = ++primaryO.begin(); po != primaryO.end(); po++) {
        Node* no = (*po)->input[0];
        if ( no->getName() != (*po)->getName() ) {
            (*name2node)[no->getName()] = (Node*)Network::DELETED_NODE;
            no->nameRep = (*po)->getName(); // marimo: nameRep = nodeの名前?
            (*name2node)[no->getName()] = no;
        }
    }
}

/* pla.cpp */
void Network::readFilePla(FILE *fp, const char* fn)
{
    if_format = PLA;

    alineSize = 128;
    aline = new char[alineSize];
    row = 0;

    // 入出力の数と名前の読み込み
    int n_pi, n_po;
    std::string *pi_names = 0;
    std::string *po_names = 0;
    while ( 1 ) {
        readline(fp, aline, alineSize, row);
        if ( aline[0] == EOF ) ifile_error(row, (char*)"incomplete input file");
        else if ( !strncmp(aline, ".i ", 3) ) {
            int res = sscanf(aline, ".i %d", &n_pi);
            if ( res != 1 ) 
                my_abort((char*)".i 1st line is incorrect");
            pi_names = new std::string[n_pi+1];
        }
        else if ( !strncmp(aline, ".o ", 3) ) {
            if (sscanf(aline, ".o %d", &n_po) != 1)
                my_abort((char*)".o line is incorrect");
            po_names = new std::string[n_po+1];
        }
        else if ( !strncmp(aline, ".ilb", 4) ) {
            if ( pi_names == 0 ) my_abort((char*)"no .i");
            createIlb(n_pi, pi_names);
        }
        else if ( !strncmp(aline, ".ob", 3) ) {
            if ( po_names == 0 ) my_abort((char*)"no .o");
            createOb(n_po, po_names);
        }
        else if ( !strncmp(aline, ".type", 5) )
            fprintf(stderr, "[Warning!] ESPRESSO default type is assumed!!\n");
        else if ( !strncmp(aline, ".p", 2) ) continue; // .p は無視
        else if ( !strncmp(aline, ".", 1 ) ) continue; // .ilb .ob 等も無視
        else break;
    }

    // もし，入出力の名前(ilbとob)が無かったら，作る．
    char tmp[16];
    if ( PIorig_name == 0 ) {
        if ( pi_names == 0 ) my_abort((char*)"no .i");
        for ( int i = n_pi; i>0; i-- ) {
            sprintf(tmp, "x%d", i);
            pi_names[i] = std::string(tmp);
        }
    }
    if ( POorig_name == 0 ) {
        if ( po_names == 0 ) my_abort((char*)"no .o");
        for ( int i = n_po; i>0; i-- ) {
            sprintf(tmp, "f%d", i);
            po_names[i] = std::string(tmp);
        }
    }

    // PI, POのNodeの作成
    for ( int i=1; i<=n_pi; i++ ) createPI(pi_names[i]);
    for ( int i=1; i<=n_po; i++ ) createPO(po_names[i]);

    // cubeの読み込み
    allcube = readCube(fp, nPI(), nPO());

    setName(fn);
    fprintf(stderr, "module: %s   n_in: %d   n_out: %d\n",
            name(), nPI(), nPO());

    delete [] pi_names;
    delete [] po_names;
    delete [] aline;
}

void Network::createIlb(int n_pi, std::string *pi_names)
{
    char* c = &(aline[4]);
    for ( int i=1; i<=n_pi; i++ ) {
        while (*c == ' ') c++;
        pi_names[i] = getAword(c);
        while ( (*c != ' ') && (*c != '\0') ) c++;
    }
    PIorig_name = 1;
}

void Network::createOb(int n_po, std::string *po_names)
{
    char* c = &(aline[3]);
    for ( int i=1; i<=n_po; i++ ) {
        while (*c == ' ') c++;
        po_names[i] = getAword(c);
        while ( (*c != ' ') && (*c != '\0') ) c++;
    }
    POorig_name = 1;
}

SopM Network::readCube(FILE *fp, int numPI, int numPO)
{
    int		second_more = 0;
    SopM result = SopM(Sop::zero, numPI);
    while(1) {
        if ( second_more ) readline(fp, aline, alineSize, row);
        else second_more = 1;
        char* c = aline;
        if ( *c == '.' || *c == '\0' ) break;	//// aline[0]

        Sop cube = Sop::one;
        //////// read input part ////////
        for ( int i=1; i<=numPI; i++ ) {
            while ( *c == ' ' || *c == '|' || *c == '\t' ) c++;
            if ( *c != '0' && *c != '1' && *c != '-' )
                ifile_error(row, (char*)"Unknown charactor");
            if ( *c == '0' ) cube = cube.and0(i);
            else if ( *c == '1' ) cube = cube.and1(i);
            c++;
        }
        SopM cubePO = SopM(cube, numPI);
        //////// read output part ( fDC and fON ) ////////
        for ( int i=1; i<=numPO; i++ ) {
            while ( *c == ' ' || *c == '|' || *c == '\t' ) c++;
            if ( *c != '0' && *c != '1' && *c != '2' && *c != '3' &&
                 *c != '4' && *c != '-' && *c != '~' )
                ifile_error(row, (char*)"Unknown charactor");
            if ( *c == '1' || *c == '4' ) cubePO.putOnset(i);
            else if ( *c == '-' || *c == '2' ) cubePO.putDcset(i);
            c++;
        }
        result += cubePO;
    }
    return result;
}

void Network::calAllSpecPla(char swBddOrder)
{
    /*------ 変数の順序付け ------*/
    plaPIorder(swBddOrder);

    /*------ Cubeの集合から ON-set, Off-set, DC-set を計算 ------*/
    sopPO.resize(nPO()+1);

    Bdd* tmfDC = new Bdd[nPO()+1];
    Bdd* tmfON = new Bdd[nPO()+1];
    Bdd* tmfOF = new Bdd[nPO()+1];
    for ( int i=1; i<=nPO(); i++ )
        tmfDC[i] = tmfON[i] = tmfOF[i] = Bdd::zero;

    vector<int> tmpinv = inverse(lev2piidx);

    for ( int i=1; i<=nPO(); i++ ) {
        Sop onset = allcube.getOnset(i);
        onset = onset.changeOrder( tmpinv );
        tmfON[i] = onset.getFunc();
        sopPO[i] = onset;

        Sop dcset = allcube.getDcset(i);
        dcset = dcset.changeOrder( tmpinv );
        tmfDC[i] = dcset.getFunc();
    }

    /*------ 各出力が満たすべき関数 specPO[i] を求める ------*/
    specPO.resize(nPO()+1);
    for ( int i=1; i<=nPO(); i++ ) {
        tmfON[i] = tmfON[i] & ~(tmfDC[i]);
        tmfOF[i] = ~(tmfON[i] | tmfDC[i]);

        int topvar = tmfDC[i].top();
        if ( topvar < tmfON[i].top() ) topvar = tmfON[i].top();
        if ( topvar < tmfOF[i].top() ) topvar = tmfOF[i].top();
        specPO[i] = Bdd::varIte(topvar+1, tmfON[i], tmfOF[i]);
    }
    delete [] tmfDC;
    delete [] tmfON;
    delete [] tmfOF;
}

void Network::plaPIorder(char swBddOrder)
{
    char* env = getenv("_BDD_ORDER"); // o: orignal order, h: heuristic order

    if ( env ) {
        if ( env[0] == 'o' ) swBddOrder = '0'; // original
        else if ( env[0] == 'h' ) swBddOrder = '1'; // heuristic
    }

    /*------ 変数の順序付け ------*/
    lev2piidx.resize(nPI()+1);
    lev2piidx[0] = 0;

    switch( swBddOrder ) {	//// change case num orig. <-> heu. 1996.4.16
    case '0':
        //// original order
        for ( int i=1; i<=nPI(); i++ ) lev2piidx[i] = i;
        fprintf(stderr, "plaPIorder(): original order\n");
        break;
    case '1':
        //// heuristic order
        cubeOrder(allcube.elimOidx(), lev2piidx, nPI());
        fprintf(stderr, "plaPIorder(): heuristic order\n");
        break;
    }
    //     printf("plaPIorder(): ");
    //     for ( int i=1; i<=nPI(); i++ ) printf("%d ", lev2piidx[i]);
    //     printf("\n");

    for ( int lev = lev2piidx.size(); --lev>=1; ) {
        int piidx = lev2piidx[lev];
        if ( (1 <= piidx) && (piidx <= nPI()) ) {
            variables[ getPI(piidx) ] = lev;	// variables
        }
        // cubeOrderでlevelが与えられなかった入力に対しては，何もしない
    }
}

/* blif.cpp */
// NodeをSOPで作るかLUTで作るか，変数順をどうするかなどは createNames() 依存

void Network::readFileBlif(FILE *filep, const char* module)
{
    if_format = BLIF;

    alineSize = 128;
    aline = new char[alineSize];
    row = 0;
    //////// read the header of the input file ////////
    while ( 1 ) {
        readline(filep, aline, alineSize, row);
        // .model は無視，ファイル名を名前にする
        if (strncmp(aline, ".model", 6) == 0) continue;
        else if (strncmp(aline, ".inputs", 7) == 0) createInputs();
        else if (strncmp(aline, ".outputs", 8) == 0) createOutputs();
        else if (strncmp(aline, ".names", 6) == 0) break;
        else if (strncmp(aline, ".end", 4) == 0) break;
        else {
            fprintf(stderr, "Incomplete blif file.\n");
            abort();
        }
    }
    //////// each iteration read one gate ////////
    while (strncmp(aline, ".end", 4) != 0) {
        if (strncmp(aline, ".names", 6) != 0)
            ifile_error(row, (char*)"Unknown case");
        createNames(filep);
        if ( aline[0] == '\0' ) break;
    }

    // すべてTypeが定義されたかのチェック
    for(const auto& it : (*name2node)) {
        if (it.second->getType() == UNDEFINED) {
            fprintf(stderr, "%s\n", it.first.c_str());
            it.second->print(1);
        }
    }

    setName(module);
    fprintf(stderr, "module: %s  n_in: %d  n_out: %d\n", name(), nPI(), nPO());
    delete [] aline;
}

void Network::createInputs()
{
    int i;
    int	n_pi = nPI();
    int old_n_pi = n_pi;

    if ( old_n_pi == 0 ) n_pi = word_cnt(&aline[7]);
    else n_pi += word_cnt(&aline[7]);

    char* c = &(aline[7]);
    for ( i=old_n_pi+1; i<=n_pi; i++) {
        while (*c == ' ') c++;
        newGateName(c, INPUT);
        while ( (*c != ' ') && (*c != '\0') ) c++;
    }
}

// Spec-blif.txt に従う
void Network::createOutputs()
{
    int i;
    int n_po = nPO();
    int old_n_po = n_po;

    if ( old_n_po == 0 ) n_po = word_cnt(&aline[8]);
    else n_po += word_cnt(&aline[8]);

    char* c = &(aline[8]);
    for ( i=old_n_po+1; i<=n_po; i++ ) {
        while (*c == ' ') c++;
        Node* tmpg = newGateName(c, UNDEFINED);
        Node* outn = createPO(tmpg->getName()); // name を getPO(i) にコピー
        outn->connect(tmpg);
        while ( (*c != ' ') && (*c != '\0') ) c++;
    }
}

void Network::createNames(FILE* filep)
{
    //     printf("%s\n", aline);

    char*	c = &aline[6];
    int		numIn = word_cnt(c) - 1;
    vector<Node*> in_gg(numIn+1);

    //////// first line ////////
    for ( int i = 1; i <= numIn; i++ ) {
        while (*c == ' ') c++;
        in_gg[i] = newGateName(c, UNDEFINED);
        while (*c != ' ') c++;
    }
    while (*c == ' ') c++;
    Node* or_g = newGateName(c, LUT);

    //////// second line or more ////////
    int invert;
    Sop cubes = readCubeSingle(filep, numIn, invert);

    // 変数順を変更
    vector<int> permutation;
    cubeOrder(cubes, permutation, numIn);
    cubes = cubes.changeOrder(inverse(permutation));

    vector<Node*> ordered = in_gg;
    changeOrder<vector<Node*>>(ordered, inverse(permutation));

    int polarity = 1;
    //     const int always_isop = 1;
    const int always_isop = 0;
    // ISOPの計算は別関数で，きちんとcubeOrderしてから
    if ( always_isop ) {
        IsfLU isf = IsfLU( cubes.getFunc() );
        cubes = isf.getBetterSop(polarity);
    }

    if ( invert == 1 ) polarity = !polarity;
    or_g->makeSOP(cubes, polarity, ordered);

    or_g->delDupInput();	// おなじ入力があるかもしれないので
}

Node* Network::newGateName(char *c, enum node_types tpe)
{   
   // cout << "newgatename" << endl;
    const std::string key = getAword(c);
    Node* tmpg = (*name2node)[key];

    if ( tmpg ) {
        if ( tpe != UNDEFINED ) tmpg->typeRep = tpe;
    }
    else {
        if ( tpe == INPUT ) tmpg = createPI(key);
        else tmpg = createNode(tpe, key);
    }
    //     tmpg->print(1);
    return tmpg;
}

Sop Network::readCubeSingle(FILE *fp, int numPI, int& invert)
{
    //     int	second_more = 0;
    Sop result = Sop::zero;
    int p_appear = 0;
    int n_appear = 0;
    while(1) {
        readline(fp, aline, alineSize, row);
        // 	if ( second_more ) readline(fp, aline, alineSize, row);
        // 	else second_more = 1;
        char* c = aline;
        if ( *c == '.' || *c == '\0' ) break;	//// aline[0]

        Sop cube = Sop::one;
        //////// read input part ////////
        for ( int i=1; i<=numPI; i++ ) {
            while ( *c == ' ' || *c == '|' || *c == '\t' ) c++;
            if ( *c != '0' && *c != '1' && *c != '-' )
                ifile_error(row, (char*)"Unknown charactor");
            if ( *c == '0' ) cube = cube.and0(i);
            else if ( *c == '1' ) cube = cube.and1(i);
            c++;
        }
        //////// read output part ( fON or fOFF ) ////////
        while ( *c == ' ' || *c == '|' || *c == '\t' ) c++;
        if ( *c == '0' ) {
            n_appear = 1;
        }
        else if ( *c == '1' ) {
            p_appear = 1;
        }
        else ifile_error(row, (char*)"Unknown charactor");
        c++;

        result = result + cube;
    }

    if ( p_appear && n_appear ) 
        ifile_error(row, (char*)"Both of ON-set and OFF-set are appeared");
    if ( n_appear ) invert = 1;
    else invert = 0;
    return result;
}

//////////////////////////////////////////////////////////////////

void Network::calAllSpecBlif(char swBddOrder)
{
    /*------ 変数の順序付け ------*/
    blifPIorder(swBddOrder);

    /*------ outfuncsの計算 ------*/
    clearOutfuncAll();
    calcOutfuncAll();

    /*------ 各出力が満たすべき関数 specPO[i] を求める ------*/
    specPO.resize(nPO()+1);
    for ( int i=1; i<=nPO(); i++ ) {
        Bdd f = outfuncs[ getPO(i) ];
        specPO[i] = Bdd::varIte(f.top()+1, f, ~f);
    }
}

// lev2piidxとvariablesが作られる．(以前のものがあっても上書きする)
void Network::blifPIorder(char swBddOrder)
{
    char* env = getenv("_BDD_ORDER"); // o: orignal order, h: heuristic order

    if ( env ) {
        if ( env[0] == 'o' ) swBddOrder = '0'; // original
        else if ( env[0] == 'h' ) swBddOrder = '1'; // heuristic
    }

    /*------ 変数の順序付け ------*/
    lev2piidx.resize(nPI()+1);
    lev2piidx[0] = 0;

    switch( swBddOrder ) {	//// change case num orig. <-> heu.
    case '0':
        //// original order
        for ( int i=1; i<=nPI(); i++ ) lev2piidx[i] = i;
        break;
    case '1':
        //// heuristic order
        list<Node*> vlist;
        dfsOrder(vlist);

        // vlistからPIだけを取り出しlev2piidxに．
        int varLevel = nPI();
        for (const auto& it : vlist) {
            Node* no = it;
            if ( no->getType() == INPUT ) {
                int idx = no->getId();
                lev2piidx[ varLevel-- ] = idx;
            }
        }

        // PIに空きがあるときの処理
        for ( ; varLevel>0; varLevel-- ) lev2piidx[ varLevel ] = 0;
        assert( varLevel == 0 );

        break;
    }

    for ( int lev = lev2piidx.size(); --lev>=1; ) {
        int piidx = lev2piidx[lev];
        if ( (1 <= piidx) && (piidx <= nPI()) ) {
            variables[ getPI(piidx) ] = lev;	// variables
        }
        // dfsOrderでlevelが与えられなかった入力に対しては，何もしない
    }

    const int report = 0;
    if ( report ) {
        for ( int lev = 1; lev <= nPI(); lev++ ) {
            int idx = lev2piidx[lev];
            fprintf(stderr, "%s ", getPI(idx)->name());
        }
        fprintf(stderr, "\n");
    }
}

//////////////////////////////////////////////////////////////////

void Network::outBlif(FILE *fp)
{
    int i;
    if ( name() != 0 ) fprintf(fp, ".model %s\n", name());

    fprintf(fp, ".inputs");
    for ( i=1; i<=nPI(); i++ ) {
        assert( getPI(i)->getName().length() > 0 );
        getPI(i)->printName(fp);
    }
    fprintf(fp, "\n");

    fprintf(fp, ".outputs");
    for ( i=1; i<=nPO(); i++ ) {
        getPO(i)->printName(fp);     // OUTPUTの名前をそのまま出力
        assert( getPO(i)->getFunc().rstrtop1() == Bdd::one );
        // OUTPUTの関数は恒真関数であることの確認
    }
    fprintf(fp, "\n");

    // 内部Node
    for (auto no = ++intNode.begin(); no != intNode.end(); no++) {
        assert((*no)->nOut() >= 1);
        if((*no)->checkBeforeOutput() == true){
            //★出力の1つ前の場合は特殊にする
            (*no)->printBlif_before_output(fp);
        }
        else{
            createNameIfNot(*no);
            (*no)->printBlif(fp);
        }
    }
    //★ 入力が出力に直結する場合 (apex7など) に対応する
    for ( i=1; i<=nPI(); i++ ) {
        if(getPI(i)->checkBeforeOutput() == true){
            //★出力の1つ前の場合は特殊にする
            getPI(i)->printBlif_before_output(fp);
        }
    }
    fprintf(fp, ".end\n");
}

// Network 内の nodeで名前を全て付け直す．（PI, POは除く)
// "GNo"+UniqueID とする．
void Network::trimNodeName()
{
    int MaxIdDigit = 8; // UniqueId の最大値が 2 << 20 と仮定
    char NewName[MaxIdDigit];
    for (auto no = ++this->intNode.begin(); no != this->intNode.end(); no++) {
        //node->name() に対応する  name2node のエントリを向こうにしとかなだめ
        (*(this->name2node))[(*no)->getName()] = (Node*)Network::DELETED_NODE;
        int uid = (*no)->getUniqueId();
        if(uid > (2<<20) ){
            //      cerr << "Sorry: Too many node's (UniqueID). Please refer the programmer \n";
            exit(1);
        }
        itoa(uid, NewName);
        (*no)->nameRep = "GNo" + string(NewName);
        (*(this->name2node))[(*no)->getName()] = *no;
        this->checkChangeName(*no); //一応POと重なっていたらいやなので
    }
}



//PO->firstIn を POと同じ名前にする．
//出力時の無駄なバッファを除く
// ★★ PO -> another PO というのがある． vda.blifなど
void Network::copyPOname()
{
    for (auto no = ++this->primaryO.begin(); no != this->primaryO.end(); no++) {
        Node * fin = (*no)->input[0];
        if(fin->getType() == INPUT || fin->getType() == OUTPUT ){
            //★本とは PO-> PI 直結いけないが，lutsynだとそうなることある．
            //この場合はコピーしなくても，出力時に無駄なバッファは出ない．
            ;
        }
        else{
            if(fin->name() !=  (*no)->name()){
                string result = (*no)->getName();  //PO の 名前 
                Node * sameName = (*name2node)[result]; //同じname (POの名前はregisterされてないはず）
                if( (sameName!=0)  && (sameName != (Node*)Network::DELETED_NODE) ){
                    //	  cerr << " Error:: PO's name is registered for a node (not PO's fanin, not DELETED_NODE) \n";
                    exit (1);
                }
                fin->nameRep = result;
                (*name2node)[fin->getName()] = fin;
	
            }
        }
    }
}

int Network::calcIdepth(Node* no)
{
    // todo: clearが-1で埋めるなので一応チェック
    // todo: idepths[no]でアクセスすると0が埋められるので注意
    if ( (idepths.count(no) > 0) && (idepths[no] > -1) ) return idepths[no];

    int depth = 0; // INPUT等，入力がないものは0となる．
    for (auto&& fin : no->input) {
        int dp = calcIdepth(fin);
        if ( dp+1 > depth ) depth = dp+1;
    }
    return idepths[no] = depth;
}

int Network::calcIdepth(Node* no, unordered_map<void*, int>& idepths_tmp)
{
    if ( (idepths_tmp.count(no) > 0) && (idepths_tmp[no] > -1) )
        return idepths_tmp[no];

    int depth = 0; // INPUT等，入力がないものは0となる．
    for (auto&& fin : no->input) {
        int dp = calcIdepth(fin);
        if ( dp+1 > depth ) depth = dp+1;
    }
    return idepths_tmp[no] = depth;
}

void Network::clearIdepth(Node* no)
{
    if ( (idepths.count(no) == 0) || (idepths[no] <= -1)) return;

    idepths[no] = -1;
    for (auto&& fout : no->output) clearIdepth(fout);
}

void Network::clearIdepth(Node* no, unordered_map<void*, int>& idepths_tmp)
{
    if ( (idepths_tmp.count(no) == 0) || (idepths_tmp[no] <= -1) ) return;

    idepths_tmp[no] = -1;
    for (auto&& fout : no->output) clearIdepth(fout);
}

int Network::calcOdepth(Node* no)
{
    if ( (odepths.count(no) > 0) && (odepths[no] > -1) ) return odepths[no];

    int depth = 0; // OUTPUT等，出力がないものは0となる．
    for (auto&& fout : no->output) {
        int dp = calcOdepth(fout);
        if ( dp+1 > depth ) depth = dp+1;
    }
    return odepths[no] = depth;
}

int Network::calcOdepth(Node* no, unordered_map<void*, int>& odepths_tmp)
{
    if ( (odepths_tmp.count(no) > 0) && (odepths_tmp[no] > -1) )
        return odepths_tmp[no];

    int depth = 0; // OUTPUT等，出力がないものは0となる．
    for (auto&& fout : no->output) {
        int dp = calcOdepth(fout);
        if ( dp+1 > depth ) depth = dp+1;
    }
    return odepths_tmp[no] = depth;
}

void Network::clearOdepth(Node* no)
{
    if ( (odepths.count(no) == 0) || (odepths[no] <= -1) ) return;

    odepths[no] = -1;
    for (auto&& fin : no->input) clearOdepth(fin);
}

void Network::clearOdepth(Node* no, unordered_map<void*, int>& odepths_tmp)
{
    if ( (odepths_tmp.count(no) == 0) || (odepths_tmp[no] <= -1) ) return;

    odepths_tmp[no] = -1;
    for (auto&& fin : no->input) clearOdepth(fin);
}

Bdd Network::calcOutfunc(Node* no)
{
    if ( (outfuncs.count(no) > 0) && (outfuncs[no] != Bdd::null) )
        return outfuncs[no];

    Bdd result;
    if ( no->nIn() == 0 ) {
        result = no->makeFuncIfNot(); // CONSTANT
    }
    else {
        for (auto&& fin : no->input) calcOutfunc(fin);
        result = no->calcFunc_sub(outfuncs);
    }
    return outfuncs[no] = result; // outfuncs[no]に結果が入る
}

Bdd Network::calcOutfunc(Node* no, unordered_map<void*, Bdd>& outfuncs_tmp)
{
    if ( (outfuncs_tmp.count(no) > 0) && (outfuncs_tmp[no] != Bdd::null) ) return outfuncs_tmp[no];

    Bdd result;
    if ( no->nIn() == 0 ) {
        result = no->makeFuncIfNot(); // CONSTANT
    }
    else {
        for (auto&& fin : no->input) calcOutfunc(fin);
        result = no->calcFunc_sub(outfuncs_tmp);
    }
    return outfuncs_tmp[no] = result; // outfuncs[no]に結果が入る
}

void Network::clearOutfunc(Node* no)
{
    if ( (outfuncs.count(no) == 0) || (outfuncs[no] == Bdd::null) ) return;

    outfuncs[no] = Bdd::null;
    for (auto&& fout : no->output) clearOutfunc(fout);
}

void Network::clearOutfunc(Node* no, unordered_map<void*, Bdd>& outfuncs_tmp)
{
    if ( (outfuncs_tmp.count(no) == 0) || (outfuncs_tmp[no] != Bdd::null) ) return;

    outfuncs_tmp[no] = Bdd::null;
    for (auto&& fout : no->output) clearOutfunc(fout);
}

// Bdd::var( variables[node] ) を outfuncs[node] にセットする
void Network::setOutfuncs()
{
    for(const auto& it : variables) {
        Node* no = (Node*)it.first;
        outfuncs[no] = Bdd::var(it.second);
    }
}

void Network::setOutfuncs(const unordered_map<void*, int>& variables_tmp,
                          unordered_map<void*, Bdd>& outfuncs_tmp)
{
    for(const auto& it : variables_tmp) {
        Node* no = (Node*)it.first;
        outfuncs_tmp[no] = Bdd::var(it.second);
    }
}

int Network::recalcIdepthRecursive(Node* no) {
    int old_idepth =   idepths[no];
    idepths[no] = -1; //自分だけclearしておく．
    int new_idepth = calcIdepth(no);
    if(new_idepth != old_idepth){
        // 注意★★ 再帰的に呼ぶのは，普通に計算するのと逆向きだよ
        for (auto&& fout : no->output) recalcIdepthRecursive(fout);
    }
    return new_idepth;
}

int Network::recalcOdepthRecursive(Node* no) {
    int old_odepth = odepths[no];
    odepths[no] = -1; //自分だけclearしておく．
    int new_odepth = calcOdepth(no);
    if(new_odepth != old_odepth) {
        for (auto&& fin : no->input) recalcOdepthRecursive(fin);
    }
    return new_odepth;
}

Bdd Network::recalcOutfuncRecursive(Node* no){
    Bdd old_outfunc =   outfuncs[no];
    outfuncs[no] = Bdd::null; //自分だけclearしておく．
    Bdd new_outfunc = calcOutfunc(no);
    if(new_outfunc != old_outfunc){
        // 注意★★　再帰的に呼ぶのは，普通に計算するのと逆向きだよ　
        for (auto&& fout : no->output) recalcOutfuncRecursive(fout);
    }
    return new_outfunc;
}

// ファイルname (openする)を渡して、そのファイルをopenして、
// Blif形式で書き込む file が openできなければ retun 1
// 書き込めればreturn 0
int Network::printFileNameBlif(const char * filename)
{
    FILE* out_fp = fopen(filename, "w");
    if ( !out_fp ) {
        fprintf(stderr, "%s can not open\n", filename);
        return 1;
    }

    //  this->outBlif(out_fp);
    this->printBlifNoChange(out_fp);

    fclose(out_fp);
    return 0;
}


void Network::readFileName(const char * filename)
{
    FILE* ifs = fopen(filename, "r");
    if ( !ifs ) {
        fprintf(stderr, "%s does not exist\n", filename);
        exit(1);
    }
    //pla では、名前にはいってるやつがあるのでだめ、Mcnc/alupla.blif
    //    int is_pla = string(filename).find("pla") != string::npos;
    bool is_pla = string(filename).find(".pla") != string::npos;
    bool in_format = ( is_pla ) ? false : true;

    string module = string(filename);
    size_t pos_sl = module.find_last_of('/'); // marimo: 現在の文字列のインデックス 番目の文字から検索を開始し、文字列 の中に含まれる文字が最後に見つかった位置を返す。見つからない場合にはstring::nposを返す
    size_t pos_pl = module.find_last_of('.');
    module = string( module, pos_sl+1, pos_pl-pos_sl-1 );

    if ( in_format == false ) {
        this->readFilePla(ifs, module.c_str());
        // 	this->readFilePla(ifs, (const char*)module);
    }
    else if ( in_format == true ) {
        this->readFileBlif(ifs, module.c_str());
        // 	this->readFileBlif(ifs, (const char*)module);
    }
    fclose(ifs);
}



// ● network.cc の adding by ger 以下を  以下の部分と差し替えてください。
/////////////////////////////////////////
//Adding by ger
/////////////////////////////////

int Network::calcOlevAll()
{
    int maxOlev = 0;
    int nowOlev;

    for (auto no = ++primaryI.begin(); no != primaryI.end(); no++) {
        nowOlev = calcOdepth(*no);
        if(nowOlev > maxOlev) maxOlev = nowOlev;
    }
    num_level = maxOlev;
    return num_level;
}

void Network::clearOlevAll()
{
    //  for ( Node* no = firstPI(); no; no = nexPI() ){
    //★注意     clear_odepth(no, odepths)はfanin に対して再帰的によばれる 修正19980526
    for (auto no = ++primaryO.begin(); no != primaryO.end(); no++)
        clearOdepth(*no);
}

int Network::calcIlevAll()
{
    int maxOlev = 0;
    int nowOlev;
    for (auto no = ++primaryO.begin(); no != primaryO.end(); no++) {
        nowOlev = calcIdepth(*no);
        if(nowOlev > maxOlev) maxOlev = nowOlev;
    }
    num_level = maxOlev;
    return num_level;
}

void Network::clearIlevAll()
{
    //  for ( Node* no = firstPO(); no; no = nextPO() ){
    //★注意     clear_idepth(no, odepths)はfanin に対して再帰的によばれる 修正19980526
    for (auto no = ++primaryI.begin(); no != primaryI.end(); no++) {
        clearIdepth(*no);
    }
}

// cvarset 回路的に依存する変数集合を calculate, clear, set

Bdd Network::calcCvarset(Node* no, unordered_map<void*,Bdd>& cvarset)
{
    if ( cvarset[no] != Bdd::null ) return cvarset[no];

    Bdd result;
    if ( no->nIn() == 0 ) {
        result = no->makeFuncIfNot(); // CONSTANT
    }
    else {
        result = Bdd::empty;
        for (auto&& fin : no->input) {
            result = result + calcCvarset(fin, cvarset);
        }
    }
    return cvarset[no] = result; // cvarset[no]に結果が入る
}

void Network::clearCvarset(Node* no, unordered_map<void*,Bdd>& cvarset)
{
    if ( cvarset[no] == Bdd::null ) return;

    cvarset[no] = Bdd::null;
    for (auto&& fout : no->output) clearOutfunc(fout);
}

// Bdd::var( variables[node] ) を cvarset[node] にセットする
void Network::setCvar(unordered_map<void*,Bdd>& cvarset)
{
    for(const auto& it : variables) {
        Node* no = (Node*)it.first;
        cvarset[no] = Bdd::var(it.second);
    }
}

///////////////////////  Modfified by ger sice 05/12

//static int  hashfunc2(Node*&  g) { return ((int)g/4); }

// Inputs に指定されたノード のタイプをINPUTに
// Outputsに指定された各ノードの後ろに同じ名前で、ノードタイプがOUTPUTのノードをつけ、
// Inputs, Outputsで区切られたその他のノードは同じノードをつくって新しいNet
// をつくりそれを返す。
// 各ノードに対してコピーされる情報は、lutFunc, lev2idx, 結線情報, name、type(ただし、
//  Inputs に指定したものは、すべてINPUTタイプになる). Netの入出力ノードの順は、
// Inputs、Outputsで指定した順番と同じである。ただし、内部ノードの順番は保存されない
//のでnetviewしたら、回路の形状はかわるかもしれない。また、ノードタイプはLUTしか想定
// していないので、ほかだと問題あるかも
///////////////////
//★重要★ 渡す部分回路が以下の性質をもってないと、おちたり、
// へんなNetをかえすかもしれない。
// ●Inputs, Outputs は index=0 からいれておくこと。
// ●Inputs, Outputs には、同じノードが重複していれられていてはならない。
// ●Inputs, Outputsで指定した部分回路がInputs 以外の入力、Outputs以外の出力をもたない
// ●Outputs には、OUTPUTタイプのノードを指定しては基本的にだめ （＊１）

// ★一部以外の回路で全体を抜き出して、また埋め込む操作で動作確認

Network *  Network::createSubNet( vector<Node*> Inputs,   vector<Node*> Outputs)
{
    Network* SubNet = new Network();
    SubNet->setName(string("Sub-" + this->nameRep).c_str());
    //  SubNet->nameRep = "Sub-" + this->nameRep;
    //   cat("Sub-", this->nameRep, SubNet->nameRep);
    unsigned int i;

    PList<Node>*  NotDone = nil;

    // Cmap<void*,Node*>  *net1to2 = new Cmap<void*, Node*>(0, ptrHash, 1000); // todo
    unordered_map<void*,Node*>  *net1to2 = new unordered_map<void*, Node*>;
    //thisのNode* -> SubNetの対応するノードつくるたびに、Cmapに登録

    //create_input
    for(i=0; i<Inputs.size(); i++){            //順番はこれでいいかな
        Node * tempg1 = new Node(*(Inputs[i]));
        //★ コピーコンストラクタでは　type, input がコピーされるので　それは適切に処理しないとだめ
        tempg1->setType(INPUT);
        tempg1->input.resize(0);
        SubNet->regPI(tempg1);
        if ( tempg1->name() != nil ) {
            tempg1->nameRep = Inputs[i]->name();
        }
        (*net1to2)[Inputs[i]] = tempg1;
    }

    //create_output
    for(i=0; i<Outputs.size(); i++){          //順番はこれでいいかな
        Node * tempg = new Node(*(Outputs[i]));
        tempg->setType(Outputs[i]->getType());    
        tempg->input.resize(0);
        SubNet->regNode(tempg);
        if ( tempg->name() != nil ) {
            tempg->nameRep = Outputs[i]->name();
        }
        Node* outn = SubNet->createPO();
        outn->connect(tempg);            //これで論理は、x1 となる。
        outn->nameRep = tempg->name();    //name は保存list
        (*net1to2)[Outputs[i]] = tempg;   
        NotDone = NotDone->newPList(Outputs[i]);      
    }

    PList<Node>*  tempPList;
    Node * nowG;
    Node * nowGsub; //対応するsubNetの方
    vector<Node*> in_gg;    //nowGの入力
    vector<Node*> in_gg_sub;//nowGsubの入力
    while(NotDone != nil){
        nowG = NotDone->body;    //先頭から１つとる
        tempPList = NotDone;
        NotDone = NotDone->next;
        tempPList->next = nil;
        delete tempPList;
        // nowG->SubNet は 既に登録されている。 が、 そのIPはまだかもしれない。
        //少なくとも nowG と そのIPとはつながれていないよ。

        // todo: if(Inputs.contain(nowG) == 1){ // 見つかった
        if(find(Inputs.begin(), Inputs.end(), nowG) != Inputs.end()){
            ; //do nothing
        }
        else{
            nowGsub = (*net1to2)[nowG];
            assert(nowGsub);
            //nowGsub = 0 -> nowGsubはまだ未登録 -> おかしい
            //IP を 全て用意
            int numnIn = nowG->nIn();
            in_gg.resize(numnIn);
            //0から fan-in をいれる。以下のconnectInput参照
            in_gg_sub.resize(numnIn);

            // とりあえず、０で初期化しとかなあかんやろ
            for(int j=0;j<numnIn;j++){
                in_gg_sub[j] = 0;
            }
            //      i = 1;
            i = 0;
            //    in_gg_subは[0]から入れる、以下のconnectInput参照
            for (const auto& tempnode : nowG->input) {
                in_gg[i] = tempnode;
                in_gg_sub[i] = (*net1to2)[tempnode];
                if (! in_gg_sub[i]) {
                    Node * now_g = in_gg[i];
                    Node * now_sub_g = new Node(*(now_g));
                    now_sub_g->input.resize(0);
                    now_sub_g->setType(now_g->getType());
                    SubNet->regNode(now_sub_g);
                    if ( now_sub_g->name() != nil ) {
                        now_sub_g->nameRep = now_g->name();
                    }
                    in_gg_sub[i] = now_sub_g;
                    (*net1to2)[in_gg[i]] = in_gg_sub[i];
                }
                //ここで、IPのうちで (subnet側で★nip==0のもの で かつ
                // list にないもの のみをNotDoneの最後に
                if(in_gg_sub[i]->nIn() == 0){
                    NotDone=NotDone->apendLastIfNot(in_gg[i]);  //なければ最後につける
                }
                ///////////////////////////
                i++;
            }
            //IPとつなぐ

            //nowG no lev2idx が 抜けてるところがあると困るので、-> 1stREADME 参照
            //   nowG->compressFunc();　
            // よく考えてみると、　lev2idxが歯抜でも、lutfがそれに対応しているのなら、
            //   connectInputがちゃんとつないでくれる。
            //  つないでから、必要ならnowGsubで compressFuncすればいい
            //  ★　そうしないと、csetもちゃんとcompressされないし。困ったことになる。

            //      Bdd nodeFunc = nowG->getFunc();
            Bdd nodeFunc = nowG->makeFuncIfNot();  //1997/01/23から
            nowGsub->connectInput(nodeFunc, in_gg_sub, nowG->getLev2idx().data());
            //in_gg_sub は[0]から入っており、その場合、connectInput のlev2idxには  getLev2idx()
            // をそのまま渡しても大丈夫( READMEforNode 参照)
            //★      nowGsub->compressFunc(); //必要かな？　いらんのかもしれん。

        } 
    }
    delete net1to2;
    return SubNet;
}


//SubNet を 埋め込む
// Inputs, Outputs の ノードの順と SubNetの 入出力の順は保存してうめこむ。
// Inputs, Outputs はそのまま（typeやname）その間のnodeはすべてunregして、
//  新しくnodeをつくって挿入する。そのnodeは対応するSubnetのノードと同じ情報をもつ
// ★★ ただし，名前は既にNetにあるなら，変更されて登録される
//Inputs, Outputsの条件としては、createSubNetの場合と同じとする。
//当然、SubNetの入出力数はInputs, Outputsとあってないといけない。
// ●★また、SubNetのOUPUT は 論理をもっていない単なる端子であること
//   -> SubNetのOUTPUT の firstIn()がNet側のOutputsに対応させられる。
// ★ Inputs, Outputs で指定した部分回路が Outputs以外の出力を持つ場合
//   は動作保証しない。

void  Network::insertSubNet( vector<Node*> Inputs,   vector<Node*> Outputs,  Network* SubNet)
{
    //軽くチェック
    assert(Inputs.size() == (unsigned int)SubNet->nPI());
    assert(Outputs.size() == (unsigned int)SubNet->nPO());

    // Cmap<void*,Node*>  *Sub2Net = new Cmap<void*, Node*>(0, ptrHash, 1000); //  todo
    unordered_map<void*,Node*>  *Sub2Net = new unordered_map<void*, Node*>;

    int i;
    // 入力の関係をSub->Net を作っておく
    i=0;
    for (auto sub_netg = ++SubNet->primaryI.begin(); sub_netg != primaryI.end(); sub_netg++) {
        (*Sub2Net)[*sub_netg] = Inputs[i];
        i++;
    }
    // SubNet 側の Node で処理すべきもの（それ自身はすんでるが）
    PList<Node>*  NotDone = nil;
    vector<Node*> inputReserve;

    //NetのOutputsの部分の処理
    i=0;
    for (auto sub_netg = ++SubNet->primaryO.begin();
         sub_netg != SubNet->primaryO.end(); sub_netg++) {
        assert((*sub_netg)->nIn() == 1);
        //    assert(sub_netg->getFunc() == Bdd::var(1));
        assert((*sub_netg)->makeFuncIfNot() == Bdd::var(1));  //1997/01/23から
        Node * nodeP = ((*sub_netg)->input[0]);
        NotDone = NotDone->newPList(nodeP);
        (*Sub2Net)[nodeP] = Outputs[i];
        //Output[i] とその入力をすべてきる
        // その前にこの入力を覚えておいて後でrecLevNouseよぶ
        // 但しこれは、部分回路をはめ込んでからにしないと、先に消すと
        //部分回路以外の部分も消えることある。

        for (const auto& tempnodeip : Outputs[i]->input) {
            inputReserve.push_back(tempnodeip);
        }
        //    Outputs[i]->delAllInput(); //connectInputsでどうせよばれる
        i++;
    }

    // ここで、 inputReserve のをきったらだめ
    PList<Node>*  tempPList;
    Node * nowG;
    Node * nowGsub; //対応するsubNetの方
    vector<Node*> in_gg;    //nowGの入力
    vector<Node*> in_gg_sub;//nowGsubの入力
    while(NotDone != nil){
        nowGsub = NotDone->body;    //先頭から１つとる
        tempPList = NotDone;
        NotDone = NotDone->next;
        tempPList->next = nil;
        delete tempPList;
        // nowG は 既に登録されている。 が、 そのIPはまだかもしれない。
        //少なくとも nowG と そのIPとはつながれていないはず
        if(nowGsub->getType() == INPUT){
            ; //do nothing
        }
        else{
            nowG = (*Sub2Net)[nowGsub];
            assert (nowG );  //nowGsubはまだ未登録 -> おかしい
            //IP を 全て用意
            int numnIn = nowGsub->nIn();
            //      in_gg.clear();   // いらんはず
            //      in_gg_sub.clear();
            in_gg.resize(numnIn);
            //0から fan-in をいれる。以下のconnectInput参照
            in_gg_sub.resize(numnIn);
            // とりあえず、０で初期化しとかなあかんやろ
            for(int j=0;j<numnIn;j++){
                in_gg[j] = 0;
            }

            //      i = 1;
            i = 0;
            for (const auto& tempnode : nowGsub->input) {
                in_gg_sub[i] = tempnode;
                in_gg[i] = (*Sub2Net)[tempnode];
                if (!in_gg[i] ) {
                    Node * now_sub_g = tempnode;
                    Node * now_g = new Node(*(now_sub_g));
                    now_g->input.resize(0);
                    now_g->setType(now_sub_g->getType());
                    this->regNode(now_g);
                    //★★ now_g の名前は，既にthisのnetworkないにあるものではだめ (08・20より)
                    // regNodeは，名前の衝突も解決してくれる．
                    //	  if ( now_sub_g->name() != nil ) {
                    //	    now_g->nameRep = now_sub_g->name();
                    //	  }
                    in_gg[i] = now_g;
                    (*Sub2Net)[in_gg_sub[i]] = in_gg[i];
                }

                //ここで、IPのうちで (net側で★nip==0のもの で かつ
                // list にないもの のみをNotDoneの最後に
                if(in_gg[i]->nIn() == 0){
                    NotDone=NotDone->apendLastIfNot(in_gg_sub[i]);  //なければ最後につける
                }

                i++;
            }
            //IPとつなぐ

            //      Bdd nodeFunc = nowGsub->getFunc(); 
            Bdd nodeFunc = nowGsub->makeFuncIfNot();  //1997/01/23から
            nowG->connectInput(nodeFunc, in_gg, nowGsub->getLev2idx().data());
            //in_gg_sub は[0]から入っており、その場合、connectInput のlev2idxには  getLev2idx()
            // をそのまま渡しても大丈夫( READMEforNode 参照)
            //    nowG->compressFunc(); //いるかな？なくても問題なし。
        }
    }

    // ★ ここで はじめにOutput[i]の入力となってたものに対し、clean処理

    for (int j = inputReserve.size()-1; j>=0; j-- ) inputReserve[j]->recLetNouse();
    //  inputReserve.clear();   いるか？
    //  cerr << "Non node = " << this->nNode() << " **************\n";
    this->clean();
    //  cerr << "Non node = " << this->nNode() << " **************\n";
    //  this->sweepClean();

    delete Sub2Net;
    //  NotDone = nil のはずなので消さんんでいい。
}







vector<Node*>  Network::insertSubNetWithNode( Node* node,  Network* SubNet,
                                              vector<Node*> Correspond)
{
    //軽くチェック
    assert(node->nIn() == SubNet->nPI());
    vector<Node*> Inputs, Outputs;
    for (const auto& no : node->input) Inputs.push_back(no);
    Outputs.push_back(node);
    return this->insertSubNetWithNode(Inputs,  Outputs, SubNet, Correspond);
}


//SubNet を 埋め込む
//insertSubNet と同じだが、それに加えて、
// Correspond[i] で指定されたnode(必ずSubNet内のノードのこと）に対応して
// Networkで新しく作られたノードを return するArrayw<Node*>[i]に入れる。
//  (Correspond[i]にNewSubNetのPOを指定したらだめよん。なぜなら
//    NewSubNetのPOは単なる出力端子でNetwork側にはコピーされないので）

vector<Node*> Network::insertSubNetWithNode(vector<Node*> Inputs,
                                            vector<Node*> Outputs,  Network* SubNet, vector<Node*> Correspond)
{
    //軽くチェック
    assert(Inputs.size() == (unsigned int)SubNet->nPI());
    assert(Outputs.size() == (unsigned int)SubNet->nPO());

    // Cmap<void*,Node*>  *Sub2Net = new Cmap<void*, Node*>(0, ptrHash, 1000); todo
    unordered_map<void*,Node*>  *Sub2Net = new unordered_map<void*, Node*>;

    int i;
    // 入力の関係をSub->Net を作っておく
    i=0;
    for (auto sub_netg = ++SubNet->primaryI.begin(); sub_netg != primaryI.end(); sub_netg++) {
        (*Sub2Net)[*sub_netg] = Inputs[i];
    }
    // SubNet 側の Node で処理すべきもの（それ自身はすんでるが）
    PList<Node>*  NotDone = nil;
    vector<Node*> inputReserve;

    //NetのOutputsの部分の処理
    i=0;
    for (auto sub_netg = ++SubNet->primaryO.begin();
         sub_netg != SubNet->primaryO.end(); sub_netg++) {
        assert((*sub_netg)->nIn() == 1);
        //   assert(sub_netg->getFunc() == Bdd::var(1));
        assert((*sub_netg)->makeFuncIfNot() == Bdd::var(1)); //1997/01/23から
        //  ０９／２９で仕様がかわったよう。出力のlutf = x1 ではなくなった。
        //  というか，SOPタイブが加わって，Bdd::nullになったのだろう．(1997/01/23 判明)
        Node * nodeP = ((*sub_netg)->input[0]);
        NotDone = NotDone->newPList(nodeP);
        (*Sub2Net)[nodeP] = Outputs[i];
        //Output[i] とその入力をすべてきる
        // その前にこの入力を覚えておいて後でrecLevNouseよぶ
        // 但しこれは、部分回路をはめ込んでからにしないと、先に消すと
        //部分回路以外の部分も消えることある。

        for (const auto& tempnodeip : Outputs[i]->input) {
            inputReserve.push_back(tempnodeip);
        }
        //    Outputs[i]->delAllInput(); //connectInputsでどうせよばれる
        i++;
    }

    // ここで、 inputReserve のをきったらだめ
    PList<Node>*  tempPList;
    Node * nowG;
    Node * nowGsub; //対応するsubNetの方
    vector<Node*> in_gg;    //nowGの入力
    vector<Node*> in_gg_sub;//nowGsubの入力
    while(NotDone != nil){
        nowGsub = NotDone->body;    //先頭から１つとる
        tempPList = NotDone;
        NotDone = NotDone->next;
        tempPList->next = nil;
        delete tempPList;
        // nowG は 既に登録されている。 が、 そのIPはまだかもしれない。
        //少なくとも nowG と そのIPとはつながれていないはず
        if(nowGsub->getType() == INPUT){
            ; //do nothing
        }
        else{
            nowG = (*Sub2Net)[nowGsub];
            assert (nowG );  //nowGsubはまだ未登録 -> おかしい
            //IP を 全て用意
            int numnIn = nowGsub->nIn();
            //      in_gg.clear();   // いらんはず
            //      in_gg_sub.clear();
            in_gg.resize(numnIn);
            //0から fan-in をいれる。以下のconnectInput参照
            in_gg_sub.resize(numnIn);
            // とりあえず、０で初期化しとかなあかんやろ
            for(int j=0;j<numnIn;j++){
                in_gg[j] = 0;
            }

            //      i = 1;
            i = 0;
            for (const auto& tempnode : nowGsub->input) {
                in_gg_sub[i] = tempnode;
                in_gg[i] = (*Sub2Net)[tempnode];
                if (!in_gg[i] ) {
                    Node * now_sub_g = tempnode;
                    Node * now_g = new Node(*(now_sub_g));
                    now_g->input.resize(0);
                    now_g->setType(now_sub_g->getType());
                    this->regNode(now_g);
                    //★★ now_g の名前は，既にthisのnetworkないにあるものではだめ (08・20より)
                    // regNodeは，名前の衝突も解決してくれる．
                    //	  if ( now_sub_g->name() != nil ) {
                    //	    now_g->nameRep = now_sub_g->name();
                    //	  }
                    in_gg[i] = now_g;
                    (*Sub2Net)[in_gg_sub[i]] = in_gg[i];
                }

                //ここで、IPのうちで (net側で★nip==0のもの で かつ
                // list にないもの のみをNotDoneの最後に
                if(in_gg[i]->nIn() == 0){
                    NotDone=NotDone->apendLastIfNot(in_gg_sub[i]);  //なければ最後につける
                }
            }
            //IPとつなぐ

            //      Bdd nodeFunc = nowGsub->getFunc();   
            Bdd nodeFunc = nowGsub->makeFuncIfNot();   //1997/01/23から
            nowG->connectInput(nodeFunc, in_gg, nowGsub->getLev2idx().data());
            //in_gg_sub は[0]から入っており、その場合、connectInput のlev2idxには  getLev2idx()
            // をそのまま渡しても大丈夫( READMEforNode 参照)
            //    nowG->compressFunc(); //いるかな？なくても問題なし。
        }
    }

    // ★ ここで はじめにOutput[i]の入力となってたものに対し、clean処理

    for (int j = inputReserve.size()-1; j>=0; j-- ) inputReserve[j]->recLetNouse();  
    //  inputReserve.clear();   いるか？
    //  cerr << "Non node = " << this->nNode() << " **************\n";
    this->clean();
    //  cerr << "Non node = " << this->nNode() << " **************\n";
    //  this->sweepClean();

    //  NotDone = nil のはずなので消さんんでいい。

    // ここまで、insertSubNetと同じ
    int ArraySize = Correspond.size();
    vector<Node*> ReturnArray = vector<Node*>(ArraySize);
    for (int m=0; m<ArraySize; m++ ){
        ReturnArray[m] = (*Sub2Net)[Correspond[m]];
    }

    delete Sub2Net;
    assert(NotDone == nil); //NotDone = nil のはずなので消さんんでいい。
    return ReturnArray;
}








// intNode の中で =0 の部分をつめる。その時 nodeのid も付け直して、
//intNodeの添え字と一致するようにする。 for netview.cc 
void  Network::compressIntNode()
{
    int oldSize = intNode.size();
    /*
     int i, j;	
     //intNode[1]からはいっているはず
     for(i=1, j=1; i< oldSize ; i++){
     if(intNode[i] != 0 ){
     intNode[j] = intNode[i];
     intNode[j]->id = j;
     j++;
     }	
     }
     intNode.reserve(j);
     */

    int newSize =0;
    //intNode[1]からはいっているはず
    int first0 = 0; //0である一番最初のindex はじめつめるべき0がないときはこれも0
    //じつは、first0 と newSize は 1つにまとめれるが....
    for(int i=1;i<oldSize;i++){
        if(first0 == 0) {
            if(intNode[i] == 0){ //つめるべきことがわかった。後はずらしていくみ
                first0=i;
            }
            else{
                newSize++; //そのままコピーしたことにする（そこまでは抜けがなかたった
                intNode[newSize]->id = newSize; // ★id をそろえる場合はこれがいる。
            }
        }
        else{
            if(intNode[i] == 0){ 
                ; //do nothing 
            }
            else{
                intNode[first0] = intNode[i];
                newSize++;
                intNode[first0]->id = newSize; // ★id をそろえる場合はこれがいる。
                first0 ++;

            }
        }
    }
    //newSize はつまりその時までに格納したノード数となる。
    intNode.resize(newSize+1);


}




//Network 内のあるノードに対して、 
// その node の ip を 入力とし、 その node を 出力とする subnetを新た
//につくる。
// ★  具体的には、 INPUT -> 入力用の端子node の ip のノードと同じ名前
// ★               OUTPUT-> 出力用の端子
// ★               IntNode-> nodeのコピーで１つだけ
// という回路ができる。
Network *  Network::createSubNet( Node* node)
{
    vector<Node*> Inputs, Outputs;
    for (const auto& no : node->input) Inputs.push_back(no);
    Outputs.push_back(node);
    Network * createSubNet = this->createSubNet(Inputs, Outputs);
    return createSubNet;	
}







//SubNet を net内の node の部分に 埋め込む 
//★SubNet はいかの条件を満たしていないとだめ
// ●SubNetのOUPUT は 論理をもっていない単なる端子であり、 1出力であること
// ●SubNetの入力数はnodeの入力数と一致してないと当然だめ。
//  （注意）
//  for(g = net->firstN(); g; g= net->nextN() ) 
//    などのなかで、 insertSubNet(g, subnet) などをすると多分うまくいかない場合がある
//    (intNodeを insertSubNetは書き換えるので)


void  Network::insertSubNet( Node* node,  Network* SubNet)
{
    //軽くチェック
    assert(node->nIn() == SubNet->nPI());
    vector<Node*> Inputs, Outputs;
    for (const auto& no : node->input) Inputs.push_back(no);
    Outputs.push_back(node);
    this->insertSubNet(Inputs,  Outputs, SubNet);
}


//////

//結線数を計算して、 num_connect に 設定する。
// また結線数を返す。
// OUTPUT <- OUPUT->firstIn() の間の結線もいれてるので、
// 完全なコストはいえないかもしれない。
// networkのcleanは行われていること
int Network::calcNumConnection()
{
    num_connect = 0;
    for (auto no = ++primaryO.begin(); no != primaryO.end(); no++) {
        num_connect = num_connect + (*no)->nIn(); // g->nIn() == 1 のはずだが
    }
    for (auto no = ++intNode.begin(); no != intNode.end(); no++) {
        num_connect = num_connect + (*no)->nIn();
    }
    return num_connect;
}



/////*********** for debug //////////

void Network::printInfoForDebug(){
    for (auto no = ++primaryI.begin(); no != primaryI.end(); no++) {
        (*no)->printInfo(stderr, 0, 0);
        cerr << "** Node Func Printing **\n";
        (*no)->printIntFunc(stderr, *no);
    }
    for (auto no = ++primaryO.begin(); no != primaryO.end(); no++) {
        (*no)->printInfo(stderr, 0, 0);
        cerr << "** Node Func Printing **\n";
        (*no)->printIntFunc(stderr, *no);
    }
    for (auto no = ++intNode.begin(); no != intNode.end(); no++) {
        (*no)->printInfo(stderr, 0, 0);
        cerr << "** Node Func Printing **\n";
        (*no)->printIntFunc(stderr, *no);
    }
}




// ノード変化指せないバージョン

void Network::printBlifNoChange(FILE *fp)
{
    int i;
    if ( name() != 0 ) fprintf(fp, ".model %s\n", name());

    fprintf(fp, ".inputs");
    for ( i=1; i<=nPI(); i++ ) {
        assert( getPI(i)->getName().length() > 0 );
        getPI(i)->printName(fp);
    }
    fprintf(fp, "\n");

    fprintf(fp, ".outputs");
    for ( i=1; i<=nPO(); i++ ) {
        getPO(i)->printName(fp);     // OUTPUTの名前をそのまま出力
    }
    fprintf(fp, "\n");

    // 内部Node
    for (auto no = ++intNode.begin(); no != intNode.end(); no++) {
        assert((*no)->nOut() >= 1);
        if((*no)->checkBeforeOutput() == true) {
            //★出力の1つ前の場合は特殊にする
            (*no)->printBlif_before_output_no_change(fp);
        }
        else{
            // createNameIfNot(node);
            assert((*no)->getName().length() > 0 );  //nameが定義されてないのはおかしいとする．
            (*no)->printBlif_no_change(fp);
        }
    }
    //★ 入力が出力に直結する場合 (apex7など) に対応する
    for ( i=1; i<=nPI(); i++ ) {
        if(getPI(i)->checkBeforeOutput() == true){
            //★出力の1つ前の場合は特殊にする
            getPI(i)->printBlif_before_output_no_change(fp);
        }
    }
    fprintf(fp, ".end\n");
}

/* dfsorder.cpp */
static int calMaxLevel(Node* ggg)
{
    int maxLevel = gateMaxLevels[ggg];
    if ( maxLevel != 0 ) return maxLevel;    // terminal case

    assert( ggg->getType() != INPUT );
    for (auto&& no : ggg->getInput()) {
        int tmpLevel = calMaxLevel( no );    // recursive
        if ( maxLevel < tmpLevel ) maxLevel = tmpLevel;
    }
    gateMaxLevels[ggg] = maxLevel;
    return maxLevel;
}

void Network::dfsOrder(list<Node*>& vlist)
{
    // calc gateMaxLevels from odepth
    clearOdepthAll();
    calcOdepthAll(); // 回路の各Nodeのレベルなどを調べる．
    gateMaxLevels.clear();
    for (auto no = ++primaryI.begin(); no != primaryI.end(); no++)
        gateMaxLevels[*no] = odepths[*no]; // terminal case
    for ( int i=1; i<=nPO(); i++ ) calMaxLevel( getPO(i) );

    // calc nodeInput from odepth and idepth
    clearIdepthAll();
    calcIdepthAll(); // 回路の各Nodeのレベルなどを調べる．
    local_idepths = &idepths;

    nodeInput.clear();

    // 各Nodeの入力集合をnodeInputに入れてソートする．
    for (auto no = ++intNode.begin(); no != intNode.end(); no++) {
        nodeInput[*no] = new vector<Node*>();
        for (const auto& fin : (*no)->input) nodeInput[*no]->push_back( fin );
        //(nodeInput[node])->sort(maxLevelCmp);
        sort(nodeInput[*no]->begin(), nodeInput[*no]->end(), maxLevelCmp);
    }

    // 各POをpoCollectionに入れてソートする．
    vector<Node*>* poCollection = new vector<Node*>();
    for ( int i=1; i<=nPO(); i++ )
        poCollection->push_back( getPO(i)->input[0] );
    //poCollection->sort(maxLevelCmp);
    sort(poCollection->begin(), poCollection->end(), maxLevelCmp);

    // depth first traversal で順序を求める．結果はvlistに．
    // unordered_map<void*,int> visited(0, ptrHash);
    // Cmap<void*,void*> from(0, ptrHash); // todo
    unordered_map<void*,int> visited;
    unordered_map<void*,void*> from;
    Node* last = 0;

    int n = poCollection->size();
    for ( int i=0; i<n; i++ ) {
        Node* g = (*poCollection)[i];
        df_interleave(g, g, last, vlist, visited, from);
    }

    // nodeInput, poCollection 後片づけ．
    for (auto no = ++intNode.begin(); no != intNode.end(); no++) {
        delete nodeInput[*no];
    }
    delete poCollection;
}

// depth first traversal, interleave, [fujii93] ICCAD'93 pp.38-
static void df_interleave(Node* g, Node* frm, Node*& last,
                          list<Node*>& vlist,
                          unordered_map<void*,int>& visited, unordered_map<void*,void*>& from)
{
    // if ( visited.seek(g) ) {
    if (visited.count(g) > 0) {
        if ( from[g] != frm ) {
            last = g;
            from[g] = frm;
        }
        return;
    }
    visited[g] = 1;
    from[g] = frm;

    vector<Node*>* noinput = nodeInput[g];
    if ( noinput ) {
        int n = noinput->size();
        for ( int i=0; i<n; i++ ) {
            df_interleave( (*noinput)[i], frm, last, vlist, visited, from);
        }
    }

    if ( last == 0 ) {
        vlist.push_front(g);
    }
    else {
        vlist.insert(++find(vlist.begin(), vlist.end(), last), g); // todo: 元insertAfter
    }

    last = g;
}

static int maxLevelCmp(const void* lhs, const void* rhs)
{
    int lhsLevel = gateMaxLevels[ *((Node**)lhs) ];
    int rhsLevel = gateMaxLevels[ *((Node**)rhs) ];

    if ( lhsLevel == rhsLevel ) {
        return - ( (*local_idepths)[ *((Node**)lhs) ] -
                   (*local_idepths)[ *((Node**)rhs) ] );
    }
    else {
        return - (lhsLevel - rhsLevel);
    }
}
/* ---- dfsorder.cpp------ */





/* cspf.cpp */
void Network::propagateCSPF(Node* node) {
    assert(node2cspf.count(node) > 0);
    CSPF cspf = node2cspf[node];
    vector<Node*> nfin = node->getInput();
    //cout << "node type is " << node->getType() << endl;
    //cout << "node id is " << node->getId() << endl;
    
    if (node->getType() == AND) {

        Bdd zero_f = cspf.f0; // 0をまだ割り当てていない位置が1
        for (const auto& fin : node->getInput()) {
            con2cspf[make_pair(fin, node)].f1 = cspf.f1;
        }

        /* todo: nfinをループの順を優先度順に(現在は適当) */

        for (const auto& fin : nfin) {
            con2cspf[make_pair(fin, node)].f0 = zero_f & ~outfuncs[fin];
            zero_f = zero_f & outfuncs[fin];
        }
        assert(zero_f == Bdd::zero);
    }

    if(node->getType() == OR){

      Bdd one_f = cspf.f1;
      for(const auto& fin : node->getInput()){
	con2cspf[make_pair(fin, node)].f0 = cspf.f0;
      }

      for(const auto& fin : nfin) {
	con2cspf[make_pair(fin, node)].f1 = one_f & outfuncs[fin];
	one_f = one_f & outfuncs[fin];
      }
      assert(one_f == Bdd::zero);
    }

    if(node->getType() == NOT){
      for(const auto& fin : nfin){
	con2cspf[make_pair(fin, node)].f1 = cspf.f0;
	con2cspf[make_pair(fin, node)].f0 = cspf.f1;
      }      
    }
}
/* nodeのCSPFを設定 */
void Network::mergeCSPF(Node* node) {
    Bdd f0, f1;
    f0 = f1 = Bdd::zero;
    //cout << "saveCSPF1" << endl;
    //cout << "node name is " << node->getName() << endl;
    for (const auto& fout : node->getOutput()) {
      //cout << "fout name is " << fout->getName() << endl;
      //cout << "loop" << endl ;
      //cout << "count" <<  con2cspf.count(make_pair(node, fout)) << endl;
        assert( con2cspf.count(make_pair(node, fout)) > 0 );
        f0 = f0 | con2cspf[make_pair(node, fout)].f0;
        f1 = f1 | con2cspf[make_pair(node, fout)].f1;
    }
    //cout << "saveCSPF2" << endl;
    node2cspf[node] = CSPF{f0, f1};
}

/* 外部出力nodeのCSPFおよびその入力側結線のCSPFを設定 */
void Network::setCSPF_PO() {
    for (auto no = ++primaryO.begin(); no != primaryO.end(); no++) {
        assert((*no)->nIn() == 1);
        // nodeと結線の両方にCSPFを設定
	for(auto i: (*no)->getInput()){
        node2cspf[*no].f0
            = con2cspf[make_pair(i, *no)].f0
            = ~outfuncs[*no];
	}
	for(auto i: (*no)->getInput()){
        node2cspf[*no].f1
            = con2cspf[make_pair(i, *no)].f1
            = outfuncs[*no];
	}
    }
}

/* nodeのCSPF及びnodeの入力側結線のCSPFを設定  */
void Network::setCSPF(Node* node) {
    if (node2cspf.count(node) > 0) return;
    assert(node->getType() != OUTPUT);

    for (const auto& fout : node->getOutput()) {
        // 出力側の結線のCSPFがセットされていなければ
        // その結線CSPF及び出力側ノードのCSPFを設定
        if ( con2cspf.count(make_pair(node, fout)) == 0 ) setCSPF(fout);
    }
    mergeCSPF(node); // CSPFを設定
    propagateCSPF(node); // 入力側結線のCSPFを設定
}

void Network::clearCSPF() {
    if (node2cspf.empty() != false) node2cspf.clear();
    if (con2cspf.empty() != false) con2cspf.clear();
}

/* 回路内の全てのnodeのCSPFを設定 */
void Network::setCSPF_All() {
    clearCSPF();
    setCSPF_PO();
    for (auto no = ++primaryI.begin(); no != primaryI.end(); no++) setCSPF(*no);
}
/* --- cspf.cpp --- */
