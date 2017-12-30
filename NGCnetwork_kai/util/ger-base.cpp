#include "ger-base.h"
#include "ger-base-old.h"
#include "isf.h"
#include "Cmap.H"
using namespace std;


//template class Array<Bdd>;
//template class PList<Node>;

int word_len(const char* c) {
    assert( *c != ' ' );        // 1$BJ8;zL\$O6uGr$G$J$$$3$H(B
    int length=0;
    while ( *c != ' ' && *c != '\0' ) {
        c++;
        length++;
    }
    return length;
}





void printArrayInt(Arrayint Array)
{
    cerr << " ** printqing Arrayint ** \n";
    int size = Array.size();
    cerr << "    Size =  " << size << "  \n";
    for(int i =0 ; i<size; i++){
        cerr << " index  " << i  << " -> " << Array[i] << "  \n";
    }
    cerr << " ** Finished printing Arrayint ** \n";
}




// var2idx: Bdd$B$N(Bvar(1$B$+$i;O$^$k(B)$B$+$i!$I=<(;~$N(Bidx(0$B$+$i;O$^$k(B)$B$X$N<LA|(B
//void BddPrintPLA(FILE * sout, const Bdd& lf, int* var2idx=0)
void BddPrintPLA(FILE * sout, const Bdd& lf, int* var2idx)
{
    if(lf == Bdd::null){
        cerr << "print PLA Warning!! Bdd::null cannot display \n";
        return;
    }
    if(lf == Bdd::zero){ //$B2?$bI=<($7$J$$(B
        return;
    }
    else if(lf == Bdd::one){ //1 $B$@$1I=<((B
        fprintf(stderr, "1\n");
        return;
    }
    //  ISF isf(lf, lf);
    //  Sop isop =  isf.makeISOPnew();
    // $B!z(B 12/25  hashtable $B$d$d$3$7$$$N$G!$(BIsfLU->makeISOP $B$KE}0l(B
    IsfLU isf = IsfLU(lf, lf);
    Sop isop =  isf.makeISOP();

    //  ISF NOTisf(~lf, ~lf);
    //  Sop NOTisop = NOTisf.makeISOPnew();
    IsfLU notisf = IsfLU(~lf, ~lf);
    Sop NOTisop =  notisf.makeISOP();


    if( NOTisop.nCube() < isop.nCube()){
        CubeSet::printSop(NOTisop, sout, (char*)" 0", var2idx);
    }
    else{
        CubeSet::printSop(isop, sout, (char*)" 1", var2idx);
    }
}






//x1, x2 $B$HI=<($9$k$?$a$K!"(Blevel = 1$B$NJQ?t$+$iE83+$7$F$$$/!#(B
//$B8=:_$^$G$K(Blevel$BHV$^$GE83+:Q$_(B
static void bddprint_table_normal(FILE *sout, int n_in, const Bdd f, 
                                  unsigned int assigned, int level) 
{
    if ( level == n_in ) {         //$BE83+=*N;!J=*C<%N!<%I$r;X$7$F$k$O$:!K(B
        for ( int i=n_in-1; i>= 0; i--)
            fprintf(sout, " %d ", (assigned>>i) & 1);

        if ( f == Bdd::zero ) fprintf(sout, "| 0\n");
        else if ( f == Bdd::one ) fprintf(sout, "| 1\n");
        else  cerr << "Bdd print_table error -- something wrong \n";
        return;
    }
    else {
        Bdd f0 = f.rstr0(level+1);   
        Bdd f1 = f.rstr1(level+1);   
        bddprint_table_normal(sout, n_in, f0, assigned, level+1);
        bddprint_table_normal(sout, n_in, f1, assigned | ( 1<<(n_in - level -1)), level+1);
    }
}


//x1, x2 $B$HI=<($9$k$?$a$K!"(Blevel = 1$B$NJQ?t$+$iE83+$7$F$$$/!#(B
// assigned $B$N0lHV2<$N%S%C%H$,!"(BXi (i=n_in)$B$KAjEv(B
// Gray-Code $B$G(B $BI=8=(B(flag = 1 $B$G$=$N<!$rH?E>$5$;$k!K(B
static void bddprint_table_gray(FILE *sout, int n_in, const Bdd f, 
                                unsigned int assigned, int level, int flag) 
{
    if ( level == n_in ) {         //$BE83+=*N;!J=*C<%N!<%I$r;X$7$F$k$O$:!K(B
        for ( int i=n_in-1; i>= 0; i--)
            fprintf(sout, " %d ", (assigned>>i) & 1);

        if ( f == Bdd::zero ) fprintf(sout, "| 0\n");
        else if ( f == Bdd::one ) fprintf(sout, (char*)"| 1\n");
        else  cerr << "Bdd print_table error -- something wrong \n";
        return;
    }
    else {
        Bdd f0 = f.rstr0(level+1);   
        Bdd f1 = f.rstr1(level+1);   
        if(flag==0){
            bddprint_table_gray(sout, n_in, f0, assigned, level+1, 0);
            bddprint_table_gray(sout, n_in, f1, assigned|(1<<(n_in-level-1)), level+1, 1);
        }
        else{
            bddprint_table_gray(sout, n_in, f1, assigned|(1<<(n_in-level-1)), level+1, 0);
            bddprint_table_gray(sout, n_in, f0, assigned, level+1, 1);
        }

    }
}

void Bdd_printTable_normal(FILE *sout, int n_in, const Bdd F)
{
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout,"***** BDD print_table (Normal Coding) \n");
    //  fprintf(sout,"*****    -- xi corresponds to Bdd::var(i) \n");
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout," \n");
    if(F == Bdd::null){ 
        fprintf(stderr,"BDD NULL \n");
        return;
    }
    if ( n_in > 9 ){              //$B:GBg$G$b(B32=int$B$N%S%C%HI}$^$G(B
        cerr << "********Sorry: too many input to display.******* \n";
        cerr << "******** BDD_PRINT can treat within 8 inputs ******* \n";
        return;
    }
    int i;
    for( i=1; i <= n_in;i++){
        fprintf(sout, "x%d ",i);
    }               
    fprintf(sout, "| F\n");
    for(i=n_in; i > 0;i--){
        fprintf(sout, "---");
    }               
    fprintf(sout, "+---\n");
    bddprint_table_normal(sout, n_in, F, 0, 0);
}



