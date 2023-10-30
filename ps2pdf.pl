#!/usr/bin/perl
#
# wbc April 11 2023 - for new MacOS one has to convert ps to pdf manually
# to uses preview to look at the tablature
#
# $ps2pdf="/usr/bin/gs -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=- - ";
$ps2pdf="/opt/homebrew/bin/gs  -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=- ";
#
#
$ifile = "out.ps";
$ofile = "out.pdf";

open (my $t, "-|", "$ps2pdf  $ifile") or die "can't tab file\n";

open (my $o, ">" ,$ofile) or die "Can't open output file";

while (<$t>) {
    print  $o $_;
}

close $t;
close $o;
exit;
