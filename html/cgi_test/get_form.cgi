#!/usr/bin/perl -wT
use CGI qw(:standard);
use CGI::Carp qw(warningsToBrowser fatalsToBrowser);
use strict;

#my $q = CGI->new($ENV{'QUERY_STRING'});
my $q = CGI->new();
print start_html("Get Form");
#my(%Variables);
#my buffer = $ENV{'QUERY_STRING'};
my $data = $q->param('fname');
print "$data\n";
$data = $q->param('lname');
print "$data";
#print "$buffer<br>\n";
my %form;
foreach my $p (param()) {
    $form{$p} = param($p);
    print "$p = $form{$p}<br>\n";
}
print end_html();
