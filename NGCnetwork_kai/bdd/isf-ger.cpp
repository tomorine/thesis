#include "isf.h"
#include <stdlib.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "Array.H"

// #include "Array.C"
//template class Array<TwoBdd>;

#ifdef DEBUG_ISOP
int  IsfLU::makeISOPcount=0;
int  IsfLU::ISOPhashHit=0;
#endif // DEBUG_ISOP


Sop IsfLU::makeISOP(int* lev2idx)
{
    #ifdef DEBUG_ISOP 
    makeISOPcount++;
    #endif // DEBUG_ISOP

    //------ terminal cases ------
    if(AllFalse()){       // �P�U�֐�
        return Sop::zero;
    }
    else if(AllTrue()){
        return Sop::one;  // �P�^�֐�   
    }   

    //------ normalize (swap f and g if needed) ------
    ///  �Ώ̂�2�����Z�łȂ����ߕK�v�Ȃ��͂�??
    //  int flev = f.topvar();
    //  int glev = g.topvar();
    //  if ( flev < glev ) return g * f;

    Bdd Low = getL(); 
    Bdd Up = getU(); 

    //------ read the result table ------
    Bdd cache = Bdd::readCache(Bdd::ISOP, Low, Up);
    if ( cache != Bdd::null ){
        #ifdef DEBUG_ISOP
        ISOPhashHit++;
        //       std::cerr << " Hash Hit \n";
        #endif // DEBUG_ISOP
        return Sop(cache);
    }


    //-------- recursive computation --------

    int level = this->topVarId(); //�W�J���ׂ��ϐ��̔ԍ��irank�ł͂Ȃ��j
    //   int level=this.topVarId(); //�W�J���ׂ��ϐ��̔ԍ��irank�ł͂Ȃ��j

    //    std::cerr << "\n*** expandinv id = " << level << "*****\n"
    IsfLU f0(rstr0(level));
    IsfLU f1(rstr1(level));
    Bdd f0L = f0.getL(); 
    Bdd f0U = f0.getU(); 
    Bdd f1L = f1.getL(); 
    Bdd f1U = f1.getU(); 
    IsfLU f00( (f0L & ~f1U) ,f0U);
    IsfLU f11( (f1L & ~f0U) ,f1U);
    Sop result_f0  = f00.makeISOP();           // isop0�ɑ���
    Sop result_f1  = f11.makeISOP();
    Bdd g0 = (result_f0).getFunc(0);
    Bdd g1 = (result_f1).getFunc(0);
    //  IsfLU f000( (f0L & ~g0), f0U);    ���͂���K�v�Ȃ��D
    //  IsfLU f111( (f1L & ~g1), f1U);
    Bdd f000L = f0L & ~g0;
    //   Bdd f000U = f0U;             ���͂���K�v�Ȃ��D
    Bdd f111L = f1L & ~g1;
    //   Bdd f111U = f1U;             ���͂���K�v�Ȃ��D
    IsfLU isopAlpha( ((f000L & f1U) | (f0U & f111L))  , (f0U & f1U)); 

    Sop result_fr = isopAlpha.makeISOP(); 

    int index;
    if(lev2idx == 0){
        index = level;
    }
    else{
        index = lev2idx[level];
    }
    Sop result = result_f0.and0(index) + result_f1.and1(index) + result_fr;

    //write to the result table
    Bdd::writeCache(Bdd::ISOP, Low, Up, result.getRep());
    return result;
}


















// ����bit���ɑ΂��āAbinary���Q�i���Ƃ������āA�����gray code�ɂ������̂�Ԃ��B
// binary �̏�ʃr�b�g�̂P���̃r�b�g�ibit+1�r�b�g��)�͕K���O�łȂ��Ƃ��߁B
static int binary2gray(int binary, int bit)
{
    //�^����ꂽ���Ƃ�binary�̂P���̃r�b�g��1�ł���Ƃ��A�����𔽓]����΁A
    // gray�R�[�h�ɂȂ�B
    int gray = binary;
    int checkbit; //���ݒ��ڂ��Ă�r�b�g�̂P�O�̃r�b�g
    int mask;
    for(int i = bit; i >= 1 ; i--){ //����i�r�b�g�ڂ𒍖�
        checkbit = (binary>>i) & 1;  //���̃r�b�g
        if(checkbit == 1){   //gray �� i�r�b�g�ڂ𔽓]
            mask = 1<<(i-1); //i�r�b�g�߂�����1
            gray = gray ^ mask; //gray �� i�r�b�g�ڂ������]
        }
    }
    return gray;
}

