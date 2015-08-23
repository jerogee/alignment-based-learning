/******************************************************************************»
 **
 **   Filename    : tools.h
 **
 **   Description : This file contains several generic functions.
 **
 **   Version     : $Id: tools.h 3755 2010-02-19 11:23:46Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */
#ifndef __tools__
#define __tools__

#include <stdlib.h>
#include <string>
#include <ctime>

namespace ns_tools {

using namespace std;

template <class In, class Out, class Pred>
Out copy_if(In first, In last, Out res, Pred p) {
// This procedure copies all elements for which p is true from first
// to last into res. This procedure is defined in the Bjarne
// Stroustrup C++ book.
   while (first != last) {
      if (p(*first)) *res++ = *first;
      ++first;
   }
   return res;
}

void error(const string program_name, const string text, const int line_nr=-1) {
// This procedure writes an error message (in GNU format) to cerr and
// exits. It first prints the program name, followed by a colon,
// followed by a line number (if that was given as a parameter),
// followed by text.
   cerr << program_name << ": ";
   if (line_nr>=0) {
      cerr << line_nr << ": ";
   }
   cerr << text << endl;
   exit(1);
}

void warning(const string program_name, const string text, const int line_nr=-1) {
// This procedure writes a warning to cerr. It writes program_name,
// followed by a colon, followed by WARNING:, followed by line_nr (if
// defined), followed by the text. (Is this GNU format?)
   cerr << program_name << ": WARNING: ";
   if (line_nr>=0) {
      cerr << line_nr << ": ";
   }
   cerr << text << endl;
}

void debug(const string program_name, const bool debug_flag, const string text,
      const int line_nr=-1) {
// This procedure writes a debug statement to cerr if debug_flag is
// true. It writes program_name, followed by a colon and DEBUG:, then
// line_nr if defined, followed by text. (Is this GNU format?)
   if (debug_flag) {
      cerr << program_name << ": DEBUG: ";
      if (line_nr>=0) {
         cerr << line_nr << ": ";
      }
      cerr << text << endl;
   }
}

string getDate() {

   time_t rawtime;
   struct tm * timeinfo;

   time (&rawtime);
   timeinfo = localtime ( &rawtime );

   return asctime (timeinfo);
}


} // namespace

#endif // __tools__
