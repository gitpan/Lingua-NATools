package Lingua::NATools::PTD::SQLite;

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

use base 'Lingua::NATools::PTD';
our $VERSION = '1.0';
use DBI;


=head1 NAME

Lingua::NATools::PTD::SQLite - Sub-module to handle PTD files in sqlite format

=head1 SYNOPSIS

  use Lingua::NATools::PTD;

  $ptd = Lingua::NATools::PTD->new( "file.sqlite" );

=head1 DESCRIPTION

Check L<<Lingua::NATools::PTD>> for complete reference.

=head1 SEE ALSO

NATools, perl(1)

=head1 AUTHOR

Alberto Manuel Brandão Simões, E<lt>ambs@cpan.orgE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2010-2012 by Alberto Manuel Brandão Simões

=cut

sub new {
    my ($class, $filename) = @_;
    my $dbh = DBI->connect("dbi:SQLite:dbname=$filename", "", "",
                          { sqlite_unicode => 1} ) or die "Cant connect to database";
    my $self = {
                dbh => $dbh,
                get_meta => $dbh->prepare("SELECT v FROM meta WHERE k = ?;"),
                get_occs => $dbh->prepare("SELECT occ FROM occs WHERE w = ?;"),
               };
    bless $self => $class # amen
}

sub trans {
    my ($self, $word, $trans) = @_;
    if ($trans) {
        my $sth = $self->{dbh}->prepare("SELECT p FROM trans WHERE w = ? AND t = ?;");
        $sth->execute($word, $trans);
        my @row = $sth->fetchrow_array;
        return (@row)?1:0;
    } else {
        my $sth = $self->{dbh}->prepare("SELECT t FROM trans WHERE w = ?");
        $sth->execute($word);
        my @ans;
        my @row;
        while (@row = $sth->fetchrow_array) { push @ans, $row[0] };
        return @ans;
    }
}


sub prob {
    my ($self, $word, $trans) = @_;
    my $sth = $self->{dbh}->prepare("SELECT p FROM trans WHERE w = ? AND t = ?;");
    $sth->execute($word, $trans);
    my @row = $sth->fetchrow_array;
    return (@row)?$row[0]:0;
}

sub words {
    my $self = shift;
    my $sort = $_[0] ? "ORDER BY w ASC" : "";

    my $sth = $self->{dbh}->prepare("SELECT w FROM occs $sort;");
    $sth->execute;
    my @answer = ();
    my @row;
    while (@row = $sth->fetchrow_array()) { push @answer, $row[0] };
    return @answer;
}

sub _calculate_sizes {
    my $self = shift;
    my $sth = $self->{dbh}->prepare("SELECT COUNT(w) FROM occs");
    $sth->execute;
    my @row = $sth->fetchrow_array;
    $self->_update_meta("count", $row[0]);

    $sth = $self->{dbh}->prepare("SELECT SUM(occ) FROM occs");
    $sth->execute;
    @row = $sth->fetchrow_array;
    $self->_update_meta("size", $row[0]);
}

sub _set_word_translation {
    my ($self, $w, $t, $p) = @_;
    my $sth = $self->{dbh}->prepare("INSERT OR REPLACE INTO trans VALUES(?,?,?);");
    $sth->execute($w, $t, $p);
}

sub _delete_word_translation {
    my ($self, $w, $t) = @_;
    my $sth = $self->{dbh}->prepare("DELETE FROM trans WHERE w = ? AND t = ?");
    $sth->execute($w, $t);
}

sub _set_word_count {
    my ($self, $w, $c) = @_;
    my $sth = $self->{dbh}->prepare("INSERT OR REPLACE INTO occs VALUES(?,?);");
    $sth->execute($w, $c);
}

sub _update_meta {
    my ($self, $k, $v) = @_;
    my $sth = $self->{dbh}->prepare("INSERT OR REPLACE INTO meta VALUES(?,?);");
    $sth->execute($k,$v);
}


