/******************************************************************************»
 **
 **   Filename    : cluster.cpp
 **
 **   Description : This file implements the clustering part of the aligning
 **                 phase.
 **
 **   Version     : $Id: cluster.cpp 3780 2010-02-23 12:21:11Z menno $
 **
 ******************************************************************************»
 **   This file is part of the Alignment-Based Learning package
 **
 **   See the file "LICENCE" for information on usage and redistribution
 **   of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************»
 */

#include <fstream>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include "tools.h"
#include "constituent.h"
#include "nonterminal.h"
#include "tree.h"
#include "treebank.h"

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
using ns_tree::Tree;
using ns_treebank::Treebank;
using ns_tools::error;
using ns_tools::warning;
using ns_tools::debug;
using ns_tools::getDate;

using std::ifstream;
using std::ofstream;
using std::cin;
using std::cerr;
using std::cout;
using std::make_pair;
using std::map;
using std::ostringstream;
using std::setw;

static struct option long_options[] = {
   {"debug", no_argument, 0, 'd'},
   {"help", no_argument, 0, 'h'},
   {"input", required_argument, 0, 'i'},
   {"output", required_argument, 0, 'o'},
   {"verbose", no_argument, 0, 'v'},
   {"version", no_argument, 0, 'V'},
   {0, 0, 0, 0}
};

// Input stream (defaults to cin).
istream *ifs=&cin;
// Name of the input file.
string ifname;
// Output stream (defaults to cout).
ostream *ofs=&cout;
// Name of the program as it was called.
string program_name;
// Count input hypotheses
int hyps_cnt=0;
// Print debug information.
bool debug_flag=false;
// Print processing information
bool verbose_flag=false;

// unique NT input
int unti=0;
// unique NT output
int unto=0;

void
usage() {
   cerr << "ABL " << VERSION << endl;
   cerr << "Alignment-Based Learner" << endl;
   cerr << __DATE__ << " " << __TIME__ << "\n" << endl;
   cerr << "Usage:" << program_name;
   cerr << " [OPTION]..." << endl;
   cerr << "This program clusters hypotheses from the fuzzy trees in the ";
   cerr << "input file. It" << endl;
   cerr << "corresponds to the cluster phase of the ";
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
   cerr << "  -d, --debug        ";
   cerr << "Output debug information" << endl;
   cerr << "  -h, --help         ";
   cerr << "Show this help and exit" << endl;
   cerr << "  -v, --verbose      ";
   cerr << "Show details about the clustering process" << endl;
   cerr << "  -V, --version      ";
   cerr << "Show version information and exit" << endl;
   exit(0);
}

void handle_arguments(int argc, char* argv[]) {
   int opt;
   int option_index;
   const char* optstring="dhi:mo:vV";
   program_name=argv[0];
   bool input_ok=false, output_ok=false;
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
               ifname=optarg;
               if (!ifs) {
                  error(program_name, string("cannot open input file")+optarg);
               }
               input_ok=true;
            }
            break;
         case 'o':
            if (strcmp(optarg, "-")!=0) {
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
         case 'v':
            verbose_flag = true;
            break;
         case 'V':
            cout << "cluster (Alignment-Based Learning) version 0.1" << endl;
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
   if (optind != argc) {
      warning(program_name, "extraneous argument(s)");
   }
}

void read_treebank(Treebank& tb) {
   *ifs >> tb;
}

void write_treebank(const Treebank& tb) {
   *ofs << tb;
}

void outit() {
   if (ifs!=&cin) {
      delete ifs;
   }
   if (ofs!=&cout) {
      delete ofs;
   }
}

int upper_nt_new;

class NT_map:public map<Nonterminal, Nonterminal> {
   // The class NT_map contains the mapping from existing non-terminals in the
   // treebank to new non-terminals for hypotheses with similar context.
   public:
      // constructor
      NT_map () {
         upper_nt_new = 0;
      }

      // The procedure getNewNtCount returns the number of non-terminals
      // that have been created
      int getNewNtCount() {
         return upper_nt_new;
      }

