////////////////////////////////////////////////////////////////////////////
//Support$BJQ?t=89g$r07$&$?$a$N%/%i%9(B

#include <stdio.h>
#include <stdlib.h>
#include <string>

class SupVars{
private:
  Bdd       zbdd;    //zbdd $BI=8=$G$"$k$3$H(B     cube $B$h$jJQ99(B
public:
    inline int SupNum() const { return zbdd.lit();};  
//card$B$G$b9T$1$k$O$:(B
//  inline int SupNum() const { return zbdd.card();};  
  //  inline Bdd CubeBdd() const { return cube;}; 
  inline Bdd GetZBdd() const { return zbdd;}; 

  // $B!z!z(B ZBDD $B$r(B $BEO$5$J$1$l$P$$$1$J$$!%(B lf.varSet()
  inline SupVars(Bdd zbdd);  

  inline SupVars();   //$B6u=89g(B = Bdd::empty
  inline friend int operator == (const SupVars lhs, const SupVars rhs);  

  inline SupVars delete_sup_vars(SupVars * DeleteSup);
//SupVars$B$G!"(B DeleteSup$B$K4^$^$l$kJQ?t$r:o=|$7$?(BSupVars$B$r$D$/$C$F$+$($9(B
//$B!z<+J,<+?H(B, DeleteSup$B$OJQ99$5$l$J$$(B

//merge_Sup $B$r<+J,$H(Bmerge$B$7$?(Bsup$B$r?7$?$K$D$/$jJV$9!#(B
// x1x2x3 + x3x4x5 -> x1x2x3x4x5 $B$rJV$9!#(B
  inline SupVars merge_sup_vars(SupVars * merge_Sup) const;

  SupVars(const SupVars& orig){
    this->zbdd = orig.zbdd;
  };

  inline int delete_top(); //top $B$NJQ?t$NHV9f$rJV$7!"$^$?$=$l$r=89g$+$iH4$/(B
         //$B6u=89g$N$H$-$O$J$K$b$;$:$K!"#0$rJV$9(B($B$3$l$r%k!<%W$NH=Dj$K$D$+$($P$$$$!K(B
   //$B!z!z!!MWCm0U!!$3$l$O(B this$B<+?H$rJQ99$9$k$N$GCm0U$7$J%P%0$N860x$H$J$k$h!#(B

////////// 
/*    SupVars this $B$N(B $B3FJQ?t$KBP$7$F$J$K$+$9$k(B $B%3!<%INc(B

      for(SupVars temp = this; temp.SupNum() > 0 ; ){
         Xi = temp.delete_top();
	
         do something to Xi; 
  
      }
 */
////////// 


//For debug $B4^$^$l$F$kJQ?tHV9f$rA4$FI=<((B
  void print_sup();

  inline int includeKa(int var_id) const ; 
     //var_id $B$NJQ?t$r4^$s$G$$$?$i(B1$B$rJV$9!#(B $B$=$&$G$J$$$J$i(B0


  inline  SupVars     operator = (const SupVars oprand);

//  int SupVars::return_top() const;
  int return_top() ;
       //top $B$NJQ?t$NHV9f$rJV$9!#6u=89g$N$H$-$O#0$rJV$9(B
       // supvars$B<+BN$O$J$K$bJQ99$7$J$$!#(B

  inline void add_supVars(int varid); 
     //varid$BHV$NJQ?t$rDI2C(B
     //$B!J$9$G$K$"$k$+$N%A%'%C%/$b$7$F$"$l$P2?$b$;$:!"(Bwarning $B$N$_$@$9(B)
};



