/******************************************************************************»
 **
 **   Filename    : suffixtree.h
 **
 **   Description : This file contains the definition of the class Suffixtree.
 **                 The suffix tree data structure is used to find frequently
 **                 occuring patterns and to hypothesize constituents given
 **                 these patterns.
 **
 **   Version     : $Id: suffixtree.h 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   Special usage and redistribution restrictions apply on this part of the
 **   package (see Authors field above).
 ******************************************************************************»
 */

#ifndef __suffixtree__
#define __suffixtree__

#include "treebank.h"

using ns_treebank::Treebank;
using namespace std;

typedef map<int,int> vecpos;
typedef map<int,vecpos> senpos; 
typedef map<int,int> mnterms;
typedef map<int,int> cb;

namespace ns_suffixtree {

// used to indicate either a opening bracket or a closing bracket
struct constituentborder {
   int p;        // position of bracket in the sentence
   int n;        // non-terminal
};

// class fuzzy tree
class Ftree {
  public:
    map <int,mnterms> prefs;
    map <int,mnterms> sufs;
};

class ctfactory {
  private:
    map<int,cb> ctstore;
    int nterminal;
  public:
    ctfactory(int startnr) {
        nterminal = startnr;
    }
    int get(int o, int c) {
       if (ctstore[o][c] == 0) {
          ctstore[o][c] = nterminal++;
       }
       return ctstore[o][c];
    }
    void put(int o, int c, int n) {
       ctstore[o][c] = n;
    }
};

class Suffixtree {
   public:
      Suffixtree(Treebank& tb) : SS( tb ) {
         nodecount=2;
      }
      int fix; // 1 prefix, 0 suffix

      void construct(const int i);
      void align(Ftree* ft);

   private:
      Treebank& SS;
      int nodecount;

      int M;    // the size of the current line
      int N;    // the id of the current line

      // mappings
      map<int,int> slinks_ft; // slinks from-to
      map<int,int> slinks_tf; // slinks to-from

      class Edge;
      class Suffix;

      void add_prefix( Suffix &active, int pos_stop );

      typedef map<int,Edge> Edgecol;
      map<int,Edgecol> Edges;
      map<int,int> prevnode;

      void  add_edge( Edge &e );
      void  del_edge( Edge &e );
      int split_edge( Edge &e, Suffix &s );

      void add_slink( int node_from, int node_to );
      int get_slink_ft( int node_from );
      int get_slink_tf( int node_to );

      void cano_suffix( Suffix &s );
};

class Suffixtree::Suffix {
  public :
    int origin_node;
    int pos_start;
    int pos_stop;
    Suffix( int node, int start, int stop )
        : origin_node( node ),
          pos_start( start ),
          pos_stop( stop ){};
    int Explicit(){ return (pos_start >  pos_stop); }
};

class Suffixtree::Edge {
  public :
    senpos posities;
    int line_nr;
    int pos_begin;
    int pos_end;
    int node_begin;
    int node_end;
    Edge(){};
    Edge( int line_nr,
          int init_pos_begin,
          int init_pos_end,
          int node_parent,
          int node_child);

    void adjust_points(int l);
};


} // namespace

#endif //__suffixtree__
