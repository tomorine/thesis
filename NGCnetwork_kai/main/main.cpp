#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <unistd.h>

#include "network.h"
#include "node.h"

using namespace std;

#define DBG
#ifdef DBG
#define DEBUG(statement) statement
#else
#define DEBUG(statement)
#endif


Network* readFileName(char * filename);

Bdd GetMaj(Node* node, int i);

Network* makeNetwork(Bdd func);
  
int main(int argc, char **argv)
{
    if (argc != 2) {
      std::cerr << "Usage diffnet <file1> \n";
        exit(1);
    }
      
    int maxbdd_power = 20;
    // int match_n = 0;
    Bdd::alloc(16, maxbdd_power);
    Network* circ1 = readFileName(argv[1]);

    //int in_format = ( MyString(argv[1]).contains(".pla") ) ? 0 : 1;
    bool is_pla = std::string(argv[1]).find("pla") != std::string::npos;
    bool in_format = ( is_pla ) ? false : true;
    if ( in_format == false ) {
      circ1->calAllSpecPla('0');
    }
    else{
       circ1->calAllSpecBlif('0');
    }
    
    //circ1->setCSPF_All(); //CSPFを設定
      
    //vector<Node*>& list = circ1->getnodelist();//こいつが怪しい
    //assert(!list.empty());
   
    //unordered_map<Node*, Bdd> funclist =  makeFuncIfNot_All(list);
    //unordered_map<Node*, Bdd> funclist;
    //assert(!funclist.empty());
    vector<Node*> nodelist = circ1->getnodelist();
    assert(nodelist.size() != 0 );
    Bdd func_n = nodelist[355]->makeFuncIfNot();
    cout << "node name is " << nodelist[355]->getName() <<endl; 
    
    Network* circ2 = makeNetwork(func_n);

    cout << "---------sub network----------- " << endl <<endl;
    

    vector<Node*> sublist = circ2->getPIlist();

    unordered_map<void*, Bdd> outfuncs_c1 = circ1->getOutfunc();
    int count = 1;
    for(auto i: nodelist[355]->getInput()){
      cout << " x_" << count << " is " << i->getName() << endl; 
      circ2->setOutFuncIm(sublist[count], outfuncs_c1[i]);
      ++count;
    }

    cout << "number of outfuncs is" << circ2->getOutfunc().size() << endl;

    cout << "~~input~~" << endl;
    assert(sublist.size() != 0);
    cout << "num  : " << sublist.size()-1 << endl;
    for (auto i: sublist){
      if(i){
      cout << "name : " << i->getName() << " : type : " << i->getType() <<  endl;
      }
    }

    vector<Node*> sublist2 = circ2->getnodelist();
    cout << "~~intnode~~" << endl;
    assert(sublist2.size() != 0);
    cout << "num  : " << sublist2.size() -1<< endl;
    for (auto i: sublist2){
      if(i){
        cout << "name : " << i->getName() << " : type : " << i->getType() << " : fanin : " ;
        for(auto j: i->getInput()){
          cout << j->getName() << " "; 
        }
        cout <<  endl;
      }
    }
    
    vector<Node*> sublist3 = circ2->getPOlist();
    cout << "~~output~~" << endl;
    assert(sublist3.size() != 0);
    cout << "num  : " << sublist3.size() -1<< endl;
    for (auto i: sublist3){
      if(i){
        cout << "name : " << i->getName() << " : type : " << i->getType() << " : fanin : " ;
        for(auto j: i->getInput()){
          cout << j->getName() << " "; 
        }
        cout <<  endl;
        circ2->calcOutFuncIm_All(i);
      }
    }

    if(outfuncs_c1[nodelist[355]] == circ2->getOutfuncidx(sublist3[1])){
      cout << "success!!" << endl;
    }else{
      cout << "failed.." <<endl;
    }

  return 0;
}

  

