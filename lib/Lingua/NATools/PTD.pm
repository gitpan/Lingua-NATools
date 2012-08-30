package Lingua::NATools::PTD;

#------------------------------------------------------------------#
# NATools - Package with parallel corpora tools                    #
# Copyright (C) 2008-2012 Alberto Simões                           #
#                                                                  #
# This package is free software; you can redistribute it and/or    #
# modify it under the terms of the GNU Lesser General Public       #
# License as published by the Free Software Foundation; either     #
# version 2 of the License, or (at your option) any later version. #
#                                                                  #
# This library is distributed in the hope that it will be useful,  #
# but WITHOUT ANY WARRANTY; without even the implied warranty of   #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU #
# Lesser General Public License for more details.                  #
#                                                                  #
# You should have received a copy of the GNU Lesser General Public #
# License along with this library; if not, write to the            #
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,     #
# Boston, MA 02111-1307, USA.                                      #
#------------------------------------------------------------------#

use base 'Exporter';
our @EXPORT = 'toentry';
our @EXPORT_OK = qw/bws ucts/;
use warnings;
use strict;

use POSIX qw/setlocale/;
setlocale(&POSIX::LC_CTYPE, "pt_PT");
use locale;

use Time::HiRes;
use Lingua::NATools::PTD::Dumper;
use Lingua::NATools::PTD::BzDmp;
use Lingua::NATools::PTD::XzDmp;
use Lingua::NATools::PTD::SQLite;

our $VERSION = '1.1';

=encoding UTF-8

=head1 NAME

Lingua::NATools::PTD - Module to handle PTD files in Dumper Format

=head1 SYNOPSIS

  use Lingua::NATools::PTD;

  $ptd = Lingua::NATools::PTD->new( $ptd_file );

=head1 DESCRIPTION

PTD files in Perl Dumper format are simple hashes references. But they
use a specific structure, and this module provides a simple interface to
manipulate it.

=head2 C<new>

The C<new> constructor returns a new Lingua::NATools::PTD object. This constructor
receives a PTD file in dumper format.

  my $ptd = Lingua::NATools::PTD->new( $ptd_file );

If the filename matches with C<< /dmp.bz2$/ >> (that is, ends in
dmp.bz2) it is considered to be a bzip2 file and will be decompressed
in the fly.

If it ends in C<<.sqlite>>, then it is supposed to contain an SQLite
file with the dictionary (with Lingua::NAToolsools standard schema!).

Extra arguments are a flatenned hash with configuration
variables. Following options are recognized:

=over 4

=item C<verbose>

Sets verbosity.

  my $ptd = Lingua::NATools::PTD->new( $ptd_file, verbose => 1 );

=back

=cut

sub new {
    my ($class, $filename, %ops) = @_;
    die "Can't find ptd [$filename]\n" unless -f $filename;

    my $self;
    # switch
    $self = Lingua::NATools::PTD::Dumper->new($filename) if $filename =~ /\.dmp$/i;
    $self = Lingua::NATools::PTD::BzDmp ->new($filename) if $filename =~ /\.dmp\.bz2$/i;
    $self = Lingua::NATools::PTD::XzDmp ->new($filename) if $filename =~ /\.dmp\.xz$/i;
    $self = Lingua::NATools::PTD::SQLite->new($filename) if $filename =~ /\.sqlite$/i;

    # default
    $self = Lingua::NATools::PTD::Dumper->new($filename) unless $self;

    $self->_calculate_sizes() unless $self->size; # in case it is already calculated

    # configuration variables
    $self->verbose($ops{verbose}) if exists $ops{verbose};

    return $self;
}

=head2 C<verbose>

With no arguments returns if the methods are configured to use verbose
mode, or not. If an argument is supplied, it is interpreted as a
boolean value, and sets methods verbosity.

   $ptd->verbose(1);

=cut

sub verbose {
    my $self = shift;
    if (defined($_[0])) {
        $self->{' verbose '} = shift
    } else {
        $self->{' verbose '} || 0
    }
}

=head2 C<dump>

The C<dump> method is used to write the PTD in its own format, but
taking care to sort words lexicographically, and sorting translations by
their probability (starting with higher probabilities).

The format is Perl code, and thus, can be used independetly of this module.

   $ptd->dump;

