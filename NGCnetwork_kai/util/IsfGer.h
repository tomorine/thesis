//specify = 1 $B$H(B $B$J$kItJ,$r(B lf$B$G$*$-$+$($k!#!J!v$r69$a$k!K(B
inline void ISF::specify(Bdd* lf, Bdd* specify)
{
    low =low | ( (*specify) & (*lf) ) ;
    up = up & ~( (*specify) & ~(*lf) ) ;
}



//bdd $B$N$H$3$m$r(B*$B$K$9$k(B
// low ---> f=1 $B$N$H$3$m$O$9$Y$F#0$K(B
// up  ---> f=1 $B$N$H$3$m$O$9$Y$F(B1$B$K(B
inline void ISF::maskISF(Bdd* f)
{
    low = low & ~(*f);
    up =  up | (*f);
}


inline int ISF::topVarId() const{
    int rankl = low.top();   
    int ranku = up.top();    
    if(rankl > ranku){
      return low.top();   // low $B$N:G>e0L$NJ}$,%0%i%U$N>e0L$J$N$G$=$N(B
                         //$BHV9f$rJV$9!#(B
      }
    else{
      return up.top();
    }
}


/* $B%l%Y%k(B lev(varid) $B$NJQ?t$K#0!J#1!K$rF~$l$?(BISF$B%*%V%8%'%/%H$r$D$/$jJV$9(B*/
// $BC1$K(Blow$B$H(Bup$B$N(Bcofactor$B$r$H$C$F$$$k$@$1(B
inline ISF ISF::rstr0(const int lev) const{
    ISF result(low.rstr0(lev),up.rstr0(lev));
    return result;
}

inline ISF ISF::rstr1(const int lev) const{
    ISF result(low.rstr1(lev),up.rstr1(lev));
    return result;
}

// ISF = 1 $B$H$J$k4X?t$rJV$9!#(B
inline Bdd ISF::pf1() const{
    return low;
}

// ISF = 0 $B$H$J$k4X?t$rJV$9!#(B
inline Bdd ISF::pf0() const{
//    return ( (~low) & (~up) );
    return (~up) ;
} 

// ISF = 0 $B$^$?$O(B $B#1$H$J$k4X?t$rJV$9!#(B
inline Bdd ISF::pfSpecified(){
    return ( (~up) | low );
}


// ISF = * $B$H$J$k4X?t$rJV$9!#(B
inline Bdd ISF::pfStar() const{
    return ( (~low) & (up) );
  }

inline int ISF:: IsSatisfy(Bdd lf){
  if( (low <= lf) && (lf<=up)) return 1;
  else return 0;
}


inline int ISF::AllTrue(){
   return( (up == Bdd::one) );
}

inline int ISF::AllFalse(){
   return( (low == Bdd::zero) );
}





inline ISF::ISF():low(Bdd::zero),up(Bdd::zero){;}

inline ISF::ISF(const ISF& oprand) : low(oprand.low),up(oprand.up){;}
inline ISF::ISF(Bdd f0, Bdd f1):low(f0),up(f1){;}    
inline ISF::ISF(Bdd lf):low(lf),up(lf){;}    

//inline ISF::ISF():low(Bdd()),up(Bdd()){}
//inline ISF::ISF(const ISF& oprand) : low(Bdd(oprand.low)),up(Bdd(oprand.up)){}
//inline ISF::ISF(Bdd f0, Bdd f1):low(Bdd(f0)),up(Bdd(f1)){}    
                     // Bdd$B$N%3%T!<%3%s%9%H%i%/%?8F$P$l$F$k$+$J!)(B
    
inline ISF ISF::operator = (const ISF oprand) {
    if ( *this != oprand ) {
        low = oprand.low;       
        up  = oprand.up;
    }
    return *this;
}


//inline ISF ISF::operator ~ () const {
//    ISF  result(up,low);                   // low <-> up
//    return result;
//}

inline ISF ISF::operator ~ () const {
  Bdd newlow = pf0();        //$B$5$C$-(B0$B$@$C$?$H$3$m(B
  Bdd newup = newlow | pfStar();
    ISF  result(newlow, newup); 
    return result;
}



// low <= up $B$H$J$C$F$J$+$C$?$i(B0, $B$J$C$F$?$i(B1 
inline int ISF::check(){
    return ((low <= up));
}


// $B$I$A$i$K$b(B $B4^$^$l$k(B $B5vMF4X?t=89g$r5a$a$k!#(B
// $B$=$N$h$&$J=89g$,$J$1$l$P!"(Blow=Bdd::one ,up=Bdd::zero$B$H$9$k!#(B
//$B!!$=$&$9$l$P(Bcheck$B$G$R$C$+$+$k$N$G(B

inline ISF operator & (const ISF lhs, const ISF rhs) {
    ISF result((lhs.low | rhs.low),(lhs.up & rhs.up));
    if( !(result.check())){
	result.low = Bdd::one;
	result.up = Bdd::zero;
    }
    return result;
}




inline int operator == (const ISF lhs, const ISF rhs) {
    return ((lhs.low == rhs.low) && (lhs.up == rhs.up));
}

inline int operator != (const ISF lhs, const ISF rhs) {
    return (! ((lhs.low == rhs.low) && (lhs.up == rhs.up)) );
}

inline int operator <= (const ISF lhs, const ISF rhs) {
    return ( (rhs.low <= lhs.low) && (lhs.up <= rhs.up) );
}
