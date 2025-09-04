/* DB Client (iot_client 스타일 그대로 출력, MariaDB 버전)
 * 실행: ./db_client <IP> <port> <name>
 *
 * 인증    : "[<name>:PASSWD]"
 * 송신    : "[TO]msg" / 없으면 자동 [ALLMSG]
 * 수신출력: 서버 문자열 그대로 fputs (원본과 동일)
 *
 * 백그라운드 처리:
 *   [RASPBERRYPI] Q@1  -> INSERT INTO events(locate, distance_level)  // 항상 숫자(1/2/3)만 저장
 *   [ANY] GETDB        -> 전체 (time, locate, distance_level) 라인 단위 스트리밍 응답
 *                         GETDB_BEGIN / per-row / GETDB_END
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <mariadb/mysql.h>
#include <errno.h>

#define BUF_SIZE   100
#define NAME_SIZE  20

/* ---- 네트워크 ---- */
static int  sock = -1;
static char name_[NAME_SIZE] = "[Default]";
static char msg[BUF_SIZE];

/* ---- MariaDB 접속 정보 ---- */
#define DB_HOST "127.0.0.1"
#define DB_USER "aiot_user"
#define DB_PASS "aiot_pwd"
#define DB_NAME "INTEL_AI"
#define DB_PORT 3306

/* ---- MariaDB 핸들 ---- */
static MYSQL      *g_conn = NULL;
static MYSQL_STMT *g_stmt_insert = NULL;

/* 원형 */
void * send_msg(void * arg);
void * recv_msg(void * arg);
void   error_handling(const char * msg);

/* DB 관련 */
static void db_connect(void);
static void db_close(void);
static int  db_prepare_insert(void);
static int  db_insert_event_num(const char *locate, unsigned char level); /* level: 1/2/3 */
static void handle_from_rpi(const char *payload);
static void handle_getdb(const char *reply_to);

/* utils */
static void ltrim_inplace(char *s){ while(*s && isspace((unsigned char)*s)) memmove(s, s+1, strlen(s)); }
static void rtrim_inplace(char *s){ size_t n=strlen(s); while(n>0 && isspace((unsigned char)s[n-1])) s[--n]='\0'; }
static void trim_inplace(char *s){ rtrim_inplace(s); ltrim_inplace(s); }

static int is_valid_loc(char c){ return (c && strchr("QWEASDZXC", (char)toupper((unsigned char)c))); }
static int is_valid_level_digit(char c){ return (c>='1' && c<='3'); }

/* 안전 송신(부분 write 대비) */
static ssize_t write_all(int fd, const void *buf, size_t len){
    const char *p = (const char*)buf;
    size_t left = len;
    while(left){
        ssize_t n = write(fd, p, left);
        if(n < 0){
            if(errno == EINTR) continue;
            return n;
        }
        if(n == 0) break;
        p += n; left -= n;
    }
    return (ssize_t)(len - left);
}

/* ========================== main ========================== */
int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;

    if(argc != 4) {
        printf("Usage : %s <IP> <port> <name>\n",argv[0]);
        exit(1);
    }
    snprintf(name_, sizeof(name_), "%s", argv[3]);

    db_connect();
    if(!db_prepare_insert()){
        fprintf(stderr, "prepare insert failed: %s\n", mysql_error(g_conn));
        db_close();
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1) error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port        = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    /* 인증: 원본과 동일 포맷 */
    char auth[NAME_SIZE + 16];
    snprintf(auth, sizeof(auth), "[%s:PASSWD]", name_);
    write_all(sock, auth, strlen(auth));

    pthread_create(&rcv_thread, NULL, recv_msg, (void *)&sock);
    pthread_create(&snd_thread, NULL, send_msg, (void *)&sock);

    void *tr;
    pthread_join(snd_thread, &tr);
    //pthread_join(rcv_thread, &tr);

    close(sock);
    db_close();
    return 0;
}

/* ======================= 송신(원본 그대로) ======================= */
void * send_msg(void * arg)
{
    int *psock = (int *)arg;
    int ret;
    fd_set initset, newset;
    struct timeval tv;
    char out[NAME_SIZE + BUF_SIZE + 2];

    FD_ZERO(&initset);
    FD_SET(STDIN_FILENO, &initset);

    fputs("Input a message! [ID]msg (Default ID:ALLMSG)\n",stdout);
    while(1) {
        memset(msg, 0, sizeof(msg));
        out[0] = '\0';
        tv.tv_sec = 1; tv.tv_usec = 0;
        newset = initset;
        ret = select(STDIN_FILENO + 1, &newset, NULL, NULL, &tv);
        if(FD_ISSET(STDIN_FILENO, &newset))
        {
            if(!fgets(msg, BUF_SIZE, stdin)) { *psock = -1; return NULL; }
            if(!strncmp(msg,"quit\n",5))      { *psock = -1; return NULL; }

            if(msg[0] != '[') {           // 대괄호 없으면 [ALLMSG] 붙임
                strcpy(out, "[ALLMSG]");
                strncat(out, msg, sizeof(out)-strlen(out)-1);
            } else {
                strncpy(out, msg, sizeof(out)-1);
                out[sizeof(out)-1] = '\0';
            }
            if(write_all(*psock, out, strlen(out)) <= 0) { *psock = -1; return NULL; }
        }
        if(ret == 0) { if(*psock == -1) return NULL; }
    }
}

