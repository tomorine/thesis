#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstdlib>


//$B$3$l$O!"(BISF $B$N!!FbItE*$K;H$&$b$N!!!J8E$$<B8=$G?7$7$$<B8=$NJ}$r$3$l$+$i;H$*$&(B(06/31)
//$B!v!v!v!v!v!v!v!v!v(B $BHs>iD9@QOB7A$NI=8=$9$k$?$a$N%/%i%9(B
// $BHs>iD9(B $B@QOB7A(B  Vi(f1) + ~Vi(f0) + fr 
//**********************************
// $B!v!v!v!JCm0U;v9`(B)
/*

print_for_debug$B$GI=<($G$-$k!#(B

ISOP              i         f0                   f1               fr
Bdd::var(3)      2        BDDF                 BDDT              BDDF
BDDF            -2          0                    0                0
BDDT            -1          0                    0                0

$BAH$_9g$o$;$N>l9g(B

ISOP = x4 ~x2 $B$J$i(B

ISOP           3          ISOP0                ISOP1           ISOPR
ISOP0         -2            0                    0               0   -> $BMWCm0U(B
ISOPR         -2            0                    0               0
ISOP1          2          ISOP10               ISOP11           ISOP1R

ISOP10        -2            0                    0               0
ISOP11        -2            0                    0               0   
ISOP1R         1         ISOP110               ISOP111           ISOP11R 
                         ( i= -1)              ( i= -2)          ( i= -2) 
                                    --> $B$J$$>l9g$O(B -2 

$B!z(B $BI,$:$7$b(B i $B$O(B 4, 3, 2, 1 $B$H8:$C$F$$$/$H$O8B$i$J$$(B($BH4$1$F$k>l9g$"$j(B)
 -> makeISOP$B$O(Btop$B$NJQ?t$GE83+$7$F$=$l$r!"(Bi $B$H$7$F$k$N$G!#(B
  $B$?$^$K0MB8$7$F$J$$ItJ,$,$"$l$P(B i $B$NHV9f$O$OH4$1$K$J$k(B
 // $B$?$@$7!"(B f0, f1 $B$,(B $B$H$b$K(B BDDF $B$r$5$9$h$&$J(B ISOP $B$O(B $B:n$i$:$K$=$N>l9g$O!"(B
 //  $B$=$N(BISOP$B$N(B fr $B<+?H$rJV$9$h$&$K(B makeISOP$B$rJQ99$7$?(B(1997/05/21)
 */

class ISOP{
private:
    short int    i;          // $BJQ?tHV9f(B (0$B$+$i$J$N$G!"(B(BDD$B$N(Blev -1) )  
    ISOP* f0;               // $BJQ?tHV9f(B -1 $B$O(B $B91??4X?t$rI=$9!#(B
                             // $BJQ?tHV9f(B -2 $B$O(B $B91564X?t$rI=$9!#(B
    ISOP* f1;               //f0, f1, fr$B$,$J$$>l9g$O(B = 0 (nil)$B$N$O$:(B
    ISOP* fr;
//    ISOP(){std::cerr<<"ISOP Error\n";exit(1);}        // $B0z?t$J$7$O%(%i!<(B
    ISOP(){}        // $B0z?t$J$7$O%(%i!<(B
    static int objnum;
    int  TrueOrFalse();       // $B91??(B $B!<(B $B#1(B $B9156(B $B!<(B 0 $B$=$NB>(B 2 $B$rJV$9!#(B
public:
  ISOP(int j);
  ~ISOP();
  ISOP * F0() { return f0;};
  ISOP * F1() { return f1;};
  ISOP * FR() { return fr;};
  short int Number(){return i;};
  void print(int flag);         // $BI=<((B flag=0 $B$O3g8LHsI=<((B
  Bdd makeLF(); // ISOP $B$N(B $BI=8=$9$k(B $BO@M}4X?t$rI=$9(BBdd$B$r$D$/$C$FJV$9!#(B
  void print_for_debug();   //debug $B$N(B $B$?$a!"(B
friend ISOP* ISF::makeISOP();
};


//int ISOP::objnum=0;

inline int ISOP::TrueOrFalse () {
    if(i == -1) return 1;
    else if(i == -2 ) return 0;
    else return 2;
}

