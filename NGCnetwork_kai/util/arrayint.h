#ifndef _arrayint_h
#define _arrayint_h

#include <stdlib.h>
#include <assert.h>

// center: sop��bdd�ȂǂŎg���Ă���

class Arrayint {
private:
    int*	ent; // sizerep��0�̎��ł��C�|�C���^�����D1999.1.14
    // �ł����́C����ς�0��������Ȃ��D
    // free(0)����OK?
    int		sizerep;
public:
    Arrayint() : ent(0), sizerep(0) {}
    Arrayint(int newsize);
    //     	Arrayint(int newsize=0);
    Arrayint(const Arrayint& obj);
    Arrayint(int size, int* array);
    ~Arrayint() { free(ent); }

    int&	operator [] (int index) const;
    Arrayint&	operator = (const Arrayint& obj);
    int	operator == (const Arrayint& obj);

    int		size() const { return sizerep; }
    int*	getPtr() { return ent; } //todo:  v.data()�ň�����n��
    void	clear() { sizerep = 0; free(ent); ent = 0; }
    void	add(int adding); // = push_back
    void	add_a(const Arrayint& obj); //
    int		del(int deling);
    void	del_i(int index);
    void	replace(int deling, int adding);
    void	reserve(int newsize);
    int	position(int item);
    int	contain(int item) { return (position(item) == -1) ? 0 : 1; }
    void	fill(int value);
    // �����܂ł̊֐���Array.h�Ɠ���
    void	print();// �t�@�C����Array�̒��g���o�́i�G���[�̎��Ɏg�p�j


    // permutation�Ƃ��Ă݂��Ƃ��̋t�ʑ������߂ĕԂ��D�������g�͕ω����Ȃ�.
    Arrayint	inverse();// ���̔z���index�Ƃ���ɑΉ������l�̊֌W���t�]�����z���Ԃ� -> 

    // �ȉ��͂��ׂĎ������g��ύX
    void	sort(int (*cmp)(const void* a, const void* b));// Array�ɂ����� -> sort()
    void	jumpup(int i, int j);// i�ɓ����Ă���l��j�ֈړ�������(i < j) -> erase(),insert()
    void	jumpdown(int i, int j);// i�ɓ����Ă���l��j�ֈړ�������(i > j) -> erase(),insert()
    void	jumpud(int i, int j);// i,j�̒l�ɂ���āAjumpup��jumpdown�ɐU�蕪����
    // util.h��jumpud(v, i, j)
    void	shift(int index, int degree, int defv);// index����degree���������炷(degree > 0�Ȃ�E�ɂ��炷�A�󂢂��Ƃ����defv)(degree < 0�Ȃ獶�ɂ��炷�A�����O�̒l�͏�����) -> insert()
    //     Arrayint&	changeOrder(const Arrayint& permutation);
    void	changeOrder(const Arrayint& permutation);// index�Ƃ��̒��̒l�������ɂȂ�悤�Ƀ\�[�g -> sort()?
};

#endif /* _arrayint_h */
