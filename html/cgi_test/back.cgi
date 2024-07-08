#!/usr/bin/perl -wT
use CGI qw(:standard);
use CGI::Carp qw(warningsToBrowser fatalsToBrowser);
#use strict;

my $q = CGI->new(STDIN);
print start_html("Post cgi");
my $firstname = $q->param('fname');
my $lastname = $q->param('lname');
print h1("hello! $firstname $lastname");
print end_html();

