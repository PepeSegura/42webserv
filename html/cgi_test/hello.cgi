#!/usr/bin/perl -wT
use CGI qw(:standard);
use CGI::Carp qw(warningsToBrowser fatalsToBrowser);
use strict;

my $q = CGI->new($ENV{'QUERY_STRING'});
print start_html("Post cgi");
#my(%Variables);
#my buffer = $ENV{'QUERY_STRING'};
my $firstname = $q->param('fname');
my $lastname = $q->param('lname');
print h1("hello! $firstname $lastname");
print end_html();
