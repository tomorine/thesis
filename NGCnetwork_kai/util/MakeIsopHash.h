/////////  //$B0J2<$O(BmakeISOP$B$N$_$,;H$&(B (hash$B$N$?$a(B)

const int ISOPhashSize = 1 << 20;


#define DEBUG_ISOP 1

//For Debug
#ifdef DEBUG_ISOP 
extern int ISOPhashHit; 
extern int makeISOPcount;    //for debug  (called $B$N2s?t(B)
extern int ISOPhashMissHit;  //Cmap$B$K$OF1$8%-!<CM$G%(%s%H%j$"$C$?$1$I!"0c$&>l9g(B
#endif DEBUG_ISOP



class ResultPair{
public:
  ResultPair();   //Cmap$B$N%G%U%)%k%H!!$H$7$F;H$&$d$D(B
  ResultPair(ISF isf, Sop isop);
  ResultPair(const ResultPair& oprand);
//  ~ResultPair() {}
  ISF isf;
  Sop isop;
  ResultPair   operator = (const ResultPair& oprand);
  inline int Equal(ISF& isf1){ return (isf == isf1);};
 inline friend int   operator != (const ResultPair& lhs, const ResultPair& rhs);
 inline friend int   operator == (const ResultPair& lhs, const ResultPair& rhs);
};

inline int operator == (const ResultPair& lhs, const ResultPair& rhs)
{
  return (lhs.isf == rhs.isf) && (lhs.isop == rhs.isop);
}

inline int operator != (const ResultPair& lhs, const ResultPair& rhs)
{
  return (lhs.isf != rhs.isf) || (lhs.isop != rhs.isop);
}

inline ResultPair::ResultPair(const ResultPair& oprand)
: isf(oprand.isf), isop(oprand.isop) {
 }
  

inline ResultPair::ResultPair(ISF isf, Sop isop) 
: isf(isf), isop(isop) { }

inline ResultPair::ResultPair() 
{ }



inline ResultPair ResultPair::operator = (const ResultPair& oprand) {
  if ( *this != oprand ) {
    isf = oprand.isf;
    isop = oprand.isop;
  }
  return *this;
}




