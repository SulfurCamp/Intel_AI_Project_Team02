CC=gcc
CFLAGS=-pthread

TARGET_SERVER=iot_server
TARGET_CLIENT=iot_client

all: $(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_SERVER): iot_server.c
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) iot_server.c

$(TARGET_CLIENT): iot_client.c
	$(CC) $(CFLAGS) -o $(TARGET_CLIENT) iot_client.c

clean:
	rm -f $(TARGET_SERVER) $(TARGET_CLIENT) *.o