static void bddprint_table_gray_array(FILE *sout, int n_in, Array<TwoBdd> array, 
                                      unsigned int assigned, int level, int flag, char * string ) 
{
    int array_num = array.size();
    TwoBdd f;  //temporaly
    int j;
    if ( level == n_in ) {         //�W�J�I���i�I�[�m�[�h���w���Ă�͂��j
        for ( int i=n_in-1; i>= 0; i--)
            fprintf(sout, " %d ", (assigned>>i) & 1);
        for(j = 0; j < array_num ; j++){
            f = array[j];
            if ( f.Bit1() == Bdd::zero && f.Bit2() == Bdd::zero ) {
                fprintf(sout, "| %c%c ", string[0], string[1]);
            }
            else if ( f.Bit1() == Bdd::zero && f.Bit2() == Bdd::one ) { 
                //	fprintf(sout, "| 01 ");
                fprintf(sout, "| %c%c ", string[2], string[3]);
            }
            else if ( f.Bit1() == Bdd::one && f.Bit2() == Bdd::zero ) { 
                //	fprintf(sout, "| 10 ");
                fprintf(sout, "| %c%c ", string[4], string[5]);
            }
            else if ( f.Bit1() == Bdd::one && f.Bit2() == Bdd::one ) {
                //	fprintf(sout, "| 11 ");
                fprintf(sout, "| %c%c ", string[6], string[7]);
            }
            else  std::cerr << "TwoBitsFunc print_table error -- something wrong \n";
        }
        fprintf(sout, "|\n");
        return;
    }
    else {
        TwoBdd f1, f0; //temporaly
        Array<TwoBdd> array_0, array_1;
        for(j = 0; j < array_num ; j++){
            f = array[j];
            //    f0 = f.rstr0(level+1);   
            f0.change_bit1(f.Bit1().rstr0(level+1));   
            f0.change_bit2(f.Bit2().rstr0(level+1));   
            //    f1 = f.rstr1(level+1);   
            f1.change_bit1(f.Bit1().rstr1(level+1));   
            f1.change_bit2(f.Bit2().rstr1(level+1));   
            array_0.add(f0);
            array_1.add(f1);
        }
    
        if(flag==0){
            bddprint_table_gray_array(sout, n_in, array_0, assigned, level+1, 0, string);
            bddprint_table_gray_array(sout, n_in, array_1, assigned|(1<<(n_in-level-1)), level+1, 1, string);
        }
        else{
            bddprint_table_gray_array(sout, n_in, array_1, assigned|(1<<(n_in-level-1)), level+1, 0, string);
            bddprint_table_gray_array(sout, n_in, array_0, assigned, level+1, 1, string);
        }

    }
}



