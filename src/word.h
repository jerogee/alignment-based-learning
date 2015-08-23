/*******************************************************************************
 **
 **   Filename    : word.h
 **
 **   Description : This file contains the definition of the class Word. 
 **                 It is used to store a word in the form of a string. A 
 **                 word is delimited by whitespaces.
 **
 **   Version     : $Id: word.h 3755 2010-02-19 11:23:46Z menno $
 **
 *******************************************************************************
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *******************************************************************************
 */

#ifndef __word__
#define __word__

#include <iostream>
#include <map>
#include <string>
#include <vector>

using std::istream;
using std::ios;
using std::map;
using std::ostream;
using std::string;
using std::vector;

namespace ns_word {

const int UNDEF=-1;
typedef vector<int> vecul;

struct InvalidWord {
   // This exception class indicates that a word is not well formed.
   InvalidWord(string w) throw():word(w) {}
   string word;
};

class Word {
   // This class is used to store a word. The ABL framework uses strings of
   // words as input, where a word might contain all characters except
   // for the white space and cariage return. The input operator skips leading
   // whitespaces and reads characters until another whitespace or a newline·
   // is encountered.
   friend ostream& operator<<(ostream&, const Word&);
   friend istream& operator>>(istream&, Word&);

   private:
      operator string() const throw(InvalidWord);

      // This procedure associates to each unique space and newline delimited·
      // input string, a word, a unique number for internal representation
      // and stores this in the Word class.
      void storeWord(const string&) throw();

      // This procedure converts the internal representation of the Word
      // to a string and writes it to the ostream.
      void write(ostream&) const throw();

      // This procedure reads a word from istream. It skips initial
      // whitespaces.
      void read(istream&) throw();

      // storage
      int idx;                           // index of current word
      static map<string, int> wrd2idx;   // mapping word  ->index
      static map<int, string> idx2wrd;   // mapping index -> word
      static map<int, vecul> idx2sens;   // mapping index -> sens

   public:

      // constructors
      Word() throw() { idx=UNDEF; }              // value undefined
      Word(const string&) throw(InvalidWord);

      int getIdx() {
         return idx;
      }

      string getWrd() {
         return idx2wrd[idx];
      }

      // This procedure registers the use of the current word in a sentence
      // specified with parameter "sen". 
      void addSenToWord(int sen) throw();

      // Definitions supporting the iteration over the registered sentences
      // for the current word.
      typedef vector<int>::iterator simil_iterator;
      simil_iterator simil_begin() { return idx2sens[idx].begin(); }
      simil_iterator simil_end() { return idx2sens[idx].end(); }

      // operators
      bool operator<(const Word& w) const throw() { return w.idx < idx; }
      bool operator==(const Word& w) const throw() { return w.idx == idx; }
};


} // namespace

#endif // __word__
