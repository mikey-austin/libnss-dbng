#use 5.018004;
use 5.010001; # EL6 version
use ExtUtils::MakeMaker;

WriteMakefile(
    NAME         => 'DBNG::Service',
    VERSION_FROM => 'lib/DBNG/Service.pm',
    LIBS         => ['-L../../lib/.libs -L/usr/lib -L/usr/local/lib -ldbng'],
    DEFINE       => '-DHAVE_CONFIG_H -DHAVE_DB_H',
    INC          => '-I. -I../../lib -I/usr/include/dbng -I/usr/local/include',
    );
