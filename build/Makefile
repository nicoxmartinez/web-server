CC = gcc
CFLAGS = -Wall -Wextra -std=c11

SRC_DIR = ../src
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
TARGET = $(BIN_DIR)/main

all: $(TARGET)

# Compilacion del ejecutable
$(TARGET): $(SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Ejecucion en la raiz del proyecto para que las rutas de static/ funcionen
run: all
	cd .. && ./build/bin/main

# Limpiar archivos generados dentro de build/bin
clean:
	rm -rf $(BIN_DIR)/*

.PHONY: all run clean