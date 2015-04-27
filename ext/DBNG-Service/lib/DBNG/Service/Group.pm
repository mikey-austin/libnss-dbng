package DBNG::Service::Group;

use strict;
use warnings;
use base qw(DBNG::Service);

sub new {
    my ($class, $base) = @_;
    $class->SUPER::new(DBNG::Service::TYPE_GROUP, $base);
}

sub get {
    my ($self, $key) = @_;
    $self->group_get($key);
}

1;
