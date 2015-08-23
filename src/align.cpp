/******************************************************************************»
 **
 **   Filename    : align.cpp
 **
 **   Description : This file implements the aligning phase.
 **
 **   Version     : $Id: align.cpp 3780 2010-02-23 12:21:11Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <utility>
#include <ctime>
#include <cstring>
#include "all_alignment.h"
#include "constituent.h"
#include "edit_distance.h"
#include "edit_operations.h"
#include "nonterminal.h"
#include "sentence.h"
#include "tools.h"
#include "treebank.h"
#include "wagner_fisher.h"
#include "config.h"
#include "suffixtree.h"

#define TIMING (HAVE_SIGNAL_H && HAVE_UNISTD_H)

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if !TIMING
#warning \
Alarm signal not found: \
cannot implement timing checkpoints
#endif

#ifdef HAVE_GETOPT_H
#define GNU_SOURCE
#include <getopt.h>
#else
extern "C" {
   char* optarg;
   extern int optind, opterr, optopt;
   struct option { const char *name; int has_arg; int *flag; int val; };
#define no_argument            0
#define required_argument      1
#define optional_argument      2
#ifdef HAVE_GETOPT_LONG_ONLY
   extern int getopt_long_only (int argc, char * const argv[],
           const char *optstring, const struct option *longopts, int
           *longindex);
#else

#warning \
Gnu Getopt Library not found: \
cannot implement long option handling

   extern int getopt(int argc, char* const argv[], const char*
           optstring);
   inline int getopt_long_only(int argc, char * const argv[],
           const char *optstring, const struct option *longopts, int
           *longindex) {
        return getopt(argc, argv, optstring);
   }
#endif
} // extern "C"
#endif // #ifdef HAVE_GETOPT_H #else

using ns_constituent::Constituent;
using namespace ns_edit_distance;
using ns_nonterminal::Nonterminal;
using ns_sentence::Sentence;
using ns_treebank::Treebank;
using namespace ns_suffixtree;
using ns_tools::error;
using ns_tools::warning;
using ns_tools::debug;
using ns_tools::getDate;

static struct option long_options[] = {
   {"align", required_argument, 0, 'a'},
   {"debug", no_argument, 0, 'd'},
   {"excl_empty", no_argument, 0, 'e'},
   {"help", no_argument, 0, 'h'},
   {"input", required_argument, 0, 'i'},
   {"output", required_argument, 0, 'o'},
   {"part", required_argument, 0, 'p'},
   {"nomerging", no_argument, 0, 'n'},
   {"seed", required_argument, 0, 's'},
   {"time", required_argument, 0, 't'},
   {"check", required_argument, 0, 'c'},
   {"verbose", no_argument, 0, 'v'},
   {"version", no_argument, 0, 'V'},
   {"exhaustive", no_argument, 0, 'x'},
   {0, 0, 0, 0}
};

// Input stream (defaults to cin).
istream *ifs=&cin;
// Output stream (defaults to cout).
ostream *ofs=&cout;
// All possible alignments.
enum Align_type {UNDEF, WM, WB, FM, FB, AA, L, R, B, ST1, ST2, ST3, ST4};
// Chosen alignment (defaults to undefined).
Align_type align_type=UNDEF;
// Parts of the sentences that should be hypotheses.
enum Part_type {EQUAL, UNEQUAL, BOTH};
// Chosen part of the sentences that should be hypotheses. (default to
// unequal)
Part_type part_type=UNEQUAL;
// 0 is the startsymbol (initial nonterminal type).
const int startsymbol=0;
// Nomerge flag.
bool nomerge_flag=false;
// Name of the program as it was called.
string program_name;
// Print debug information.
bool debug_flag=false;
// Number of second to set with alarm
unsigned int timer=0;
// Prefix string
string tmp_prefix("abl_align");
// Global treebank: this is where all the data is stored
Treebank tb;
// Exclude empty hypotheses (where B == E)
bool excl_empty=false;
// Print aligning information
bool verbose_flag=false;
// Do exhaustive comparisons: N(N-1)/2
bool exhaustive_flag=false;
// Counter for number of constituent hypotheses being generated
int consts_cnt=0;


typedef Edit_distance Edit_distance_sen;
typedef Del<Sentence::const_iterator> del;
typedef Ins<Sentence::const_iterator> ins;
typedef Sub<Sentence::const_iterator> sub;
typedef Sub_dis<Sentence::const_iterator> sub_dis;

class Rand {
// This class is a wrapper around a random number generator. It is
// constructed with a seed. When the operator() is called on the
// object, a random number between 0 and n is returned.
public:
   Rand(int seed) { srand(seed); }
   int operator()(int n) {
      return rand() % n;
   }
};

// Default seeds (which are chosen with the -s flag).
int seeds[]={0, 76, 44, 68, 66, 43, 82, 34, 48, 32};
// Default seed value.
int seed=0;


void usage() {
   cerr << "ABL " << VERSION << endl;
   cerr << "Alignment-Based Learner" << endl;
   cerr << __DATE__ << " " << __TIME__ << "\n" << endl;
   cerr << "Usage:" << program_name;
   cerr << " [OPTION]..." << endl;
   cerr << "This program learns structure from the sentences in the input ";
   cerr << "file. It" << endl;
   cerr << "corresponds to the alignment learning phase of the ";
   cerr << "Alignment-Based Learning" << endl;
   cerr << "framework." << endl;
   cerr << endl;
#ifndef HAVE_GETOPT_H
   cerr << "<BEGIN WARNING>" << endl;
   cerr << "This program has been compiled without the long options ";
   cerr << "installed." << endl;
   cerr << "This means that none of the --options work." << endl;
   cerr << "<END WARNING>" << endl;
   cerr << endl;
#endif // #ifndef HAVE_GETOPT_H
   cerr << "  -i, --input=FILE   ";
   cerr << "Name of input file (- means stdin, default)" << endl;
   cerr << "  -o, --output=FILE  ";
   cerr << "Name of output file (- means stdout, default)" << endl;
   cerr << "  -a, --align=TYPE   ";
   cerr << "TYPE is one of:" << endl;
   cerr << "                       - wagner_min, wm:" << endl;
   cerr << "                           wagner_fisher edit distance" << endl;
   cerr << "                           default gamma" << endl;
   cerr << "                       - wagner_biased, wb:" << endl;
   cerr << "                           wagner_fisher edit distance" << endl;
   cerr << "                           biased gamma" << endl;
   cerr << "                       - suffix_tree_1, st1:" << endl;
   cerr << "                           suffix tree based alignment" << endl;
   cerr << "                           method 1" << endl;
   cerr << "                       - suffix_tree_2, st2:" << endl;
   cerr << "                           suffix tree based alignment" << endl;
   cerr << "                           method 2" << endl;
   cerr << "                       - suffix_tree_3, st3:" << endl;
   cerr << "                           suffix tree based alignment" << endl;
   cerr << "                           method 3" << endl;
   cerr << "                       - suffix_tree_4, st4:" << endl;
   cerr << "                           suffix tree based alignment" << endl;
   cerr << "                           method 4" << endl;
   cerr << "                       - all, aa, a:" << endl;
   cerr << "                           all possible alignments" << endl;
   cerr << "                       - left, l:" << endl;
   cerr << "                           left branching trees" << endl;
   cerr << "                       - right, r:" << endl;
   cerr << "                           right branching trees" << endl;
   cerr << "                       - both, b:" << endl;
   cerr << "                           right or left branching trees" << endl;
   cerr << "  -p, --part=TYPE    ";
   cerr << "Part of the sentences that should be used as hypotheses." << endl;
   cerr << "                      (defaults to unequal)" << endl;
   cerr << "                       - equal, e:" << endl;
   cerr << "                           equal parts" << endl;
   cerr << "                       - unequal, u:" << endl;
   cerr << "                           unequal parts" << endl;
   cerr << "                       - both, b:" << endl;
   cerr << "                           equal and unequal parts" << endl;
   cerr << "  -s, --seed NUMBER  ";
   cerr << "Seed (for the both alignment type)" << endl;
#if TIMING
   cerr << "  -t, --time NUMBER  ";
   cerr << "Number of seconds between each checkpoint" << endl;
   cerr << "  -c, --check string ";
   cerr << "prefix of checkpoint files (default is abl_align)" << endl;
#endif
   cerr << "  -e, --excl_empty   ";
   cerr << "Do not generate hypotheses that span 0 words" << endl;
   cerr << "  -n, --nomerge      ";
   cerr << "Do not try to merge hypotheses" << endl;
   cerr << "  -d, --debug        ";
   cerr << "Output debug information" << endl;
   cerr << "  -h, --help         ";
   cerr << "Show this help and exit" << endl;
   cerr << "  -v, --verbose      ";
   cerr << "Show details about the aligning learning process" << endl;
   cerr << "  -V, --version      ";
   cerr << "Show version information and exit" << endl;
   cerr << "  -x, --exhaustive   ";
   cerr << "Compare exhaustively each possible sentence pair" << endl;
   exit(0);
}

#if TIMING
static void alarm_handler(int signal) {
   if (signal == SIGALRM) {
      stringstream tmpfile;
      tmpfile << tmp_prefix << "." << tb.current_index();
      ofstream tmp_out(tmpfile.str().c_str());
      if (!tmp_out.good()) {
         warning(program_name, string("couldn't open ")+tmpfile.str());
      }
      tb.write_partial(tmp_out);
      tmp_out.close();
      debug(program_name, debug_flag, \
            string("Writing checkpoint: ")+tmpfile.str());
      alarm(timer);
   }
}
#endif

void handle_arguments(int argc, char* argv[]) {
   int opt;
   int option_index;
   const char* optstring="a:dehi:mo:p:t:vVx";
   program_name=argv[0];
   bool input_ok=false,output_ok=false;
   while ((opt=getopt_long_only(argc,argv,optstring,long_options,&option_index))
           !=-1){
      switch (opt) {
         case 'a':
            if ((strcmp(optarg, "wm") == 0)
              || (strcmp(optarg, "wagner_min") == 0)) {
               align_type=WM;
            } else if ((strcmp(optarg, "wb") == 0)
              || (strcmp(optarg, "wagner_biased") == 0)) {
               align_type=WB;
            } else if ((strcmp(optarg, "st1") == 0)
              || (strcmp(optarg, "suffix_tree_1") == 0)) {
               align_type=ST1;
            } else if ((strcmp(optarg, "st2") == 0)
              || (strcmp(optarg, "suffix_tree_2") == 0)) {
               align_type=ST2;
            } else if ((strcmp(optarg, "st3") == 0)
              || (strcmp(optarg, "suffix_tree_3") == 0)) {
               align_type=ST3;
            } else if ((strcmp(optarg, "st4") == 0)
              || (strcmp(optarg, "suffix_tree_4") == 0)) {
               align_type=ST4;
            } else if ((strcmp(optarg, "a") == 0)
              || (strcmp(optarg, "aa") == 0)
              || (strcmp(optarg, "all") == 0)) {
               align_type=AA;
            } else if ((strcmp(optarg, "l") == 0)
              || (strcmp(optarg, "left") == 0)) {
               align_type=L;
            } else if ((strcmp(optarg, "r") == 0)
              || (strcmp(optarg, "right") ==0 )) {
               align_type=R;
            } else if ((strcmp(optarg, "b") == 0)
              || (strcmp(optarg, "both") == 0)) {
               align_type=B;
            } else {
               error(program_name, "Unknown alignment algorithm");
               usage();
            }
            break;
         case 'd':
            debug_flag=true;
            break;
         case 'h':
            usage();
            break;
         case 'i':
            if (strcmp(optarg, "-") != 0) {
               if (input_ok) {
                  delete ifs;
               }
               ifs=new ifstream(optarg);
               if (!ifs->good()) {
                  error(program_name, string("cannot open input file ")+optarg);
               }
               input_ok=true;
            }
            break;
         case 'n':
            nomerge_flag=true;
            break;
         case 'o':
            if (strcmp(optarg, "-") != 0) {
               if (output_ok) {
                  delete ofs;
               }
               ofs=new ofstream(optarg);
               if (!ofs->good()) {
                  error(program_name,string("cannot open output file ")+optarg);
               }
               output_ok=true;
            }
            break;
         case 'p':
            if ((strcmp(optarg, "e") == 0)
              || (strcmp(optarg, "equal") == 0)) {
               part_type=EQUAL;
            } else if ((strcmp(optarg, "u") == 0)
                  || (strcmp(optarg, "unequal") == 0)) {
               part_type=UNEQUAL;
            } else if ((strcmp(optarg, "b") == 0)
                  || (strcmp(optarg, "both") == 0)) {
               part_type=BOTH;
            } else {
               error(program_name, "Unknown part algorithm");
               usage();
            }
            break;
         case 's':
            seed=atoi(optarg);
            if (!((seed>=0)&&(seed<=9))) {
               error(program_name,string("seed out of bounds ")+optarg);
            }
            break;
#if TIMING
         case 't':
            timer=atoi(optarg);
            signal(SIGALRM, alarm_handler);
            alarm(timer);
            break;
         case 'c':
            tmp_prefix=optarg;
            break;
#endif
         case 'e':
            excl_empty=true;
            break;
         case 'v':
            verbose_flag = true;
            break;
         case 'V':
            cout << "align (" << PACKAGE << ") version " << VERSION << endl;
            exit(0);
            break;
         case 'x':
            exhaustive_flag=true;
            break;
         case '?': // ambiguous match or extraneous parameter
            usage();
            break;
         default:
            error(program_name, "internal getopt error");
            usage();
      }
   }
   if (align_type == UNDEF) {
      error(program_name, "No alignment type supplied.");
      usage();
   }
   if (optind != argc) {
      warning(program_name, "extraneous argument(s)");
   }
}

void read_treebank(Treebank& tb) {
   *ifs >> tb;
}

Edit_distance_sen *find_alignment(const Sentence& s1, const Sentence& s2) {
   switch (align_type) {
   case WM:
      return new WF_default<Sentence::const_iterator>
        (s1.begin(), s1.end(), s2.begin(), s2.end());
      break;
   case WB:
      return new WF_biased<Sentence::const_iterator>
        (s1.begin(), s1.end(), s2.begin(), s2.end());
      break;
   default:
      break;
   }
   return 0;
}

void insert_constituent(Tree* t, Constituent& c) {
   if (!(excl_empty && c.empty()) && c.valid()) {
      consts_cnt += t->add_structure(c);
   }
}

void insert_constituent_raw(Tree* t, const int b, const int e, const int nt) {
   Constituent c=Constituent(b, e);
   c.push_back(nt);
   insert_constituent(t,c);
}

void insert_constituents(Tree* t1, Tree* t2, Constituent& c1, Constituent& c2) {
   if (!nomerge_flag) {
      Tree::struc_iterator c1pos=t1->find_structure(c1);
      Tree::struc_iterator c2pos=t2->find_structure(c2);
      if (c1pos != t1->struc_end()) {
         if (c2pos != t2->struc_end()) { // merge
            c1.push_back(*c2pos->begin());
            c2.push_back(*c1pos->begin());
            insert_constituent(t1,c1);
            insert_constituent(t2,c2);
         } else {
            c2.push_back(*c1pos->begin());
            insert_constituent(t2,c2);
         }
      } else {
         if (c2pos != t2->struc_end()) {
            c1.push_back(*c2pos->begin());
            insert_constituent(t1,c1);
         } else {
            Nonterminal n=Nonterminal();
            c1.push_back(n);
            c2.push_back(n);
            insert_constituent(t1,c1);
            insert_constituent(t2,c2);
         }
      }
   } else {
      Nonterminal n=Nonterminal();
      c1.push_back(n);
      c2.push_back(n);
      insert_constituent(t1,c1);
      insert_constituent(t2,c2);
   }
}

void handle_ED_alignment(Tree* t1, Tree* t2, Alignment::const_iterator a_b,
  Alignment::const_iterator a_e) {
   enum Insert_mode { NONE, SAME, DIFF };
   Insert_mode current_mode=NONE, next_word=NONE;
   pair<Tree::size_type, Tree::size_type> begin(0, 0), current(0, 0);

   for (Alignment::const_iterator ai=a_b; ai != a_e; ++ai) {
      if ((dynamic_cast<const sub_dis*>(*ai)|| dynamic_cast<const sub*>(*ai))
         && (*(t1->begin()+current.first) == *(t2->begin()+current.second))) {
         next_word=SAME; // match
      } else {
         next_word=DIFF;
      }
      if (current_mode == NONE) {
         current_mode=next_word;
      }
      if (current_mode != next_word) { // handle hypothesis
         Constituent c1=Constituent(begin.first, current.first);
         Constituent c2=Constituent(begin.second, current.second);
         if (((part_type == BOTH)||(part_type==EQUAL))&&(current_mode==SAME)) {
            insert_constituents(t1, t2, c1, c2);
         } else if (((part_type == BOTH)||(part_type == UNEQUAL))
               &&(current_mode == DIFF)) {
            insert_constituents(t1, t2, c1, c2);
         }
         begin=current;
         current_mode=next_word;
      }
      current=(*ai)->next_coord(current);
   }
   // Handle hypotheses at the end of the sentence
   if ((part_type == BOTH)||((part_type == EQUAL)&&(current_mode == SAME))
         ||((part_type == UNEQUAL)&&(current_mode == DIFF))) {
      Constituent c1=Constituent(begin.first, t1->size());
      Constituent c2=Constituent(begin.second, t2->size());
      insert_constituents(t1, t2, c1, c2);
   }
}

void handle_ED_structure(Treebank& tb, Treebank::iterator& current) {
   if ((align_type == R)||(align_type == L)) { // left and right branching
      Tree::size_type end=(align_type == R)?current->size():0;
      for (Tree::size_type i=1; i != current->size(); ++i) {
         Nonterminal n=Nonterminal();
         Constituent c=Constituent(min(i,end), max(i,end));
         c.push_back(n);
         current->add_structure(c);
      }
   } else if (align_type == B) { // both left and right
      static Rand r(seeds[seed]);
      Align_type a;
      (r(100)<50)?a=R:a=L;
      Tree::size_type end=(a == R)?current->size():0;
      for (Tree::size_type i=1; i != current->size(); ++i) {
         Nonterminal n=Nonterminal();
         Constituent c=Constituent(min(i,end), max(i,end));
         c.push_back(n);
         current->add_structure(c);
      }
   } else if (align_type == AA) { // all alignments
      Treebank::iterator new_pos=current;
      ++new_pos;
      for(Treebank::iterator t=new_pos;t != tb.end();++t) {
         All_alignment<Sentence::const_iterator>
           aa(current->begin(), current->end(), t->begin(), t->end());
         for (vector<Alignment>::const_iterator a=aa.align_begin();
           a != aa.align_end(); ++a) {
            handle_ED_alignment((Tree*)&*current, (Tree*)&*t, a->begin(), a->end());
         }
      }
   } else {
      Treebank::iterator new_pos=current;
      ++new_pos;
      for(Treebank::iterator t=new_pos;t != tb.end();++t) {
         bool do_align=true;

         if (!exhaustive_flag && !current->isSimilarTo(t->getID())) {
            do_align=false;
         }

         if (do_align) {
            Edit_distance_sen *a=find_alignment(*current, *t);
            handle_ED_alignment((Tree*)&*current, (Tree*)&*t, a->align_begin(), a->align_end());
            delete a;
         }
      }
   }
}

void handle_ST_structure(  Treebank& tb, Treebank::iterator current,
                           Suffixtree& st) {
   int i = tb.current_index();
   st.construct(i);
}

void find_structure(Treebank& tb) {
   // declare St objects in the case of methods ST*
   Suffixtree st(tb);
   Suffixtree pt(tb);
   st.fix = 0;
   pt.fix = 1;

   if (align_type == ST2) {
      st.fix =1;
      tb.doReverse();
   }

   Nonterminal start(startsymbol);
   debug(program_name, debug_flag, "Finding structure");

   for(;tb.current_index()<tb.size();tb.inc_current_index()) {
      Treebank::iterator s=tb.begin()+tb.current_index();
      debug(program_name, debug_flag, "Aligning sentence", tb.current_index());

      if ((align_type == ST1) || (align_type == ST2) 
                              || (align_type == ST3) || (align_type == ST4)) {
         // suffix tree alignment
         handle_ST_structure(tb, s, st);
      } else {
         // start symbol
         Constituent c=Constituent(0, s->size());
         c.push_back(start);
         consts_cnt += s->add_structure(c);

         // edit distance alignment
         if (!exhaustive_flag) {
            s->getSimilars();
         }
         handle_ED_structure(tb, s);
     }
   }

   if ((align_type == ST3) || (align_type == ST4)) {
      // for ST3 and ST4 also construct a prefix tree
      tb.set_current_index(0);
      tb.doReverse();
      for(;tb.current_index()<tb.size();tb.inc_current_index()) {
         Treebank::iterator s=tb.begin()+tb.current_index();
         handle_ST_structure(tb, s, pt);
      }
   }

   if ((align_type == ST1) || (align_type == ST2)
                           || (align_type == ST3)  || (align_type == ST4)) {
      // structure to store suffixes and prefixes
      Ftree *ftrees;
      ftrees = new Ftree [tb.size()];

      st.align(ftrees);

      if ((align_type == ST3) || (align_type == ST4)) {
         tb.doReverse();
         pt.align(ftrees);
         tb.doReverse();
      }

      if ((align_type == ST2) || (align_type == ST3) || (align_type == ST4)) {
         // reverse back
         tb.doReverse();
      }

      map<int,mnterms>::iterator sufit1;
      map<int,mnterms>::iterator prefit1;
      map<int,int>::iterator sufit2;
      map<int,int>::iterator prefit2;

      for (unsigned int i = 0; i < tb.size();i++) {
         if ((align_type == ST1) || (align_type == ST3)) {
            ftrees[i].sufs[ 0 ][ 0 ]++;
            for (sufit1 = ftrees[i].sufs.begin(); sufit1 != ftrees[i].sufs.end(); sufit1++ ) {
               for (sufit2 = ftrees[i].sufs[ (*sufit1).first ].begin(); sufit2 != ftrees[i].sufs[ (*sufit1).first ].end(); sufit2++ ) {
                  int b = (*sufit1).first;
                  int e = tb[i].size();
                  insert_constituent_raw(&tb[i],b,e,(*sufit2).first);
               }
            }
         }

         if ((align_type == ST2) || (align_type == ST3)) {
            ftrees[i].prefs[ 0 ][ 0 ]++;
            for (prefit1 = ftrees[i].prefs.begin(); prefit1 != ftrees[i].prefs.end(); prefit1++ ) {
               for (prefit2 = ftrees[i].prefs[ (*prefit1).first ].begin(); prefit2 != ftrees[i].prefs[ (*prefit1).first ].end(); prefit2++ ) {
                  int b = 0;
                  int e = tb[i].size() - (*prefit1).first;
                  insert_constituent_raw(&tb[i],b,e,(*prefit2).first);
               }
            }
         }

         if (align_type == ST4) {
            ctfactory hypos(0);

            ftrees[i].sufs[ 0 ][ 0 ]++;
            ftrees[i].prefs[ 0 ][ 0 ]++;
            for (sufit1 = ftrees[i].sufs.begin(); sufit1 != ftrees[i].sufs.end(); sufit1++ ) {
               for (sufit2 = ftrees[i].sufs[ (*sufit1).first ].begin(); sufit2 != ftrees[i].sufs[ (*sufit1).first ].end(); sufit2++ ) {
                  int b = (*sufit1).first;
                  for (prefit1 = ftrees[i].prefs.begin(); prefit1 != ftrees[i].prefs.end(); prefit1++ ) {
                     for (prefit2 = ftrees[i].prefs[ (*prefit1).first ].begin(); prefit2 != ftrees[i].prefs[ (*prefit1).first ].end(); prefit2++ ) {
                        int e = tb[i].size() - (*prefit1).first;
                        insert_constituent_raw(&tb[i],b,e,hypos.get(b,e));
                     }
                  }
               }
            }
         }
      }
   }
}

void write_treebank(const Treebank& tb) {
   *ofs << tb;
}

void outit() {
   if (ifs != &cin){
      delete ifs;
   }
   if (ofs != &cout) {
      delete ofs;
   }
}

void write_infoheader(const Treebank& tb, char** args, int& argsc, int& c) {

   *ofs << "# " << setiosflags(ios::left) << setw(14) << program_name
      << " :: " << setiosflags(ios::left) << setw(16) << "execution time"
      << " :: " << getDate();
   *ofs << "# " << setiosflags(ios::left) << setw(14) <<  program_name
      << " :: " << setiosflags(ios::left) << setw(16) << "command call"
      << " :: ";

   for(int i=0; i<argsc;i++) {
      *ofs << args[i] << " ";
   }
   *ofs << "\n";

   *ofs << "# " << setiosflags(ios::left) << setw(14) << program_name
      << " :: " << setiosflags(ios::left) << setw(16) << "hyps generated"
      << " :: " << c << "\n";

   for (unsigned int i=0; i < tb.comments.size(); i++) {
      *ofs << tb.comments.at(i) << endl;
   }
}

int main(int argc, char* argv[]) {
   const clock_t startTime = clock();
   handle_arguments(argc, argv);
   tb.setExhaustive(exhaustive_flag);
   read_treebank(tb);
   if (verbose_flag) {
      cerr << program_name << "  : # sentences loaded            : "
         << tb.size() << endl;
   }
   find_structure(tb);
   write_infoheader(tb, argv, argc, consts_cnt);
   write_treebank(tb);
   outit();
   if (verbose_flag) {
      cerr << program_name << "  : # hypotheses generated        : "
         << consts_cnt << endl;
      cerr << program_name << "  : # seconds execution time      : "
         << (double)(clock()-startTime)/CLOCKS_PER_SEC << endl;
   }
   return 0;
}