void Bdd_printTable_gray(FILE *sout, int n_in, const Bdd F)
{
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout,"***** BDD print_table (Gray Coding) \n");
    //  fprintf(sout,"*****    -- xi corresponds to Bdd::var(i) \n");
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout," \n");
    if(F == Bdd::null){ 
        fprintf(stderr,"BDD NULL \n");
        return;
    }
    if ( n_in > 9 ){              //$B:GBg$G$b(B32=int$B$N%S%C%HI}$^$G(B
        cerr << "********Sorry: too many input to display.******* \n";
        cerr << "******** BDD_PRINT can treat within 8 inputs ******* \n";
        return;
    }
    int i;
    for( i=1; i <= n_in;i++){
        fprintf(sout, "x%d ",i);
    }               
    fprintf(sout, "| F\n");
    for(i=n_in; i > 0;i--){
        fprintf(sout, "---");
    }               
    fprintf(sout, "+---\n");
    bddprint_table_gray(sout, n_in, F, 0, 0, 0);
}



void bddprintR(FILE *sout, Bdd f, int flag) {  // flag =0 $B$G(B $B!J!K(B=$BHsI=<((B
    if(f == Bdd::null){ 
        fprintf(stderr,"BDD NULL \n");
        return;
    }
    Bdd f_, f0, f1;                             // flag =1 de $B$=$NA0$K(B
    if (f == Bdd::zero) {                           // $B9`$,$"$k!#(B
        fprintf(sout, "BDDF");
        return;
    }
    f_ = ~f;
    if (f_ == Bdd::zero) {
        fprintf(sout, "BDDT");
        return;
    }

    //    f0 = f.rstrtop(0);
    //    f1 = f.rstrtop(1);

    f0 = f.rstrtop0();
    f1 = f.rstrtop1();

    int varid = f.top();
    
    if(! bddInter(f1, ~(f0))){                // f1 <= f0
        if(flag==1 && (f1 != Bdd::zero)){
            fprintf(sout, "(");
        }
        if(f1 != Bdd::zero){
            bddprintR(sout,f1,0);
            fprintf(sout, "+");
        }
        fprintf(sout, "~x%d", varid); 
        if(f0 != Bdd::one){
            bddprintR(sout,f0,1);
        }

        if(flag==1 && (f1 != Bdd::zero)){
            fprintf(sout, ")");
        }

    }
    else if(! bddInter(f0, ~(f1))){       // f0 <= f1
        if(flag==1 && (f0 != Bdd::zero)){
            fprintf(sout, "(");
        }
        fprintf(sout, "x%d", varid );
        if(f1 != Bdd::one){
            bddprintR(sout,f1,1);
        }
        if(f0 != Bdd::zero){
            fprintf(sout, "+");
            bddprintR(sout,f0,0);
        }
        if(flag==1 && (f0 != Bdd::zero)){
            fprintf(sout, ")");
        }

    }
    else{   // f1 f0 = Bdd::zero Bdd::one $B$G$O$J$$$O$:(B
        if(flag == 1){
            fprintf(sout, "(");
        }
        fprintf(sout, "x%d", varid);
        bddprintR(sout,f1,1);
        fprintf(sout, "+~x%d", varid);
        bddprintR(sout,f0,1);
        if(flag == 1){
            fprintf(sout, ")");
        }
    }
}



void Bdd_print_SOP(FILE *sout, Bdd F) {
    //  fprintf(sout,"         ******************************************* \n");
    //  fprintf(sout,"         ***** BDD print_SOP  \n");
    //  fprintf(sout,"         *****    -- xi corresponds to Bdd::var(i) \n");
    //  fprintf(sout,"         ******************************************* \n");
    //  fprintf(sout, " **-- The logic function = ");
    bddprintR(sout, F, 0);
    fprintf(sout, "\n");
}







static void bddprint_table_gray_array(FILE *sout, int n_in, Array<Bdd> array, 
                                      unsigned int assigned, int level, int flag) 
{
    int array_num = array.size();
    Bdd f;  //temporaly
    int j;
    if ( level == n_in ) {         //$BE83+=*N;!J=*C<%N!<%I$r;X$7$F$k$O$:!K(B
        for ( int i=n_in-1; i>= 0; i--)
            fprintf(sout, " %d ", (assigned>>i) & 1);
        for(j = 0; j < array_num ; j++){
            f = array[j];
            if ( f == Bdd::zero ) fprintf(sout, "| 0  ");
            else if ( f == Bdd::one ) fprintf(sout, "| 1  ");
            else  cerr << "Bdd print_table error -- something wrong \n";
        }
        fprintf(sout, "|\n");
        return;
    }
    else {
        Bdd f1, f0; //temporaly
        Array<Bdd> array_0, array_1;
        for(j = 0; j < array_num ; j++){
            f = array[j];
            f0 = f.rstr0(level+1);   
            f1 = f.rstr1(level+1);   
            array_0.add(f0);
            array_1.add(f1);
        }
    
        if(flag==0){
            bddprint_table_gray_array(sout, n_in, array_0, assigned, level+1, 0);
            bddprint_table_gray_array(sout, n_in, array_1, assigned|(1<<(n_in-level-1)), level+1, 1);
        }
        else{
            bddprint_table_gray_array(sout, n_in, array_1, assigned|(1<<(n_in-level-1)), level+1, 0);
            bddprint_table_gray_array(sout, n_in, array_0, assigned, level+1, 1);
        }

    }
}



