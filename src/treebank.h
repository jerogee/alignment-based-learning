/******************************************************************************»
 **
 **   Filename    : treebank.h
 **
 **   Description : This file contains the definition of the class Treebank.
 **                 A treebank is effectively a list of trees. Note that it
 **                 is useful (when aligning) to insert unique trees only.
 **                 This implementation does not do this.
 **
 **   Version     : $Id: treebank.h 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#ifndef __treebank__
#define __treebank__

#include "tree.h"

using ns_tree::Tree;
using std::endl;

namespace ns_treebank {

class Treebank:public vector<Tree> {

   friend ostream& operator<<(ostream&, const Treebank&);
   friend istream& operator>>(istream&, Treebank&);

public:
   vector<string> comments;

   Treebank() {
      exhaustive=false;
      comments.clear();
   }

   // This procedure writes the partially analysed treebank to ostream.
   void write_partial(ostream&) const throw();

   // This procedure sets whether or not to preserve memory by aligning
   // all possible sentence pairs.
   void setExhaustive(bool v) {
      exhaustive = v;
   }

   void doReverse();

   // Definitions supporting iteration over the treebank.
   Treebank::size_type current_index() const throw() { return current; }
   void inc_current_index() throw() { current++; }
   void set_current_index(Treebank::size_type new_cur) throw() {
      current=new_cur;
   }

private:

   // This procedure writes the treebank to ostream.
   void write(ostream&) const throw();

   // This procedure reads a treebank from istream.
   void read(istream&) throw();

   // This procedure reads a partially analysed treebank from istream. 
   // It returns the current tree number that should be analysed.
   void read_partial(istream&) throw();

   size_type current;
   bool exhaustive;
   int nterm;
};

ostream& operator<<(ostream&, const Treebank&);
istream& operator>>(istream&, Treebank&);


} // namespace

#endif //__treebank__
