#ifndef _MaxHeap_H
#define _MaxHeap_H

#include "stdio.h"
#include "Cmap.H"

/*
  MaxHeap<T>: $B3JG<$9$k$N$O(BT$B$X$N%]%$%s%?$N$_!%<BBN$O%3%T!<$7$J$$!%(B
 */

typedef int ktype;

struct HeapElem
{
    HeapElem(int k, void* i) : key(k), info(i) {}

    ktype key;
    void* info;
};

class MaxHeapBase {
public:
    static const int INITS = 1<<3; // 8
    MaxHeapBase() : elements( new HeapElem*[INITS] ),
                array_size(INITS), n_elem(0), hash(0,ptrHash) {}
    ~MaxHeapBase();

    void	insert(ktype key, void* item); // $BA^F~$7$F(Bupheap
    void*	delmax(); // $B:GBgCM$N:o=|(B
    ktype       maxkey(); // $B:GBgCM$rF@$k(B
    void*       maxinfo(); // $B:GBgCM$r;}$D(Binfo
    void        update(ktype newkey, void* item); // key$B$r99?7(B
    void        del(void* item); // item$B$N:o=|(B
    ktype       itemkey(void* item); // item$B$NCM(B
    int         exist(void* item); // item$B$,$"$k$+$I$&$+(B

    int         size() const { return n_elem; }
    void        fprint(FILE* fp) const;

    ktype       key(int i) { return elements[i]->key; }
    void*       info(int i) { return elements[i]->info; }

private:
    void        upheap(int idx);
    void        downheap(int idx);
    void        resize(int newsize);

    HeapElem**      elements;
    int             array_size;
    int		    n_elem;
    Cmap<void*,int> hash;
};

template <class T>
class MaxHeap {
private:
    MaxHeapBase rep;
public:
    MaxHeap() {}
    ~MaxHeap() {}

    void	insert(ktype key, T* item) { rep.insert(key, item); }
    T*	        delmax() { return (T*)rep.delmax(); } // $B:GBgCM$N:o=|(B
    ktype       maxkey() { return rep.maxkey(); } // $B:GBgCM$rF@$k(B
    T*          maxinfo() { return (T*)rep.maxinfo(); } // $B:GBgCM$r;}$D(Binfo
    void        update(ktype newkey, T* item) {
	return rep.update(newkey, item); // key$B$r99?7(B
    }
    void        del(T* item) { rep.del(item); } // item$B$N:o=|(B
    ktype       itemkey(void* item) { return rep.itemkey(item); } // item$B$NCM(B
    int         exist(void* item) { return rep.exist(item); } // $B$"$k$+$I$&$+(B

    int         size() const { return rep.size(); }
    void        fprint(FILE* fp) const { rep.fprint(fp); }

    ktype       key(int i) { return rep.key(i); }
    T*          info(int i) { return (T*)rep.info(i); }
};


#endif /* _MaxHeap_H */
