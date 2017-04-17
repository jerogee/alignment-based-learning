#!/usr/bin/perl

use strict;
use warnings;

sub remove {
   my($line) = $_[0];
   if ($line =~ /-NONE-/) { # WSJ
      $line =~ s/\(\s*-NONE-[^\)]*\s*\)//g; # remove -NONE- constituent
      while ($line =~ /\([^\s]+\s*\)/) {
         $line =~ s/\([^\s]+\s*\)//g; # remove (nonterm ) constituents
      }
   }
   if ($line =~ /\*/) {
      $line =~ s/\([^\s]+\s+\*[^\)]*\s*\)//g; # remove simple * constituent
      while ($line =~ /\([^\s]+\s*\)/) {
         $line =~ s/\([^\s]+\s*\)//g; # remove (nonterm ) constituents
      }
   }
   $line =~ s/\s+/ /g; # remove multiple spaces
   return $line;
}

while (my $line = <>) {
   chomp($line);
   my($rm_line)=remove($line);
   print "$rm_line\n";
}
