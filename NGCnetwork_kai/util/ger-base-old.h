
#ifndef _ger_base_old_h
#define _ger_base_old_h


//cubeBdd $B$K4^$^$l$k%j%F%i%k$9$Y$F$K$D$$$F(Bsmooth$B$7$?$N$rJV$9!#(B
// cubeBdd $B$O(B $B@5%j%F%i%k$N@Q$G$J$$$H$@$a(B
Bdd  smooth_cube_all(Bdd F, Bdd cubeBdd);




//////////////// $BIT40A4$7$F$$O@M}4X?t$r#2%S%C%H$N(BBdd$B$GI=$9$/$i$9!#(B

class ISF; 
class ISOP; 

/////// $B!z!z(B makeISOP$B4XO"$r4^$aA4$F(BObsolete IsfLU $BEy$r$D$+$&$Y$7!%(B



static ISF      operator & (const ISF lhs, const ISF rhs);
static int      operator == (const ISF lhs, const ISF rhs);
static int      operator != (const ISF lhs, const ISF rhs);
static int      operator <= (const ISF lhs, const ISF rhs);


//    ISF $B$N%3!<%G%#%s%0(B
// F           low   up
// 0 ------>    0     0
// 1 ------>    1     1
// * ----->     0     1  



class ISF{
private:
    Bdd       low;     // lower bound
    Bdd       up;      // upper bound
public:
//  Sop makeISOPnew(int* lev2idx=0);  -> ResultPair dft $B$D$+$o$s$h$&$K(B
//                                       $B$b$&;H$&$N$d$a$?!%(B
//      -> $B<BBN$O!$(BMakeIsopHash.h$B$KJ]B8$7$F$"$k!%(B


  Sop makeISOPnewNoHash(int* lev2idx=0);  //Hash $B$J$7(B
    inline ISF();
    inline ISF(const ISF& oprand);
    inline ISF(Bdd lf);
    inline ISF(Bdd f0, Bdd f1);
  inline Bdd Low(){return low;};
  inline Bdd Up(){return up;};
//    ~ISF();                   // $B<+F0E*$K(Bup,low$B$N%G%9%H%i%/%?!<$h$V$O$:(B
    ISF     operator = (const ISF oprand);
    ISF     operator ~ () const;
        //ISF$B$NH]Dj$r:n$C$FJV$9(B 0->1, 1->0. *->* $B$HJQ2=$5$;$k!#(B
    ISOP*  makeISOP();
    int    check();    // $B$3$N(BISF $B$,(B $B$*$+$7$+$C$?$i(B $B#0$rJV$9!#(B($B$=$l0J30(B1)
    ISF     rstr0(const int lev) const;
    ISF     rstr1(const int lev) const;
    int     AllTrue();                        // $B91??4X?t$r4^$a$P#1(B
    int     AllFalse();
    ISF    makeNECK(int i);
    void   print(FILE *sout,int n_in);   //Truth Table$B$G(B         
    void   print();                      //$B@QOB7A$G(B
    Bdd    pf1() const;
    Bdd    pf0() const;
    Bdd    pfStar() const;
    Bdd    pfSpecified();

    int    IsSatisfy(Bdd lf);
      // lf $B$,!"$3$N(BISF$B$K4^$^$l$k$J$i(B 1,
      // $B4^$^$l$J$$$J$i!!(B0$B$rJV$9!#(B
      // $B4^$^$l$k$H$O!"(Blow <= lf <= up $B$rK~$?$9$H$$$&$3$H$G$"$k!#(B

    int    topVarId() const;
      // low up $B$N%0%i%U$N:G>e0L$NJQ?t%N!<%I$N%0%i%U$G>e$K$"$kJ}$NJQ?tHV9f(B
      // $B!J_7ED$5$s$J$i(Blev$B!"(Bgeert$B$J$i(Bvarid)
    void   maskISF(Bdd* f);         //bdd $B$N$H$3$m$r(B*$B$K$9$k(B
    void  specify(Bdd* lf, Bdd* specify);
    //specify = 1 $B$H(B $B$J$kItJ,$r(B lf$B$G$*$-$+$($k!#!J!v$r69$a$k!K(B
friend ISF      operator & (const ISF lhs, const ISF rhs);
inline friend int       operator == (const ISF lhs, const ISF rhs);
inline friend int       operator != (const ISF lhs, const ISF rhs);
inline friend int        operator <= (const ISF lhs, const ISF rhs);
friend void printISOP(Bdd lf);
friend void supportMain(int sup, ISF *f,int* varTable, int Nsup, int elim, 
		 ISF* minf,int* minNsup,int* varResult, int* Usedflag);


friend ISF support_min(ISF* f,int sup,int* varTable,int* nsup, int* varResult);
// $BIT40A4;XDjO@M}4X?t(B f $B$N(B support$B?t:G>.$NIT40A4;XDjO@M}4X?t$rJV$9!#(B
// sup   --- $B:GDc$G$b4X78$7$F$$$kJQ?t$N?t(B
// varTable -- $B:GDc$G$b4X78$7$F$$$kJQ?tHV9f!J(BBDD$B$NHV9f$J$N$G!"#1$+$i!K(B
// varTable[0]-varTable[sup-1] $B$NG[Ns(B
// nsup -- $B:G>.$N(Bsupport$B?t(B
// varResult[i] = 0 $B$J$i(B varTable[i] $B$NJQ?t$O;HMQ$5$l$k!#(B
// varResult[i] = 1 $B$J$i(B varTable[i] $B$NJQ?t$O;HMQ$5$l$J$$!#(B
// $B$3$N!"#0!"#1$b@_Dj$9$k!#(B
// varTable[sup-1] $B$+$i(B $B=|$1$k$+%A%'%C%/$7$F$$$/$N$G!"(BBDD$B$NJQ?t$,>:=g$K$3$N(B
// $BG[Ns$KF~$C$F$$$kJ}$,=hM};~4V$,Aa$/$J$k$3$H$,4|BT$5$l$k(B


};


