use strict;
use warnings;

use Test::More tests => 14;
BEGIN {
    use_ok('DBNG::Service');
    use_ok('DBNG::Service::Shadow');
};

my $shadow = DBNG::Service::Shadow->new('/home/mikey/tmp/dbng-test');
isa_ok($shadow, 'DBNG::Service::Shadow');
$shadow->truncate;

my $res = $shadow->get('chris');
is($res, undef, 'get failed as expected');

$shadow->add('chris:$6$EUWi2ag0$OYHV5d9A8Rgz2174oaiteDfYXXkqmERMOZGjdZ248vJwM0law5B/dmHDDdUo56YvHveaAB7oP2Z221L1p0xaq/:16552:0:99999:7:::');
$res = $shadow->get('chris');
is(ref $res, 'HASH', 'user found');
is($res->{name}, 'chris');
is($res->{passwd}, '$6$EUWi2ag0$OYHV5d9A8Rgz2174oaiteDfYXXkqmERMOZGjdZ248vJwM0law5B/dmHDDdUo56YvHveaAB7oP2Z221L1p0xaq/');
is($res->{lstchg}, 16552);
is($res->{min}, 0);
is($res->{max}, 99999);
is($res->{warn}, 7);
is($res->{inact}, -1);
is($res->{expire}, -1);

$shadow->delete('chris');
$res = $shadow->get('chris');
is($res, undef);
