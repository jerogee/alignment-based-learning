/******************************************************************************»
 **
 **   Filename    : nonterminal.h
 **
 **   Description : This file contains the definition of the class Nonterminal.
 **                 The current implementation only allows nonterminals that
 **                 represent integer numbers. When nonterminals should be
 **                 represented as strings, a conversion is needed.
 **
 **   Version     : $Id: nonterminal.h 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#ifndef __nonterminal__
#define __nonterminal__

#include <iostream>
#include <map>
#include <string>

using std::ios;
using std::istream;
using std::ostream;

namespace ns_nonterminal {

class Nonterminal {
   // This class is used to store a nonterminal.  Creating a
   // nonterminal actually gives an unused nonterminal ``constant''.
   // Creating a nonterminals always returns a new, unique one.
   // They are printed as integers. It is also possible to create a
   // nonterminal by initialising it with an integer. However, it
   // may be the case that when initialising the nonterminal with an
   // integer, it is has the same value as an already existing one.

   friend ostream& operator<<(ostream&, const Nonterminal&);
   friend istream& operator>>(istream&, Nonterminal&);

public:

   // constructors
   Nonterminal(const unsigned long&n) throw():value(n) { if (n>=upper_nt) upper_nt=n+1; }
   Nonterminal() throw() { value=upper_nt++; }

   // operators. Since no ordering in nonterminals is
   // guaranteed, this is an arbitrary but consistent ordering.
   bool operator<(const Nonterminal& n) const throw() {
      return value < n.value;
   }
   bool operator==(const Nonterminal& n) const throw() {
      return value == n.value;
   }

private:

   operator unsigned long() const { return value; }

   // writes the nonterminal in integer format to ostream
   void write(ostream&) const throw();

   // reads a nonterminal from istream. It skips initial whitespaces 
   // and reads an integer. The integer should be digits only
   // (no + or - or E characters).
   void read(istream&) throw();

   // data objects
   unsigned long value;             // the value of the nonterminal
   static unsigned long upper_nt;   // highest value
};


} // namespace

#endif //__nonterminal__