Note that the C<dump> method writes to the Standard Output stream.

=cut

sub dump {
    my $self = shift;

    binmode STDOUT, ":utf8";
    print "use utf8;\n";
    print "\$a = {\n";
    $self->downtr(
                  sub {
                      my ($w,$c,%t) = @_;			
                      printf "  '%s' => {\n", _protect_quotes($w);
                      printf "      count => %d,\n", $c;
                      printf "      trans => {\n";
                      for my $t (sort { $t{$b} <=> $t{$a} } keys %t) {
                          printf "          '%s' => %.6f,\n", _protect_quotes($t), $t{$t};
                      }
                      printf "      }\n";
                      printf "  },\n";
                  },
                  sorted => 1,
                 );
    print "}\n";
}

=head2 C<words>

The C<words> method returns an array (not a reference) to the list of
words of the dictionary: its domain. Pass a true value as argument and
the list is returned sorted.

   my @words = $ptd->words;

=cut

sub words {
    my $self = shift;
    my $sorted = shift;
    if ($sorted) {
        return sort grep {!/^ /} keys %$self;
    } else {
        return grep {!/^ /} keys %$self;
    }
}

=head2 C<trans>

The C<trans> method receives a word, and returns the list of its possible
translations.

   my @translations = $ptd->trans( "dog" );

=cut

sub trans {
    my ($self, $word, $trans) = @_;
    return () unless exists $self->{$word};
    if ($trans) {
        return (exists($self->{$word}{trans}{$trans}))?1:0;
    } else {
        return keys %{$self->{$word}{trans}};
    }
}


=head2 C<exists>

Checks if a word is in a dictionary

=cut

sub exists {
    my ($self, $word ) = @_;
    return exists $self->{$word};
}

=head2 C<transHash>

The C<transHash> method receives a word, and returns an hash where
keys are the its possible translations, and values the corresponding
translation probabilities.

   my %trans = $ptd->transHash( "dog" );

Returns the empty hash if the word does not exist.

=cut

sub transHash {
    my ($self, $word) = @_;
    my %h = ();
    for my $t ($self->trans($word)) {
        $h{$t} = $self->prob($word, $t);
    }
    return %h;
}

=head2 C<prob>

The C<prob> method receives a word and a translation, and returns the
probability of that word being translated that way.

   my $probability = $ptd->prob("cat", "gato");

=cut

sub prob {
    my ($self, $word, $trad) = @_;
    return 0 unless exists $self->{$word}{trans}{$trad};
    return $self->{$word}{trans}{$trad};
}

=head2 C<size>

Returns the total number of words from the source-corpus that originated
the PTD. Basically, the sum of the C<count> attribute for all words.

   my $size = $ptd->size;

=cut

sub size {
    return $_[0]->{' size '}; # space is relevant
}

=head2 C<count>

The C<count> method receives a word and returns the occurrence count for
that word.

   my $count = $ptd->count("cat");

If no argument is supplied, returns the total dictionary count (sum of
all words).

=cut

sub count {
    my ($self, $word) = @_;
    if (defined($word)) {
        if (exists($self->{$word})) {
            return $self->{$word}{count}
        } else {
            return 0;
        }
    } else {
        return $self->{" count "};
    }
}

=head2 C<stats>

Computes a bunch of statistics about the PTD and returns them in an
hash reference.

=cut

sub stats {
    my $self = shift;
    my $stats = {
                 size  => $self->size,
                 count => $self->count,
                };

    $self->downtr( sub {
                       my ($w, $c, %t) = @_;
                       $c ||= 1;
                       $stats->{avgTransNr} += scalar(keys %t);
                       $stats->{occTotal}   += $c;
                       if (!$stats->{occMin} || $stats->{occMin} > $c) {
                           $stats->{occMin} = $c;
                           $stats->{occMinWord} = $w;
                       }
                       if (!$stats->{occMax} || $stats->{occMax} < $c) {
                           $stats->{occMax} = $c;
                           $stats->{occMaxWord} = $w;
                       }
                       if (%t) {
                           my ($bestProb) = sort { $b <=> $a } values %t;
                           if (!$stats->{probMax} || $stats->{probMax} < $bestProb) {
                               $stats->{probMax} = $bestProb;
                           }
                           if (!$stats->{probMin} || $stats->{probMin} > $bestProb) {
                               $stats->{probMin} = $bestProb;
                           }
                           $stats->{avgBestTrans} += $bestProb;
                       }
                   });
    $stats->{avgTransNr}   /= $stats->{count};
    $stats->{avgBestTrans} /= $stats->{count};
    $stats->{avgOcc}        = $stats->{occTotal} / $stats->{count};
    return $stats;

}

