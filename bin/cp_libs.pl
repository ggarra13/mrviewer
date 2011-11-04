#!/usr/bin/perl

$dir = $0;
$dir =~ s#/[^/]*$##;

chdir $dir;

@libs=`ldd ../BUILD/Linux-3.0.0-12-generic-64/Release/bin/mrViewer.exe`;

foreach (@libs)
{
    $lib = $_;
    $lib =~ s/^.*=>(.*)/\1/g;
    $lib =~ s/\s*\(.*\)\n//g;
    $lib =~ s/^\s*//g;
    print "LIB=",$lib,"\n";
  
    $lnk=`ls -l $lib`;
    $lnk =~ /^.*->\s(.*)\s*/;
    $lnk = $1;

    print "LINK=$lnk\n";
    
    $dir=$lib;
    $dir =~ s/\/([^\/]*)$//;
    print "DIR=",$dir,"\n";

    print "cp \"$lib\" ../lib/\n";
    print "cp \"$dir/$lnk\" ../lib/\n";

    system("cp \"$lib\" ../lib/");
    system("cp \"$dir/$lnk\" ../lib/");
}
