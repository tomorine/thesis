//#define DEBUG_ISOP 1

#ifdef DEBUG_ISOP 
  static int makeISOPcount;
  static int  ISOPhashHit;
#endif 

//�� IsfFD �́@support_min �́CIsfLU �ɂ��Ă��狁�߂�D

//(0, 1, *)��p���� �^���l�\�i�O���C�R�[�h)�ɕ\�� 
//8���͈ȉ��̊֐������\�����Ȃ��D�����I�ɁCn_in�̎w������Ȃ��Ƃ��߁D
// (X)�ƕ\�������̂́Clow = 0, up=1�@�̂悤�Ȃ��蓾�Ȃ��r�b�g���Ӗ�����D

//  void Karnaugh_Map(FILE *sout, int n_in){
//      Print_Karnaugh_Map(this, sout, n_in, " 0 * X 1");
//   }

void Print_Karnaugh_Map(FILE *sout, int n_in) const;

//stderr �� �_������\��
void PrintStderr() const;


//int IsfLU::topVarId() const{
//int IsfLU::topVarId() {
int topVarId() {
  int rankl = getL().top();
  int ranku = getU().top();
  if(rankl > ranku){
    return rankl;   // low �̍ŏ�ʂ̕����O���t�̏�ʂȂ̂ł��̔ԍ���Ԃ��B
  }
  else{
    return ranku;
  }
}




// bit1  <= bit2 �ƂȂ��ĂȂ�������0, �Ȃ��Ă���1 
// IsfLU �̘_���I�Ȑ��������`�F�b�N����D
 int check(){
    return ((Bit1() <= Bit2()));
}


// X_lev = 0 �Ƃ����@IsfLU ���@�V��������ĕԂ�
IsfLU rstr0(const int lev) const{
    IsfLU result(Bit1().rstr0(lev), Bit2().rstr0(lev));
    return result;
}

// X_lev = 1 �Ƃ����@IsfLU ���@�V��������ĕԂ�
IsfLU rstr1(const int lev) const{
    IsfLU result(Bit1().rstr1(lev), Bit2().rstr1(lev));
    return result;
}


// IsfLU = 1 �ƂȂ�֐�(Bdd)��Ԃ��B
Bdd OnSet() const{
    return Bit1();
}

// IsfLU = 0 �ƂȂ�֐���Ԃ��B
Bdd OffSet() const{
    return (~Bit2()) ;
} 

// IsfLU = 0 �܂��� �P�ƂȂ�֐���Ԃ��B
Bdd CareSet(){
    return ( (~Bit2()) | Bit1() );
}

// IsfLU = * �ƂȂ�֐���Ԃ��B
Bdd DontCareSet() const{
    return ( (~Bit1()) & (Bit2()) );
}

//lf �� IsfLU �Ɋ܂܂��֐��ł���� true��Ԃ��D
bool  includeFunc (const Bdd& lf) const{
  if( (Bit1() <= lf) && (lf<=Bit2())) return true;
  else return false;
}

//IsfLU �� ���S�ȃh���g�P�A��\������Ȃ�C�P
int AllDC() const{
   return( (Bit1() == Bdd::zero) && (Bit2() == Bdd::one) );
}


//IsfLU �� �P�^�֐����܂�ł�����C�P
int AllTrue(){
   return( (Bit2() == Bdd::one) );
}

//IsfLU �� �P�U�֐����܂�ł�����C�P
int AllFalse(){
   return( (Bit1() == Bdd::zero) );
}


//IsfLU �́@�ے��\��IsfLU ���@�V��������ĕԂ�
// 0 -> 1, 1-> 0 *->* �ƂȂ�D
IsfLU operator ~ () const {
  Bdd newlow = OffSet();        //������0�������Ƃ���
  Bdd newup = newlow | DontCareSet();
  IsfLU  result(newlow, newup); 
  return result;
}


Sop makeISOP(){return this->makeISOP(0);} 
//��IsfLU -> ISOP ������ĕԂ��D

Sop makeISOP(Arrayint lev2idx){return this->makeISOP(lev2idx.getPtr());} 
//��IsfLU -> ISOP ������ĕԂ��D���̍ۂɁCIsfLU�̒��ł�Bdd�̕ϐ��ԍ��� lev2idx��
// �^������K���œǂݑւ��āCISOP������D
//  (lev2idx[2]=3 �Ȃ�CIsfLU ��x2 �� Sop�ł́Cx3�ƈ�����D

Sop makeISOP(int* lev2idx);  //���2�̎��̂͂���


// �ǂ���ɂ� �܂܂�� ���e�֐��W�������߂�B
// ���̂悤�ȏW�����Ȃ���΁Alow=Bdd::one ,up=Bdd::zero�Ƃ���B
//�@���������check�łЂ�������̂�


friend IsfLU operator & (const IsfLU& lhs, const IsfLU& rhs)
{
IsfLU result((lhs.Bit1() | rhs.Bit1()),(lhs.Bit2() & rhs.Bit2()));
if( !(result.check())){
result.change_bit1(Bdd::one);
	result.change_bit2(Bdd::zero);
    }
    return result;
}


friend int operator <= (const IsfLU& lhs, const IsfLU& rhs) {
    return ( (rhs.Bit1() <= lhs.Bit1()) && (lhs.Bit2() <= rhs.Bit2()) );
}



/*

friend IsfLU support_min(IsfLU* f,int sup,int* varTable,int* nsup, int* varResult);
// �s���S�w��_���֐� f �� support���ŏ��̕s���S�w��_���֐���Ԃ��B
// sup   --- �Œ�ł��֌W���Ă���ϐ��̐�
// varTable -- �Œ�ł��֌W���Ă���ϐ��ԍ��iBDD�̔ԍ��Ȃ̂ŁA�P����j
// varTable[0]-varTable[sup-1] �̔z��
// nsup -- �ŏ���support��
// varResult[i] = 0 �Ȃ� varTable[i] �̕ϐ��͎g�p�����B
// varResult[i] = 1 �Ȃ� varTable[i] �̕ϐ��͎g�p����Ȃ��B
// ���́A�O�A�P���ݒ肷��B
// varTable[sup-1] ���� �����邩�`�F�b�N���Ă����̂ŁABDD�̕ϐ��������ɂ���
// �z��ɓ����Ă�������������Ԃ������Ȃ邱�Ƃ����҂����

 */
