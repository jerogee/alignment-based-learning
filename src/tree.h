/******************************************************************************»
 **
 **   Filename    : tree.h
 **
 **   Description : This file contains the definition of the class Tree.
 **                 A tree is a sentence with a vector of 0..n constituents.
 **
 **   Version     : $Id: tree.h 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#ifndef __tree__
#define __tree__

#include <iostream>
#include <vector>
#include <algorithm>
#include "constituent.h"
#include "sentence.h"

using ns_sentence::Sentence;
using ns_constituent::Constituent;

using std::istream;
using std::ostream;
using std::vector;

namespace ns_tree {

class Tree:public Sentence {
   // This class is used to store a (fuzzy) tree. It contains a
   // sentence together with a list of constituents (or hypotheses).

   friend ostream& operator<<(ostream&, const Tree&);
   friend istream& operator>>(istream&, Tree&);

public:
   // Constructors and typecasts all inherited from Sentence.

   // Adds a constituent c to the tree. If a constituent with the same begin
   // and end indices already exists, the nonterminals of c are added to 
   // that constituent (retval=0). Otherwise, c is added itself (retval=1).
   int add_structure(const Constituent&) throw();

   // This procedure clears the entire tree (sentence and structure).
   void clear() {
       Sentence::clear();
       structure.clear();
       comment_line.clear();
   }

   void changeSentence(vector<Word> wv) {
      Sentence::clear();
      for (vector<Word>::const_iterator it=wv.begin(); it != wv.end(); it++) {
         push_back((*it));
      }
   }

   // This procedure returns the number of hypotheses the tree contains
   unsigned long getHypothesisCount() {
      return structure.size();
   }

   // Iterators.
   typedef vector<Constituent>::const_iterator const_struc_iterator;
   typedef vector<Constituent>::const_reverse_iterator const_struc_reverse_iterator;
   typedef vector<Constituent>::iterator struc_iterator;
   typedef vector<Constituent>::reverse_iterator struc_reverse_iterator;

   // This procedure returns an interator to c in the tree.
   struc_iterator find_structure(const Constituent& c) throw() {
      return find(structure.begin(), structure.end(), c);
   }

   struc_iterator struc_begin() throw() { return structure.begin(); }
   struc_iterator struc_end() throw() { return structure.end(); }

   struc_reverse_iterator struc_rbegin() throw() { return structure.rbegin(); }
   struc_reverse_iterator struc_rend() throw() { return structure.rend(); }

   const_struc_iterator struc_begin() const throw() { return structure.begin(); }
   const_struc_iterator struc_end() const throw() { return structure.end(); }

   const_struc_reverse_iterator struc_rbegin() const throw() { return structure.rbegin(); }
   const_struc_reverse_iterator struc_rend() const throw() { return structure.rend(); }

   // Return the number of constituents in the tree
   size_type struc_size() const throw() { return structure.size(); }

   struc_iterator erase(struc_iterator b, struc_iterator e) { return structure.erase(b,e); }

private:

   // Writes the tree to ostream os. It first writes the sentence,
   // followed by the 0..n constituents.
   void write(ostream&) const throw();

   // Reads a tree from istream is. It first reads a sentence followed
   // by 0 or more constituents.
   void read(istream&) throw();

   // Data members
   vector<Constituent> structure;
};

// IO procedures. These are friends of the class Tree.
ostream& operator<<(ostream&, const Tree&);
istream& operator>>(istream&, Tree&);


} // namespace

#endif //__tree__