=head2 C<subtractDomain>

This method subtracts to the domain of a PTD, the elements present on
a set of elements. This set can be defines as another PTD (domain is
used), as a Perl array reference, as a Perl hash reference (domain is
used) or as a Perl array (not reference). Returns the dictionary after
domain subtraction takes place.

  # removes portuguese articles from the dictionary
  $ptd->subtractDomain( qw.o a os as. );

  # removes a set of stop words from the dictionary
  $ptd->subtractDomain( \@stopWords );

  # removes the words present on other_ptd from ptd
  $ptd->subtractDomain( $other_ptd );

=cut

sub subtractDomain {
    my ($self, $other, @more) = @_;
	
    my @domain;
    if (ref($other) =~ /Lingua::NATools::PTD/ and $other->isa("Lingua::NATools::PTD")) {
        @domain = $other->words;
    }
    elsif (ref($other) eq "ARRAY") {
        @domain = @$other
    }
    elsif (ref($other) eq "HASH") {
        @domain = keys %$other
    }
    else {
        @domain = ($other, @more);
    }
    my %domain;
    @domain{@domain} = @domain;
	
    $self -> downtr (
                     sub {
                         my ($w,$c,%t) = @_;
                         return exists($domain{$w}) ? undef : toentry($w,$c,%t)
                     },
                     filter => 1,
                    );
    $self->_calculate_sizes();
    return $self;
}



=head2 C<restrictDomain>

Domain restrict function: interface is similar to subtractDomain function

This method restricts the domain of a PTD to a set of elements.  This
set can be defines as another PTD (domain is used), as a Perl array
reference, as a Perl hash reference (domain is used) or as a Perl
array (not reference). Returns the dictionary after domain restriction
takes place.

  # restrict the dictionary to a set of words
  $ptd->restrictDomain( \@someWords );

=cut

sub restrictDomain {
    my ($self, $other, @more) = @_;
	
    my @domain;
    if (ref($other) =~ /Lingua::NATools::PTD/ and $other->isa("Lingua::NATools::PTD")) {
        @domain = $other->words;
    }
    elsif (ref($other) eq "ARRAY") {
        @domain = @$other
    }
    elsif (ref($other) eq "HASH") {
        @domain = keys %$other
    }
    else {
        @domain = ($other, @more);
    }
    my %domain;
    @domain{@domain} = @domain;
	
    $self -> downtr (
                     sub {
                         my ($w,$c,%t) = @_;
                         return exists($domain{$w}) ? toentry($w,$c,%t):undef
                     },
                     filter => 1,
                    );
    $self->_calculate_sizes();
    return $self;
}

=head2 C<reprob>

This method recalculates all probabilities accordingly with the number
of translations available.

For instance, if you have

    home => casa => 25%
         => lar  => 25%

The resulting dictionary will have

   home => casa => 50%
        => lar  => 50%

Note that this methods B<replaces> the object.

=cut

sub reprob {
    my $self = shift;
    $self->downtr(
                  sub {
                      my ($w, $c, %t) = @_;
                      my $actual = 0;
                      $actual += $t{$_} for (keys %t);
                      return undef unless $actual > 0.1;
                      $t{$_} /= $actual for (keys %t);
                      return toentry($w, $c, %t);
                  },
                  filter => 1
                 );
    return $self;
}

=head2 C<intersect>

This method intersects the current object with the supplied PTD.
Note that this method B<replaces> the object values.

Occurrences count in the final dictionary is the minimum occurrence
value of the two dictionaries.

Only translations present on both dictionary are kept. The probability
will be the minimum on the two dictionaries.

=cut