// $BJ#?t$N(BBdd $B!J(BArray<Bdd)$B$r2#$KJB$Y$FI=<((B --> $BHf3S$7$d$9$$(B
// 10$B$30J>e$NI=<($O$:$l$k$N$G!"$d$a$k$Y$-(B
void Array_Bdd_print(FILE *sout, Array<Bdd> Bdd_array, int n_in)
{
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout,"***** Array<Bdd> print_table (Gray Coding) \n");
    //  fprintf(sout,"*****    -- xi corresponds to Bdd::var(i) \n");
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout," \n");
    if ( n_in > 9 ){              //$B:GBg$G$b(B32=int$B$N%S%C%HI}$^$G(B
        cerr << "********Sorry: too many input to display.******* \n";
        return;
    }
    int array_num = Bdd_array.size();
    int i;
    for(i=n_in*3 + array_num *5; i > 0;i--){
        fprintf(sout, "-");
    }            
    fprintf(sout, "+\n");

    for(i=1; i <= n_in;i++){
        fprintf(sout, "x%d ",i);
    }
    for(i = 1; i<= array_num ;i++){
        fprintf(sout, "| F%i ", i);
    }
    fprintf(sout, "|\n");

    for(i=n_in; i > 0;i--){
        fprintf(sout, "---");
    }            
    for(i=array_num; i > 1;i--){
        fprintf(sout, "+----");
    }            
    fprintf(sout, "+----|\n");
    bddprint_table_gray_array(sout, n_in, Bdd_array, 0, 0, 0);

    for(i=n_in; i > 0;i--){
        fprintf(sout, "---");
    }            
    for(i=array_num; i > 1;i--){
        fprintf(sout, "+----");
    }            
    fprintf(sout, "+----|\n");
}


// $B2<0L(Bbit$BJ,$KBP$7$F!"(Bbinary$B$r#2?J?t$H$*$b$C$F!"$=$l$r(Bgray code$B$K$7$?$b$N$rJV$9!#(B
// binary $B$N>e0L%S%C%H$N#1$D:8$N%S%C%H!J(Bbit+1$B%S%C%HL\(B)$B$OI,$:#0$G$J$$$H$@$a!#(B
static int binary2gray(int binary, int bit)
{
    //$BM?$($i$l$?$b$H$N(Bbinary$B$N#1$D:8$N%S%C%H$,(B1$B$G$"$k$H$-!"$=$3$rH?E>$9$l$P!"(B
    // gray$B%3!<%I$K$J$k!#(B
    int gray = binary;
    int checkbit; //$B8=:_CmL\$7$F$k%S%C%H$N#1$DA0$N%S%C%H(B
    int mask;
    for(int i = bit; i >= 1 ; i--){ //$B8=:_(Bi$B%S%C%HL\$rCmL\(B
        checkbit = (binary>>i) & 1;  //$B0l$D>e$N%S%C%H(B
        if(checkbit == 1){   //gray $B$N(B i$B%S%C%HL\$rH?E>(B
            mask = 1<<(i-1); //i$B%S%C%H$a$@$1$,(B1
            gray = gray ^ mask; //gray $B$N(B i$B%S%C%HL\$@$1H?E>(B
        }
    }
    return gray;
}


//$B8zN($o$k$$$1$I!"#8JQ?t$G$b$$$1$F$k$h$&(B
//gray-code$B$N:n$j$,$"$C$F$k$+$A$g$C$HIT0B$d$1$I(B...$B$H$j$"$($:$O$$$1$F$k$h$&!#(B
void Bdd_Karnaugh_Map(FILE *sout, int n_in, const Bdd F)
{
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout,"*****  Printing Karnaugh Map  \n");
    //  fprintf(sout,"*****    -- xi corresponds to Bdd::var(i) \n");
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout," \n");
    if(F == Bdd::null){ 
        fprintf(stderr," cannot display BDD null\n");
        return;
    }
    if ( n_in > 9 ){              //$B:GBg$G$b(B32=int$B$N%S%C%HI}$^$G(B
        cerr << "********Sorry: too many input to display.******* \n";
        return;
    }
    int upper = n_in /2;      //$BI=$N>e$K$/$kJQ?t$N?t(B ($BH>J,$h$j$O>/$J$$(B)
    int lower = n_in - upper;
    int array_num =  1 << upper; //$B2#$K$J$i$VJQ?t$N$+$:(B
    int i;

    //$B>eIt$K@~(B

    int lowerHaba = 3;   //$B:8B&$N#1JQ?t$K3d$jEv$F$kHO0O(B
    int upperHaba = 5; //$B>eB&$N#1$D$NItJ,4X?t$K3d$jEv$F$kHO0O(B

    for(i=lower * lowerHaba + 1 + array_num * upperHaba; i > 0;i--){
        fprintf(sout, "-");
    }
    fprintf(sout, "\n");
    //2$B9TL\(B
    for(i=1; i <= (lower*lowerHaba);i++){  
        fprintf(sout, " ");
    }
    fprintf(sout, "|");
    for(i=lower+1; i <= n_in;i++){
        fprintf(sout, "  x%d ",i);
    }            
    fprintf(sout, "\n");
    //3$B9TL\(B
    for(i=1; i <= lower;i++){
        fprintf(sout, "x%d ",i);
    }
    fprintf(sout, "|");

    //$B$3$3$+$i2#J}8~$X$NI=<((B
    Array<Bdd> Bdd_array;
    Bdd tempBdd;
    int k, j; //for loop only
    int varid;
    int checkbit;
    int assigned;  
    for(i=1; i<= array_num; i++){
        //assigned$B$N7W;;(B 
        assigned = binary2gray(i-1, upper);  //assigned = gray code$B$K(B
        fprintf(sout, " ");
        for(k=upper-1; k>=0; k--){ //assigned$B$r(B upper bit$B$@$1I=<((B
            fprintf(sout, "%d", ((assigned>>k) &1) );
        }
        for(j=1+upper;j<= (upperHaba-1); j++){
            fprintf(sout, " ");
        }
        //assigned$B$+$i!"(BBdd$B$N7W;;(B
        tempBdd = F;
        for(k=1; k<=upper; k++){ //upper$B$N$&$A$N(Bk$BHVL\$N=hM}(B
            varid = lower + k;
            checkbit = (assigned >>(upper-k)) & 1;
            if(checkbit == 1) tempBdd = tempBdd.rstr1(varid);
            else tempBdd = tempBdd.rstr0(varid);
        }
        //assigned$B$+$i!"(BBdd$B$N7W;;=*$o$j(B
        Bdd_array.add(tempBdd);
    }
    fprintf(sout, "\n");

    //$B6-3&@~(B
    for(i=lower; i>0 ; i--){
        fprintf(sout, "---");
    }    
    fprintf(sout, "+");
    for(i=array_num; i >= 1;i--){
        fprintf(sout, "-----");
    }            
    fprintf(sout, "\n");

    bddprint_table_gray_array(sout, lower, Bdd_array, 0, 0, 0);

    for(i=(lower*3 + 1 + array_num * 5); i > 0;i--){
        fprintf(sout, "-");
    }
    fprintf(sout, "\n");  
}

