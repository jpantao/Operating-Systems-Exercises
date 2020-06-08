/*
 ============================================================================
 Name        : fso-sh.c
 Author      : vad
 Version     :
 Copyright   : FSO 17/18, 18/19 - DI-FCT/UNL
 Description : mini proj - sistema de ficheiros
 ============================================================================
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

static int do_copyin(const char *filename, char *fsname);
static int do_copyout(char *fsname, const char *filename);

int main(int argc, char *argv[]) {

    char line[1024];
    char cmd[1024];
    char arg1[1024];
    char arg2[1024];
    int args, nblocks;

    if (argc != 3 && argc != 2) {
        printf("use: %s <diskfile> [nblocks]\n", argv[0]);
        return 1;
    }
    if (argc == 3)
        nblocks = atoi(argv[2]);
    else
        nblocks = -1;

    if (!disk_init(argv[1], nblocks)) {
        printf("couldn't initialize %s: %s\n", argv[1], strerror(errno));
        return 1;
    }

    printf("opened emulated disk image %s with %d blocks\n", argv[1],
           disk_size());

    while (1) {
        printf("fso-sh> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
            break;
        if (line[0] == '\n')
            continue;
        line[strlen(line) - 1] = 0;

        args = sscanf(line, "%s %s %s", cmd, arg1, arg2);
        if (args == 0)
            continue;

        if (!strcmp(cmd, "format")) {
            if (args > 2)
                printf("use: format [label]\n");
            else if (args == 1 && fs_format("NOLABEL"))
                printf("disk formatted.\n");
            else if (args == 2 && fs_format(arg1))
                printf("disk formatted.\n");
            else
                printf("format failed!\n");

        } else if (!strcmp(cmd, "debug")) {
            if (args == 1)
                fs_debug();
            else
                printf("use: debug\n");

        } else if (!strcmp(cmd, "dir")) {
            if (args == 1)
                fs_dir();
            else
                printf("use: dir\n");

        } else if (!strcmp(cmd, "mount")) {
            if (args == 1) {
                if (fs_mount())
                    printf("disk mounted.\n");
                else
                    printf("mount failed!\n");
            } else
                printf("use: mount\n");

        } else if (!strcmp(cmd, "delete")) {
            if (args == 2) {
                if (fs_delete(arg1) == 0)
                    printf("file %s deleted.\n", arg1);
                else
                    printf("delete failed!\n");
            } else
                printf("use: delete name\n");

        } else if (!strcmp(cmd, "copyin")) {
            if (args == 3) {
                if (do_copyin(arg1, arg2))
                    printf("copied file %s to fs %s\n", arg1, arg2);
                else
                    printf("copy failed!\n");
            } else
                printf("use: copyin <filename> <name>\n");

        } else if (!strcmp(cmd, "copyout")) {
            if (args == 3) {
                if (do_copyout(arg1, arg2))
                    printf("copied fs %s to file %s\n", arg1, arg2);
                else
                    printf("copy failed!\n");
            } else
                printf("use: copyout <name> <filename>\n");

        } else if (!strcmp(cmd, "cat")) {
            if (args == 2) {
                if (!do_copyout(arg1, "/dev/stdout"))
                    printf("cat failed!\n");
            } else
                printf("use: cat <name>\n");

        } else if (!strcmp(cmd, "help")) {
            printf("Commands are:\n");
            printf("    format\n");
            printf("    mount\n");
            printf("    debug\n");
            printf("    dir\n");
            printf("    delete <name>\n");
            printf("    cat     <name>\n");
            printf("    copyin  <file> <name>\n");
            printf("    copyout <name> <file>\n");
            printf("    help\n");
            printf("    quit\n");
            printf("    exit\n");
        } else if (!strcmp(cmd, "quit")) {
            break;
        } else if (!strcmp(cmd, "exit")) {
            break;
        } else {
            printf("unknown command: %s\n", cmd);
            printf("type 'help' for a list of commands.\n");
        }
    }

    printf("closing emulated disk.\n");
    disk_close();

    return EXIT_SUCCESS;
}


static int do_copyin(const char *filename, char *fsname) {
    FILE *file;
    int offset = 0, result, actual;
    char buffer[1030];

    file = fopen(filename, "r");
    if (!file) {
        printf("couldn't open %s: %s\n", filename, strerror(errno));
        return 0;
    }
    do {
        result = fread(buffer, 1, sizeof(buffer), file);
        if (result < 0)
            break;
        actual = fs_write(fsname, buffer, result, offset);
        if (actual < 0) {
            printf("ERROR: fs_write returned %d\n", actual);
            break;
        }
        if (actual != result) {
            printf("WARNING: fs_write only wrote %d bytes, not %d bytes\n",
                   actual, result);
            break;
        }
        offset += actual;
    } while ( result>0 );

    printf("%d bytes copied\n", offset);

    fclose(file);
    return 1;
}

static int do_copyout(char *fsname, const char *filename) {
    FILE *file;
    int offset = 0, result;
    char buffer[1030];

    file = fopen(filename, "w");
    if (!file) {
        printf("couldn't open %s: %s\n", filename, strerror(errno));
        return 0;
    }

    do {
        result = fs_read(fsname, buffer, sizeof(buffer), offset);
        if (result < 0) {
            printf("ERROR: fs_read returned %d\n", result);
            break;
        }
        if (result == 0)
            break;
        fwrite(buffer, 1, result, file);
        offset += result;
    } while ( result>0 );

    printf("%d bytes copied\n", offset);

    fclose(file);
    return 1;
}
