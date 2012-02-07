#!/usr/bin/perl

use Test::More tests => 5;
use Test::Pod::Coverage;

pod_coverage_ok "Lingua::NATools::Lexicon";
pod_coverage_ok "Lingua::NATools::CGI";
pod_coverage_ok "Lingua::NATools::Client";
pod_coverage_ok "Lingua::NATools::Config";
pod_coverage_ok "Lingua::NATools::PTD";