sub intersect {
    my ($self, $other) = @_;

    $self->downtr
      (
       sub {
           my ($w, $c, %t) = @_;
           if ($other->trans($w)) {
               $c = _min($c, $other->count($w));
               for my $t (keys %t) {
                   if ($other->trans($w,$t)) {
                       $t{$t} = _min($t{$t}, $other->trans($w,$t));
                   }
                   else {
                       delete($t{$t});
                   }
               }
               return toentry($w, $c, %t);
           } else {
               return undef;
           }
       },
       filter => 1);
    $self->_calculate_sizes();
}

sub _set_word_translation {
    my ($self, $word, $translation, $probability) = @_;
    $self->{$word}{trans}{$translation} = $probability;
}

sub _delete_word_translation {
    my ($self, $word, $translation) = @_;
    delete($self->{$word}{trans}{$translation});
}

sub _set_word_count {
    my ($self, $word, $count) = @_;
    $self->{$word}{count} = $count;
}

sub _delete_word {
    my ($self, $word) = @_;
    delete $self->{$word};
}

=head2 C<add>

This method adds the current PTD with the supplied one (first
argument).  Note that this method B<replaces> the object values.

=cut

sub add {
    my ($self, $other, %ops) = @_;

    $ops{verbose} //= $self->verbose;

    my ($S1,$S2) = ($self->size,  $other->size);

    $other->_init_transaction;
    $self->downtr(sub {
                      my ($w, $c, %t) = @_;
                      if ($other->trans($w)) {
                          my ($c1, $c2) = ($c, $other->count($w));
                          for my $t (_uniq(keys %t, $other->trans($w))) {
                              my ($p1, $p2) = ($t{$t} || 0, $other->prob($w,$t));
                              my ($w1, $w2) = ($c1 * $S2, $c2 * $S1);
                              if ($w1 + $w2) {
                                  $t{$t} = ($w1 * $p1 + $w2 * $p2)/($w1 + $w2);
                              } else {
                                  delete $t{$t};
                              }
                          }
                          toentry($w, $c1+$c2, %t);
                      } else {
                          toentry($w,$c,%t);
                      }
                  },
                  filter => 1,
                  verbose => $ops{verbose},
                 );
    $other->_commit;

    $self->_init_transaction;
    print STDERR "\tAdding new words\n" if $ops{verbose};
    $other->downtr(sub {
                       my ($w, $c, %t) = @_;
                       return if $self->trans($w);
                       $self->_set_word_count($w, $c);
                       for my $t (keys %t) {
                           $self->_set_word_translation($w, $t, $t{$t});
                       }
                   },
                   verbose => $ops{verbose}
                  );
    $self->_commit;
    $self->_calculate_sizes();
    return $self;
}

sub _uniq {
    my %f;
    $f{$_}++ for @_;
    return keys %f;
}

=head2 C<downtr>

This method iterates over a dictionary and calls the function supplied
as argument. This function will receive, in each call, the word in the
source language, the number of occurrences, and the hash of
translations.

  $ptd->downtr( sub { my ($w,$c,%t) = @_;
                      if ($w =~ /[^A-Za-z0-9]/) {
                          return undef;
                      } else {
                          return toentry($w,$c,%t);
                      }
              },
             filter => 1);

Set the filter flag if your downtr function is replacing the original
dictionary.

=cut

sub _init_transaction { }
sub _commit { }

sub downtr {
    my ($self, $sub, %opt) = @_;

    $opt{verbose} //= $self->verbose;

    my $time = [Time::HiRes::gettimeofday];
    my $counter = 0;
    $self->_init_transaction;

    my @keys = $opt{sorted} ? $self->words(1) : $self->words;
    for my $word (@keys) {
        my $res = $sub->($word,
                         $self->count($word),
                         $self->transHash($word));
        if ($opt{filter}) {
            if (!defined($res)) {
                $self->_delete_word($word)
            } else {
                $self->_update_word($word, $res);
            }			
        }

        $counter ++;
        print STDERR "\r\tProcessing ($counter entries)..." if $opt{verbose} && !($counter%100);
    }
    $self->_commit;
    $self->_calculate_sizes() if $opt{filter};

    my $elapsed = Time::HiRes::tv_interval($time);
    printf STDERR "\r\tProcessed %d entries (%.2f seconds).\n", 
             $counter, $elapsed if $opt{verbose};
}

sub _update_word {
    my ($self, $word, $res) = @_;
    my ($k) = keys %$res;
    $res = $res->{$k};
    if ($k eq $word) {
        $self->{$word} = $res;
    } else {
        delete $self->{$word};
        $self->{$k} = $res;
    }
}

