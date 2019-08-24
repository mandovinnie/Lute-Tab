#!/bin/perl

# $basedir = "/net/syno-fs/wbc/src/nnt";
$basedir = "/net/syno-fs/wbc/src/m";

opendir my $dirh, $basedir or die "can't open $basedir \n";

foreach ( readdir $dirh ) {
    if ($_ =~ /\.h$/ || $_ =~ /\.cc$/) {
	my $orig = $_;
	my $tmpf = $orig . ".tmp";
	open my $in,  "<", $orig or die "can't open $orig \n";
	open my $tmp, ">", $tmpf or die "can't open $tmpf \n";
	print ("dir $orig \n");
	if ($orig =~ /\.h$/) {
	    print ($tmp "#pragma once\n");
	}
	while ( <$in> ) {
	    my $line = $_;
	    $line =~ s/[ \t]+$//g;
	    print ($tmp $line );
	}
	close $in;
	close $tmp;
	rename $tmpf, $orig;
	# unlink $tmpf;
	
	# exit;
    }
}