      // The procedure getNewNT associates a given NT with a new NT value if
      // it has not been encountered before and associates this given NT with
      // an existing NT value when it has.
      Nonterminal getNewNT(const Nonterminal& nt_old) {
         map<Nonterminal, Nonterminal>::iterator it_ntmap;

         // look if the old NT exists in the map
         it_ntmap = find(nt_old);

         if (it_ntmap == end()) {
            // no -> create new NT
            Nonterminal nt_new(upper_nt_new++);
            putNTmapping(nt_old, nt_new);
            return nt_new;
         } else {
            // yes -> return NT
            return it_ntmap->second;
         }
      }

      // The procedure putNTmapping associates the first NT provided with the
      // second NT provided
      void putNTmapping(const Nonterminal& nt_old, const Nonterminal& nt_new) {
         insert(make_pair(nt_old, nt_new));
         ostringstream dmsg;
         dmsg << "map NT [" << nt_old << "] to [" << nt_new << "]";
         debug(program_name, debug_flag, dmsg.str(), 0);
     }
   private:
      static int upper_nt_new;
};

int NT_map::upper_nt_new;

void find_clusters_in_tree(Tree& t, NT_map& ntm) {
   Constituent::const_iterator i;
   Nonterminal nt_new;
   static int treenr=0;

   debug(program_name, debug_flag, "Finding clusters", ++treenr);

   // for each hypothesis in the tree
   for (Tree::const_struc_iterator h_i=t.struc_begin();  h_i!=t.struc_end();
                                                        h_i++) {
      // get the first NT
      i=h_i->begin();

      // get new NT for first old NT
      nt_new=ntm.getNewNT(*i);

      // take the next NT
      i++;

      // and assign the others the new NT
      while(i != h_i->end()) {
         ntm.putNTmapping(*i, nt_new);
         i++;
      }
   }
}

void merge_clusters_in_tree(Tree& t, NT_map& ntm) {
   Constituent::const_iterator i;
   Nonterminal nt_new;
   static int treenr=0;

   debug(program_name, debug_flag, "Merging clusters", ++treenr);

   // for each hypothesis in the tree
   for (Tree::const_struc_iterator h_i=t.struc_begin();  h_i!=t.struc_end();
                                                        ++h_i) {
      // get the first NT
      i=h_i->begin();

      // get new NT for first old NT
      nt_new=ntm.getNewNT(*i);

      // merge all remaining NTs to the first one
      ((Constituent*)&*h_i)->merge_nonterminals(nt_new);
   }
}


void find_clusters(Treebank& tb, NT_map& mapping) {
   // consider all trees in treebank
   for (Treebank::iterator t_i=tb.begin(); t_i != tb.end(); ++t_i) {
      hyps_cnt += t_i->getHypothesisCount();
      find_clusters_in_tree(*t_i, mapping);
   }
}

void merge_clusters(Treebank& tb, NT_map& mapping) {
   // consider all trees in treebank
   for (Treebank::iterator t_i=tb.begin(); t_i!=tb.end(); ++t_i) {
      merge_clusters_in_tree(*t_i, mapping);
   }
}

void cluster(Treebank& tb) {
   NT_map ntm;
   debug(program_name, debug_flag, "Finding clusters");
   find_clusters(tb, ntm);
   unti=ntm.size();
   unto=ntm.getNewNtCount();

  debug(program_name, debug_flag, "Merging clusters");
   merge_clusters(tb, ntm);
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
      << " :: " << setiosflags(ios::left) << setw(16) << "unique NTs in"
      << " :: " << c << endl;
   *ofs << "# " << setiosflags(ios::left) << setw(14) << program_name
      << " :: " << setiosflags(ios::left) << setw(16) << "unique NTs out"
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
   if (verbose_flag) {
      cerr << program_name << ": # sentences loaded            : "
         << tb.size() << endl;
   }
   cluster(tb);
   write_infoheader(tb, argv,argc,unti,unto);
   write_treebank(tb);
   outit();
   if (verbose_flag) {
      cerr << program_name << ": # hypotheses loaded           : "
         << hyps_cnt << endl;
      cerr << program_name << ": # unique non-terminals input  : "
         << unti << endl;
      cerr << program_name << ": # unique non-terminals output : "
         << unto << endl;
      cerr << program_name << ": # hypotheses clustered        : "
         << (unti - unto)
         << " (" << (double)(unti - unto)/unti
         << ")" << endl;
      cerr << program_name << ": # seconds execution time      : "
         << (double)(clock()-startTime)/CLOCKS_PER_SEC << endl;
   }
   return 0;
}
