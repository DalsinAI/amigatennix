
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

#include <iostream>

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __amigaos__
/* No arpa/inet.h in libnix; the 68k is big-endian (network order). */
#define ntohl(x) SDL_SwapBE32(x)
#define htonl(x) SDL_SwapBE32(x)
#else
#include <arpa/inet.h>
#endif

#include "archive.h"

static void
xormem(char* mem, uint32_t length, char key)
{
   char *i = mem, *end = mem+length;
   for(; i != end; i++) {
       *i ^= key;
   }
}

TennixArchive::TennixArchive()
    : fp(NULL)
    , header()
    , items(NULL)
    , blobs(NULL)
    , offset(0)
    , current_item(0)
{
    strcpy(header.header, TENNIX_ARCHIVE_HEADER);
    header.items = 0;
}

TennixArchive::TennixArchive(const char* filename, const char* fallback)
    : fp(NULL)
    , header()
    , items(NULL)
    , blobs(NULL)
    , offset(0)
    , current_item(0)
{
    fp = fopen(filename, "rb");
    if (fp == NULL && fallback != NULL) {
        fp = fopen(fallback, "rb");
    }

    if (fp == NULL) {
        fprintf(stderr, "Cannot open archive: %s\n", filename);
#ifdef __amigaos__
        tnx_amiga_message("Tennix", "Cannot open the game data, tennix.tnx.\n"
                "It belongs in the Tennix drawer, beside the program.");
#endif
        exit(1);
    }

    offset = sizeof(TennixArchiveHeader)*fread(&(header), sizeof(TennixArchiveHeader), 1, fp);
    tnx_assert(offset == sizeof(TennixArchiveHeader));
    tnx_assert(strncmp(header.header, TENNIX_ARCHIVE_HEADER, TENNIX_ARCHIVE_HEADER_LEN) == 0);
    tnx_assert(header.versionmajor == TENNIX_ARCHIVE_VERSIONMAJOR);
    tnx_assert(header.versionminor == TENNIX_ARCHIVE_VERSIONMINOR);

    items = (TennixArchiveItem*)calloc(header.items, sizeof(TennixArchiveItem));
    tnx_assert(items != NULL);
    offset += sizeof(TennixArchiveItem)*fread(items, sizeof(TennixArchiveItem), header.items, fp);
    tnx_assert(offset == sizeof(TennixArchiveHeader) + header.items*sizeof(TennixArchiveItem));

    xormem((char*)(items), header.items*sizeof(TennixArchiveItem), header.key);

    for (int i=0; i<header.items; i++) {
        /* convert offset + length from network byte order */
        items[i].offset = ntohl(items[i].offset);
        items[i].length = ntohl(items[i].length);
    }
}

std::ostream&
operator<<(std::ostream& out, TennixArchiveHeader& header)
{
    out << "Header: " << header.header << std::endl;
    out << "Version: " <<
        (int)header.versionmajor << '.' <<
        (int)header.versionminor << std::endl;
    out << "Master key: " << header.key << std::endl;
    out << "Items: " << header.items;

    return out;
}

std::ostream&
operator<<(std::ostream& out, TennixArchiveItem& item)
{
    out << "File: " << item.filename << std::endl;
    out << "Size: " << item.length << std::endl;
    out << "Offset: " << item.offset << std::endl;
    out << "Key: " << (int)item.key;

    return out;
}

int
TennixArchive::setItemFilename(const char* filename)
{
    int i;

    for (i=0; i<header.items; i++) {
        if (strncmp(items[i].filename, filename, TENNIX_ARCHIVE_ITEM_MAXNAME) == 0) {
            current_item = i;
            return 1;
        }
    }

    return 0;
}

char*
TennixArchive::getItemBytes()
{
    size_t size = getItemSize();
    char* data = (char*)malloc(size+1);
    tnx_assert(data != NULL);
    /* the last char is a null character, so this works for strings, too */
    data[size]='\0';
    fseek(fp, items[current_item].offset, SEEK_SET);
    tnx_assert(fread(data, size, 1, fp) == 1);
    xormem(data, size, items[current_item].key);
    return data;
}

void
TennixArchive::appendItem(char* filename, char* data, uint32_t length)
{
    TennixArchiveItem* item;

    header.items++;
    items = (TennixArchiveItem*)realloc(items, sizeof(TennixArchiveItem)*header.items);
    blobs = (char**)realloc(blobs, sizeof(char*)*header.items);

    item = &(items[header.items-1]);
    blobs[header.items-1] = data;
    for (int i=0; i<TENNIX_ARCHIVE_ITEM_MAXNAME; i++) {
        item->filename[i] = data[(i*2)%length];
    }
    strcpy(item->filename, filename);
    item->length = length;
}

void
TennixArchive::buildFile(char* filename)
{
    size_t offset = 0;
    size_t *memsize = NULL;

    memsize = (size_t*)calloc(header.items, sizeof(size_t));

    fp = fopen(filename, "wb");
    tnx_assert(fp != NULL);

    offset += sizeof(TennixArchiveHeader) + header.items*sizeof(TennixArchiveItem);

    header.versionmajor = TENNIX_ARCHIVE_VERSIONMAJOR;
    header.versionminor = TENNIX_ARCHIVE_VERSIONMINOR;

    header.key = (0xaa + 0x77*header.items*3) % 0xff;

    for (int i=0; i<header.items; i++) {
        items[i].offset = htonl(offset); /* network byte order */
        items[i].key = 0xaa ^ ((i<<2)%0x100);
        xormem(blobs[i], items[i].length, items[i].key);
        memsize[i] = items[i].length;
        offset += items[i].length;
        items[i].length = htonl(items[i].length); /* network byte order */
        xormem((char*)(items + i), sizeof(TennixArchiveItem), header.key);
    }

    tnx_assert(fwrite(&(header), sizeof(TennixArchiveHeader), 1, fp) == 1);
    tnx_assert(fwrite(items, sizeof(TennixArchiveItem), header.items, fp) == header.items);
    for (int i=0; i<header.items; i++) {
        tnx_assert(fwrite(blobs[i], memsize[i], 1, fp) == 1);
        free(blobs[i]);
    }

    free(memsize);
    free(blobs);
}

std::ostream&
operator<<(std::ostream& out, TennixArchive& archive)
{
    out << "Tennix Archive" << std::endl;
    out << archive.header << std::endl;

    for (int i=0; i<archive.header.items; i++) {
        out << "=======================" << std::endl;
        out << archive.items[i] << std::endl;
    }
    out << "=== END OF ARCHIVE ====" << std::endl;

    return out;
}

