use 5.018004;
use ExtUtils::MakeMaker;

WriteMakefile(
    NAME         => 'DBNG::Service',
    VERSION_FROM => 'lib/DBNG/Service.pm',
    LIBS         => ['-L../../lib/.libs -ldbng'], # e.g., '-lm'
    DEFINE       => '-DHAVE_CONFIG_H -DHAVE_DB_H',
    INC          => '-I. -I../../lib',
    );