# sub _trans_hash {
#     my ($self, $word) = @_;
#     return %{$self->{$word}{trans}};
# }

=head2 C<toentry>

This function is exported by default and creates a dictionary entry
given the word, word count, and hash of translations. Check C<downtr>
for an example.

=cut

sub toentry {
    ## word, count, ref(%hash)
    if (ref($_[2]) eq "HASH") {
        return { $_[0] => { count => $_[1], trans => $_[2] }}
    }
    else {
        my ($w, $c, %t) = @_;
        return { $w => { count => $c, trans => \%t } }
    }
}

=head2 C<saveAs>

Method to save a PTD in another format. First argument is the name of
the format, second is the filename to be used. Supported formats are
C<<dmp>> for Perl Dump format, C<<bz2>> for Bzipped Perl Dump format,
C<<xz>>, for Lzma xz Perl Dump format and C<<sqlite>> for SQLite
database file.

Return undef if the format is not known. Returns 0 if save failed. A
true value in success.

=cut

sub saveAs {
    my ($self, $type, $filename) = @_;

    warn "Lingua::NATools::PTD saveAs called without all required parameteres" unless $type && $filename;

    my $done = undef;
    # switch
    Lingua::NATools::PTD::Dumper::_save($self => $filename) and $done = 1 if $type =~ /dmp/i;
    Lingua::NATools::PTD::BzDmp::_save( $self => $filename) and $done = 1 if $type =~ /bz2/i;
    Lingua::NATools::PTD::XzDmp::_save( $self => $filename) and $done = 1 if $type =~ /xz/i;
    Lingua::NATools::PTD::SQLite::_save($self => $filename) and $done = 1 if $type =~ /sqlite/i;
    # XXX - add above in the documentation.

    # default
    warn "Requested PTD filetype is not known" unless defined $done;

    return $done;
}

=head2 C<lowercase>

This method replaces the dictionary, B<in place>, lowercasing all
entries. This is specially usefull to process transation dictionaries
obtained with the C<-utf8> flag that (at the moment) does case
sensitive alignment.

   $ptd->lowercase(verbose => 1);

=cut

sub lowercase {
    my ($self, %ops) = @_;

    $ops{verbose} //= $self->verbose;

    $self->downtr(
                  sub {
                      my ($w, $c, %t) = @_;

                      for my $k (keys %t) {
                          next unless $k =~ /[[:upper:]]/;

                          my $lk = lc $k;
                          $t{$lk} = exists($t{$lk}) ? $t{$lk} + $t{$k} : $t{$k};
                          delete $t{$k};
                      }

                      if ($w =~ /[[:upper:]]/) {
                          my $lw = lc $w;

                          my %ot = $self->transHash($lw);
                          if (%ot) {
                              my ($c1, $c2) = ($c, $self->count($lw));
                              for my $k (_uniq(keys %t, keys %ot)) {
                                  my ($p1, $p2) = ($t{$k} || 0, $ot{$k} || 0);
                                  if ($c1 + $c2) {
                                      $t{$k} = ($c1 * $p1 + $c2 * $p2)/($c1+$c2);
                                  } else {
                                      delete $t{$k};
                                  }
                              }
                              toentry($lw, $c1+$c2, %t)
                          } else {
                              toentry($lw, $c, %t)
                          }
                      } else {
                          toentry($w, $c, %t);
                      }
                  },
                  filter  => 1,
                  verbose => $ops{verbose},
                 );
}

=head2 C<ucts>

Create unambiguous-concept traslation sets.

  my $result =  ucts($ptd1, $ptd2, m=>0.1, M=>0.8);

Available options are:

=over 4

=item C<m>

Mininum number of occurences of each token. Must be an
integer (default: 10).

=item C<M>

Manixum number of occurences of each token. Must be an
integer (default: 100).

=item C<p>

Minimum probabilty for translation. Must be a probability
in the interval [0,1] (default: 0.2).

=item C<P>

Minimum probabilty for the inverse translations. Must be a
probability in the interval [0,1] (default: 0.8).

=item C<r=0|1>

Print rank (default: 0).

=item C<pp=0|1>

Pretty print output (default: 0).

=item C<output=filename>