// stderr $B$K(B $BI=<((B $B#29`0J>e$"$l$P(B()$B$r$D$1$k!#(B
// $B91??!"91564X?t$G$O2?$b$7$J$$!#(B
// flag = 0 $B$N;~$O(B $B3g8L$r$D$1$J$$!#(B

void  sop_print(FILE * sout, Sop  sop, int flag)
{
    if(sop == Sop::one){fprintf(sout,"BDDT"); return;}
    else if(sop == Sop::zero){fprintf(sout,"BDDF"); return;}
    int varid = sop.topvar();
    int termNum = 0;
    Sop f0 = sop.factor0(varid);
    Sop f1 = sop.factor1(varid);
    Sop fr  = sop.factorR(varid);

    if( (f0 != Sop::zero) ) termNum++;
    if( (f1 != Sop::zero) ) termNum++;
    if( (fr != Sop::zero) ) termNum++;

    if((termNum > 1) &&(flag) ){
        fprintf(sout,"(");
    }
    int plusNeed = 0 ; //1$B$J$i!!!\!!$r(B $B$+$+$J$@$a(B
    if( f0 != Sop::zero ) {
        fprintf(sout,"~x%d", varid);   
        if( f0 != Sop::one){ 
            sop_print(sout, f0, 1);
        }
        plusNeed = 1;
    }

    if( f1 != Sop::zero ) {
        if(plusNeed == 1) fprintf(sout,"+");
        fprintf(sout,"x%d", varid);   
        if( f1 != Sop::one){ 
            sop_print(sout, f1, 1);
        }
        plusNeed = 1;
    }

    if( fr != Sop::zero ) {
        if(plusNeed == 1){ //$BA0$K(Bf0$B$+(Bf1$B$,$"$k!#(B
            fprintf(sout,"+");	
            sop_print(sout, fr, 0);
        }
        else{ 
            sop_print(sout, fr, flag);
        }
    }
    
    if((termNum > 1) &&(flag) ){
        fprintf(sout,")");
    }
}



// Bdd $B$N(Bstderr$B$X$NHs>iD9@QOB7A$NI=<((B
void printISOP(Bdd lf)
{
    if(lf == Bdd::null){ 
        fprintf(stderr,"BDD NULL \n");
    }
    else if(lf == Bdd::one){
        fprintf(stderr,"BDDT \n");
    }
    else if(lf == Bdd::zero){
        fprintf(stderr,"BDDF \n");
    }
    else{

        //    ISF isf(lf, lf);
        //    Sop isop =  isf.makeISOPnew();
        IsfLU isf = IsfLU(lf, lf);
        Sop isop =  isf.makeISOP();
        sop_print(stderr, isop, 0 );

    }
}




//cubeBdd $B$K4^$^$l$k%j%F%i%k$9$Y$F$K$D$$$F(Bsmooth$B$7$?$N$rJV$9!#(B
// cubeBdd $B$O(B $B@5%j%F%i%k$N@Q$G$J$$$H$@$a(B
Bdd  smooth_cube_all(Bdd F, Bdd cubeBdd)
{
    Bdd Result = F;
    Bdd temp = cubeBdd;
    int lev;
    while( temp != Bdd::one){
        lev = temp.top();
        Result = Result.smooth(lev);
        //    temp = temp.rstrtop(1);
        temp = temp.rstrtop1();
    }
    return Result;
}



//SupVars $B$K4^$^$l$k%j%F%i%k$9$Y$F$K$D$$$F(Bsmooth$B$7$?$N$rJV$9!#(B
Bdd  smooth_all(Bdd& F,  SupVars& sup_vars)
{
    Bdd Result = F;
    int Xi;
    for(SupVars temp = sup_vars; temp.SupNum() > 0 ; ){
        Xi = temp.delete_top();
        Result = Result.smooth(Xi);
    }
    return Result;
}




// $BO@M}4X?t$N0lHLE*$JA`:n(B


/////////////////////////////////////////////////////////////////////
// $BIT40A4;XDjO@M}4X?t(B $B$N(B $B%/%i%9(B ISF
/////////////////////////////////////////////////////////////////////


