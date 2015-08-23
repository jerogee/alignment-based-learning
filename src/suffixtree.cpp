/******************************************************************************»
 **
 **   Filename    : suffixtree.cpp
 **
 **   Description : This file contains the definition of the class Suffixtree.
 **                 The suffix tree data structure is used to find frequently
 **                 occuring patterns and to hypothesize constituents given
 **                 these patterns.
 **
 **   Version     : $Id: suffixtree.cpp 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   Special usage and redistribution restrictions apply on this part of the
 **   package (see Authors field above).
 ******************************************************************************»
 */

#include <iostream>
#include <string>
#include "suffixtree.h"
#include "word.h"

namespace ns_suffixtree {

void Suffixtree::add_slink( int node_from, int node_to ) {
   slinks_ft[ node_from ] = node_to;
   slinks_tf[ node_to ] = node_from;
}

int Suffixtree::get_slink_ft( int node_from ) {
   return slinks_ft[ node_from ];
}

int Suffixtree::get_slink_tf( int node_to ) {
   return slinks_tf[ node_to ];
}

void Suffixtree::add_edge( Edge &e ) {
   int a,b;
   a = e.node_begin;
   b = SS[ e.line_nr ][ e.pos_begin ].getIdx();
   Edges[ a ][ b ] = e;
}

void Suffixtree::del_edge( Edge &e ) {
   int a,b;
   a = e.node_begin;
   b = SS[ e.line_nr ][ e.pos_begin ].getIdx();
   Edges[ a ].erase( b );
}

Suffixtree::Edge::Edge( int n, int init_first, int init_last, int parent_node, int child_node) {
   line_nr = n;
   pos_begin = init_first;
   pos_end = init_last;
   node_begin = parent_node;
   node_end = child_node;
   posities[ n ][ pos_end ]++;
}

void Suffixtree::Edge::adjust_points(int l) {
   for(map<int,vecpos>::iterator it1 = posities.begin(); it1 != posities.end(); ++it1) {
      vecpos t;
      t = posities[ (*it1).first ];
      posities[ (*it1).first ].clear();
      for (map<int,int>::iterator it2 = t.begin(); it2 != t.end(); ++it2) {
         posities[ (*it1).first ][ (*it2).first - l ]++;
      }
   }
}

int Suffixtree::split_edge( Edge &e, Suffix &s ) {
   del_edge(e);
   Edge *new_edge =
        new Edge( e.line_nr,
                  e.pos_begin,
                  e.pos_begin + s.pos_stop - s.pos_start,
                  s.origin_node,
                  nodecount++);

   new_edge->posities = e.posities;
   int edge_span = e.pos_end - e.pos_begin;
   new_edge->adjust_points(edge_span - (s.pos_stop - s.pos_start));
   new_edge->posities[ N ][ s.pos_stop ]++;
   add_edge(*new_edge);
   prevnode[ new_edge->node_end ] = new_edge->node_begin;
   add_slink( new_edge->node_end, s.origin_node );
   e.pos_begin += s.pos_stop - s.pos_start + 1;
   e.node_begin = new_edge->node_end;
   add_edge(e);
   prevnode[ e.node_end ] = e.node_begin;
   int retval = new_edge->node_end;
   delete new_edge;
   return retval;
}

void Suffixtree::cano_suffix( Suffix &s ) {
   if ( !s.Explicit() ) {
      if (Edges[ s.origin_node ].count( SS[ N ][ s.pos_start ].getIdx() ) ) {
         Edge *e = &Edges[ s.origin_node ][ SS[ N ][ s.pos_start ].getIdx() ];
         int edge_span = e->pos_end - e->pos_begin;
         while ( edge_span <= ( s.pos_stop - s.pos_start ) ) {
            e->posities[ N ][ s.pos_stop ]++;
            s.pos_start = s.pos_start + edge_span + 1;
            s.origin_node = e->node_end;
            if ( s.pos_start <= s.pos_stop ) {
               if (Edges[ e->node_end ].count( SS[ N ][ s.pos_start
                       ].getIdx() ) ) {

                  e = &Edges[ e->node_end ][ SS[ N ][ s.pos_start
                       ].getIdx() ];

                  edge_span = e->pos_end - e->pos_begin;
               }
            }
         }
      }
   }
}
void Suffixtree::add_prefix( Suffix &active, int pos_stop ) {
   int parent_node;
   int last_parent_node = -1;
   int finished = 0;
   int just_jumped = 0;

   while (!finished) {

      Edge edge;

      parent_node = active.origin_node;

      if (just_jumped) {
         int cnode = active.origin_node;
         int pnode;

         while (cnode > 1) {
            pnode = cnode;
            cnode = prevnode[pnode];

            map<int,Edge>::iterator ite = Edges[ cnode ].begin();
            int edgefound = 0;

            while ( ite != Edges[ cnode ].end() && !edgefound ) {
               Edge *e = &Edges [ cnode ][ (*ite).first ];
               if (e->node_end == pnode) {
                  edgefound++;
                  e->posities[ N ][ pos_stop-1 ];
               } else {
                  ++ite;
               }
            }
         }
         just_jumped--;
      }

      if ( active.Explicit() ) {
         if ( Edges[active.origin_node].count( SS[ N ][ pos_stop ].getIdx() ) ) {
            finished++;
         }
      } else {
         if ( Edges[ active.origin_node ].count( SS[ N ][ active.pos_start ].getIdx() ) ) {
            edge = Edges[ active.origin_node ][ SS[ N ][ active.pos_start ].getIdx() ];
         }

         int span = active.pos_stop - active.pos_start;

         if ( SS[ edge.line_nr ][ edge.pos_begin + span + 1 ] == SS[ N ][ pos_stop ] ) {
            finished++;
            if (Edges[ 1 ].count( SS[ N ][ pos_stop ].getIdx() ) ) {
               if (get_slink_ft( Edges[ 1 ][ SS[ N ][ pos_stop ].getIdx() ].node_end ) == 1) {
                  Edges[ 1 ][ SS[ N ][ pos_stop ].getIdx() ].posities[ N ][ pos_stop ];
               }
            }
         } else {
            parent_node = split_edge(edge, active );
         }
      }

      if (!finished) {

         Edge *new_edge = new Edge( N, pos_stop, M, parent_node, nodecount++ );
         add_edge( *new_edge );
         prevnode[ new_edge->node_end ] = new_edge->node_begin;
         if ( last_parent_node > 0 ) {
           add_slink( last_parent_node, parent_node );
         }

         last_parent_node = parent_node;

         if ( active.origin_node == 1 ) {
            active.pos_start++;
         } else {
           if ( (active.origin_node > 1) && (get_slink_ft ( active.origin_node ) > 1) ) {
              just_jumped++;
           }
           active.origin_node = get_slink_ft ( active.origin_node );
         }

         cano_suffix( active );
      }
   }

   if ( last_parent_node > 0 ) {
      add_slink( last_parent_node, parent_node );
   }

   active.pos_stop++;
   cano_suffix( active );
}


void Suffixtree::align(Ftree* ft){
   int nterm = 0;

   map<int,Edge>::iterator ite = Edges[ 1 ].begin();

   while ( ite != Edges[ 1 ].end() ) {
      Edge e = Edges [ 1 ][ (*ite).first ];

      if ( (e.posities.size() > 1) ) {
         nterm++;

         for(map<int,vecpos>::iterator it1 = e.posities.begin(); it1 != e.posities.end(); ++it1) {
            for (map<int,int>::iterator p = e.posities[ (*it1).first ].begin();
               p != e.posities[ (*it1).first ].end(); ++p) {

               constituentborder c_s, c_p;
               c_s.p = (*p).first + 1;
               c_s.n = nterm;
               c_p.p = (*p).first - (e.pos_end - e.pos_begin);
               c_p.n = nterm;
               if (fix) {
                  ft[ (*it1).first ].prefs[ (*p).first + 1 ][ nterm ]++;
               } else {
                  ft[ (*it1).first ].sufs[ (*p).first + 1 ][ nterm ]++;
               }
            }
         }
      }
      ++ite;
   }
}

void Suffixtree::construct(const int i){

   // Let S be the i-th sentence
   M = SS[i].size() - 1;
   N = i;

   // The AP is the first non-leaf suffix in the tree. Set the
   // initial active suffix to be the empty string at node 0
   Suffix active( 1, 0, -1 );

   // Consider every suffix in S
   for ( int j = 0 ; j <= M ; j++ ) {
      add_prefix( active, j );
   }
}

} // namespace
