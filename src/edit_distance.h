/******************************************************************************»
 **
 **   Filename    : edit_distance.h
 **
 **   Description : This file contains the abstract base class
 **                 Edit_operation. Specific instances of the edit
 **                 distance algorithm may be derived from this class.
 **
 **   Version     : $Id: edit_distance.h 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#ifndef __edit_distance__
#define __edit_distance__

#include <vector>
#include "edit_operations.h"

namespace ns_edit_distance {

typedef vector<const Edit_operation*> Alignment;

struct Bad_operations {
};

class Edit_distance {
public:
   Edit_distance(vector<Edit_operation*>& op):operations(op) { }
   virtual ~Edit_distance() { }

   virtual pair<float, const Edit_operation*>
   min_gamma(pair<const int, const int> p) const throw(Bad_operations) {
      vector<Edit_operation*>::const_iterator op=operations.begin();
      if(op==operations.end()) {
         throw Bad_operations();
      }
      pair<int, int> prev=(*op)->prev_coord(p);
      while((prev.first<0)||(prev.second<0)) {
         op++;
         if(op==operations.end()) {
            throw Bad_operations();
         }
         prev=(*op)->prev_coord(p);
      }
      pair<float, const Edit_operation*> min=(*op)->gamma(p);
      min.first+=matrix[prev.first][prev.second];
      op++;
      while(op!=operations.end()) {
         prev=(*op)->prev_coord(p);
         if ((prev.first>=0)&&(prev.second>=0)) {
            pair<float, const Edit_operation*> next=(*op)->gamma(p);
            next.first+=matrix[prev.first][prev.second];
            if (next.first<min.first) {
               min=next;
            }
         }
         op++;
      }
      return min;
   }

   // Return the edit cost between the two sentences.
   virtual float
   give_cost(const int i, const int j) const throw() {
      if ((i<0)||(j<0)) { return 0; }
      return matrix[i][j];
   }

   // Return an iterator to the begin of the alignment of the two sentences.
   Alignment::const_iterator
   align_begin() const throw() { return alignment.begin(); }

   // Return an iterator to the end of the alignment of the two sentences.
   Alignment::const_iterator
   align_end() const throw() { return alignment.end(); }

   // Return an riterator to the begin of the alignment of the two sentences.
   Alignment::const_reverse_iterator
   align_rbegin() const throw() { return alignment.rbegin(); }

   // Return an riterator to the end of the alignment of the two sentences.
   Alignment::const_reverse_iterator
   align_rend() const throw() { return alignment.rend(); }
protected:
   Alignment alignment;
   vector<vector<float> > matrix;
   vector<Edit_operation*> operations;
private:
};

}
#endif // __edit_distance__
// end of file: edit_distance.h
