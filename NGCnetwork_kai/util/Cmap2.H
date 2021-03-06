#ifndef _Cmap2_h
#define _Cmap2_h 1

#include <assert.h>

// hash-fuc $B$r0J2<$N$h$&$KJQ99$7$?%P!<%8%g%s(B
// hash-func = int (*hashf)(const K& key, const int& hastInt)

/*
#define INITCAPA 8
typedef void* Pix;
typedef unsigned int uint;

template <class K, class V>
struct KVNode
{
    K			hd;
    V			cont;
    KVNode<K,V>*	tl;

    KVNode(const K& h, const V& c, KVNode<K,V>* t=0) 
    : hd(h), cont(c), tl(t) {}
};
*/
//$B>e$GDj5A$9$kBe$o$j$K!$(BCmap.H$B$N$r$D$+$&!%(B
#include "Cmap.H"

template <class K, class V>
class Cmap2 {
public:
    Cmap2(int hashint, V dflt, int (*hashf)(const K& key, const int& hastInt), uint sz=INITCAPA);
    Cmap2(const Cmap2& a);
    ~Cmap2() { clear(); delete tab; }
    
    int		size() const { return count; }// current number of items
    void	clear(); 
    void	resize();

    Cmap2&       operator = (const Cmap2& a);
    V&		operator [] (const K& key); // $BL5$$$H$-$O!$(Bdef$B$X$N(Bmap$B$r:n$k(B
    V		get(const K& key) const;    // $BL5$$$H$-$O!$(Bdef$B$N%3%T!<$rJV$9!%(B
    Pix		seek(const K& key) const;
    void	del(const K& key);

    Pix		first() const;
    void	next(Pix& i) const;
    K&		key(Pix p) const
    	{ assert( p != 0 ); return ((KVNode<K,V>*)p)->hd; }
    V&		value(Pix p) const
    	{ assert( p != 0 ); return ((KVNode<K,V>*)p)->cont; }

private:
  // Added by ger start
  uint   HASHint; //hashfunc$B$,MxMQ$9$k!%(B
  // Added by ger end
    uint		count;
    uint		tab_size;
    V			def;
    KVNode<K,V>**	tab;
    int         	(*hashfunc)(const K& key, const int& hastInt);

// HASHTABLE_TOO_CROWDED(COUNT, SIZE) is true iff a hash table with COUNT
// elements and SIZE slots is too full, and should be resized.
// This is so if available space is less than 1/8.

    int crowded() { return (tab_size - (tab_size>>3) <= count); }

// The nodes are linked together serially via a version
// of a trick used in some vtables: odd pointers are
// actually links to the next table entry. 
// Not terrible, but not wonderful either

    static int goodCHptr(void* t) { return ((((unsigned)t) & 1) == 0); }
    static void* index_to_CHptr(int i) { return (void*)((i << 1) + 1); }
    static int CHptr_to_index(void* t) { return ( ((unsigned) t) >> 1); }
};

//inline int ptrHash(const Pix& key) { return ((int)key / sizeof(Pix)); }
//inline int intHash(const int& key) { return key; }

#include "Cmap2.cpp"

#endif