/* ======================= 수신(출력 그대로 + 프레이밍) ======================= */
void * recv_msg(void * arg)
{
    int * psock = (int *)arg;

    /* 누적 버퍼(프레이밍) */
    static char accum[4096];
    static size_t acc_len = 0;
    char rx[256];

    while(1) {
        int n = read(*psock, rx, sizeof(rx)-1);
        if(n <= 0) { *psock = -1; return NULL; }
        rx[n] = '\0';

        /* 화면엔 그대로 출력 */
        fputs(rx, stdout);

        /* 누적 */
        if(acc_len + n >= sizeof(accum)) { acc_len = 0; }
        memcpy(accum + acc_len, rx, n);
        acc_len += n;
        accum[acc_len] = '\0';

        /* [TAG]payload 단위로 분리 */
        size_t i = 0;
        while(i < acc_len) {
            char *lb = memchr(accum + i, '[', acc_len - i);
            if(!lb) break;
            char *rb = memchr(lb + 1, ']', accum + acc_len - (lb + 1));
            if(!rb) break;

            char *next_lb = memchr(rb + 1, '[', accum + acc_len - (rb + 1));
            size_t payload_len = (size_t)((next_lb ? next_lb : (accum + acc_len)) - (rb + 1));

            *rb = '\0';
            const char *from = lb + 1;
            char payload[256];
            size_t copy_len = (payload_len < sizeof(payload)-1) ? payload_len : (sizeof(payload)-1);
            memcpy(payload, rb + 1, copy_len);
            payload[copy_len] = '\0';
            ltrim_inplace(payload);
            rtrim_inplace(payload);

            if(!strcmp(from, "RASPBERRYPI")) {
                handle_from_rpi(payload);
            } else if(!strncasecmp(payload, "GETDB", 5)) {
                handle_getdb(from);
            }

            i = (size_t)((next_lb ? next_lb : (accum + acc_len)) - accum);
        }

        if(i < acc_len) {
            memmove(accum, accum + i, acc_len - i);
            acc_len -= i;
        } else {
            acc_len = 0;
        }
    }
}

/* ======================= DB 연결/정리 ======================= */
static void db_connect(void)
{
    g_conn = mysql_init(NULL);
    if(!g_conn){ fprintf(stderr, "mysql_init failed\n"); exit(1); }

    if(!mysql_real_connect(g_conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0)){
        fprintf(stderr, "mysql connect error: %s\n", mysql_error(g_conn));
        exit(1);
    }
    mysql_autocommit(g_conn, 1);
}

static void db_close(void)
{
    if(g_stmt_insert){ mysql_stmt_close(g_stmt_insert); g_stmt_insert=NULL; }
    if(g_conn){ mysql_close(g_conn); g_conn=NULL; }
}

static int db_prepare_insert(void)
{
    /* raw는 generated 컬럼이면 DB가 자동 생성 */
    const char *sql = "INSERT INTO events(locate, distance_level) VALUES(?,?)";
    g_stmt_insert = mysql_stmt_init(g_conn);
    if(!g_stmt_insert) return 0;
    if(mysql_stmt_prepare(g_stmt_insert, sql, (unsigned long)strlen(sql)) != 0) {
        fprintf(stderr, "stmt_prepare error: %s\n", mysql_stmt_error(g_stmt_insert));
        return 0;
    }
    return 1;
}

static int db_insert_event_num(const char *locate, unsigned char level)
{
    if(!g_stmt_insert) return 0;

    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));

    unsigned long len_loc = (unsigned long)strlen(locate);
    unsigned char lv      = level; /* 1/2/3 */

    bind[0].buffer_type   = MYSQL_TYPE_STRING;
    bind[0].buffer        = (char*)locate;
    bind[0].buffer_length = len_loc;
    bind[0].length        = &len_loc;

    /* 숫자 타입으로 바인딩 */
    bind[1].buffer_type   = MYSQL_TYPE_TINY;
    bind[1].buffer        = &lv;
    bind[1].is_unsigned   = 1;
    bind[1].buffer_length = sizeof(lv);

    if(mysql_stmt_bind_param(g_stmt_insert, bind) != 0) {
        fprintf(stderr, "stmt_bind_param error: %s\n", mysql_stmt_error(g_stmt_insert));
        return 0;
    }
    if(mysql_stmt_execute(g_stmt_insert) != 0) {
        fprintf(stderr, "stmt_execute error: %s\n", mysql_stmt_error(g_stmt_insert));
        return 0;
    }
    fprintf(stdout, "DB INSERT OK: locate=%s level=%u\n", locate, (unsigned)level);
    return 1;
}