/* $B0J2<$O!$@N(Bger-base$B$K$"$C$?$d$D!%(B


////////////$B?7$7$$!!(BISOP $B$O!!(BSop$B%/%i%9$G<B8=$7$h$&!!(B(06/30)
// Bdd -> $BHs>iD9$J(B sop $B$rJV$9%k!<%A%s!"$3$l$,$G$-$?$i!"(B
// ISOP* ISF::makeISOP() $B$O!!(Bobsolete $B$K$J$k$7!"(B
//  printIntFunc(FILE * sout, Node * node); $B$J$I$b=q$-49$($kI,MWM-$j!"(B

// 08/06 hash $B$r$D$+$C$?%P!<%8%g%s(B
// $B!z!!$V$D$+$C$?$H$-$O!"$"$-$i$a$F$b$$$$$s$d$C$?$i!"$o$6$o$6(BCmap$B$D$+$o$s$G$b(B
//   ResultPair $B$N!!G[Ns$@$1$G$b$h$+$C$?$J!#$G$b!"$I$&$;Aa$5$O$=$s$J$K$+$o$i$s$d$m$+$i(B
//  $B$H$j$"$($:!"0J2<$N<B8=$G$$$$$3$H$K$7$h$+$J(B

//  makeISOP $BMQ(B hash

inline int  hashfunc(ISF& lf){
  int f = lf.Low().value();
  int g = lf.Up().value();
  if(f==g) return f;
  else return  ( (f)^((f)<<4)^(g)) ;  //hashSize $B$G!!(BCmap$B$NCf$G$I$&$;3d$i$l$k!#(B
}


inline int  hashfunc2(int& a){
  return a;
}

#ifdef DEBUG_ISOP 
int ISOPhashHit = 0;
int ISOPhashMissHit = 0;  //Cmap$B$K$OF1$8%-!<CM$G%(%s%H%j$,$"$k$,!"0c$&>l9g(B
int makeISOPcount = 0;
#endif DEBUG_ISOP

//  ISF -> int (hashfunc$B$G(B)$B!!$3$NCJ3,$G!"0c$&(BISF$B$+$i!"F1$8(Bint$B$G$-$k$3$H$"$j!#(B
//  $B<!$K(B int key -> Cmap -> ResultPair ($B$3$l$O!"0c$&(Bkey $B$+$iI,$:0c$&(BResultPair$B0z$1$k!K(B
//   ($B$G$b!"0c$&(BISF$B$+$i!"$*$J$8(Bkey$B$G$-$k$3$H$"$k$N$G!J$"$^$j$J$$$h$&$@$,!K(B
//    ResultPair->isf $B$,(B $B0z$$$?(BISF$B$HF1$8$@$C$?$+$I$&$+$r%A%'%C%/$9$k!#(B


ResultPair dft;
//Cmap<int, ResultPair> ISOPhash = Cmap<int, ResultPair>(ResultPair(), hashfunc2, ISOPhashSize); 
Cmap<int, ResultPair> ISOPhash = Cmap<int, ResultPair>(dft, hashfunc2, ISOPhashSize); 
//Cmap $BFb$N(B ISF, Sop$B!J$D$^$k$H$3$m(BBdd)$B$O<B:]$"$k$b$N$r%3%T!<$7$F$$$k!#!K(B
// $B$=$N$?$a!"(BISF SOP$B$J$I$,Nc$(>C$5$l$?$H$7$F$b!"(Bhash$B$r0z$1$k!#$@$@$7L@<(E*$KA4$F$N(B
//Bdd$B$r$1$9$K$O!"0J2<$N(B ClearMakeISOPHash $B$r8F$P$J$$$H$@$a!#(B

void ClearMakeIsopHash()
{
//  Bdd::gc();
//  printf("Bdd::objnum = %d\n", Bdd::objnum);
  ISOPhash.clear();
//  Bdd::gc();
//  printf("After Bdd::objnum = %d\n", Bdd::objnum);
}

// $B!z(B Hash $B$"$j$N%P!<%8%g%s(B
Sop ISF::makeISOPnew(int* lev2idx)
{
#ifdef DEBUG_ISOP 
  makeISOPcount++;
#endif DEBUG_ISOP

  if(AllFalse()){       // $B91564X?t(B
    return Sop::zero;
  }
  else if(AllTrue()){
    return Sop::one;  // $B91??4X?t(B   
  }   

//Read the Result Table
  int key = hashfunc(*this);
  ResultPair Oldresult = ISOPhash[key];
  if(Oldresult != ResultPair()){       //Cmap $B$N(Bdflt
           //dflt $B$G$J$1$l$P$H$j$"$($:!"$3$N%-!<$N(Bitem$B$,(Btable$B$K$"$k!#(B
    if(Oldresult.Equal(*this) == 1){
#ifdef DEBUG_ISOP 
      ISOPhashHit++;
//      cerr << " Hash Hit \n";
#endif DEBUG_ISOP
      return (Oldresult.isop);     //$B$3$l$G%3%T!<$7$?$3$H$K$J$k$O$:(B
    }
#ifdef DEBUG_ISOP 
    else{
      ISOPhashMissHit++;
    }
#endif DEBUG_ISOP
  }


  int level=this.topVarId(); //$BE83+$9$Y$-JQ?t$NHV9f!J(Brank$B$G$O$J$$!K(B
//    cerr << "\n*** expandinv id = " << level << "*****\n"
  ISF f0(rstr0(level));
  ISF f1(rstr1(level));
  ISF f00((f0.low & ~(f1.up)),f0.up);
  ISF f11((f1.low & ~(f0.up)),f1.up);
  Sop result_f0  = f00.makeISOPnew();           // isop0$B$KAjEv(B
  Sop result_f1  = f11.makeISOPnew();
  Bdd g0 = (result_f0).getFunc(0);
  Bdd g1 = (result_f1).getFunc(0);
  ISF f000((f0.low & ~(g0)),f0.up);
  ISF f111((f1.low & ~(g1)),f1.up);
    ISF isopAlpha( ( (f000.low & f111.up) | (f000.up & f111.low) )
		   , (f000.up & f111.up)); 

  Sop result_fr = isopAlpha.makeISOPnew(); 

  int index;
  if(lev2idx == 0){
    index = level;
  }
  else{
    index = lev2idx[level];
  }
  Sop result = result_f0.and0(index) + result_f1.and1(index) + result_fr;

//write to the result table

  ResultPair NewResult = ResultPair(*this, result);  
  ISOPhash[key] = NewResult;   //$BBeF~;~!!:8JU(Bfree $B$9$k$O$:(B

  return result;
}
*/


