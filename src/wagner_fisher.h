/******************************************************************************»
 **
 **   Filename    : wagner_fisher.h
 **
 **   Description : This file contains the definition of the class
 **                 Wagner-fisher. It implements the Wagner Fisher edit
 **                 distance algorithm. Wagner_fisher should still be an
 **                 abstract base class, where only the gamma function is
 **                 still abstract. The WF_min_edit_distance class is derived
 **                 from the Wagner_fisher class. This class has the gamma 
 **                 function instantiated. It should find the minimum edit¶
 **                 distance between two sentences (ins=del=1, sub=2, mat=0).
 **                 WF_biased_edit_distance is also derived from the
 **                 Wagner_fisher class. The distance function is the biased
 **                 distance function (ins=del=1, sub=2,
 **                 mat=((index_S/|S|)-(index_T/|T|))*mean(|S|,|T|)
 **
 **   Version     : $Id: wagner_fisher.h 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#ifndef __wagner_fisher__
#define __wagner_fisher__

#include <vector>
#include "edit_distance.h"
#include "edit_operations.h"

namespace ns_edit_distance {

void write_matrix(vector<vector<float> > d) {
   for (vector<vector<float> >::iterator i=d.begin(); i!=d.end(); i++) {
      for (vector<float>::iterator j=i->begin(); j!=i->end(); j++) {
         cout << *j << " ";
      }
      cout << endl;
   }
}

// Ran needs to be a random access iterator.
template <class Ran>
class Wagner_fisher:public Edit_distance {
public:
   // This procedure builds the edit distance matrix and the alignment
   // (see class Edit_distance).
   Wagner_fisher(Ran b1, Ran e1, Ran b2, Ran e2, vector<Edit_operation*> op)
     throw():Edit_distance(op) {
      build_matrix(b1, e1, b2, e2);
      build_alignment();
   }
   ~Wagner_fisher() {}

private:
   void build_matrix(Ran b1, Ran e1, Ran b2, Ran e2) throw() {
      len1=len2=0;
      for (; b2!=e2;b2++) { len2++; }
      matrix.push_back(vector<float>(len2+1,float(0)));
      for (; b1!=e1;b1++) {
         len1++;
         matrix.push_back(vector<float>(len2+1,float(0)));
      }
      for (int i=0; i <= len1; i++) {
         for (int j=0; j <= len2; j++) {
            if ((i == 0)&&(j == 0)) {
               continue; // init step
            }
            pair<float, const Edit_operation*> m=min_gamma(make_pair(i, j));
            matrix[i][j]=m.first;
         }
      }
   }

   void build_alignment() throw() {
      pair<int, int> curr_coor=make_pair(len1, len2);
      while(!((curr_coor.first == 0) && (curr_coor.second == 0))) {
         pair<float, const Edit_operation*> m=min_gamma(curr_coor);
         alignment.insert(alignment.begin(), m.second);
         curr_coor=m.second->prev_coord(curr_coor);
      }
   }

   int len1, len2;
};

template <class Ran>
class WF_default:public Wagner_fisher<Ran> {
public:
   WF_default(Ran b1, Ran e1, Ran b2, Ran e2) throw()
   :Wagner_fisher<Ran>(b1, e1, b2, e2,
      *(storage=new Default<Ran>(b1, e1, b2, e2))) { }
   ~WF_default() { delete storage; }
private:
   Default<Ran>* storage;
};

template <class Ran>
class WF_biased:public Wagner_fisher<Ran> {
public:
   WF_biased(Ran b1, Ran e1, Ran b2, Ran e2) throw()
   :Wagner_fisher<Ran>(b1, e1, b2, e2,
      *(storage=new Biased<Ran>(b1, e1, b2, e2))) { }
   ~WF_biased() { delete storage; }
private:
   Biased<Ran>* storage;
};


} // namespace

#endif // __wagner_fisher__
