#!/usr/bin/perl -w

use Test::More;
use Test::Pod;

all_pod_files_ok(grep {!/~$/} all_pod_files(qw(../pods . scripts/)));



