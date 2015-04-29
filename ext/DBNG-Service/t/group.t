use strict;
use warnings;
use Env qw(TEST_BASE);

use Test::More tests => 19;
BEGIN {
    use_ok('DBNG::Service');
    use_ok('DBNG::Service::Group');
};

my $group = DBNG::Service::Group->new($ENV{TEST_BASE} || '/home/mikey/tmp/dbng-test');
isa_ok($group, 'DBNG::Service::Group');
$group->truncate;

my $res = $group->get('chris');
is($res, undef, 'get failed as expected');

$group->add('lock:x:10000:mikey,dexter,gordon');
$res = $group->get('lock');
is(ref $res, 'HASH', 'group found');
is($res->{gid}, 10000);
is($res->{name}, 'lock');
is($res->{passwd}, 'x');
is(ref $res->{members}, 'ARRAY');
is(@{$res->{members}}, 3);

my $i = 0;
my @members = ('dexter', 'gordon', 'mikey');
foreach my $member (sort { $a cmp $b } @{$res->{members}}) {
    is($member, $members[$i++]);
}

$group->delete('lock');
$res = $group->get('lock');
is($res, undef);

$group->add('lock:x:10000:mikey,dexter,gordon');
$group->add('key:x:10000:');
$group->add('you:x:10000:another,group,member,list');
$group->add('me:x:10000:apache,list');

my %seen;
while($res = $group->next) {
    $seen{$res->{name}}++;
}

is(keys %seen, 4, 'iterations ok');
is($seen{lock}, 1);
is($seen{key}, 1);
is($seen{you}, 1);
is($seen{me}, 1);
