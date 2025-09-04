CC      := gcc
CFLAGS  := -O2 -Wall -Wextra -pthread

# --- MariaDB (pkg-config 우선, 없으면 폴백) ---
PKG_DB_CFLAGS := $(shell pkg-config --cflags mariadb 2>/dev/null)
PKG_DB_LIBS   := $(shell pkg-config --libs   mariadb 2>/dev/null)

# 헤더 경로: /usr/include/mariadb 또는 /usr/include/mysql 둘 다 커버
DB_CFLAGS := $(PKG_DB_CFLAGS)
ifeq ($(strip $(DB_CFLAGS)),)
  DB_CFLAGS := -I/usr/include/mariadb -I/usr/include/mysql
endif

# 라이브러리: mariadb 우선
DB_LIBS := $(PKG_DB_LIBS)
ifeq ($(strip $(DB_LIBS)),)
  DB_LIBS := -lmariadb
endif

# Targets
all: iot_server db_client iot_client

iot_server: iot_server.c
	$(CC) $(CFLAGS) -o $@ $^

db_client: db_client.c
	$(CC) $(CFLAGS) $(DB_CFLAGS) -o $@ $^ $(DB_LIBS)

iot_client: iot_client.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f iot_server db_client iot_client

.PHONY: all clean
