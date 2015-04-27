use strict;
use warnings;

use Test::More tests => 13;
BEGIN {
    use_ok('DBNG::Service');
    use_ok('DBNG::Service::Passwd');
};

my $passwd = DBNG::Service::Passwd->new('/home/mikey/tmp/dbng-test');
isa_ok($passwd, 'DBNG::Service::Passwd');
$passwd->truncate;

my $res = $passwd->get('chris');
is($res, undef, 'get failed as expected');

$passwd->add('chris:x:2000:2000:Chris Judd:/home/chris:/bin/bash');
$res = $passwd->get('chris');
is(ref $res, 'HASH', 'user found');
is($res->{uid}, 2000);
is($res->{gid}, 2000);
is($res->{name}, 'chris');
is($res->{passwd}, 'x');
is($res->{gecos}, 'Chris Judd');
is($res->{shell}, '/bin/bash');
is($res->{homedir}, '/home/chris');

$passwd->delete('chris');
$res = $passwd->get('chris');
is($res, undef);
