/******************************************************************************»
 **
 **   Filename    : treebank.cpp
 **
 **   Description : This file contains the definition of the class Treebank.
 **                 A treebank is effectively a list of trees. Note that it
 **                 is useful (when aligning) to insert unique trees only.
 **                 This implementation does not do this.
 **
 **   Version     : $Id: treebank.cpp 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#include "tree.h"
#include "treebank.h"

using namespace std;

using ns_tree::Tree;

namespace ns_treebank {

void Treebank::doReverse() {

   for(Treebank::iterator i=begin(); i != end(); i++) {
      vector<Word> rev;
      for(vector<Word>::reverse_iterator j=(*i).rbegin(); j != (*i).rend(); j++) {
         rev.push_back(*j);
      }
      (*i).changeSentence(rev);
   }
}

void Treebank::write_partial(ostream& os) const throw() {
   os << ":" << current << endl << endl;
   for(Treebank::const_iterator i=begin(); i != end(); i++) {
      os << *i << endl;
   }
}

void Treebank::write(ostream& os) const throw() {
   for(Treebank::const_iterator i=begin(); i != end(); i++) {
      os << *i << endl;
   }
}

void Treebank::read_partial(istream& is) throw() {
   int tree_nr;
   string colon=":";
   is >> colon >> tree_nr;
   Tree t;

   while(is >> t) {
      push_back(t);
   }
   set_current_index(tree_nr);
}

void Treebank::read(istream& is) throw() {
   Tree t;
   unsigned long cnt = 0;
   while(is >> t) {
      // look if the line contains comments
      if (t.comment_line.size()) {
         comments.push_back(t.comment_line);
      }

      // keep track of sentence relation when not exhaustive comparison
      if (!exhaustive) {
         t.setID(++cnt);
         t.regWordsInSentence();
      }

      // add tree to treebank
      push_back(t);
      t.clear();
   }
   set_current_index(0);
}

ostream& operator<<(ostream& os, const Treebank& tb) {
   tb.write(os);
   return os;
}

istream& operator>>(istream& is, Treebank& tb) {
   if (is.peek()==':') {
      tb.read_partial(is);
   } else {
      tb.read(is);
   }
   return is;
}

} // namespace
