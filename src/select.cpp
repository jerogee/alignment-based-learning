/******************************************************************************»
 **
 **   Filename    : cluster.cpp
 **
 **   Description : This file implements the clustering part of the aligning
 **                 phase.
 **
 **   Version     : $Id: select.cpp 3780 2010-02-23 12:21:11Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "constituent.h"
#include "nonterminal.h"
#include "sentence.h"
#include "tools.h"
#include "tree.h"
#include "treebank.h"
#include "word.h"

using namespace std;

// configurable include files

#if HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_GETOPT_H
#define GNU_SOURCE
#include <getopt.h>
#else
extern "C" {
   char* optarg;
   extern int optind, opterr, optopt;
   struct option { const char *name; int has_arg; int *flag; int val;
};
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
using ns_nonterminal::Nonterminal;
using ns_sentence::Sentence;
using ns_tree::Tree;
using ns_treebank::Treebank;
using ns_tools::error;
using ns_tools::warning;
using ns_tools::debug;
using ns_tools::getDate;

static struct option long_options[] = {
   {"debug", no_argument, 0, 'd'},
   {"help", no_argument, 0, 'h'},
   {"input", required_argument, 0, 'i'},
   {"output", required_argument, 0, 'o'},
   {"preserve_mem", no_argument, 0, 'm'},
   {"select", required_argument, 0, 's'},
   {"verbose", no_argument, 0, 'v'},
   {"version", no_argument, 0, 'V'},
   {0, 0, 0, 0}
};

// Input stream (defaults to cin).
istream *ifs=&cin;
// Output stream (defaults to cout).
ostream *ofs=&cout;
// All possible selection methods.
enum Select_type {UNDEF, FIRST, TERMS, CONST};
// Chosen selection method (defaults to undefined).
Select_type select_type=UNDEF;
// Preserve memory.
bool preserve=false;
// Name of the program as it was called.
string program_name;
// Print debug information.
bool debug_flag=false;
// Print process information
bool verbose_flag=false;
// Count number of initial hypotheses
int hyps_cnt=0;
// Count selected constituents
int consts_cnt=0;

class Rand {
public:
   Rand() { }
   Rand(int seed) { srand(seed); }
   int operator()(int n) {
      return rand() % n;
   }
};

//Rand r(123); // random, but fixed seed
Rand r;

void
usage() {
   cerr << "ABL " << VERSION << endl;
   cerr << "Alignment-Based Learner" << endl;
   cerr << __DATE__ << " " << __TIME__ << "\n" << endl;
   cerr << "Usage:" << program_name;
   cerr << " [OPTION]..." << endl;
   cerr << "This program selects from overlapping constituents found in the ";
   cerr << "treebank in the" << endl;
   cerr << "input file. It corresponds to the selection learning phase ";
   cerr << "of the" << endl;
   cerr << "Alignment-Based Learning framework." << endl;
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
   cerr << "  -s, --select=TYPE   ";
   cerr << "TYPE is one of:" << endl;
   cerr << "                       - first, f:" << endl;
   cerr << "                           earlier learned constituents are ";
   cerr << "correct" << endl;
   cerr << "                       - terms, t, leaf, l:" << endl;
   cerr << "                           terms selection method" << endl; 
   cerr << "                       - const, c, branch, b:" << endl;
   cerr << "                           const selection method" << endl;
   cerr << "  -m, --preserve_mem ";
   cerr << "Preserves memory use (and is slower)." << endl;
   cerr << "  -d, --debug        ";
   cerr << "Output debug information" << endl;
   cerr << "  -h, --help         ";
   cerr << "Show this help and exit" << endl;
   cerr << "  -v, --verbode      ";
   cerr << "Show details about the selection learning process" << endl;
   cerr << "  -V, --version      ";
   cerr << "Show version information and exit" << endl;
   exit(0);
}

void
handle_arguments(int argc, char* argv[]) {
   int opt;
   int option_index;
   const char* optstring="dhi:mo:s:v";
   program_name=argv[0];
   bool input_ok=false,output_ok=false;
   while ((opt=getopt_long_only(argc,argv,optstring,long_options,&option_index))
           !=-1){
      switch (opt) {
         case 'd':
            debug_flag=true;
            break;
         case 'h':
            usage();
            break;
         case 'i':
            if (strcmp(optarg, "-")!=0) {
               if (input_ok) {
                  delete ifs;
               }
               ifs=new ifstream(optarg);
               if (!ifs) {
                  error(program_name, string("cannot open input file")+optarg);
               }
               input_ok=true;
            }
            break;
         case 'm':
            preserve=true;
            break;
         case 'o':
            if (strcmp(optarg, "-") != 0) {
               if (output_ok) {
                  delete ofs;
               }
               ofs=new ofstream(optarg);
               if (!ofs) {
                  error(program_name, string("cannot open output file")+optarg);
               }
               output_ok=true;
            }
            break;
         case 's':
            if ((strcmp(optarg, "f") == 0)
              || (strcmp(optarg, "first") == 0)) {
               select_type=FIRST;
            } else if ((strcmp(optarg, "t") == 0)
              || (strcmp(optarg, "terms") == 0)
              || (strcmp(optarg, "l") == 0)
              || (strcmp(optarg, "leaf") == 0)) {
               select_type=TERMS;
            } else if ((strcmp(optarg, "c") == 0)
              || (strcmp(optarg, "const") == 0)
              || (strcmp(optarg, "b") == 0)
              || (strcmp(optarg, "branch") == 0)) {
               select_type=CONST;
            } else {
               error(program_name, "Unknown selection algorithm");
               usage();
            }
            break;
         case 'v':
            verbose_flag = true;
            break;
         case 'V':
            cout << "select ("<< PACKAGE << ") version " << VERSION << endl;
            exit(0);
            break;
         case '?': // ambiguous match or extraneous parameter
            usage();
            break;
         default:
            error(program_name, "internal getopt error");
            usage();
      }
   }
   if (select_type == UNDEF) {
      error(program_name, "No selection type supplied.");
      usage();
   }
   if (optind != argc) {
      warning(program_name, "extraneous argument(s)");
   }
}

void read_treebank(Treebank& tb) {
   if (!preserve) {
      *ifs >> tb;
   }
}

class Overlap {
public:
   Overlap(const Constituent& c):base(c) { }

   bool operator() (const Constituent& current) {
      bool res= (((base.give_begin()<current.give_begin())
            && (current.give_begin()<base.give_end())
            && (base.give_end()<current.give_end()))
           || ((current.give_begin()<base.give_begin())
            && (base.give_begin()<current.give_end())
            && (current.give_end()<base.give_end())));
      return res;
   }
private:
   Constituent base;
};

void select_first_in_tree(Tree *t) {
   hyps_cnt += t->getHypothesisCount();
   Tree::struc_iterator i=t->struc_begin();
   while (i != t->struc_end()) {
      Tree::struc_iterator new_end=remove_if(i+1, t->struc_end(), Overlap(*i));
      t->erase(new_end, t->struc_end());
      ++i;
   }
   consts_cnt += t->getHypothesisCount();
}

void select_first(Treebank *tb) {
   int treenr=1;
   if (!preserve) {
      for (Treebank::iterator s=tb->begin(); s!=tb->end(); ++s) {
         debug(program_name, debug_flag, "Select in tree", treenr++);
         select_first_in_tree((Tree*)&*s);
      }
   } else {
      Tree t;
      while(*ifs >> t) {
         debug(program_name, debug_flag, "Select in tree", treenr++);
         select_first_in_tree(&t);
         *ofs << t << endl;
         t.clear();
      }
   }
}

typedef pair<Sentence, Nonterminal> Phrase;

class Probability_store {
public:
   void increase(const Phrase& phrase) {
      ++pstore[phrase];
      nstore[phrase.second]++;
   }

   const int count(const pair<Sentence, Nonterminal>& phrase) const {
      return pstore.find(phrase)->second;
   }

   const int count(const Nonterminal& nonterm) const {
      return nstore.find(nonterm)->second;
   }
protected:
private:
   map<Phrase, int> pstore;
   map<Nonterminal, int> nstore;
};

void compute_probabilities_in_tree(const Tree& t, Probability_store *prob) {
   for (Tree::const_struc_iterator c=t.struc_begin(); c != t.struc_end(); c++) {
      // take only first nonterminal (should be clustered)
      Phrase p;
      if (select_type == CONST) {
        p=make_pair(
          Sentence(t.begin()+c->give_begin(), t.begin()+c->give_end()),
          (*c)[0]);
      } else {
        p=make_pair(
          Sentence(t.begin()+c->give_begin(), t.begin()+c->give_end()),
          Nonterminal(0));
      }
      prob->increase(p);
   }
}

void compute_probabilities(Treebank &tb, Probability_store *prob) {
   int treenr=1;
   if (!preserve) {
      for (Treebank::const_iterator t=tb.begin(); t != tb.end(); t++) {
         debug(program_name, debug_flag, "Computing in tree", treenr++);
         compute_probabilities_in_tree(*t, prob);
      }
   } else {
      Tree t;
      while(*ifs >> t) {
         debug(program_name, debug_flag, "Computing in tree", treenr++);
         compute_probabilities_in_tree(t, prob);
         tb.push_back(t);
         t.clear();
      }
   }
}

double compute_probability(Phrase p, const Probability_store &prob) {
   return -log(double(prob.count(p))/prob.count(p.second));
}

double compute_combined_probability(Tree* t, const vector<Constituent> &vc,
  const Probability_store &prob) {
   double p=-log((double)1);
   Sentence::size_type size=0;
   for(vector<Constituent>::const_iterator c=vc.begin(); c != vc.end(); c++) {
      if (select_type == CONST) {
        p+=compute_probability(make_pair(
          Sentence(t->begin()+c->give_begin(), t->begin()+c->give_end()),
          (*c)[0]), prob);
      } else {
        p+=compute_probability(make_pair(
          Sentence(t->begin()+c->give_begin(), t->begin()+c->give_end()),
          Nonterminal(0)), prob);
      }
      size++;
   }
   if(size!=0) {
      return p/size;
   }
   return -1;
}

typedef pair<Sentence::size_type, Sentence::size_type> Range;
typedef map<Range, vector<Constituent> > Knowledge_base;

Tree::struc_iterator
find_constituent(Tree::struc_iterator b, Tree::struc_iterator e,
  Sentence::size_type begin, Sentence::size_type end) {
   Tree::struc_iterator tmp=b;
   while (tmp != e) {
      if ((tmp->give_begin() == begin) && (tmp->give_end() == end)) {
         return tmp;
      }
      tmp++;
   }
   return e;
}

vector<Constituent>
select_prob_in_range(Tree *t, vector<Constituent>::iterator ob,
  vector<Constituent>::iterator oe, Sentence::size_type begin,
  Sentence::size_type end, Knowledge_base &known, const Probability_store &prob) {
   Knowledge_base::iterator k=known.find(make_pair(begin, end));
   if (k!=known.end()) {
      return k->second;
   }
   if (begin+1==end) {
      known.insert(make_pair(make_pair(begin, end), vector<Constituent>()));
      return vector<Constituent>();
   }
   Sentence::size_type boundary=begin+1;
   vector<vector<Constituent> > n_best;
   n_best.push_back(vector<Constituent>());
   double best_p=compute_combined_probability(t, n_best[0], prob);
   bool uninit=true;
   double new_p;
   vector<Constituent>::size_type max_size=0;
   while(boundary!=end) {
      vector<Constituent> res
         =select_prob_in_range(t, ob, oe, begin, boundary, known, prob);
      vector<Constituent> res2
         =select_prob_in_range(t, ob, oe, boundary, end, known, prob);
      res.insert(res.end(), res2.begin(), res2.end());
      if (((new_p=compute_combined_probability(t, res, prob))<=best_p)
         ||(uninit)) {
         if (uninit) {
            best_p=new_p;
            uninit=(best_p<0);
         }
         if ((new_p>=0)&&(new_p<best_p)) {
            n_best.clear();
            max_size=0;
            best_p=new_p;
         }
         if (res.size()>max_size) {
            max_size=res.size();
         }
         n_best.push_back(res);
      }
      boundary++;
   }
   vector<vector<Constituent> > new_best;
   for (vector<vector<Constituent> >::iterator i=n_best.begin();
     i!=n_best.end(); i++) {
      if (i->size() == max_size) {
         new_best.push_back(*i);
      }
   }
   vector<vector<Constituent> >::iterator best;
   best=new_best.begin()+(new_best.size()>1?r(new_best.size()):0);
   vector<Constituent>::iterator beconst=find_constituent(ob, oe, begin, end);
   if (beconst != oe) {
      best->push_back(*beconst);
   }
   known.insert(make_pair(make_pair(begin, end), *best));
   return *best;
}

class Overlap_in_tree {
public:
   Overlap_in_tree(const Tree& c):base(c) { }

   typedef Constituent argument_type;

   bool
   operator() (const Constituent& current) const {
      Tree::const_struc_iterator i=base.struc_begin();
      while (i!=base.struc_end()) {
         if (((i->give_begin()<current.give_begin())
               &&(current.give_begin()<i->give_end())
               &&(i->give_end()<current.give_end()))
              ||((current.give_begin()<i->give_begin())
               &&(i->give_begin()<current.give_end())
               &&(current.give_end()<i->give_end()))) {
            return true;
         }
         i++;
      }
      return false;
   }
private:
   Tree base;
};

void select_prob_in_tree(Tree *t, const Probability_store& prob) {
   hyps_cnt += t->getHypothesisCount();
   Knowledge_base known;
   vector<Constituent> overlap;
   remove_copy_if(t->struc_begin(),
    t->struc_end(), back_inserter(overlap), not1(Overlap_in_tree(*t)));
   t->erase(remove_if(t->struc_begin(), t->struc_end(), Overlap_in_tree(*t)),
         t->struc_end());
   vector<Constituent> res=select_prob_in_range(t, overlap.begin(),
     overlap.end(), 0, t->size(), known, prob);
   consts_cnt += t->getHypothesisCount() + res.size();
   for (vector<Constituent>::iterator i=res.begin(); i!=res.end(); i++) {
      t->add_structure(*i);
   }
}

void select_prob(Treebank *tb, const Probability_store& prob) {
   int treenr=1;
   for (Treebank::iterator t=tb->begin(); t != tb->end(); t++) {
      debug(program_name, debug_flag, "Select in tree", treenr++);
      select_prob_in_tree((Tree*)&*t, prob);
      if (preserve) {
         *ofs << *t << endl;
      }
   }
}

void select(Treebank *tb) {
   switch (select_type) {
   case FIRST:
      debug(program_name, debug_flag, "Selecting constituents");
      select_first(tb);
      break;
   default:
      Probability_store prob;
      debug(program_name, debug_flag, "Computing probabilities");
      compute_probabilities(*tb, &prob);
      debug(program_name, debug_flag, "Selecting constituents");
      select_prob(tb, prob);
      break;
   }
}

void write_treebank(const Treebank& tb) {
   if (!preserve) {
      *ofs << tb;
   }
}

void outit() {
   if (ifs!=&cin) delete ifs;
   if (ofs!=&cout) delete ofs;
}

void write_infoheader(const Treebank& tb, char** args, int& argsc, int& c, int& d) {

   *ofs << "# " << setiosflags(ios::left) << setw(14) << program_name
      << " :: " << setiosflags(ios::left) << setw(16) << "execution time"
      << " :: " << getDate();
   *ofs << "# " << setiosflags(ios::left) << setw(14) <<  program_name
      << " :: " << setiosflags(ios::left) << setw(16) << "command call"
      << " :: ";

   for(int i=0; i<argsc;i++) {
      *ofs << args[i] << " ";
   }
   *ofs << endl;

   *ofs << "# " << setiosflags(ios::left) << setw(14) << program_name
      << " :: " << setiosflags(ios::left) << setw(16) << "hyps loaded"
      << " :: " << c << endl;
   *ofs << "# " << setiosflags(ios::left) << setw(14) << program_name
      << " :: " << setiosflags(ios::left) << setw(16) << "hyps selected"
      << " :: " << d << endl;

   for (unsigned int i=0; i < tb.comments.size(); i++) {
      *ofs << tb.comments.at(i) << endl;
   }

}


int main(int argc, char* argv[]) {
   const clock_t startTime = clock();
   handle_arguments(argc, argv);
   Treebank tb;
   read_treebank(tb);
   select(&tb);
   if (verbose_flag) {
      cerr << program_name << " : # sentences loaded            : "
         << tb.size() << endl;
   }
   write_infoheader(tb,argv,argc,hyps_cnt,consts_cnt);
   write_treebank(tb);
   outit();
   if (verbose_flag) {
     cerr << program_name << " : # hypotheses loaded           : "
         << hyps_cnt << endl;
     cerr << program_name << " : # hypotheses selected         : "
         << consts_cnt << endl;
     cerr << program_name << " : # seconds execution time      : "
         << (double)(clock()-startTime)/CLOCKS_PER_SEC << endl;
   }
   return 0;
}