ISOP* ISF::makeISOP(){

    if(AllFalse()){
        ISOP* isopfalse = new ISOP(-2);       // $B91564X?t(B
        return isopfalse;
    }
    else if(AllTrue()){
        ISOP* isoptrue = new  ISOP(-1);    // $B91??4X?t(B	
        return isoptrue;
    }   

    int level=this->topVarId(); //$BE83+$9$Y$-JQ?t$NHV9f!J(Brank$B$G$O$J$$!K(B
    //    cerr << "\n*** expandinv id = " << level << "*****\n"
    ISOP* result = new ISOP(level - 1);    //$B$3$l$G$$$$$+$J!)(B
    ISF f0(rstr0(level));
    ISF f1(rstr1(level));
    ISF f00((f0.low & ~(f1.up)),f0.up);
    ISF f11((f1.low & ~(f0.up)),f1.up);
    result->f0  = f00.makeISOP();           // isop0$B$KAjEv(B
    result->f1  = f11.makeISOP();
    Bdd g0 = (result->f0)->makeLF();
    Bdd g1 = (result->f1)->makeLF();
    ISF f000((f0.low & ~(g0)),f0.up);
    ISF f111((f1.low & ~(g1)),f1.up);
    ISF isopAlpha( ( (f000.low & f111.up) | (f000.up & f111.low) )
                   , (f000.up & f111.up)); 

    // $B<B$O!"(BtopVar $B$NHV9f$NJQ?t$,(B low up $B$O(B $B0MB8$7$F$$$F$b!"(B ISOP $B$H$7$F!"(Bf0, f1$B$b(B
    // $B$3$l$K4X78$;$:!"(Bf0=f1=BDDF($B$D$^$j(Bno =-2) $B$G!"(B fr $B$,(B $B$"$k(BISOP$B$H$J$k!#(B
    // $B$3$l$O!">/$75$;}$A0-$$$N$G!"(B $B$3$N$h$&$J>l9g$O!"(Bfr$B$r(B result$B$H$7$FJV$9(B
    // adding by ger 20/50
    // $B$3$N(B $B>J$/ItJ,$O$A$c$s$H%A%'%C%/$7$?(B 21/05
    if( (result->f0->TrueOrFalse()==0) &&  (result->f1->TrueOrFalse()==0) ){
        //      cerr << " DEBUG COND. happen - skipped id = " << level -1 << "\n";
        delete result;
        return isopAlpha.makeISOP(); 
    }
    // adding by ger 20/50
    result->fr = isopAlpha.makeISOP(); 
    return result;
}





// Bdd $B$N(Bstderr$B$X$NHs>iD9@QOB7A$NI=<((B
/*   06/31$B0J9_(B ISOP $B$O!!(Bobsolete
 void printISOP(Bdd lf)
 {
 if(lf == Bdd::null){ 
 fprintf(stderr,"BDD NULL \n");
 }
 else if(lf == Bdd::one){
 fprintf(stderr,"BDDT \n");
 }
 else if(lf == Bdd::zero){
 fprintf(stderr,"BDDF \n");
 }
 else{

 ISF isf(lf,lf);
 ISOP* isoptemp = isf.makeISOP();
 isoptemp->print(0);
 fprintf(stderr,"\n");
 delete isoptemp;
 }
 }
 */






// $BIT40A4;XDj4X?t$K$*$$$F!"$=$N(B neck $B$rI=$9IT40A4;XDj4X?t$r$D$/$j!"(B
// $B$=$l$rJV$9!#(B i $B$OCmL\$7$F$$$kJQ?tHV9f!J#1$+$i!K(B
// $B$3$3$G$O!"(Bneck$B$NA4$F$N>r7o$OI=$;$:!"(Bxi$B$N0lIt$H$J$C$F$$$k!#(B
// $B7W;;7k2L$O(Bxi$B$NB8:_$$5A6u4V$_$?$$$J$b$N$+$J!)(B

ISF ISF::makeNECK(int i){
    Bdd xi = Bdd::var(i);
    // neck $B$O(B xi $B$K(B $B$D$$$F!"(BISF $B$r(B $B@^$jJV$7$?;~!"I,$:?)$$0c$&$H$3$m$,(B
    // $B#1$K$J$k4X?t(B
    Bdd neck = (low.rstr1(i) & ~(up.rstr0(i))) | 
        (low.rstr0(i) & ~(up.rstr1(i))); 
    ISF result = ISF((xi & neck), (xi| (~neck)) );
    // result $B$N(B $B#1(B ---  xi $B$N$&$A(Bneck=1$B$NItJ,!"(B
    // result $B$N(B $B#0(B ---- ~xi $B$N$&$A(Bneck=1$B$NItJ,(B
    return result;
}

// Gray-Code $B$G(B $BI=8=(B(flag = 1 $B$G$=$N<!$rH?E>$5$;$k!K(B
// $BHs8x3+4X?t$K$7$h$&$+$J(B
void ISFprint_table(FILE *sout, int n_in, Bdd lowf, Bdd upf, char assigned, int level,int flag) 
{
    if ( level == 0 ) {
        for ( int i=n_in-1; i>= 0; i--)
            fprintf(sout, " %d ", (assigned>>i) & 1);

        if (lowf == Bdd::zero){
            if(upf == Bdd::zero) fprintf(sout, "| 0\n");
            else fprintf(sout, "| *\n");
        }
        else{
            fprintf(sout, "| 1\n");
        }
        return;
    }
    else {
        /*	int lowf0 = bddrstr(lowf, level, 0);
         int lowf1 = bddrstr(lowf, level, 1);
         int upf0 = bddrstr(upf, level, 0);
         int upf1 = bddrstr(upf, level, 1);
         */

        Bdd lowf0 = lowf.rstr0(level);
        Bdd lowf1 = lowf.rstr1(level);
        Bdd upf0 = upf.rstr0(level);
        Bdd upf1 = upf.rstr1(level);

        if(flag==0){
            ISFprint_table(sout, n_in, lowf0, upf0, assigned, level-1,0);
            ISFprint_table(sout, n_in, lowf1, upf1, assigned | 
                           (1<<(level-1)),level-1,1);
        }
        else{
            ISFprint_table(sout, n_in, lowf1, upf1, assigned | 
                           (1<<(level-1)),level-1,0);
            ISFprint_table(sout, n_in, lowf0, upf0, assigned, level-1,1);
        }
    }

}


