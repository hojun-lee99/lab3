#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <string.h>

#define SEM_NAME "/my_sem"

void get_file_size(const char *filename, size_t *size) {
    struct stat st;
    if (stat(filename, &st) == 0)
        *size = st.st_size;
    else
        *size = 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int shm_fd;
    void *shm_ptr;
    FILE *source, *destination;
    size_t read_count, file_size;

    // 세마포어 생성 및 초기화
    sem_t *sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // 파일 크기 얻기
    get_file_size(argv[1], &file_size);

    // 공유 메모리 생성
    shm_fd = shm_open("/my_shm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // 공유 메모리 크기 설정
    if (ftruncate(shm_fd, file_size) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    // 공유 메모리 매핑
    shm_ptr = mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // 부모 프로세스가 읽는 프로세스
    if (fork() > 0) {
        source = fopen(argv[1], "r");
        if (!source) {
            perror("fopen source");
            exit(EXIT_FAILURE);
        }

        // 파일 내용을 공유 메모리에 복사
        fread(shm_ptr, 1, file_size, source);

        fclose(source);
    }
    // 자식 프로세스가 쓰는 프로세스
    else {
        destination = fopen(argv[2], "w");
        if (!destination) {
            perror("fopen destination");
            exit(EXIT_FAILURE);
        }

        // 공유 메모리의 내용을 파일에 복사
        fwrite(shm_ptr, 1, file_size, destination);

        fclose(destination);
    }

    // 세마포어를 닫음
    sem_close(sem);

    // 세마포어 삭제 (부모 프로세스만 수행)
    if (getpid() > 0) {
        sem_unlink(SEM_NAME);
    }

    // 공유 메모리 및 파일 디스크립터 정리
    munmap(shm_ptr, file_size);
    close(shm_fd);

    return 0;
}