/* ======================= 핸들러들 ======================= */
static void handle_from_rpi(const char *payload)
{
    /* 허용 형식: "Q@1"
       과거 "Q@L2"도 들어오면 '2'로 변환해서 숫자만 저장 */
    if(!payload || !*payload) return;

    char tmp[BUF_SIZE];
    strncpy(tmp, payload, sizeof(tmp)-1);
    tmp[sizeof(tmp)-1] = '\0';
    trim_inplace(tmp);

    char *at = strchr(tmp, '@');
    if(!at) return;
    *at = '\0';
    char *loc = tmp;
    char *lvl = at + 1;

    trim_inplace(loc); trim_inplace(lvl);
    if(strlen(loc) != 1) return;

    char L = (char)toupper((unsigned char)loc[0]);
    if(!is_valid_loc(L)) return;

    unsigned char num = 0;
    if(strlen(lvl) == 1 && is_valid_level_digit(lvl[0])) {
        num = (unsigned char)(lvl[0]-'0');
    } else if((lvl[0]=='L' || lvl[0]=='l') && is_valid_level_digit(lvl[1]) && lvl[2]=='\0') {
        num = (unsigned char)(lvl[1]-'0');   /* L1/L2/L3 → 1/2/3 */
    } else {
        return;
    }

    char loc_str[2] = { L, '\0' };
    (void)db_insert_event_num(loc_str, num);
}

static void handle_getdb(const char *reply_to)
{
    if(!reply_to || !*reply_to) return;

    /* 1) 총 행수 */
    unsigned long total_rows = 0;
    if(mysql_query(g_conn, "SELECT COUNT(*) FROM events") == 0) {
        MYSQL_RES *rc = mysql_store_result(g_conn);
        if(rc){
            MYSQL_ROW r = mysql_fetch_row(rc);
            if(r && r[0]) total_rows = strtoul(r[0], NULL, 10);
            mysql_free_result(rc);
        }
    }
    {
        char out[NAME_SIZE + 64];
        int n = snprintf(out, sizeof(out), "[%s]GETDB_BEGIN rows=%lu\n", reply_to, total_rows);
        if(n > 0) write_all(sock, out, (size_t)n);
    }

    /* 2) 한 행 = 한 프레임으로 스트리밍 전송 */
    const char *sql =
        "SELECT DATE_FORMAT(`time`, '%Y-%m-%d %H:%i:%s') AS ts, "
        "       locate, "
        "       CAST(distance_level AS CHAR) AS lvl "
        "FROM events ORDER BY `time` ASC";

    if(mysql_query(g_conn, sql) != 0){
        char out[128];
        int n1 = snprintf(out, sizeof(out), "[%s]ERROR\n", reply_to);
        if(n1 > 0) write_all(sock, out, (size_t)n1);
        int n2 = snprintf(out, sizeof(out), "[%s]GETDB_END\n", reply_to);
        if(n2 > 0) write_all(sock, out, (size_t)n2);
        return;
    }

    MYSQL_RES *res = mysql_store_result(g_conn);
    if(!res){
        char out[128];
        int n1 = snprintf(out, sizeof(out), "[%s]ERROR\n", reply_to);
        if(n1 > 0) write_all(sock, out, (size_t)n1);
        int n2 = snprintf(out, sizeof(out), "[%s]GETDB_END\n", reply_to);
        if(n2 > 0) write_all(sock, out, (size_t)n2);
        return;
    }

    MYSQL_ROW row;
    while((row = mysql_fetch_row(res))){
        const char *ts = row[0] ? row[0] : "";
        const char *lc = row[1] ? row[1] : "";
        const char *lv = row[2] ? row[2] : "";

        char out[NAME_SIZE + 64];
        int n = snprintf(out, sizeof(out), "[%s]%s,%s,%s\n", reply_to, ts, lc, lv);
        if(n > 0) write_all(sock, out, (size_t)n);
    }
    mysql_free_result(res);

    /* 3) 끝 알림 */
    {
        char out[NAME_SIZE + 32];
        int n = snprintf(out, sizeof(out), "[%s]GETDB_END\n", reply_to);
        if(n > 0) write_all(sock, out, (size_t)n);
    }
}

/* ======================= 공용 ======================= */
void error_handling(const char * msg)
{
    fputs(msg, stderr); fputc('\n', stderr); exit(1);
}
