#ifndef _ger_base_h
#define _ger_base_h

//#include <string.h>
//#include <string>
//#include <stdio.h>
//#include <stdlib.h>
//#include <fstream.h>

#include "bddop.h"
#include "twobdd.h"
#include "Array.H"
#include "arrayint.h"
#include "sop.h"
#include "cube.h"

#include "SupVars.h"


//$B4X?t$NI=<($dA`:n$J$I(B libnetwork.a $B$h$j@h$K%j%s%/$7$J$$$H$@$a$J$N$b$"$k!#(B
#ifndef nil
#define nil 0
#endif


extern int      word_len(const char* c);
// $B%9%Z!<%9$^$G$NJ8;z?t$r$+$($9(B
// 1$BJ8;zL\$O6uGr$G$J$$$3$H(B


//SupVars $B$K4^$^$l$k%j%F%i%k$9$Y$F$K$D$$$F(Bsmooth$B$7$?$N$rJV$9!#(B
// $B!z(B 1998 07 22 $B$h$j(B smooth_cube_all $B$K(B $B<h$C$FBe$o$C$?(B.
Bdd  smooth_all(Bdd& F,  SupVars & sup_vars);



//TwoBdd $B$r(B $B??M}CMI=$GI=<((B (8$BF~NO0J2<$N$_$r07$&!%(B)
//string $B$NA0$+$i!"(B2$B$D$:$D$NJ8;z$r!"(B(bit1,2) = (00, 01, 10. 11)$B$KBP1~$5$;$k(B
//$BNc(B string = " 0*0 1*1" 
//extern void Print_Karnaugh_Map(TwoBdd& twobits, FILE *sout, int n_in, char * string);
//   -> isf-ger.cc $B$K$H$j$"$($:0\F0(B

extern void printArrayInt(Arrayint Array);


/// ZBDD $B4X78(B

const int Digit = 3; //zbdd$B$NI=<(;~$K!"2>Dj$7$F$$$kJQ?tHV9f$N:GBg7e?t!#(Bx1-x999$B$^$G2DG=(B

// ZBDD$B$N>pJs$NI=<((B - $B=89g$NMWAG$J$I(B (for Debug)
extern void printZBDDinfo(Bdd& Zbdd);

// ZBDD$B$N=89g$NMWAG$rNs5s$7$FI=<((B 
extern void printZBDD(FILE * sout, Bdd& Zbdd);


///// $B!z!!4X?t$NI=<(4X78!!(B(netview $BEy$G$D$+$C$F$k$b$N$"$j!K(B

// Sop$B$N(BBdd$B$rOB@Q7A$G(Bstderr $B$K(B $BI=<((B $B#29`0J>e$"$l$P(B()$B$r$D$1$k!#(B
// $B91??!"91564X?t$G$O2?$b$7$J$$!#(B
// flag = 0 $B$N;~$O(B $B3g8L$r$D$1$J$$!#(B
// $B:G=i$K8F$V$H$-$O!"(Bflag = 0$B!!$G8F$V(B
extern void  sop_print(FILE * sout, Sop  sop, int flag);


// var2idx: Bdd$B$N(Bvar(1$B$+$i;O$^$k(B)$B$+$i!$I=<(;~$N(Bidx(0$B$+$i;O$^$k(B)$B$X$N<LA|(B
//   (CubeSet::printSop $B$HF1$8!K(B
// var2idx $B$K$O!"!!(Bnet->getLevel2indexFrom0().getPtr() $B$r8F$Y$P$$$$$O$:(B
//  
extern void BddPrintPLA(FILE * sout, const Bdd& lf, int* var2idx=0);

extern void Bdd_printTable_normal(FILE *sout, int n_in, const Bdd F);
//***************
//n_in $B$O(BBdd$BA4BN$NJQ?t$N?t$r2>Dj$7$F$$$k!#$D$^$j0MB8$7$F$$$J$$JQ?t$b9MN8$K$$$l$F$$$k!#(B
//$B!z(Bn_in$B$K$=$l0J30$N?t$rEO$7$F$b0UL#$N$"$k7k2L$rI=<($9$k$3$H$K$O$J$i$J$$!#(B
//$B$3$3$GI=<($5$l$k!!(Bxi $B$NHV9f$O(B 1$B$+$i$G!"(B Bdd$B$NJQ?tHV9f(B(lev)$B$H0lCW(B
//***************

extern void Bdd_printTable_gray(FILE *sout, int n_in, const Bdd F);
//***************
//printTable_normal $B$N%0%l%$%3!<%I$GI=<($9$k$b$N(B
//***************

// lf $B$rHs>iD9@QOB7A$GI=<((B (ISOP$B$O?7$7$$<B8=$KJQ$($?!!!J#0#7!?#0#1!K(B
extern void printISOP(Bdd lf);
// f0 + f1 + fr $B$N=g$GI=<((B

extern void Bdd_print_SOP(FILE *sout, Bdd F);
//***************  $BE83+=g$O!"(B rstr1, rstr0$B$N=g(B(
//SOP($B@QOB7A(B)$B$H$7$F4X?t$rI=<((B
// $BE83+$O(BBdd $B$N(BTop$B$NJQ?t$+$i=g$K9T$&!#(B
// $BITI,MW$J3g8L$r$D$1$J$$$h$&$K$7$F$$$k(B
// $BE83+;~$K$o$+$k>iD9$JI=8=$O$7$F$J$$$,!"A4BN$H$7$F$O>iD9$JI=8=$G$"$k$3$H$,B?$$!#(B
//$B$3$3$GI=<($5$l$k!!(Bxi $B$NHV9f$O(B 1$B$+$i$G!"(B Bdd$B$NJQ?tHV9f(B(lev)$B$H0lCW(B
// x4x3~x2x1+~x4x3x2 $B$HI=<($5$l$l$P!"(Bx2 $B$H!!(Bx4$B$,H]Dj$H$$$&0UL#(B
//***************


extern void Array_Bdd_print(FILE *sout, Array<Bdd> Bdd_Array, int n_in);
//**$BCm0U(B Bdd::null $B$,(B Array$B$K4^$^$l$F$?$i!"Mn$A$k!#!J%(%i!<%A%'%C%/$J$7!K(B
// $BJ#?t$N(BBdd $B!J(BArray<Bdd)$B$r2#$KJB$Y$FI=<((B --> $BHf3S$7$d$9$$(B
//$BJQ?t$N?t$,(B9$B$h$jB?$$$HI=$,$:$l$k$N$GI=<($7$J$$!#(B
// Bdd_printTable_gray(FILE *sout, int n_in, const Bdd F)$B$rF1;~$K9T$&46$8(B


extern void Bdd_Karnaugh_Map(FILE *sout, int n_in, const Bdd F);
//***************
//$B%+%k%N%^%C%WI=<($r9T$&!#>eIt$K$OJQ?tHV9f$NB?$$J}!"(B
//$B:8B&$K$OJQ?tHV9f$N>/$J$$J}(B(x1$B$+$i!K$rI=<($9$k!#(B
//$BJQ?t$N?t$,(B9$B$h$jB?$$$HI=$,$:$l$k$N$GI=<($7$J$$!#(B
//***************

// char s  $B$O(B $B8F$S=P$7B&$G3NJ]$5$l$F$$$k$3$H(B
extern void itoa(int n, char *s);

#endif



 
