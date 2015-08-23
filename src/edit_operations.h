/******************************************************************************»
 **
 **   Filename    : edit_operations.h
 **
 **   Description : This file contains the definition of the class 
 **                 Edit_operations.
 **
 **   Version     : $Id: edit_operations.h 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#ifndef __edit_operations__
#define __edit_operations__

#include <cmath>

using namespace std;

namespace ns_edit_distance {

class Edit_operation {
public:
	// Empty virtual destructor.
	virtual
	~Edit_operation() { }
   // return the gamma value of the operation (the pair of ints are indices in
   // the sentences and 1 is the first word in the sentence)
   virtual pair<float, const Edit_operation*>
   gamma(pair<const int, const int>) const throw()=0;

   // return the previous coordinates (when the operation was applied)
   virtual pair<int, int>
   prev_coord(pair<const int, const int>) const throw()=0;

   // return the next coordinates (when the operation is applied)
   virtual pair<unsigned int, unsigned int>
   next_coord(pair<const unsigned int, const unsigned int>) const throw()=0;
};

template <class Ran>
class Del:public Edit_operation {
public:
   Del(Ran b1, Ran e1, Ran b2, Ran e2)
     :begin1(b1),end1(e1),begin2(b2),end2(e2) {};

   pair<float, const Edit_operation*> 
   gamma(pair<const int, const int>) const throw() {
      return make_pair(1,this);
   }

   pair<int, int>
   prev_coord(pair<const int, const int> p) const throw() {
      return make_pair(p.first, p.second-1);
   }

   pair<unsigned int, unsigned int>
   next_coord(pair<const unsigned int, const unsigned int> p) const throw() {
      return make_pair(p.first, p.second+1);
   }
protected:
   Ran begin1, end1, begin2, end2;
};

template <class Ran>
class Ins:public Edit_operation {
public:
   Ins(Ran b1, Ran e1, Ran b2, Ran e2)
     :begin1(b1),end1(e1),begin2(b2),end2(e2) {};

   pair<float, const Edit_operation*>
   gamma(pair<const int, const int>) const throw() {
      return make_pair(1,this);
   }

   pair<int, int>
   prev_coord(pair<const int, const int> p) const throw() {
      return make_pair(p.first-1, p.second);
   }

   pair<unsigned int, unsigned int>
   next_coord(pair<const unsigned int, const unsigned int> p) const throw() {
      return make_pair(p.first+1, p.second);
   }
protected:
   Ran begin1, end1, begin2, end2;
};

template <class Ran>
class Sub:public Edit_operation {
public:
   Sub(Ran b1, Ran e1, Ran b2, Ran e2)
     :begin1(b1),end1(e1),begin2(b2),end2(e2) {};
   virtual ~Sub() {};

   pair<float, const Edit_operation*>
   gamma(pair<const int, const int> p) const throw() {
      if ((p.first <= 0)||(p.second <= 0)) return make_pair(2, this);
      if (*(begin1+p.first-1) == *(begin2+p.second-1)) {
         return make_pair(0, this);
      }
      return make_pair(2, this);
  }

   bool
   match(pair<const int, const int> p) const throw() {
      return (*(begin1+p.first-1)==*(begin2+p.second-1));
   }

   pair<int, int>
   prev_coord(pair<const int, const int> p) const throw() {
      return make_pair(p.first-1, p.second-1);
   }

   pair<unsigned int, unsigned int>
   next_coord(pair<const unsigned int, const unsigned int> p) const throw() {
      return make_pair(p.first+1, p.second+1);
   }
protected:
   Ran begin1, end1, begin2, end2;
};

template <class Ran>
class Sub_dis:public Sub<Ran> {
public:
   Sub_dis(Ran b1, Ran e1, Ran b2, Ran e2) throw() 
     :Sub<Ran>(b1, e1, b2, e2) {
      for(; b1!=e1;b1++) { len1++; }
      for(; b2!=e2;b2++) { len2++; }
   };
   virtual ~Sub_dis() {};

   pair<float, const Edit_operation *>
   gamma(pair<const int, const int> p) const throw() {
      if ((p.first <= 0)||(p.second <= 0)) return make_pair(2, this);
      if (*(Sub<Ran>::begin1+p.first-1) == *(Sub<Ran>::begin2+p.second-1)) {
         return make_pair((fabs(float(p.first-1)/len1-float(p.second-1)/len2)
           *(len1+len2)/float(2)), this);
      }
      return make_pair(2, this);
   }
private:
   int len1;
   int len2;
};

template <class Ran>
class Default:public vector<Edit_operation*> {
public:
   Default<Ran>(Ran b1, Ran e1, Ran b2, Ran e2) throw() {
      push_back(new Ins<Ran>(b1, e1, b2, e2));
      push_back(new Del<Ran>(b1, e1, b2, e2));
      push_back(new Sub<Ran>(b1, e1, b2, e2));
   }

   ~Default() {
      for(iterator i=begin(); i!=end(); i++) {
         delete *i;
      }
   }
};

template <class Ran>
class Biased:public vector<Edit_operation*> {
public:
   Biased<Ran>(Ran b1, Ran e1, Ran b2, Ran e2) throw() {
      push_back(new Ins<Ran>(b1, e1, b2, e2));
      push_back(new Del<Ran>(b1, e1, b2, e2));
      push_back(new Sub_dis<Ran>(b1, e1, b2, e2));
   }

   ~Biased() {
      for(iterator i=begin(); i != end(); i++) {
         delete *i;
      }
   }
};


} // namespace

#endif // __edit_operations__