static void Karnaugh_Map(FILE *sout, int n_in, const TwoBdd& F, char * string)
{
    //  fprintf(sout,"******************************************* \n");
    fprintf(sout,"*****  Printing Karnaugh Map of 2 bits func \n");
    fprintf(sout,"***** coding rule:: (bit1,2) = 00 -> %c%c, 01 -> %c%c, 10 -> %c%c, 11 -> %c%c\n" ,string[0],string[1],string[2],string[3],string[4], string[5],string[6], string[7]);
    //  fprintf(sout,"*****    -- xi corresponds to Bdd::var(i) \n");
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout," \n");
    if ( n_in > 9 ){              //�ő�ł�32=int�̃r�b�g���܂�
        std::cerr << "********Sorry: too many input to display.******* \n";
        return;
    }
    int upper = n_in /2;      //�\�̏�ɂ���ϐ��̐� (�������͏��Ȃ�)
    int lower = n_in - upper;
    int array_num =  1 << upper; //���ɂȂ�ԕϐ��̂���
    int i;

    //�㕔�ɐ�

    int lowerHaba = 3;   //�����̂P�ϐ��Ɋ��蓖�Ă�͈�
    int upperHaba = 5; //�㑤�̂P�̕����֐��Ɋ��蓖�Ă�͈�

    for(i=lower * lowerHaba + 1 + array_num * upperHaba; i > 0;i--){
        fprintf(sout, "-");
    }
    fprintf(sout, "\n");
    //2�s��
    for(i=1; i <= (lower*lowerHaba);i++){  
        fprintf(sout, " ");
    }
    fprintf(sout, "|");
    for(i=lower+1; i <= n_in;i++){
        fprintf(sout, "  x%d ",i);
    }            
    fprintf(sout, "\n");
    //3�s��
    for(i=1; i <= lower;i++){
        fprintf(sout, "x%d ",i);
    }
    fprintf(sout, "|");

    //�������牡�����ւ̕\��
    Array<TwoBdd> Bdd_array;
    TwoBdd tempBdd(Bdd::zero, Bdd::zero);
    int k, j;  //for loop only
    int varid;
    int checkbit;
    int assigned;  
    for(i=1; i<= array_num; i++){
        //assigned�̌v�Z 
        assigned = binary2gray(i-1, upper);  //assigned = gray code��
        fprintf(sout, " ");
        for(k=upper-1; k>=0; k--){ //assigned�� upper bit�����\��
            fprintf(sout, "%d", ((assigned>>k) &1) );
        }
        for(j=1+upper;j<= (upperHaba-1); j++){
            fprintf(sout, " ");
        }
        //assigned����ABdd�̌v�Z
        tempBdd = F;
        for(k=1; k<=upper; k++){ //upper�̂�����k�Ԗڂ̏���
            varid = lower + k;
            checkbit = (assigned >>(upper-k)) & 1;
            if(checkbit == 1){
                tempBdd.change_bit1(tempBdd.Bit1().rstr1(varid));
                tempBdd.change_bit2(tempBdd.Bit2().rstr1(varid));
            }
            else{
                tempBdd.change_bit1(tempBdd.Bit1().rstr0(varid));
                tempBdd.change_bit2(tempBdd.Bit2().rstr0(varid));
            }
        }
        //assigned����ABdd�̌v�Z�I���
        Bdd_array.add(tempBdd);
    }
    fprintf(sout, "\n");

    //���E��
    for(i=lower; i>0 ; i--){
        fprintf(sout, "---");
    }    
    fprintf(sout, "+");
    for(i=array_num; i >= 1;i--){
        fprintf(sout, "-----");
    }            
    fprintf(sout, "\n");

    bddprint_table_gray_array(sout, lower, Bdd_array, 0, 0, 0, string);

    for(i=(lower*3 + 1 + array_num * 5); i > 0;i--){
        fprintf(sout, "-");
    }
    fprintf(sout, "\n");  
}



//TwoBdd �� �^���l�\�ŕ\�� (8���͈ȉ��݂̂������D)
//string �̑O����A2���̕������A(bit1,2) = (00, 01, 10. 11)�ɑΉ�������
//�� string = " 0*0 1*1"

void Print_Karnaugh_Map_String(const TwoBdd& twobits, FILE *sout, int n_in, char * string)
{
    Karnaugh_Map(sout, n_in, twobits, string);
}



void IsfLU::Print_Karnaugh_Map(FILE *sout, int n_in) const{
    Print_Karnaugh_Map_String(*this, sout, n_in, (char*)" 0 * X 1");
}

#include "ger-base.h"


void IsfLU::PrintStderr() const{
    fprintf(stderr, "** Printing ISF ** \n");
    fprintf(stderr, "ON-Set =\n  ");
    printISOP(this->OnSet());
    fprintf(stderr, "\nDC-Set =\n  ");
    printISOP(this->DontCareSet());
}
