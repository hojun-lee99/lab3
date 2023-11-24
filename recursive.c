#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

void list_files_recursive(const char *path);

void list_files(const char *path) {

    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(path))) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // . .. 파일 출력 제외
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        //파일명 출력
        printf("%s\t", entry->d_name);
    }
    printf("\n");

    closedir(dir);

    printf("\n");
    
    list_files_recursive(path);
}

void list_files_recursive(const char *path) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(path))) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char subdir[1024];
            snprintf(subdir, sizeof(subdir), "%s/%s", path, entry->d_name);
            printf("%s:\n", subdir);
            list_files(subdir);
        }
    }

    closedir(dir);
}

int main() {
    const char *target_directory = "."; // 현재 디렉토리

    list_files(target_directory);

    return 0;
}
