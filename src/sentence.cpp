/******************************************************************************»
 **
 **   Filename    : sentence.cpp
 **
 **   Description : This file contains the definition of the class Sentence.
 **                 A sentence is delimited by a newline.
 **
 **   Version     : $Id: sentence.cpp 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#include "sentence.h"

namespace ns_sentence {

void Sentence::write(ostream& os) const throw() {
   if (begin() == end()) {
      return;
   }
   for (Sentence::const_iterator i=begin(); i != end()-1; ++i) {
      os << *i << " ";
   }
   os << *(end()-1);
}

void Sentence::read(istream& is) throw() {
   ns_word::Word w;
   clear();

   while((is) && (is >> w)) {
      push_back(w);
   }
   if (is.fail() && !empty()) {
      is.clear(is.rdstate()&~ios::failbit);
   }
}

void Sentence::regWordsInSentence() {
   vector<Word>::iterator word_it;
   word_it = begin();
   while( word_it != end() ) {
      (*word_it).addSenToWord(sen_id);
      word_it++;
   }
}

void Sentence::getSimilars() {
   vector<Word>::iterator word_it;
   word_it = begin();
   while( word_it != end() ) {
      for ( Word::simil_iterator s=(*word_it).simil_begin();
                                 s != (*word_it).simil_end(); s++) {
         if (*s > sen_id) {
            similars.insert(*s);
         }
      }
      word_it++;
   }
}

bool Sentence::isSimilarTo(int id) {
   if (similars.find(id) != similars.end()) {
      return true;
   } else {
      return false;
   }
}

ostream& operator<<(ostream& os, const Sentence& s) {
   s.write(os);
   return os;
}

istream& operator>>(istream& is, Sentence& s) {
   s.read(is);
   return is;
}


} // namespace
