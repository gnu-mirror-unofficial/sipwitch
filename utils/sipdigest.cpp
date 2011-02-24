// Copyright (C) 2008-2010 David Sugar, Tycho Softworks.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <ucommon/ucommon.h>
#include <ucommon/secure.h>
#include <sipwitch/sipwitch.h>
#include <config.h>

using namespace UCOMMON_NAMESPACE;
using namespace SIPWITCH_NAMESPACE;

extern "C" int main(int argc, char **argv)
{
    char *realm = NULL;
    const char *user, *secret;
    const char *mode = "md5";
    char buffer[128];
    string_t digestbuf;

    while(NULL != *(++argv)) {
        if(!strcmp(*argv, "--")) {
            ++argv;
            break;
        }

        if(!strncmp(*argv, "--", 2))
            ++*argv;

        if(!strcmp(*argv, "-version")) {
            printf("sipdigest 0.1\n"
                "Copyright (C) 2008 David Sugar, Tycho Softworks\n"
                "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
                "This is free software: you are free to change and redistribute it.\n"
                "There is NO WARRANTY, to the extent permitted by law.\n");
            exit(0);
        }

        if(!strcmp(*argv, "-r") || !strcmp(*argv, "-realm")) {
            if(NULL == *(++argv)) {
                fprintf(stderr, "*** sipdigest: realm option missing\n");
                exit(3);
            }
            realm = strdup(*argv);
            continue;
        }

        if(String::equal(*argv, "-md5") || String::equal(*argv, "-sha1") || String::equal(*argv, "-rmd160"))
        {
            mode = strdup(*argv + 1);
            continue;
        }

        if(!strncmp(*argv, "-realm=", 7)) {
            realm = strdup(*argv + 7);
            continue;
        }

        if(!strcmp(*argv, "-?") || !strcmp(*argv, "-h") || !strcmp(*argv, "-help")) {
usage:
            fprintf(stderr, "usage: sipdigest [options] userid secret\n"
                "[options]\n"
                "  -md5                 specify md5 computation\n"
                "  -realm \"string\"    specify realm for digest\n"
                "  -rmd160              specify rmd160 computation\n"
                "  -sha1                specify sha1 computation\n"
                "Report bugs to sipwitch-devel@gnu.org\n");
            exit(3);
        }

        if(**argv == '-') {
            fprintf(stderr, "*** sipdigest: %s: unknown option\n", *argv);
            exit(3);
        }

        break;
    }

    user = *(argv++);
    if(!user) {
        fprintf(stderr, "*** sipdigest: userid missing\n");
        exit(3);
    }

    secret = *(argv++);
    if(!secret) {
        fprintf(stderr, "*** sipdigest: secret missing\n");
        exit(3);
    }

    if(!realm) {
        fsys_t fs;
        fsys::open(fs, DEFAULT_CFGPATH "/siprealm", fsys::ACCESS_RDONLY);
        if(!is(fs))
            fsys::open(fs, DEFAULT_VARPATH "/lib/sipwitch/uuid", fsys::ACCESS_RDONLY);

        if(!is(fs)) {
            fprintf(stderr, "*** sipdigest: no public realm known\n");
            exit(4);
        }
        memset(buffer, 0, sizeof(buffer));
        fsys::read(fs, buffer, sizeof(buffer) - 1);
        fsys::close(fs);

        char *cp = strchr(buffer, '\n');
        if(cp)
            *cp = 0;

        cp = strchr(buffer, ':');
        if(cp)
            *(cp++) = 0;

        if(cp && *cp)
            mode = cp;
        realm = strdup(buffer);
    }

    if(*argv)
        goto usage;

    digest_t digest = mode;
    if(digest.puts((string_t)user + ":" + (string_t)realm + ":" + (string_t)secret))
        digestbuf = *digest;
    else
        shell::errexit(6, "** sipdigest: unsupported computation");

    printf("%s\n", *digestbuf);
    exit(0);
}

