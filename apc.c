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
    if (!file_exists("/usr/bin/apc") || !file_exists("/bin/apc")) {
        printf("APC isn't installed!!! Please install it by the command: apc setup\n");
        exit(1);
    }
}

void setup_apc(const char *self_path) {
    printf("Looks like this is the first launch of APC.\n");
    printf("Setup starting..\n\n");

    char cmd[PATH_MAX];

    printf("[1/2] Copying apc binary to /usr/bin\n");
    snprintf(cmd, sizeof(cmd), "cp %s /usr/bin/apc && chmod 755 /usr/bin/apc", self_path);
    system(cmd);

    printf("[2/2] Copying apc binary to /bin/\n");
    snprintf(cmd, sizeof(cmd), "cp %s /bin/apc && chmod 755 /bin/apc", self_path);
    system(cmd);

    printf("...done\n");
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

int main(int argc, char *argv[]) {
    check_root();

    if (argc < 2) {
        printf("Usage:\n");
        printf("  %s setup\n", argv[0]);
        printf("  %s install <pkg1> [<pkg2> ...]\n", argv[0]);
        printf("  %s delete <pkgname>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "setup") == 0) {
        char self_path[PATH_MAX];
        if (realpath(argv[0], self_path) == NULL) {
            perror("realpath");
            return 1;
        }
        setup_apc(self_path);
    } else if (strcmp(argv[1], "install") == 0) {
        check_apc_installed();
        if (argc < 3) {
            printf("Usage: %s install <pkg1> [<pkg2> ...]\n", argv[0]);
            return 1;
        }

        printf("Starting APC v1.0-r1...\n\nHere's the all packets i found:\n\n");
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
        } else if (strcmp(choice, "N") == 0 || strcmp(choice, "n") == 0) {
            exit(0);
        } else {
            printf("It's not a choice.\n");
            exit(1);
        }
    } else if (strcmp(argv[1], "delete") == 0) {
        check_apc_installed();
        if (argc != 3) {
            printf("Usage: %s delete <pkgname>\n", argv[0]);
            return 1;
        }
        delete_package(argv[2]);
    } else {
        printf("Unknown command!\n");
        return 1;
    }

    return 0;
}
