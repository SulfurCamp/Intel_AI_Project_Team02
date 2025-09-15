#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <time.h>

#define SERIAL_PORT "/dev/ttyACM0"
#define BAUDRATE B115200

const char* area_keys = "QWEASDZXC";
#define AREA_CNT 9

int main()
{
    int fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CRTSCTS;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;
    tcsetattr(fd, TCSANOW, &options);

    char input[32];
    srand(time(NULL)); // 랜덤 seed

    printf("명령 입력 후 엔터 (예: Q@3), 랜덤: RANDOM@10, 종료시 ctrl+C\n");

    while (1) {
        printf("> ");
        fflush(stdout);
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        int len = strlen(input);
        if (len > 0 && input[len-1] == '\n') input[len-1] = '\0'; // 개행 제거

        // RANDOM@N 명령 처리
        if (strncmp(input, "RANDOM@", 7) == 0) {
            int n = atoi(input+7);
            if (n <= 0 || n > 1000) {
                printf("1~1000회 사이로 입력하세요\n");
                continue;
            }
            for (int i = 0; i < n; i++) {
                char cmd[8];
                int area_idx = rand() % AREA_CNT;
                int level = (rand() % 3) + 1;
                snprintf(cmd, sizeof(cmd), "%c@%d\n", area_keys[area_idx], level);
                int written = write(fd, cmd, strlen(cmd));
                if (written < 0) perror("write");
                else printf("랜덤 보냄: %s", cmd);
                usleep(200000); // 0.2s
            }
            continue;
        }

        // 일반 명령 전송
        len = strlen(input);
        if (input[len-1] != '\n') {
            input[len] = '\n';
            input[len+1] = '\0';
            len++;
        }
        int written = write(fd, input, len);
        if (written < 0) perror("write");
        else printf("보냄: %s", input);
    }
    close(fd);
    return 0;
}

