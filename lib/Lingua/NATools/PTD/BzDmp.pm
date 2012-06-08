package Lingua::NATools::PTD::BzDmp;

#------------------------------------------------------------------#
# NATools - Package with parallel corpora tools                    #
# Copyright (C) 2008-2010 Alberto Simões                           #
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

use IO::Compress::Bzip2     qw(bzip2 $Bzip2Error);
use IO::Uncompress::Bunzip2 qw(bunzip2 $Bunzip2Error);

=encoding UTF-8

=head1 NAME

Lingua::NATools::PTD::BzDmp - Sub-module to handle PTD bzipped files in Dumper Format

=head1 SYNOPSIS

  use Lingua::NATools::PTD;

  $ptd = Lingua::NATools::PTD->new( "file.dmp.bz2" );

=head1 DESCRIPTION

Check L<<Lingua::NATools::PTD>> for complete reference.

=head1 SEE ALSO

NATools, perl(1)

=head1 AUTHOR

Alberto Manuel Brandão Simões, E<lt>ambs@cpan.orgE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2011-2012 by Alberto Manuel Brandão Simões

=cut

sub new {
    my ($class, $filename) = @_;
    my $self;
    bunzip2 $filename => \$self or die "Failed to bunzip: $Bunzip2Error.";
    {
        no strict;
        $self = eval $self;
        die $@ if $@;
    }
    bless $self => $class #amen
}

sub _save {
    my ($self, $filename) = @_;

    my $z = new IO::Compress::Bzip2 $filename or return 0;
    select $z;
    $self->dump;
    $z->close;

    return 1;
}

"This isn't right.  This isn't even wrong.";
__END__
