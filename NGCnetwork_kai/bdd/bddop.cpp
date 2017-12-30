//////////////////////////////////////////////////////////////////
////////        BDD manipulator for logical operations    ////////
//////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/wait.h>

#include "bddop.h"

//////////////////////////////////////////////////////////////////

int Bdd::objnum = 0;

void Bdd::printX() const
{
    // char* bdlfilename = tempnam("/tmp", "bdl_"); // todo �댯
    char bdlfilename[] = "/tmp/bdl_XXXXXX";
    mkstemp(bdlfilename);

    FILE* bdlfile = fopen(bdlfilename, "w");
    bddprintbdl(edge, bdlfile);
    fclose(bdlfile);
    if ( fork() == 0 ) {
        execlp("xbddfig", "xbddfig", bdlfilename, 0);
    }
    else {
        sleep(1);
        // 	int status;
        // 	wait(&status);
    }
    unlink(bdlfilename);
}

vector<int> Bdd::getPermuForCompress() const
{
    int n = top();
    vector<int> result(n+1);
    for ( int i=n; i>=0; i-- ) result[i] = i; // 0 1 .. n

    vector<int> variables(n+1);
    for ( int i=n; i>0; i-- ) variables[i] = 0;

    Bdd vars = varSet();
    while ( vars != Bdd::empty ) {
        int topvar = vars.top();

        variables[topvar] = 1;
        vars = vars.subset0(topvar);
    }

    int dest = n;
    for ( int i=n; i>0; i-- ) {
        if ( variables[i] == 0 ) jumpUD<vector<int>>(result, i, dest--);
    }
    return inverse(result);
}

/* �^����ꂽpermutation�ɏ]���ĕϐ�����ύX����D
 permutation��lev2idx�ƌ��āC����ɍ��킹��悤�ɕϐ�����ύX����̂ł͂Ȃ��D
 �ϐ�����lev2idx�ɂ��邽�߂ɂ́Cpermutation��lev2idx�̋t�ʑ���^����D
 lev2idx 1342,    1234 ->[1423(�t�ʑ�)] -> 1342
 �t�ɁC�ϐ�����lev2idx�ł�����̂�normalize����ɂ́C
 permutation��lev2idx���̂��̂�^����΂悢�D
 lev2idx 1342,    1342 ->[1342] -> 1234 */
Bdd Bdd::changeOrder(const vector<int>& permutation) const
{
    int n = permutation.size();
    vector<int> localpermu(permutation);

    Bdd result = *this;
    Bdd sup = this->varSet();
    // �傫�������猈�߂Ă���
    for ( int dest=n; --dest>0; ) {
        if ( localpermu[dest] == dest ) continue;
        for ( int idx=dest; --idx>0; ) {
            if ( localpermu[idx] == dest ) {
                // 		fprintf(stderr, "jumpup( %d, %d ) sup:%d\n", idx, dest,
                // 			sup.subset1(idx) == Bdd::base );
                result = result.jumpup(idx, dest);
                jumpUD<vector<int>>(localpermu, idx, dest);
                break;
            }
        }
    }
    /* �����ł��낢��ƍH�v���Ă݂����C�����Ȃ�Ȃ��D
     Bdd�p�b�P�[�W�̒��ōH�v���Ȃ��ƃ_���݂����D*/

    //     fprintf(stderr, "\n");
    //     for ( int idx=n; --idx>0; ) assert( localpermu[idx] == idx );
    return result;
}

Bdd Bdd::getAelement() const // �W������P�����v�f������ĕԂ�
{
    int v = top();
    if ( v == 0 ) return *this;   // Bdd::base, Bdd:empty�͂��̂܂ܕԂ�
    return subset1(v).getAelement().change(v);
    // subset1�̕��ɍs���΁C�K��Bdd::base������D
}

Bdd Bdd::dupLit() const
{
    // multiLit���g���čs�Ȃ��Ă���D
    // ������ŗǂ��̂ŁC���������ǂ�����������C������D
    int n = top();
    char* literals = (char*)calloc(n+1, sizeof(char));
    multiLit(literals);

    Bdd dup_lit = Bdd::null;
    for ( int lev = n; lev >=1; lev-- ) {
        if ( literals[lev] >= 2 ) {
            dup_lit = Bdd::var(lev);
            break;
        }
    }
    free(literals);
    return dup_lit;
}	
