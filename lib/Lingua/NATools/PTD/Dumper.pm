package Lingua::NATools::PTD::Dumper;

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

=head1 NAME

Lingua::NATools::PTD::Dumper - Sub-module to handle PTD files in Dumper Format

=head1 SYNOPSIS

  use Lingua::NATools::PTD;

  $ptd = Lingua::NATools::PTD->new( "file.dmp" );

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
    my $self = do $filename;
    bless $self => $class #amen
}

sub _save {
    my ($self, $filename) = @_;

    open OUT, ">:utf8", $filename or return 0;
    select OUT;
    print "use utf8;";
    $self->dump;
    close OUT;

    return 1;
}

"This isn't right.  This isn't even wrong.";
__END__
