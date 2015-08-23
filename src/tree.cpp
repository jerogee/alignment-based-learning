/******************************************************************************»
 **
 **   Filename    : tree.cpp
 **
 **   Description : This file contains the definition of the class Tree.
 **                 A tree is a sentence with a vector of 0..n constituents.
 **
 **   Version     : $Id: tree.cpp 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#include "tree.h"

namespace ns_tree {

int Tree::add_structure(const Constituent& c) throw() {
   struc_iterator i=find_structure(c);
   if (i == structure.end()) {
      // generate a new
      structure.push_back(c);
      return 1;
   } else {
      for (Constituent::const_iterator n=c.begin(); n != c.end(); ++n) {
         if (find(i->begin(), i->end(), *n) == i->end()) {
            // add to existing
            i->push_back(*n);
         }
      }
      return 0;
   }
}

void Tree::write(ostream& os) const throw() {
   // write sentence to ostream
   os << (Sentence)*this;

   // put sentence-constituent delimiter if there is at least one constituent
   if (struc_begin() != struc_end()) {
      os << " @@@ ";
   }

   // write 0..n constituents to ostream
   int cnt = 0;
   for (Tree::const_struc_iterator i=struc_begin(); i != struc_end(); ++i) {
      os << *i;
      cnt++;
   }
}

void Tree::read(istream& is) throw() {
   char c;

   // read until end of line if it is a comments line
   c=is.peek();

   while (c == '#') {
      // comment_line += "#";
      while(is && is.get(c) && (c != '\n')) {
         comment_line += c;
      }
      is.get(c);
      if (c != '#') {
         is.unget();
      } else {
         comment_line += '\n';
         comment_line += "#";
      }
   }

   // read the sentence part of the tree
   is >> *(Sentence*)this;

   // return if there is an error
   if (!is.good()) {
      return;
   }

   // read next char on istream
   if (is.peek() != '\n') {

		is >> c;
      // when the beginning of a constituent is found -> start extracting
      if (c == '(') {
         is.unget();
         Constituent c(0,0);
         while (is && (is.peek() != '\n') && (is >> c)) {
            add_structure(c);
            while ((is) && isspace(is.peek()) && (is.peek() != '\n')) {
               is.ignore();
            }
         }
         if (is.fail()) {
            is.clear(is.rdstate()&~ios::failbit);
         }
      } else {
         if (is.fail()) {
            is.clear(is.rdstate()&~ios::failbit);
         }
      }
   }
	is.ignore(); // skip newline
}

ostream& operator<<(ostream& os, const Tree& t) {
   t.write(os);
   return os;
}

istream& operator>>(istream& is, Tree& t) {
   t.read(is);
   return is;
}


} // namespace
