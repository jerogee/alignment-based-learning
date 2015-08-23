/******************************************************************************»
 **
 **   Filename    : word.cpp
 **
 **   Description : This file contains the definition of the class Word.·
 **                 It is used to store a word in the form of a string. A·
 **                 word is delimited by whitespaces.
 **
 **   Version     : $Id: word.cpp 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#include <iostream>
#include "word.h"

namespace ns_word {

typedef vector<int> vecul;
map<string, int> Word::wrd2idx;
map<int, string> Word::idx2wrd;
map<int, vecul> Word::idx2sens;

bool isValid(const string&) throw();
bool isValid(const char) throw();

void Word::addSenToWord(int sen) throw() {
   idx2sens[idx].push_back(sen);
}

void Word::storeWord(const string& w) throw() {
   if(wrd2idx.count(w) == 0) {
      idx = wrd2idx.size() + 1;
      wrd2idx[w] = idx;
      idx2wrd[idx] = w;
   } else {
      idx = wrd2idx[w];
   }
}

Word::Word(const string& w) throw(InvalidWord) {
   if (isValid(w)) {
      storeWord(w);
   } else {
      throw InvalidWord(w);
   }
}

Word::operator string() const throw(InvalidWord) {
   if (idx >= 0) {
      return idx2wrd[idx];
   } else {
      throw InvalidWord("UNDEFINED");
   }
}

void Word::write(ostream& os) const throw() {
   os << string(*this);
}

void Word::read(istream& is) throw() {
   char c=0;
   string w;

   // when there is sth on istream -> go to first non-space char
   while ((is) && is.get(c) && c == ' ');

   // is there still sth to parse?
   if (!is) {
      is.setstate(ios::failbit);
      return;
   }

   // when there is sth on istream, this should not be a newline
   if (c == '\n') {
      is.unget();
      is.setstate(ios::failbit);
      return;
   }

   // continue building a word by reading from istream while characters
   // are valid (i.e. until a word delimiter (whitespace) or the newline
   // sentence delimiter is found
   w = c;
   while((is) && is.get(c) && isValid(c)) {
      w += c;
   }

   // if the @@@ sentence delimiter has been read -> stop
   if (w == "@@@") {
      is.unget();
      is.setstate(ios::failbit);
      return;
   }

   if (is) {
      is.unget();
   }

   // store the extracted word
   storeWord(w);
}

ostream& operator<<(ostream& os, const Word& w) {
   w.write(os);
   return os;
}

istream& operator>>(istream& is, Word& w) {
   w.read(is);
   return is;
}

bool isValid(const string& w) throw() {
   if (w.begin() == w.end()) {
      return false;
   }
   for (string::const_iterator i=w.begin(); i != w.end(); ++i) {
      if (!isValid(*i)) {
         return false;
      }
   }
   return true;
}

bool isValid(const char c) throw() {
   if (c == ' ' || c == '\n') {
      return false;
   } else {
      return true;
   }
}


} // namespace
