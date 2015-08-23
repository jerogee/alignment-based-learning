/******************************************************************************»
 **
 **   Filename    : all_alignment.h
 **
 **   Description : This file contains the definition of the class 
 **                 all_alignment.
 **
 **   Version     : $Id: all_alignment.h 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#ifndef __all_alignment__
#define __all_alignment__

#include <algorithm>
#include <set>
#include <vector>
#include "edit_distance.h"
#include "sentence.h"
#include "tools.h"
#include "tree.h"

using ns_sentence::Sentence;
using ns_tree::Tree;

namespace ns_edit_distance {

typedef pair<int, int> Link;
typedef vector<Link> Linklist; 
typedef set<Linklist> Setlinklist; 

template <class E>
class In {
public:
   In(const vector<E>& e):elem(e) { }
   bool operator() (const E &e) const {
      return find(elem.begin(), elem.end(), e) != elem.end();
   }
private:
   vector<E> elem;
};

class Overlap {
public:
   Overlap(const Link& l):link(l) { }
   bool operator() (const Link& e) const {
      return (((e.first<=link.first)&&(e.second>=link.second))
        ||((e.first>=link.first)&&(e.second<=link.second)));
   }
private:
   Link link;
};

class Subset {
public:
   Subset(const Linklist& l):link(l) { }
   bool operator() (const Linklist& e) const {
      return includes(e.begin(), e.end(), link.begin(), link.end());
   }
private:
   Linklist link;
};

template <class Ran>
class All_alignment {
public:
   All_alignment(Ran b1, Ran e1, Ran b2, Ran e2) throw()
     :begin1(b1),end1(e1),begin2(b2),end2(e2) {
      operations.push_back(new Ins<Ran>(b1, e1, b2, e2));
      operations.push_back(new Del<Ran>(b1, e1, b2, e2));
      operations.push_back(new Sub<Ran>(b1, e1, b2, e2));

      Setlinklist all=find_all_links();
      for (Setlinklist::const_iterator i=all.begin(); i != all.end(); i++) {
         int current_i=0, current_j=0;
         Ran ran_i=b1, ran_j=b2;
         Alignment new_alignment;
         for (Linklist::const_iterator ai=i->begin(); ai != i->end(); ai++) {
            while (current_i != ai->first) {
               new_alignment.push_back(operations[0]);
               current_i++; ran_i++;
            }
            while (current_j != ai->second) {
               new_alignment.push_back(operations[1]);
               current_j++; ran_j++;
            }
            new_alignment.push_back(operations[2]);
            current_i++; current_j++;
            ran_i++; ran_j++;
         }
         while (ran_i != e1) {
            new_alignment.push_back(operations[0]);
            ran_i++;
         }
         while (ran_j != e2) {
            new_alignment.push_back(operations[1]);
            ran_j++;
         }
         alignments.push_back(new_alignment);
      }
   }

   ~All_alignment() {
      for (vector<Edit_operation*>::iterator i=operations.begin();
      i != operations.end(); i++) {
         delete *i;
      }
   }

   // Return an iterator to the begin of the alignment of the two
   // sentences.
   vector<Alignment>::const_iterator
   align_begin() const throw() { return alignments.begin(); }

   // Return an iterator to the end of the alignment of the two
   // sentences.
   vector<Alignment>::const_iterator
   align_end() const throw() { return alignments.end(); }

   // Return an riterator to the begin of the alignment
   // of the two sentences.
   vector<Alignment>::const_reverse_iterator
   align_rbegin() const throw() { return alignments.rbegin(); }

   // Return an riterator to the end of the
   // alignment of the two sentences.
   vector<Alignment>::const_reverse_iterator
   align_rend() const throw() { return alignments.rend(); }

protected:
   Setlinklist
   find_all_links() throw() {
      Linklist m=find_all_matching_terminals();
      Setlinklist p;
      p.insert(Linklist());
      for (Linklist::iterator i=m.begin(); i != m.end(); i++) {
         Setlinklist p_old;
         p.swap(p_old);
         Setlinklist p_align;
         for (Setlinklist::iterator j=p_old.begin(); j != p_old.end(); j++) {
            Linklist o;
            o.clear();
            ns_tools::copy_if
              (j->begin(), j->end(), back_inserter(o), Overlap(*i));
            if (o.empty()) {
               Linklist j_new=*j;
               j_new.push_back(*i);
               p.insert(j_new);
            } else {
               Linklist j_new=*j;
               p.insert(j_new);
               Linklist::iterator new_end=
                 remove_if(j_new.begin(), j_new.end(), In<Link>(o)); // -o
               j_new.erase(new_end, j_new.end());
               j_new.push_back(*i); // +i
               p_align.insert(j_new);
            }
         }
         Setlinklist insert_true;
         for (Setlinklist::iterator
           i=p_align.begin();i != p_align.end(); i++) {
            Setlinklist::iterator next=i;
            next++;
            if ((find_if(p.begin(), p.end(), Subset(*i)) == p.end())
              &&(find_if(p_align.begin(), i, Subset(*i)) == i)
              &&(find_if(next, p_align.end(), Subset(*i)) == p_align.end())) {
               insert_true.insert(*i);
            }
         }
         p.insert(insert_true.begin(), insert_true.end());
      }
      return p;
   }

   Linklist
   find_all_matching_terminals() {
      Linklist res;
      int i=0;
      for (Ran i_i=begin1; i_i != end1; i_i++,i++) {
         int j=0;
         for (Ran j_i=begin2; j_i != end2; j_i++,j++) {
            if (*i_i == *j_i) {
               res.push_back(make_pair(i, j));
            }
         }
      }
      return res;
   }

private:
   vector<Edit_operation*> operations;
   vector<Alignment> alignments;
   Ran begin1, end1, begin2, end2;
};


} // namespace

#endif // __all_alignment__
