/*
 *  APC - AuriLinux Packet Manager
 *  Copyright (C) 2025 LICGX Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  Licensed under GPLv2
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

const char *BASE_URL = "https://pgnmirror.github.io/apc";

void check_root() {
    if (geteuid() != 0) {
        printf("You should run this program as a root!!\n");
        exit(1);
    }
}

int file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

void check_apc_installed() {
    if (!file_exists("/usr/bin/apc") && !file_exists("/bin/apc")) {
        printf("APC isn't installed!!! Please install it by the command: apc setup\n");
        exit(1);
    }
}

void setup_apc() {
    system("pacman -S wget"); // install wget
    if (system("cp /usr/bin/apc /bin/apc && chmod 755 /bin/apc") != 0)
        system("cp /bin/apc /usr/bin/apc && chmod 755 /usr/bin/apc");
}

int url_exists(const char *pkgname) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "wget --spider -q %s/%s", BASE_URL, pkgname);
    return (system(cmd) == 0);
}

void install_package(const char *pkgname) {
    if (!url_exists(pkgname)) {
        printf("%s: Not found!\n", pkgname);
        return;
    }

    printf("%s: Found!\n", pkgname);

    char tmpfile[256], cmd[512];
    snprintf(tmpfile, sizeof(tmpfile), "/tmp/%s", pkgname);
    snprintf(cmd, sizeof(cmd), "wget -q -O %s %s/%s", tmpfile, BASE_URL, pkgname);
    if (system(cmd) != 0) {
        printf("%s: Download failed!\n", pkgname);
        return;
    }

    snprintf(cmd, sizeof(cmd), "cp %s /usr/bin/%s && chmod 755 /usr/bin/%s", tmpfile, pkgname, pkgname);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "cp %s /bin/%s && chmod 755 /bin/%s", tmpfile, pkgname, pkgname);
    system(cmd);

    printf("%s: Done!\n", pkgname);
}

void delete_package(const char *pkgname) {
    char cmd[256];
    printf("Deleting [%s]..\n", pkgname);
    snprintf(cmd, sizeof(cmd), "rm -f /usr/bin/%s", pkgname);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "rm -f /bin/%s", pkgname);
    system(cmd);
    printf("Done!\n");
}

void update_package(int argc, char *argv[]) {
    check_root();

    const char *pkg_to_update;

    if (argc > 2) {
        pkg_to_update = argv[2];
    } else {
        pkg_to_update = (sizeof(void*) == 8) ? "apc" : "apc32";
    }

    char url[512];
    snprintf(url, sizeof(url), "%s/%s", BASE_URL, pkg_to_update);

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "wget --spider -q %s", url);
    if (system(cmd) != 0) {
        printf("Package '%s' not found on server!\n", pkg_to_update);
        return;
    }

    char choice[4];
    printf("Are you sure to update '%s' from %s? (Y/n): ", pkg_to_update, url);
    if (!fgets(choice, sizeof(choice), stdin)) exit(1);
    choice[strcspn(choice, "\n")] = 0;

    if (!(strcmp(choice, "Y") == 0 || strcmp(choice, "y") == 0 || choice[0] == '\0')) {
        printf("Update canceled.\n");
        return;
    }

    char tmpfile[256];
    snprintf(tmpfile, sizeof(tmpfile), "/tmp/%s", pkg_to_update);
    snprintf(cmd, sizeof(cmd), "wget -q -O %s %s", tmpfile, url);
    printf("Downloading %s...\n", pkg_to_update);
    if (system(cmd) != 0) {
        printf("Failed to download %s!\n", pkg_to_update);
        return;
    }

    snprintf(cmd, sizeof(cmd), "cp %s /usr/bin/%s && chmod 755 /usr/bin/%s", tmpfile, pkg_to_update, pkg_to_update);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "cp %s /bin/%s && chmod 755 /bin/%s", tmpfile, pkg_to_update, pkg_to_update);
    system(cmd);

    snprintf(cmd, sizeof(cmd), "rm -f %s", tmpfile);
    system(cmd);

    printf("Package '%s' updated successfully!\n", pkg_to_update);
}

int main(int argc, char *argv[]) {
    check_root();

    if (argc < 2) {
        printf("Usage:\n");
        printf("  %s setup\n", argv[0]);
        printf("  %s install <pkg1> [<pkg2> ...]\n", argv[0]);
        printf("  %s delete <pkgname>\n", argv[0]);
        printf("  %s update [<pkgname>]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "setup") == 0) {
        char self_path[PATH_MAX];
        if (realpath(argv[0], self_path) == NULL) {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                snprintf(self_path, sizeof(self_path), "%s/%s", cwd, argv[0]);
                printf("Warning: couldn't resolve full path, using %s\n", self_path);
            } else {
                perror("realpath/getcwd");
                return 1;
            }
        }
        setup_apc();
    }
    else if (strcmp(argv[1], "install") == 0) {
        check_apc_installed();
        if (argc < 3) {
            printf("Usage: %s install <pkg1> [<pkg2> ...]\n", argv[0]);
            return 1;
        }

        printf("Starting APC v1.2-1...\n\nHere's the all packets i found:\n\n");
        for (int i = 2; i < argc; i++) {
            if (url_exists(argv[i])) {
                printf("%s\n", argv[i]);
            } else {
                printf("%s: Not found!\n", argv[i]);
            }
        }

        char choice[4];
        printf("\nAre you sure to install those packets? [Y/n] ");
        if (!fgets(choice, sizeof(choice), stdin)) exit(1);
        choice[strcspn(choice, "\n")] = 0;

        if (strcmp(choice, "Y") == 0 || strcmp(choice, "y") == 0) {
            for (int i = 2; i < argc; i++) {
                install_package(argv[i]);
            }
        } else {
            printf("Installation canceled.\n");
            exit(0);
        }
    }
    else if (strcmp(argv[1], "delete") == 0) {
        check_apc_installed();
        if (argc != 3) {
            printf("Usage: %s delete <pkgname>\n", argv[0]);
            return 1;
        }
        delete_package(argv[2]);
    }
    else if (strcmp(argv[1], "update") == 0) {
        update_package(argc, argv);
    }
    else {
        printf("Unknown command!\n");
        return 1;
    }

    return 0;
}
