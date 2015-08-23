/******************************************************************************»
 **
 **   Filename    : constituent.cpp
 **
 **   Description : This file contains the definition of the class Constituent.
 **                 A constituent is a list of nonterminals with a begin and
 **                 end index (into a sentence). It describes a hypothesis.
 **
 **   Version     : $Id: constituent.cpp 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#include <iostream>
#include "constituent.h"
#include "nonterminal.h"

namespace ns_constituent {

using ns_nonterminal::Nonterminal;

void Constituent::write(ostream& os) const throw() {
  os << "(" << boundaries.first << "," << boundaries.second << ",[";
   if(begin() != end()) {
      // there is at least one non-terminal
      Constituent::const_iterator i=begin();
      os << *i;
      while(++i != end()) {
         os << "," << *i;
      }
   }
   os << "])";
}

void Constituent::read(istream& is) throw() {
   char c=0;

   // get first char
   is >> c;

   // stop if there is nothing on the istream anymore
   if (!is) {
      return;
   }

   // next char should be opening of hypothesis
   if (c != '(') {
      is.unget();
      is.setstate(ios::failbit);
      return;
   }

   // stop if there is no left-boundary
   if(!(is >> boundaries.first)) {
      is.setstate(ios::badbit);
      return;
   }

   // when there is sth on istream -> go to first non-space char
   while ((is) && is.get(c) && c == ' ');

   // next char should be komma seperator
   if(c != ',') {
      is.setstate(ios::badbit);
      return;
   }

   // stop if there is not right-boundary
   if(!(is >> boundaries.second)) {
      is.setstate(ios::badbit);
      return;
   }

   // when there is sth on istream -> go to first non-space char
   while ((is) && is.get(c) && c == ' ');

   // next char should be komma seperator
   if(c != ',') {
      is.setstate(ios::badbit);
      return;
   }

   // when there is sth on istream -> go to first non-space char
   while ((is) && is.get(c) && c == ' ');

   // next char should be non-terminal opening bracket
   if(c != '[') {
      is.setstate(ios::badbit);
      return;
   }

   // read non-terminal(s)
   clear();
   int nonterm;
   bool finished=false;

   while (!finished && (is >> nonterm)) {
      // register non-terminal
      push_back(Nonterminal(nonterm));

      // when there is sth on istream -> go to first non-space char
      while ((is) && is.get(c) && c == ' ');

      if (c == ']') {
         finished=true;
      } else if (c != ',') {
         is.setstate(ios::badbit);
         return;
      }
   }

   // when there is sth on istream -> go to first non-space char
   while ((is) && is.get(c) && c == ' ');

   if(c != ')') {
      is.setstate(ios::badbit);
      return;
   }
}

ostream& operator<<(ostream& os, const Constituent& c) {
   c.write(os);
   return os;
}

istream& operator>>(istream& is, Constituent& c) {
   c.read(is);
   return is;
}


} // namespace