sub size {
    my $self = shift;
    $self->{get_meta}->execute("size");
    my @row = $self->{get_meta}->fetchrow_array;
    return @row ? $row[0] : 0;
}

sub count {
    my ($self, $word) = @_;
    if ($word) {
        $self->{get_occs}->execute($word);
        my @row = $self->{get_occs}->fetchrow_array;
        return (@row)?$row[0]:0;
    } else {
        $self->{get_meta}->execute("count");
        my @row = $self->{get_meta}->fetchrow_array;
        return $row[0];
    }
}

sub _init_transaction {
    my $self = shift;
    $self->{dbh}->begin_work;
}

sub _commit {
    my $self = shift;
    $self->{dbh}->commit;
}

sub _update_word {
    my ($self, $word, $entry) = @_;

    my ($k) = keys %$entry;
    $entry = $entry->{$k};

    my $sth;

    if ($k ne $word) {
        $sth = $self->{dbh}->prepare("DELETE FROM occs WHERE w = ?");
        $sth->execute($word);

        $sth = $self->{dbh}->prepare("DELETE FROM trans WHERE w = ?");
        $sth->execute($word);

        $word = $k;
    }

    $sth = $self->{dbh}->prepare("DELETE FROM occs WHERE w = ?");
    $sth->execute($word);

    $sth = $self->{dbh}->prepare("INSERT INTO occs VALUES(?, ?);");
    $sth->execute($word, $entry->{count}, );

    $sth = $self->{dbh}->prepare("DELETE FROM trans WHERE w = ?");
    $sth->execute($word);

    $sth = $self->{dbh}->prepare("INSERT INTO trans VALUES (?,?,?)");
    for my $t (keys %{$entry->{trans}}) {
        $sth->execute($word, $t, $entry->{trans}{$t});
    }
}

sub _delete_word {
    my ($self, $word) = @_;
    my $sth = $self->{dbh}->prepare("DELETE FROM trans WHERE w = ?");
    $sth->execute($word);
    $sth = $self->{dbh}->prepare("DELETE FROM occs WHERE w = ?");
    $sth->execute($word);
}

sub _trans_hash {
    my ($self, $trans) = @_;
    my $sth = $self->{dbh}->prepare("SELECT t, p FROM trans WHERE w = ?");
    $sth->execute($trans);
    my %t;
    my @row;
    while (@row = $sth->fetchrow_array) {
        $t{$row[0]} = $row[1];
    }
    return %t;
}

sub _save {
    my ($self, $filename) = @_;

    unlink $filename if -f $filename;
    warn "Cant write on $filename" and return 0 if -f $filename;

    my $dbh = DBI->connect("dbi:SQLite:dbname=$filename", "", "", {sqlite_unicode => 1});
    warn "Cant create sqlite file" and return 0 unless $dbh;

    $dbh->do("CREATE TABLE trans (w, t, p REAL);");
    $dbh->do("CREATE TABLE occs  (w, occ INTEGER);");
    $dbh->do("CREATE TABLE meta  (k PRIMARY KEY, v);");

    my $insert_meta  = $dbh->prepare("INSERT INTO meta  VALUES (?, ?);");
    my $insert_trans = $dbh->prepare("INSERT INTO trans VALUES (?,?,?);");
    my $insert_occs  = $dbh->prepare("INSERT INTO occs  VALUES (?, ?);");

    $dbh->begin_work;

    $insert_meta->execute("size",  $self->size);
    $insert_meta->execute("count", $self->count);

    $self->downtr( sub {
                       my ($w, $c, %t) = @_;
                       $insert_occs->execute($w, $c);
                       for my $t (keys %t) {
                           $insert_trans->execute($w, $t, $t{$t});
                       }
                   });

    $dbh->commit;

    $dbh->do("CREATE INDEX transPM ON trans (w,t)");
    $dbh->do("CREATE INDEX occsPM ON occs (w)");

    return 1;
}


"This isn't right.  This isn't even wrong.";
__END__