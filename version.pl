#!/usr/bin/perl
#
#
open VERS, "version.h";
open NEW,  ">versnew.h";
while (<VERS>) {
    if (/(^.*VERSION \")(4.*)\"/) {
	($major, $minor, $patchold) = split /\./, $2;
	$patch = $patchold + 1;
	print     "$1$major\.$minor\.$patch\"\n";
	print NEW "$1$major\.$minor\.$patch\"\n";
    }
}
close VERS;
close NEW;
rename "versnew.h","version.h";

open MAKE, "Makefile";
open MNEW, ">Makefile.new";

while (<MAKE>) {
    s/lute_tab$major\.$minor.*\.tar/lute_tab$major\.$minor\.$patch\.tar/;
    print MNEW $_;
}
close MAKE;
close MNEW;
rename "Makefile.new", "Makefile";
exit;

