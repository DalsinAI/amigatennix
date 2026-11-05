
/**
 *
 * Tennix Archive File Format
 * Copyright (C) 2009-2010 Thomas Perl <thp@thpinfo.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA  02110-1301, USA.
 *
 **/

#include "tennix.h"

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>

#include "archive.h"

static int
extract_file(TennixArchive *tnxar, bool verbose)
{
    char *data = tnxar->getItemBytes();

    if (verbose) {
        printf("%s\n", tnxar->getItemFilename());
    }

    FILE *fp = fopen(tnxar->getItemFilename(), "wb");
    if (fp == NULL) {
        return 1;
    }
    fwrite(data, tnxar->getItemSize(), 1, fp);
    fclose(fp);

    free(data);

    return 0;
}


class ArchiveTool {
    public:
        ArchiveTool(int argc, char **argv);

        int run();

    private:
        int usage();

        int create(bool verbose);
        int test(bool verbose);
        int extract(bool verbose);

        int argc;
        char **argv;
};

ArchiveTool::ArchiveTool(int argc, char **argv)
    : argc(argc)
    , argv(argv)
{
}

int
ArchiveTool::run()
{
    bool do_create = false;
    bool do_test = false;
    bool do_extract = false;
    bool verbose = false;

    if (argc < 2) {
        return usage();
    }

    char *opts = argv[1];
    if (*opts == '-') opts++;

    while (*opts) {
        switch (*opts) {
            case 'c':
                do_create = true;
                break;
            case 't':
                do_test = true;
                break;
            case 'x':
                do_extract = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'f':
                if (opts[1]) {
                    return usage();
                }
                break;
            default:
                return usage();
                break;
        }
        opts++;
    }

    if (do_create && !do_test && !do_extract && argc > 3) {
        return create(verbose);
    } else if (do_test && !do_create && !do_extract && argc > 2) {
        return test(verbose);
    } else if (do_extract && !do_create && !do_test && argc > 2) {
        return extract(verbose);
    }

    return usage();
}

int
ArchiveTool::create(bool verbose)
{
    if (argc < 4) {
        fprintf(stderr, "Refusing to create an empty archive.\n");
        return 1;
    }

    char *archive = argv[2];
    if (strcmp(".tnx", archive + strlen(archive) - 4) != 0) {
        fprintf(stderr, "Wrong file extension: %s\n", archive);
        return 1;
    }

    TennixArchive tnxar;
    for (int i=3; i<argc; i++) {
        FILE *fp = fopen(argv[i], "rb");
        fseek(fp, 0, SEEK_END);
        size_t len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char *data = (char*)malloc(len);
        tnx_assert(fread(data, len, 1, fp) == 1);
        fclose(fp);
        char *filename = strdup(basename(argv[i]));
        if (verbose) {
            printf("%s\n", filename);
        }
        tnxar.appendItem(filename, data, len);
    }
    tnxar.buildFile(archive);
    return 0;
}

int
ArchiveTool::test(bool verbose)
{
    TennixArchive tnxar(argv[2]);

    while (!tnxar.endOfFile()) {
        if (verbose) {
            printf("%ld %s\n", tnxar.getItemSize(), tnxar.getItemFilename());
        } else {
            printf("%s\n", tnxar.getItemFilename());
        }
        tnxar.next();
    }

    return 0;
}

int
ArchiveTool::extract(bool verbose)
{
    TennixArchive tnxar(argv[2]);

    if (argc == 3) {
        while (!tnxar.endOfFile()) {
            if (extract_file(&tnxar, verbose) != 0) {
                printf("Error extracting file\n");
                return 1;
            }
            tnxar.next();
        }
        return 0;
    }

    for (int i=3; i<argc; i++) {
        if (tnxar.setItemFilename(argv[i])) {
            if (extract_file(&tnxar, verbose) != 0) {
                printf("Error extracting file: %s\n", argv[i]);
                return 1;
            }
        } else {
            fprintf(stderr, "File not in archive: %s\n", argv[i]);
            return 1;
        }
    }

    return 0;
}

int
ArchiveTool::usage()
{
    fprintf(stderr, "Usage: %s [-ctxvf] <archive.tnx> file1 ...\n", argv[0]);
    fprintf(stderr, "    -c ... Create archive\n");
    fprintf(stderr, "    -t ... Test archive (list contents)\n");
    fprintf(stderr, "    -x ... Extract archive\n");
    fprintf(stderr, "    -v ... Verbose output\n");
    fprintf(stderr, "    -f ... Filename of archive (optional)\n");
    return 1;
}

#undef main

int
main(int argc, char **argv)
{
    ArchiveTool tool(argc, argv);
    return tool.run();
}

