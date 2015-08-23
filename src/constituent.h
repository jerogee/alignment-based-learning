/******************************************************************************»
 **
 **   Filename    : constituent.h
 **
 **   Description : This file contains the definition of the class Constituent.
 **                 A constituent is a list of nonterminals with a begin and
 **                 end index (into a sentence). It describes a hypothesis.
 **
 **   Version     : $Id: constituent.h 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#ifndef __constituent__
#define __constituent__

#include <iostream>
#include "nonterminal.h"
#include "sentence.h"

using ns_nonterminal::Nonterminal;
using ns_sentence::Sentence;

using std::vector;
using std::pair;

namespace ns_constituent {

typedef Sentence::size_type Index;

class Constituent:public vector<Nonterminal> {
   // This class is used to store a hypothesis or a nonterminal. A
   // hypothesis is defined by a begin and end index (in a sentence)
   // and one or more nonterminals.

   friend ostream& operator<<(ostream&, const Constituent&);
   friend istream& operator>>(istream&, Constituent&);

   private:
      // This procedure writes the constituent to ostream os. The
      // format is ``(begin,end,[nonterminals])''.
      void write(ostream&) const throw();

      // This procedure reads a constituent from istream is. It skips initial
      // whitespaces and then reads a constituent.  The constituent may
      // have inner whitespace (which is skipped). Note that invalid
      // boundaries are not checked.
      void read(istream&) throw();

      // data members
      pair<Index, Index> boundaries;

   public:

      // constructors
      Constituent(const Index& b, const Index& e) throw():boundaries(b, e) {}

      // returns the begin index
      const Index& give_begin() const throw() { return boundaries.first; }

      // returns the end index
      const Index& give_end() const throw() { return boundaries.second; }

      // is the constituent empty (i.e. begin==end)?
      const bool empty() const throw() { return boundaries.first == boundaries.second; }

      // is the constituent valid (i.e. begin<=end)?
      const bool valid() const throw() { return boundaries.first <= boundaries.second; }

      // equality operator
      const bool operator==(const Constituent& c) const throw() { 
         return ((c.boundaries.first == boundaries.first) && (c.boundaries.second == boundaries.second));
      }

      // sets the nonterminal of the hypothesis to n
      void merge_nonterminals(Nonterminal n) throw() {
         clear();
         push_back(n);
      }
};

// IO procedures. These are friends of the class Constituent.
ostream& operator<<(ostream&, const Constituent&);
istream& operator>>(istream&, Constituent&);


} // namespace

#endif //__constituent__