// $BIT40A4;XDjO@M}4X?t$N(Btable $B>e$NI=<(!"H?<M#2?JId9g(B
void ISF::print(FILE *sout, int n_in)
{
    // ISP $B$N(Berror$B%A%'%C%/$J$7(B
    //    int lowf = low.value();
    //    int upf  = up.value();

    fprintf(sout,"ISF print_table (gray-code) \n");

    if ( n_in > 9 ){              //$B:GBg$G$b(B32=int$B$N%S%C%HI}$^$G(B
        cerr << "********Sorry: too many input to display.******* \n";
        return;
    }
    int i;
    for(i=n_in-1; i >= 0;i--){
        fprintf(sout, "x%d ",i);
    }
    fprintf(sout, "| F\n");
    for(i=n_in; i > 0;i--){
        fprintf(sout, "---");
    }
    fprintf(sout, "+---\n");

    //    ISFprint_table(sout, n_in, lowf, upf, 0, n_in,0);
    ISFprint_table(sout, n_in, low, up, 0, n_in,0);
}

//$BIT40A4;XDjO@M}4X?t$NHs>iD9@QOBI=<((B
void ISF::print()
{
    ISOP* isop = this->makeISOP();
    isop->print(0);
    delete isop;
}




// support_min $B$N(Bmain $B$N(B $B4X?t$NDj5A(B
// $B$d$j$J$*$7(B
void supportMain(int sup, ISF *f,int* varTable, int Nsup, int elim, 
                 ISF* minf,int* minNsup,int* varResult, int* Usedflag){
    //Nsup -- $B8=:_0MB8$7$F$kJQ?t$N?t(B($B=i4|CM(Bsup)
    //elim -- $B$"$H$H$k$3$H$,2DG=$JJQ?t$N?t(B($B=i4|CM(Bsup)
    int Xelim = varTable[elim-1]; //elim $B$9$k(BBDD$B$NJQ?tHV9f(B
    Bdd newL = f->low.rstr1(Xelim) | f->low.rstr0(Xelim);
    Bdd newU = f->up.rstr1(Xelim) & f->up.rstr0(Xelim);
    ISF newf(newL,newU);

    if(newf.check()){  //Xeim$B=|5n2DG=(B

        Usedflag[elim-1] = 1;

        if( Nsup == (*minNsup)){    // Nsup -1 < *minNsup $B$r0UL#$9$k$O$:(B
            (*minNsup) = Nsup - 1;
            (*minf) = newf;        //$B$3$l$G$$$$$+$J!)(B

            for(register int i=0;i<sup;i++){
                varResult[i] = Usedflag[i];
            }
        }

        //        cerr << Xelim << " removed \n";

        if(elim == 1)return;
        else if((Nsup-elim) >= (*minNsup) )return;
        supportMain(sup, &newf, varTable, Nsup-1, elim-1, minf, minNsup,
                    varResult, Usedflag);
    }
    //Xelim$B=|5n$;$:(B
    //    cerr << Xelim << "not removed \n";

    if(elim == 1) return;
    else if( (Nsup-elim+1) >= (*minNsup) )return;


    for(register int i=0;i<elim;i++){
        Usedflag[i] = 0;        //elim-1 $B$h$jA0$O$9$Y$F=i4|>uBV$K$b$I$9(B
    }

    supportMain(sup, f, varTable, Nsup, elim-1, minf, minNsup, 
                varResult, Usedflag);
}


//$BB?J,!"Nc$($P!"(Bx1$B$N0MB8EY$r$H$C$?8e$N2r$G$b(B x1, ~x1$BBP>N$NItJ,$,$H$b$K%I%s%H%1%"$J(B
//$B>l9g!"$=$N$^$^$N$O$:!#(B
ISF support_min(ISF* f,int sup,int* varTable,int* nsup, int* varResult)
{
    // $B:GNI2r$rJ];}$9$kJQ?t(B
    int minNsup = sup;
    ISF minf = (*f);
    int* Usedflag = new int[sup];
    for(register int i=0;i<sup;i++){
        Usedflag[i] = 0;      //$B=|$$$?>l9g$@$1#1$K$9$k!#(B
        varResult[i] = 0;     //$B=i4|2=!J#1$D$b=|$1$J$+$C$?;~$N$?$a!K(B
    }
    supportMain(sup, f, varTable, sup, sup, &minf, &minNsup,
                varResult, Usedflag);
    (*nsup) = minNsup;
    delete [] Usedflag;
    return minf;
}





//  $B@N$N(Bhash$B$J$7(B 
Sop ISF::makeISOPnewNoHash(int* lev2idx){
    #ifdef DEBUG_ISOP 
    makeISOPcount++;
    #endif 
    if(AllFalse()){       // $B91564X?t(B
        return Sop::zero;
    }
    else if(AllTrue()){
        return Sop::one;  // $B91??4X?t(B   
    }   

    int level=this->topVarId(); //$BE83+$9$Y$-JQ?t$NHV9f!J(Brank$B$G$O$J$$!K(B
    //    cerr << "\n*** expandinv id = " << level << "*****\n"
    ISF f0(rstr0(level));
    ISF f1(rstr1(level));
    ISF f00((f0.low & ~(f1.up)),f0.up);
    ISF f11((f1.low & ~(f0.up)),f1.up);
    Sop result_f0  = f00.makeISOPnewNoHash();           // isop0$B$KAjEv(B
    Sop result_f1  = f11.makeISOPnewNoHash();
    Bdd g0 = (result_f0).getFunc(0);
    Bdd g1 = (result_f1).getFunc(0);
    ISF f000((f0.low & ~(g0)),f0.up);
    ISF f111((f1.low & ~(g1)),f1.up);
    ISF isopAlpha( ( (f000.low & f111.up) | (f000.up & f111.low) )
                   , (f000.up & f111.up)); 

    Sop result_fr = isopAlpha.makeISOPnewNoHash(); 

    int index;
    if(lev2idx == 0){
        index = level;
    }
    else{
        index = lev2idx[level];
    }
    Sop result = result_f0.and0(index) + result_f1.and1(index) + result_fr;
    return result;
}