Pretty print output to file C<filename>.

=back

=cut

sub ucts {
    my ($fileA, $fileB, %my_opts) = @_;

    my $min_occur = $my_opts{m} || 10;
    my $max_occur = $my_opts{M} || 100;
    my $prob = $my_opts{p} || 0.2;
    my $probi = $my_opts{P} || 0.8;
    my $rank = $my_opts{r} || 0;
    my $pp = $my_opts{pp} || 0;
    my $output = $my_opts{output} || '';

    # check files exist
    unless ($fileA and $fileB) {
       die "Error: need at least two PTDs given as argument.";
    }

    # handle output handles
    $pp = 1 if $output;
    open STDOUT, '>', $output if $output;
    binmode(STDOUT, ':utf8'); # XXX

    # load PTDs
    my $ptd;
    if (ref($fileA) =~ m/^Lingua::NATools::PTD/) {
        $ptd = $fileA;
    }
    else {
       if (-e $fileA) {
           $ptd = Lingua::NATools::PTD->new($fileA);
       }
       else {
           die "Error: file not found: $_";
       }
    }
    my $ptd_inv;
    if (ref($fileB) =~ m/^Lingua::NATools::PTD/) {
        $ptd_inv = $fileB;
    }
    else {
       if (-e $fileB) {
           $ptd_inv = Lingua::NATools::PTD->new($fileB);
       }
       else {
           die "Error: file not found: $_";
       }
    }

    if ($pp and $fileA =~ m/.*?(\w\w)\-(\w\w)/) { # XXX
        print "Langs: $1, $2\n" if $pp;
    }

    my (%left, %right);

    # process each word in the PTD
    my @words = $ptd->words;
    foreach (@words) {
        my $r = __build_ucts($ptd, $ptd_inv, $min_occur, $max_occur, $prob, $probi, $_);
        $left{$_} = $r if $r;
    }
    # process each word in the inverse PTD
    @words = $ptd_inv->words;
    foreach (@words) {
        my $r = __build_ucts($ptd_inv, $ptd, $min_occur, $max_occur, $prob, $probi, $_);
        $right{$_} = $r if $r;
    }

    my @final = ();
    foreach my $l (keys %left) {
        my %ll = ($l=>1);
        my %rr;
        $rr{$_}++ for @{$left{$l}->{trans}};
        my $rank = $left{$l}->{rank};

        foreach (@{$left{$l}->{trans}}) {
            $rr{$_}++;
            if (exists($right{$_})) {
                $ll{$_}++ for @{$right{$_}->{trans}};
                delete $right{$_};
            }
        }
        push @final, {l=>[keys %ll], r=>[keys %rr], rank=>$rank};
    }
    foreach my $r (keys %right) {
        my %ll;
        my %rr = ($r=>1);;
        $ll{$_}++ for @{$right{$r}->{trans}};
        my $rank = $right{$r}->{rank};

        push @final, {l=>[keys %ll], r=>[keys %rr], rank=>$rank};
    }

    if ($pp) {
        __pp_ucts($_,$rank) foreach (@final);
    }
    else {
        return [@final];
    }
}

sub __build_ucts {
   my ($ptd, $ptd_inv, $min_occur, $max_occur, $prob, $probi, $word) = @_;

   my $count = $ptd->count($word); ##  or print STDERR "### $word\n";
   $count //= 0;
   return undef unless ($min_occur <= $count and $count <= $max_occur);

   my $total = 0;
   my %trans = ();
   my %transHash = $ptd->transHash($word);

   foreach (keys %transHash) {
      my $p = $transHash{$_};
      next unless ($p >= $prob);
      my $p_inv = $ptd_inv->prob($_, $word);
      next unless ($p_inv >= $probi);

      my $counti = $ptd_inv->count($_);
      if ( ($min_occur <= $counti) and ($counti <= $max_occur) ) {
         if ($total) { $total = ($total+$p+$p_inv)/2; }
         else { $total = $p+$p_inv; }

         $trans{$_}++;
      }
   }

   return undef unless %trans;
   return {trans=>[keys %trans], rank=>$total};
}

=head2 C<bws>

Create bi-words sets given a PTD pair.

  my $result = bws($ptd1, $ptd2, m=>0.1, p=>0.4);

