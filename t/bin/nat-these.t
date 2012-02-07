#!/usr/bin/perl -w

use Test::More;

print STDERR "Warning: some of these tests can take about 1 minute on slow machines\n";

our ($DIC1,$DIC2) = (undef,undef);

ok -f "t/bin/input/EN-tok", "Testing English corpus existence";
ok -f "t/bin/input/PT-tok", "Testing Portuguese (BR) corpus existence";

# Following code will overlap with nat-these code.
# This "must" be done so tests can be done at each step.

###
### NAT-PRE
###
# 3 a 8
my @prefiles = qw{t/PT.lex t/EN.lex t/PT.crp t/PT.crp.index t/EN.crp.index t/EN.crp};
for (@prefiles) {
    unlink if -f;
    ok !-f, "Checking if file $_ does no exist";
}

`_build/apps/nat-pre -q -i t/bin/input/PT-tok t/bin/input/EN-tok t/PT.lex t/EN.lex t/PT.crp t/EN.crp`;

# 9 a 14
for (@prefiles) {
    ok -f, "Checking if file $_ exists";
}

# 15
ok -s "t/PT.crp.index" == -s "t/EN.crp.index", "Index files have the same size";

open P, "_build/apps/nat-css t/PT.lex t/PT.crp t/EN.lex t/PT.crp all|"
  or die "Cannot open nat-css as pipe";
my @corpus = <P>;
close P;

like($corpus[12], qr/^Diante das/, "Index 1 is what we hope it is");
like($corpus[16], qr/^O planejamento/, "Index 2 is what we hope it is");
like($corpus[24], qr/^Este trabalho/, "Index 5 is what we hope it is");

###
### nat-initmat
###
my @initmatfiles = qw!t/PT-EN.mat!;
for (@initmatfiles) {
  unlink if -f;
  ok(! -f, "Checking if file $_ was correctly deleted");
}
`_build/apps/nat-initmat t/PT.crp t/EN.crp t/PT-EN.mat 2>/dev/null`;
die "Failed 'nat-initmat t/PT.crp t/EN.crp t/PT-EN.mat'\n" if $?;

for (@initmatfiles) {
  ok(-f, "Checking if file $_ exists");
}

###
### nat-ipfp
###
my @ipfpfiles = qw!t/PT-EN.ipfp.mat!;
for (@ipfpfiles) {
  unlink if -f;
  ok(! -f, "Checking if file $_ was correctly deleted");
}
`_build/apps/nat-ipfp 3 t/PT.crp t/EN.crp t/PT-EN.mat t/PT-EN.ipfp.mat 2>/dev/null`;
for (@ipfpfiles) {
  ok(-f, "Checking if file $_ exists");
}

###
### nat-mat2dic   (ipfp.mat ipfp.dic)
###
my @mat2dicfiles = qw{t/PT-EN.ipfp.dic};
for (@mat2dicfiles) {
  unlink if -f;
  ok ! -f, "Checking if file $_ was correctly deleted";
}
`_build/apps/nat-mat2dic t/PT-EN.ipfp.mat t/PT-EN.ipfp.dic 2>/dev/null`;
for (@mat2dicfiles) {
  ok(-f, "Checking if file $_ exists");
}

###
### nat-postbin (PT-EN.ipfp.dic PT.lex EN.lex PT-EN.pl EN-PT.pl)
###
my @postfiles = qw{t/EN-PT.bin t/PT-EN.bin};
for (@postfiles) {
    unlink if -f;
    ok ! -f, "Checking if file $_ was correctly deleted";
}
`_build/apps/nat-postbin t/PT-EN.ipfp.dic t/PT.crp.partials t/EN.crp.partials t/PT.lex t/EN.lex t/PT-EN.bin t/EN-PT.bin`;
for (@postfiles) {
  ok -f, "Checking if file $_ exists";
}

###
### nat-dumpDict
###
my @DumpDict = qw{t/PT-EN.txt};
for (@DumpDict) {
  unlink if -f;
  ok ! -f, "Checking if file $_ still does not exist";
}
`perl -Mblib scripts/nat-dumpDicts -full t/PT.lex t/PT-EN.bin t/EN.lex t/EN-PT.bin > t/PT-EN.txt`;
for (@DumpDict) {
  ok(-f, "Checking if file $_ exist");
}

###
###
###
eval{ require "t/PT-EN.txt" };
ok(!$@, "NATools output does not have errors");

ok(defined($DIC1));
ok(defined($DIC2));

{
  my $ORIGINAL = do "t/bin/input/PT-tok.wc";
  for (keys %$ORIGINAL) {
    next if $_ eq '$';
    print STDERR "\n\n$_\n\n" unless $ORIGINAL->{$_} == $DIC1->{$_}{count};
    is($DIC1->{$_}{count}, $ORIGINAL->{$_}, "Testing word '$_'");
  }

}


{
  my $ORIGINAL = do "t/bin/input/EN-tok.wc";
  for (keys %$ORIGINAL) {
    next if $_ eq '$';
    print STDERR "\n\n$_\n\n" unless $ORIGINAL->{$_} == $DIC2->{$_}{count};
    is($DIC2->{$_}{count}, $ORIGINAL->{$_}, "Testing word '$_'");
  }

}

unlink(@prefiles,@initmatfiles,@ipfpfiles,@mat2dicfiles,@postfiles);

done_testing;

### EOF
