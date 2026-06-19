#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Программе передаются два аргумента:
//   argv[1] — IPv4-адрес сервера в десятичной записи (например, "127.0.0.1")
//   argv[2] — номер порта
//
// Программа должна:
//   1. Установить TCP-соединение с указанным сервером.
//   2. В цикле читать со stdin целые знаковые числа в текстовом формате.
//   3. Отправлять каждое число на сервер в бинарном виде (int32, Little Endian).
//   4. Получать от сервера int32 LE в ответ и выводить его в stdout в текстовом виде.
//   5. Если сервер закрыл соединение — завершиться с кодом возврата 0.

static int send_all(int fd, const void *buf, size_t len) {
    const char *p = buf;

    while (len > 0) {
        ssize_t n = send(fd, p, len, 0);
        if (n <= 0) {
            return -1;
        }
        p += n;
        len -= n;
    }

    return 0;
}

static int recv_all(int fd, void *buf, size_t len) {
    char *p = buf;

    while (len > 0) {
        ssize_t n = recv(fd, p, len, 0);

        if (n == 0) {
            return 0;
        }
        if (n < 0) {
            return -1;
        }

        p += n;
        len -= n;
    }

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ipv4_addr> <port>\n", argv[0]);
        return 1;
    }

    // TODO: создайте TCP-сокет (AF_INET, SOCK_STREAM),
    //       заполните struct sockaddr_in с помощью inet_aton/inet_pton,
    //       подключитесь через connect,
    //       реализуйте цикл чтения/отправки/приёма/вывода чисел.
    //       Порядок байт — Little Endian (на x86/x86_64 это нативный порядок).
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &addr.sin_addr) != 1) {
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return 1;
    }

    int32_t value;

    while (scanf("%" SCNd32, &value) == 1) {
        if (send_all(sock, &value, sizeof(value)) < 0) {
            close(sock);
            return 0;
        }

        int32_t answer;
        int rc = recv_all(sock, &answer, sizeof(answer));

        if (rc == 0) {
            close(sock);
            return 0;
        }
        if (rc < 0) {
            close(sock);
            return 1;
        }

        printf("%" PRId32 "\n", answer);
        fflush(stdout);
    }

    close(sock);
    return 0;
}
