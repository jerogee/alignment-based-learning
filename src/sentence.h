/******************************************************************************»
 **
 **   Filename    : sentence.h
 **
 **   Description : This file contains the definition of the class Sentence.
 **                 A sentence is delimited by a newline.
 **
 **   Version     : $Id: sentence.h 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#ifndef __sentence__
#define __sentence__

#include <iostream>
#include <vector>
#include <set>
#include "word.h"

using std::vector;
using std::set;
using ns_word::Word;

namespace ns_sentence {

class Sentence:public vector<Word> {
   // This class is used to store a sentence. A sentence is a sequence of
   // words.

   friend ostream& operator<<(ostream&, const Sentence&);
   friend istream& operator>>(istream&, Sentence&);

   private:

      // This procedure writes the sentence to ostream. The words in the
      // sentence are delimited by whitespaces. There is no final whitespace.
      void write(ostream&) const throw();

      // This procedure reads a sentence from istream. It skips initial
      // whitespaces.
      void read(istream&) throw();

      // data structure
      int sen_id;            // storing the sentence ID
      set<int> similars;     // keeping track of sentences to
                                       // compare with
   public:
      string comment_line;

      // This procedure sets the sentence ID
      void setID(int id) {
         sen_id = id;
      }

      // This procedure returns the sentence ID
      int getID() {
         return sen_id;
      }

      // This procedure considers each word in the sentence and registers
      // its appearance in the current sentence to facilitate future
      // decisions on which sentences to compare with. It should be called
      // while reading in the full treebank line by line.
      void regWordsInSentence();

      // This procedure considers each word in the sentence and fills the
      // set "similars" with sentences to compare with as these sentences
      // have at least one word in common.
      void getSimilars();

      // This procedure returns true if the current sentence has in its
      // "similars" set the sentence specified with argument "id", and
      // returns false if it has not.
      bool isSimilarTo(int id);

      // constructors
      Sentence() throw() :vector<Word>() { comment_line=""; }
      template <class In>
      Sentence(In first, In last) throw() :vector<Word>(first, last) { }
};


} // namespace

#endif // __sentence__
