/******************************************************************************»
 **
 **   Filename    : nonterminal.cpp
 **
 **   Description : This file contains the definition of the class Nonterminal.
 **                 The current implementation only allows nonterminals that
 **                 represent integer numbers. When nonterminals should be
 **                 represented as strings, a conversion is needed.
 **
 **   Version     : $Id: nonterminal.cpp 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#include "nonterminal.h"

namespace ns_nonterminal {

unsigned long Nonterminal::upper_nt;

void Nonterminal::write(ostream& os) const throw() {
   os << value;
}

void Nonterminal::read(istream& is) throw() {
   char c=0;
   is >> c;
   if (!is) return;
   if (!isdigit(c)) {
      is.unget();
      is.setstate(ios::failbit);
      return;
   }
   is.unget();
   is >> value;
   if (value >= upper_nt) upper_nt=value;
}

ostream& operator<<(ostream& os, const Nonterminal& n) {
   n.write(os);
   return os;
}

istream& operator>>(istream& is, Nonterminal& n) {
   n.read(is);
   return is;
}


} // namespace