/*
 Sop ISF::makeISOPnew(){
 if(AllFalse()){       // $B91564X?t(B
 return Sop::zero;
 }
 else if(AllTrue()){
 return Sop::one;  // $B91??4X?t(B	
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
 Sop result = result_f0.and0(level) + result_f1.and1(level) + result_fr;
 return result;
 }

 */







///////////////////////////////////////////////////////////////////////
/////////////// $BHs>iD9@QOB7AI=8=(B $B$N(B $B$?$a$N(B Clss ISOP (06/31$B$+$i(BObsolete) 

// ISOP $B$N(B $BI=8=$9$k(B $BO@M}4X?t$rI=$9(BBdd$B$r$D$/$C$FJV$9!#(B
Bdd ISOP::makeLF(){
    if(i == -1) return Bdd::one;
    if(i == -2) return Bdd::zero;
    Bdd b0,b1,br;
    if(f0 == 0){
        b0 = Bdd::zero;
    }
    else{
        b0 = f0->makeLF();
    }
    if(f1 == 0){
        b1 = Bdd::zero;
    }
    else{
        b1 = f1->makeLF();
    }
    if(fr == 0){
        br = Bdd::zero;
    }
    else{
        br = fr->makeLF();
    }
    Bdd vi = Bdd::var(i+1);        //$B$3$l$G$$$$$O$:(B
    Bdd result = (vi & b1) | ((~vi) & b0) | br ;
    return result;
}

ISOP::ISOP(int j):i(j),f0(0),f1(0),fr(0){
    //    objnum++;
    //    fprintf(stderr,"Construct %d\n",objnum);
}

ISOP::~ISOP(){    // $B$A$c$s$H>C$7$F$?$h(B
    //    objnum--;
    //    fprintf(stderr,"delete %d\n",objnum);
    delete f0;
    delete f1;
    delete fr;
}

// stderr $B$K(B $BI=<((B $B#29`0J>e$"$l$P(B()$B$r$D$1$k!#(B
// $B91??!"91564X?t$G$O2?$b$7$J$$!#(B
// flag = 0 $B$N;~$O(B $B3g8L$r$D$1$J$$!#(B

void ISOP::print(int flag){
    int temp = TrueOrFalse();
    if(temp == 1){cerr << "BDDT \n"; return;}
    else if(temp == 0){cerr << "BDDF \n"; return;}
    int termNum = 0;
    if( (f0->TrueOrFalse() != 0) ) termNum++;
    if( (f1->TrueOrFalse() != 0) ) termNum++;
    if( (fr->TrueOrFalse() != 0) ) termNum++;
    if((termNum > 1) &&(flag) ){
        fprintf(stderr,"(");
    }
    if( (f0->TrueOrFalse() != 0) ){ //f0->i $B$,(B -2 $B$G$J$$$H$-(B
        // f0->i == -2 $B$O$=$l0J2<$,$^$C$?$/$J$$$3$H$r0UL#$9$k!#(B  
        fprintf(stderr,"~x%d",i+1);   //Bdd_var$B$H$"$o$;$k(B
        if((f0->Number())!= -1){  //$B$3$N;~$O!"(Bf0 $B$O(BBDDT$B$r$5$7$F$k$N$GI=<($7$J$$!#(B
            f0->print(1);
        }
    }
    if( (f1->TrueOrFalse() != 0)){
        if((f0->TrueOrFalse() != 0))fprintf(stderr,"+");
        fprintf(stderr,"x%d",i+1);
        if((f1->Number())!= -1){ //
            f1->print(1);
        }
    }
    if((fr->TrueOrFalse() != 0)){
        if( ((f0->TrueOrFalse() != 0))||((f1->TrueOrFalse() != 0)))fprintf(stderr,"+");
        if((fr->Number())!= -1){
            //	    fr->print(0);  $B$3$l$@$H!J!K$D$-$G8F$P$l$?<!$NJQ?t$G(Bfr$B$7$+$+$J$1$l$P(B
            //                  ()$B$$$k$O$:$J$N$K!"(B()$B$D$+$J$/$J$k!#(B
            fr->print(flag);
        }
    }
    
    if((termNum > 1) &&(flag) ){
        fprintf(stderr,")");
    }
}





// Debug $B$N$?$a(B
void ISOP::print_for_debug()
{
    // ISOP$B<+?H$N%]%$%s%?!"(B i,  f0 $B$N%]%$%s%?!"(B f1$B$N%]%$%s%?!"#f#r$N%]%$%s%?(B
    cerr << "ISOP = " << (int) this  ;
    cerr << " i = " <<  (int) this->i  ;
    cerr << " f0 = " <<  (int) this->f0  ;
    cerr << " f1 = " <<  (int) this->f1  ;
    cerr << " fr = " <<  (int) this->fr  ;
    cerr << "\n";
    if(this->f0 != nil) this->f0->print_for_debug();
    if(this->f1 != nil) this->f1->print_for_debug();
    if(this->fr != nil) this->fr->print_for_debug();

}

 
/// ZBDD $B4X78(B