C<$ptd1> and C<$ptd2> can be filenames for the PTDs or already create
PTD objects.

The following options are available:

=over 4

=item C<m>

Mininum number of occurences of each token. Must be an integer
(default: 10).

=item C<p>

Minimum probabilty for translation. Must be a probability
in the interval [0,1] (default: 0.4).

=item C<r=0|1>

Print rank (default: 0).

=item C<pp=0|1>

Pretty print output (default: 0).

=item C<output=filename>

Pretty print output to file C<filename>.

=back

=cut

sub bws {
    my ($fileA, $fileB, %my_opts) = @_;

    my $min_occur = $my_opts{m} || 10;
    my $prob = $my_opts{p} || 0.4;
    my $rank = $my_opts{r} || 0;
    my $pp = $my_opts{pp} || 0;
    my $output = $my_opts{output} || '';

    # check files exist
    unless ($fileA and $fileB) {
       die "Error: need at least two PTDs given as argument.";
    }

    # handle output handles
    $pp = 1 if $output;
    open STDOUT, '>', $output if $output;
    binmode(STDOUT, ':utf8'); # XXX

    # load PTDs
    my $ptd;
    if (ref($fileA) =~ m/^Lingua::NATools::PTD/) {
        $ptd = $fileA;
    }
    else {
       if (-e $fileA) {
           $ptd = Lingua::NATools::PTD->new($fileA);
       }
       else {
           die "Error: file not found: $_";
       }
    }
    my $ptd_inv;
    if (ref($fileB) =~ m/^Lingua::NATools::PTD/) { 
        $ptd_inv = $fileB; 
    } 
    else { 
       if (-e $fileB) {
           $ptd_inv = Lingua::NATools::PTD->new($fileB);
       }
       else {
           die "Error: file not found: $_";
       }
    }

    if ($pp and $fileA =~ m/.*?(\w\w)\-(\w\w)/) { # XXX
        print "Langs: $1, $2\n" if $pp;
    }

    my @final;

    my @words = $ptd->words;
    foreach my $word (@words) {
        my $count = $ptd->count($_);
        next unless ($count >= $min_occur);

        my %transHash = $ptd->transHash($word);
        foreach (keys %transHash) {
            my $p = $transHash{$_};
            next unless ($p >= $prob);

            __pp_ucts({l=>[$word],r=>[$_],rank=>$p}, $rank) if $pp;
            push @final, {l=>$word, r=>$_, rank=>$p} unless $pp;
        }
    }
    @words = $ptd_inv->words;
    foreach my $word (@words) {
        my $count = $ptd->count($_);
        next unless ($count >= $min_occur);

        my %transHash = $ptd->transHash($word);
        foreach (keys %transHash) {
            my $p = $transHash{$_};
            next unless ($p >= $prob);

            __pp_ucts({l=>[$_],r=>[$word],rank=>$p}, $rank) if $pp;
            push @final, {l=>$_, r=>$word, rank=>$p} unless $pp;
        }
    }

    close STDOUT;
    return [@final] unless $pp;
}

sub __pp_ucts {
    my ($r, $rank) = @_;

    if ($rank) {
        printf "[%f]%s=%s\n", $r->{rank}, (join ',', @{$r->{l}}), join ',', @{$r->{r}};
    }
    else {
        printf "%s=%s\n", (join ',', @{$r->{l}}), join ',', @{$r->{r}};
    }
}

=head1 SEE ALSO

NATools, perl(1)

=head1 AUTHOR

Alberto Manuel Brandão Simões, E<lt>ambs@cpan.orgE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2008-2012 by Alberto Manuel Brandão Simões

=cut

sub _calculate_sizes {
    my $self = shift;
    my $total = 0;
    my $count = 0;
    $self->downtr( sub { $count++; $total += $_[1] }, verbose => 0);
    $self->{" size "}  = $total;           ## Private keys are kept with spaces.
    $self->{" count "} = $count;
}

sub _min { $_[0] < $_[1] ? $_[0] : $_[1] }
sub _max { $_[0] > $_[1] ? $_[0] : $_[1] }

sub _protect_quotes {
    my $f = shift;
    for ($f) {
        s/\\/\\\\/g;
        s/'/\\'/g;
    }
    return $f;
}


"This isn't right.  This isn't even wrong.";
__END__
