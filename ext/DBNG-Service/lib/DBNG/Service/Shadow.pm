package DBNG::Service::Shadow;

use strict;
use warnings;
use base qw(DBNG::Service);

sub new {
    my ($class, $base) = @_;
    $class->SUPER::new(DBNG::Service::TYPE_SHADOW, $base);
}

1;