//#include "ISF.h"
#include "IsfGer.h"
#include "OldISOP.h"

////////////////////  // PList $B4X78(B   --- $B%]%$%s%?$N%j%9%H!!(B
/*
template <class T>
class PList{
public:
    PList*	next;
    T*          body;

}
$B!v!v!v!v!v!v!vCm0U(B
$B$"$/$^$G$b!"%]%$%s%?$N%j%9%H$G$"$k$+$i!"(BPList$B$N%G%9%H%i%/%?$O(Bbody$B<+?H$O(B
$B>C$5$J$$!#$=$l$b>C$7$FM_$7$$>l9g$O!"@h$K(Bbody$B$r>C$7$F$+$i!"(BPList$B$r>C$5$M(B
$B$P$J$i$J$$!#!JNc$($P!"(BLut->logic$B$J$I!"(B~Lut()$B$r$_$h!K(B

$B%G%9%H%i%/%?(B $B$O(Bclear$B$,$h$P$l$k!#$=$N$?$a!"%j%9%HA4$F$,>C$5$l$k!#(B

$B%3%s%9%H%i%/%?(B
  //		PList() : next(nil), body(nil){}; -> $B<Y0-$J$N$G$d$a(B
    		PList(PList<T>* n, T* b) : next(n), body(b){};
    		PList(T* b) : next(nil),body(b){};
                ~PList(){};

$B%j%9%H$N(Bdelete($B%]%$%s%?$N$_!K!#%j%9%H$?$I$C$F!"$9$Y$F>C$9!#(B
body $B$N(B $B;X$9%*%V%8%'%/%H<+BN$N(Bdelete $B$O$7$F$J$$!#(B
    void	clear();

$B%N!<%I(Bitem$B$r%j%9%H$N:G=i$K$K$D$1!"$=$N?7$7$$%j%9%H$rJV$9!#(B
    PList*    newPList(T* item);

 PList<T>* reverse()
 $B%j%9%H$rF~$l49$($?$b$N$rJV$9!#85$N%j%9%H$O$A$c$s$H%]%"$9$k!#(B
 //$B$?$@$7!"(Bthis$B$O(Bnil$B$K$O$J$i$:!"85$N$^$^!#(Bthis$B$K$3$N4X?t$NJV$jCM$rBeF~(B
$B$7$J$$$H$-$O!"MWCm0U!#(B

  PList<T>*      PListRemove(T* item);
// $B3:Ev$9$k(Bitem$B$r=|5n$7!"$=$N%j%9%H$rJV$9!#(B
// $B3:Ev$9$k$b$N$,$J$1$l$P!"$($i!<$rI=<($7$F!"(Bthis$B$rJV$9!#(B
$B!v!v!v(B nil PList $B$K$?$$$7$F$O%(%i!<$r5/$3$9!#!J%(%i!<=hM}$J$7!K(B


bool       includeKa(T* item);   // $B4^$^$l$F$?$i(Btrue
// nil PList $B$J$i(B false$B$rJV$9$@$1$@$,!"(Bitem =nil $B$J$i!"%(%i!<$H$J$k!#(B


   PList<T>*       apendPList(PList<T>*  apend); 
          //apend $B$r:G8e$K$/$C$D$1$k!#$3$l$O!"%]%$%s%?$r$/$C$D$1$F$k$@$1(B



    PList<T>*  apendLastIfNot(T* item);   //$B4^$^$l$F$$$J$+$C$?$i!":G8e$K$D$1$k!#(B
	       //$B$=$7$F$=$N%j%9%H$rJV$9!#(B
           aholist = aholist->apendLast(boke) $B$N$h$&$K;H$&$Y$7!#(B
      //(aholist = nil $B$N>l9g$b$&$^$/9T$/$h$&$A$c$s$H8F$S=P$7B&$GBeF~$9$Y$7(B)
*/

#include "PList.h"

//template class PList<Node>;

/*   makdISOP $B$O$b$&(B obsolete -> IsfLU->makeISOP$B!!$D$+$&$Y$7(B
#include "MakeIsopHash.h"

extern void ClearMakeIsopHash();
// makeISOP$B$G$D$+$C$F$$$k(BHashTable $B$N!!(Bentry$B$rA4$F6u$K$9$k!#(B
// $B$3$l$r$7$J$$$H!"<B:]$K$OI,MW$J$$$N$K!"(BmakeISOP$B$NESCf$G$D$+$o$l(Bhash$B$5$l$F$$$k(B
// Bdd $B$,!!(Bfree$B$5$l$J$$!#(B
*/

#endif
