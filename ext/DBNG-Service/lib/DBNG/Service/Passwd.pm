package DBNG::Service::Passwd;

use 5.018004;
use strict;
use warnings;
use base qw(DBNG::Service);

sub new {
    my ($class, $base) = @_;
    $class->SUPER::new(DBNG::Service::TYPE_PASSWD, $base);
}

sub get {
    my ($self, $key) = @_;
    $self->passwd_get($key);
}

1;
