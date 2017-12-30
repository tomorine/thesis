#include "bddop.h"
#include "cube.h"

main()
{
    Bdd::alloc();

    {
	Bdd a = Bdd::var(1);
	Bdd b = Bdd::var(2);
	Bdd c = Bdd::var(3);

	Bdd f = ( a | b ) & c;

	fprintf(stderr, "size = %d\n", f.size());

	f.printX();

	// $B$J$<$+$3$l$rF~$l$F$*$+$J$$$H$&$^$/%j%s%/$G$-$J$$(B
	CubeSet::printSop(Bdd::null);
    }

    Bdd::gc();
}

