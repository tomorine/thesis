

template <class T>
class PList{
public:
//  static int objnum;     $B%G%P%C%0MQ(B
    PList<T>*	next;
    T*          body;
//    		PList() : next(nil), body(nil){/*objnum++;*/};
    		PList(PList<T>* n, T* b) : next(n), body(b){/*objnum++;*/};
    		PList(T* b) : next(nil),body(b){/*objnum++;*/};
                ~PList(){clear();/*objnum--;*/}; 

    void	clear();
    PList<T>*    newPList(T* item){
	                   PList<T>* returnPList = new PList(this,item);
	                   return returnPList;
                 };
// $B3:Ev$9$k(Bitem$B$r=|5n$7!"$=$N%j%9%H$rJV$9!#(B
// $B3:Ev$9$k$b$N$,$J$1$l$P!"$($i!<$rI=<($7$F!"(Bthis$B$rJV$9!#(B
    PList<T>*      PListRemove(T* item);
    bool       includeKa(T* item);  //was not declared in   // $B4^$^$l$F$?$i(Btrue
    PList<T>*       apendLastIfNot(T* item);   //$B4^$^$l$F$$$J$+$C$?$i!":G8e$K$D$1$k!#(B
    PList<T>*       reverse();
    PList<T>*       apendPList(PList<T>*  apend); 
          //apend $B$r:G8e$K$/$C$D$1$k!#$3$l$O!"%]%$%s%?$r$/$C$D$1$F$k$@$1(B
};



/*
template <class T>
inline void PList<T>::print(FILE *fp)
{
    PList<T>*  temp;
    for(temp=this;temp != nil;temp=temp->next){
	temp->body->print(fp);
    }
}
*/


template <class T>
inline PList<T>* PList<T>::reverse()
{
    PList<T>* temp = this;
    PList<T>* temppp;
    PList<T>* newPList = nil;
    while (temp != nil  ) {
        temppp = newPList->newPList(temp->body);
	newPList = temppp;
	temp = temp->next;
    }
    delete this;    
  //this$B$NCM$O$+$o$i$i$J$$!#$^$?!"(Bdelete$B$5$l$F$F$b%"%/%;%92DG=$J>l9g$*$*$$!#(B
    return newPList;
}


// body $B$N(B $B;X$9%*%V%8%'%/%H<+BN$N(Bdelete $B$O$7$F$J$$!#(B
template <class T>
inline void PList<T>::clear()
{
    if(this == nil)return;
    if(next == nil) return;     
    else{
	delete next;
    }

}



// $B3:Ev$9$k(Bitem$B$r=|5n$7!"$=$N%j%9%H$rJV$9!#(B
// $B3:Ev$9$k$b$N$,$J$1$l$P!"$($i!<$rI=<($7$F!"(Bthis$B$rJV$9!#(B
template <class T>
inline PList<T>*  PList<T>::PListRemove(T* item){
    PList<T>*  temp;
    PList<T>*  backuptemp;
/*    if(this == nil){         nil$B$N%(%i!<=hM}$J$7(B
//	cerr << "PList removing Error -- nil PList \n";
	return this;
    }
*/
    if(this->body == item){
temp = this->next;

       this->next = nil;
       delete this;  

	return temp;
    }
    for(temp=this;temp->next != nil;temp=temp->next){
	if(temp->next->body == item){
	    backuptemp = temp->next;
	    temp->next = temp->next->next;

            backuptemp->next = nil;
	    delete backuptemp;

	    return this;
	}
    } 
//    cerr << "PList removing Error -- Item is not found \n";
    return this;
}

// PList $B$K(B item $B$,4^$^$l$F$$$?$i(B true$B$rJV$9!#(B
template <class T>
inline bool  PList<T>::includeKa(T* item){
    PList<T>*  temp;
    for(temp=this;temp != nil;temp=temp->next){
	if(temp->body == item){
	    return true;
	}
    }
    return false;
}


//$B4^$^$l$F$$$J$+$C$?$i!":G8e$K$D$1$k!#(B
// $B$=$7$F$=$N%j%9%H$rJV$9!#(B
template <class T>
PList<T>* PList<T>:: apendLastIfNot(T* item)
{
  if(this==nil){
    return new PList(item);    //$B$&$^$/$$$/$J$+!)(B $B2<$N%F%9%H$G$*$C$1$$(B
  }
  PList<T>*  temp;
  for(temp=this;temp->next != nil;temp=temp->next){
    if(temp->body == item){
      return this;          //$B$"$k$N$G$J$K$b$7$J$$!#(B
    }
  }
  // $B$3$N;~$N(Btemp $B$O(B next=nil $B$@$,(B body$B$O2?$+F~$C$F$F$=$N%A%'%C%/$O$7$F$J$$!#(B
  if(temp->body == item){
    return this;          //$B$"$k$N$G$J$K$b$7$J$$!#(B
  }
  PList<T>*  newlplist = new PList(item);
  temp->next = newlplist;
  return this;
}

/* $B%F%9%H%W%m%0%i%`(B
#include "ger-util.h"
int main(int argc, char **argv)
{
  PList<int>*  NotDone = nil;
  int a1 =1;
  int a2 =2;
  int a3 =3;
  int a4 =1;
  NotDone = NotDone->newPList(&a1);
  NotDone = NotDone->newPList(&a2);
  NotDone = NotDone->apendLastIfNot(&a3);
  NotDone = NotDone->apendLastIfNot(&a3);
  NotDone = NotDone->apendLastIfNot(&a4);
  NotDone = NotDone->apendLastIfNot(&a3);
  NotDone = NotDone->apendLastIfNot(&a3);
  NotDone = NotDone->apendLastIfNot(&a1);
  NotDone = NotDone->apendLastIfNot(&a2);
  NotDone = NotDone->apendLastIfNot(&a4);
  PList<int>*  temp;
  for(temp=NotDone;temp!=nil;temp=temp->next){
//    cerr << *(temp->body) << "\n";
  }

}
 */





template <class T>
inline PList<T>* PList<T>::apendPList(PList<T>*  apend)
{
    register PList<T>*  temp;
    if(this==nil)return apend;
    for(temp=this;temp->next != nil;temp=temp->next){
      ;
    } 
    temp->next = apend;
    return this;

}