static void reverse(char *s)
{
    int     c,i,j;
    for(i=0,j=strlen(s)-1;i<j;i++,j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char *s)
{
    int     i,sign;
    i=0;
    if((sign = n) < 0)
        n= -n;
    do{
        s[i++] = n % 10 + '0';
    } while((n /= 10) >0 );
    if(sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}



//prestring $B$O7hDj$7$?$H$-$KI=<($9$k(B x1, x2 $B$J$I!J$=$l$^$G$N(Bpath$B>e$NJQ?t(B)
// first = 1 $B$J$i!"=89g$N:G=i$NMWAG$r=q$3$&$H$7$F$$$k!#(B
static void printZBDDrecusive(FILE * sout, char * prestring, Bdd& Zbdd)
{
    if(Zbdd == Bdd::base){
        if(strlen(prestring) == 0){ //Base $B$NI=<((B 0000...$B$N(Bpath$B$KAjEv(B
            fprintf(sout, " ($B&E(B)");
        }
        else{
            fprintf(sout, " (%s)", prestring);
        }
        return;
    }
    else if(Zbdd == Bdd::empty){
        return;
    }
    int lev = Zbdd.top();
    char Now[Digit+2]; 
    strcpy(Now, "x\0");  //x000\n
    char NowLev[Digit+1];  //x000\n
    itoa(lev, NowLev);
    strcat(Now, NowLev);
    char * newString  = new char [Digit+2 + strlen(prestring)];  //x000\n
    newString = strcpy(newString, prestring);
    newString = strcat(newString, Now);
    //  Bdd f0= Zbdd.rstrtop0();   //$B$3$l$G$O$@$a$G$7$g$&!#(B
    //  Bdd f1= Zbdd.rstrtop1();
    Bdd f0= Zbdd.subset0(lev);
    Bdd f1= Zbdd.subset1(lev);
    printZBDDrecusive(sout, prestring, f0);
    printZBDDrecusive(sout, newString, f1);
    delete newString;
}


// ZBDD$B$N=89g$NMWAG$rNs5s$7$FI=<((B 
void printZBDD(FILE * sout, Bdd& Zbdd){
    if(Zbdd == Bdd::empty){
        fprintf(sout, "the Set of this ZBDD = Empty \n");
    }
    else{
        fprintf(sout, "the Set of this ZBDD = \{ ");
        //Base $B$NI=<((B   000000 $B$N(Bpath$B$,(B1$B$K$$$C$F$?$i!"I=<($9$l$P$$$$$G$7$g$&(B.
        //    if(Zbdd.value() < 0){
        //      fprintf(sout, "$B&E(B");
        //    }
        char prestring[1];
        prestring[0] = '\0';
        printZBDDrecusive(sout,  prestring, Zbdd);
        fprintf(sout, " } \n");
    }

}



// ZBDD$B$N>pJs$NI=<((B - $B=89g$NMWAG$J$I(B (for Debug)
void printZBDDinfo(Bdd& Zbdd)
{
    cerr << " *** Print Info. of a ZBDD \n";
    cerr << "    number of 1-paths (elements) = " << Zbdd.card()  << " \n";
    cerr << "    total number of literals (variables) in all elements  = " << Zbdd.lit()  << " \n";
    printZBDD(stderr, Zbdd);
}





// /////////////////////////////////



//var_id $B$NJQ?t$r4^$s$G$$$?$i(B1$B$rJV$9!#(B $B$=$&$G$J$$$J$i(B0
// $B$b$C$H8zN($$$$$[$&$[$&$J$$$+$J!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<(B
inline int SupVars::includeKa (int var_id) const
{
    //  Bdd Xi = Bdd::var(var_id);
    Bdd Xi = Bdd::base.change(var_id);
    Bdd interSec = Bdd::intersec(Xi, this->zbdd);
    if(interSec == Bdd::empty) return 0;
    else return 1;
}






//SupVars$B$G!"(B DeleteSup$B$K4^$^$l$kJQ?t$r:o=|$7$?(BSupVars$B$r$D$/$C$F$+$($9(B
//$B!z<+J,<+?H(B, DeleteSup$B$OJQ99$5$l$J$$(B
// - $B$G!!<BAu$7$?$?$a!"(B DeleteSup$B$K$7$+4^$^$l$J$$JQ?t$,(B
//  $B$"$C$F$b%`%7%5%l%k!%$h$&$O(Bintersec(this, DeleteSup)$B$r:9$70z$$$?=89g$H$$$&$3$H(B
inline SupVars SupVars::delete_sup_vars(SupVars * DeleteSup)
{
    SupVars returnSup = SupVars(this->GetZBdd() - DeleteSup->GetZBdd());
    return returnSup;
}



inline SupVars SupVars::merge_sup_vars(SupVars * merge_Sup) const
{
    SupVars returnSup = SupVars(this->GetZBdd() + merge_Sup->GetZBdd());
    return returnSup;
}

//For debug $B4^$^$l$F$kJQ?tHV9f$rA4$FI=<((B
inline void SupVars::print_sup()
{
    //cerr << "\n *** printting level in sup_vars \n ";
    for(SupVars temp = *this; temp.SupNum() > 0 ; ){
        //     cerr << " " << temp.delete_top();
    }
    //    cerr << "\n ";
}


inline void SupVars::add_supVars(int varid)
{
    //  Bdd Xi = Bdd::var(varid);
    Bdd Xi = Bdd::base.change(varid);

    Bdd newZbdd = this->zbdd + Xi;

    if(newZbdd == this->zbdd){
        //    cerr << "Warning: SupVars -- adding vars already exists - varid = " 
        //	  << varid << "\n";
        return;
    }
    else{
        this->zbdd = newZbdd;
        return;
    }
}

int SupVars::return_top() 
{
    if(SupNum() == 0) {
        return 0;
    }
    else{
        int toplev = zbdd.top();
        return toplev;
    }
}


inline int SupVars::delete_top()
{
    //  if( GetZBdd() == Bdd::base) {
    if( GetZBdd() == Bdd::empty) {
        return 0;
    }
    else{
        int toplev = zbdd.top();
        //    this->zbdd = this->zbdd.rstrtop1(); //$B!z$3$l$@$H$&$^$/$$$+$s$h(B 
        this->zbdd = this->zbdd.subset0(toplev);
        return toplev;
    }
}





inline SupVars::SupVars(Bdd Zbdd):
    zbdd(Zbdd)
{;}

inline SupVars::SupVars():
    //zbdd(Bdd::base){;}
    zbdd(Bdd::empty){;}


inline SupVars SupVars::operator = (const SupVars oprand) {
    if ( this != &oprand ) {
        zbdd = oprand.zbdd;       
    }
    return *this;
}


inline int operator == (const SupVars lhs, const SupVars rhs){
    return ( (lhs.zbdd == rhs.zbdd) );
}


