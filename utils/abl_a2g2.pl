#! /usr/bin/perl

use strict;
use warnings;

while (my $line = <>) {
    my %spans;
    my ($text, $hyps) = split(/\s*@@@\s*/, $line);
    my @words = split(/\s+/,$text);

    while($hyps =~ m/\((\d+),(\d+),\[(.*?)\]\)/g) {
        my $p1 = $1;
        my $p2 = $2;
        my $nts = $3;

        $spans{$p1}{$p2} = $nts;
    }

    my %opened;
    my %closed;

    for (my $i=0; $i <= scalar @words + 1; $i++) {

        # closing
        foreach my $begin (sort {$b <=> $a} keys %spans) {
            if (defined($spans{$begin}{$i})) {
                my $nt = $spans{$begin}{$i};
               if (defined($opened{$nt})) {
                    print "<span class='nt'>)<sub>$nt</sub></span> ";
                    $closed{$nt}++;
               }
            }
         }

        # opening
        my %lens;
        foreach my $end (sort {$a <=> $b} keys %{$spans{$i}}) {
            my $nt = $spans{$i}{$end};
            $lens{$nt}=$end-$i;
        }
        foreach my $nt (sort {$b <=> $a} keys %lens) {
            print "<span class='nt'>(<sub>$nt</sub></span> ";
            $opened{$nt}++;
        }

        # closing
        foreach my $begin (sort {$b <=> $a} keys %spans) {
            if (defined($spans{$begin}{$i})) {
                my $nt = $spans{$begin}{$i};
                if (defined($opened{$nt}) && !defined($closed{$nt})) {
                    print "<span class='nt'>)<sub>$nt</sub></span> ";
                }
            }
        }

        print $words[$i]." " if (exists($words[$i]));
    }
    print "\n";
}