Bdd GetMaj(Node* node, int i)
{
  cerr << node->getName() << endl;
  vector<Node*>& v = node->getInput();
  
  cerr << "save3.6loop" << endl;
  if(!v.empty()){
    cerr << "save3.6loop" << endl;
    Bdd b1 = v[0]->getFunc();
    Bdd b2 = v[1]->getFunc();
    Bdd b3 = v[2]->getFunc();

    cerr << "save3.7loop" << endl;

    switch(i){
    case 1  : return b2  * b3;
    case 2  : return b2  * ~b3;
    case 3  : return ~b2 * b3;
    case 4  : return ~b2 * ~b3;
    case 5  : return b1  * b3;
    case 6  : return b1  * ~b3;
    case 7  : return b1  | b3;
    case 8  : return b1  | ~b3;
    case 9  : return b1  * b2;
    case 10 : return b1  | b2;
    case 11 : return (b1*b2)|(b1*b3)|(b2*b3);
    case 12 : return (b1*b2)|(b1*~b3)|(b2*~b3);
    case 13 : return b1  * ~b2;
    case 14 : return b1  | ~b2;
    case 15 : return (b1*~b2)|(~b2*b3)|(~b2*b3);
    case 16 : return (b1*~b2)|(b1*~b3)|(~b2*~b3) ;
    case 17 : return ~b1 * b3;
    case 18 : return ~b1 * ~b3;
    case 19 : return ~b1 | b3;
    case 20 : return ~b1 | ~b3;
    case 21 : return ~b1 * b2;
    case 22 : return ~b1 | b2;
    case 23 : return (~b1*b2)|(~b1*b3)|(b2*b3);
    case 24 : return (~b1*b2)|(~b1*~b3)|(b2*~b3);
    case 25 : return ~b1 * ~b2;
    case 26 : return ~b1 | ~b2;
    case 27 : return (~b1*~b2)|(~b1*b3)|(~b2*b3);
    case 0  : return (~b1*~b2)|(~b1*b3)|(~b2*~b3);
    default : cout << "error" << endl;
    }
  }
  exit(1);
}   

Network * readFileName(char * filename)
{
    Network* net = new Network();
    net->readFileName(filename);
    return net;
}

Network* makeNetwork(Bdd func)
{
    Network* circ = new Network();
    vector<Node*> intnode;   //[0]=null; [1]~[3]=input; [4]=output; [5]~[n]=AND; [n+1]=OR
    intnode.push_back(0);    

    for(int i=1; i<=3; ++i ){       //入力は固定個なので
      Node* tmp = new Node(INPUT);
      intnode.push_back(tmp);
        
      string str = "x_" ;
      string num = to_string(i);  
      str = str + num;
      tmp->changeName(str);      
      circ->getNodeInf()->insert(make_pair(str, tmp));
      //circ->getNodeInf()[str].changeName(str);
    }

    for(int i=1; i<=1; ++i){        //出力は固定個なので
      Node* tmp = new Node(OUTPUT);
      intnode.push_back(tmp);

      string str = "f_" ;
      string num = to_string(i);  
      str = str + num;
      tmp->changeName(str);
      circ->getNodeInf()->insert(make_pair(str, tmp));
    }

    vector<bool> n_frg;
    circ->bddChangeNet(func,intnode,n_frg);   //OR以外のnodeを作る

    Node* tmp = new Node(OR);
    for(unsigned int i = 5; i<intnode.size(); ++i){
      if(intnode[i]->getType() != NOT){
      tmp -> connect(intnode[i]);
      }
    }
    intnode[4]->connect(tmp);    //output
    intnode.push_back(tmp);

    int f = 0;  
    for(auto i: intnode){
        if(i && i->getType() == INPUT){ 
            circ->regPI(i);
        }else if(i && i->getType() == OUTPUT){
            circ->regPO(i);
        }else if(i){
          circ->regNode(i);
          ++f;
        }
    }
  return circ;
      
}
  
  //todo: node::func -> network::outfunc

/*vector<Node*> test_node;   //1から
  test_node.push_back(0);
   cout << "save2" << endl;

  for(int i=0; i<3; ++i ){
      Node* tmp = new Node(INPUT);
      test_node.push_back(tmp);    
    }

    for(int i=0; i<2; ++i){
      Node* tmp = new Node(AND);
      test_node.push_back(tmp);
    }

    for(int i=0; i<1; ++i){
      Node* tmp = new Node(OUTPUT);
      test_node.push_back(tmp);
    }regPI
    cout << "print test_node num :" << test_node.size() <<  endl;

    test_node[4]->addInput(test_node[1]);
    cout << "save3" << endl;
    test_node[4]->addInput(test_node[2]);
    test_node[5]->addInput(test_node[2]);
    test_node[5]->addInput(test_node[3]);
    test_node[6]->addInput(test_node[4]);
    test_node[6]->addInput(test_node[5]);
      cout << test_node[4]->nIn() << endl; 
  int f=0;
  for(auto i: test_node){
    if(i){
      string str = "node" ;
      string num = to_string(f);  
      str = str + num;
      char word[5];
      str.copy(word, str.size());
      cout << "node type is" << i->getType() << endl;
      circ2->newGateName(word, i->getType());
      ++f;
    }
  }
  cout << "outfuncs size is " << circ2->getOutfunc().size() <<endl;
*/