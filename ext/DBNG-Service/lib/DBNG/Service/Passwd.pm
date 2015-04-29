package DBNG::Service::Passwd;

use strict;
use warnings;
use base qw(DBNG::Service);

sub new {
    my ($class, $base) = @_;
    $class->SUPER::new(DBNG::Service::TYPE_PASSWD, $base);
}

1;
