# Compilador y banderas
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -I./src/lib
LDFLAGS = -lssl -lcrypto

# Directorios
SRC_DIR   = src
LIB_DIR   = $(SRC_DIR)/lib
BUILD_DIR = build
BIN_DIR   = $(BUILD_DIR)/bin
OBJ_DIR   = $(BUILD_DIR)/obj
CERTS_DIR = certs

# Certificados SSL/TLS
CERT_FILE = $(CERTS_DIR)/cert.pem
KEY_FILE  = $(CERTS_DIR)/key.pem

# Archivos fuente (.c) y objetos (.o)
SRCS = $(SRC_DIR)/main.c \
       $(LIB_DIR)/sendFile.c \
       $(LIB_DIR)/server.c \
       $(LIB_DIR)/sslConfig.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Nombre del ejecutable
TARGET = $(BIN_DIR)/main

# Regla por defecto (genera certificados si no existen y compila)
all: $(CERT_FILE) $(TARGET)

# Regla para generar certificados SSL/TLS autofirmados
$(CERT_FILE):
	@mkdir -p $(CERTS_DIR)
	openssl req -x509 -newkey rsa:2048 -nodes \
		-keyout $(KEY_FILE) \
		-out $(CERT_FILE) \
		-days 365 \
		-subj "/CN=localhost" \
		-addext "subjectAltName=DNS:localhost,IP:127.0.0.1"

certs: $(CERT_FILE)

# Regla para enlazar los archivos objeto y crear el ejecutable
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Regla para compilar los archivos .c a .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para compilar y ejecutar directamente
run: all
	./$(TARGET)

# Limpieza de archivos generados
clean:
	rm -rf $(BUILD_DIR)

# Limpieza total (incluyendo certificados)
clean-all: clean
	rm -rf $(CERTS_DIR)

.PHONY: all certs run clean clean-all