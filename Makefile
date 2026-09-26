# Compilador y Opciones
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -I./src/lib

# Nombres y Nombre de Imagen Docker
IMAGE_NAME = web-server
CONTAINER_NAME = mi_servidor

# Directorios
SRC_DIR   = src
BUILD_DIR = build
BIN_DIR   = $(BUILD_DIR)/bin
OBJ_DIR   = $(BUILD_DIR)/obj

# Archivos Fuente y Objetos
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/lib/sendFile.c
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/main

# Regla por defecto: Compilar todo
all: $(TARGET)

# Enlace (Linking): Crea el ejecutable final a partir de los .o
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJS) -o $(TARGET)

# Compilación (Compiling): Compila cada archivo .c a su correspondiente .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Ejecutar el servidor nativo desde la raíz
run: all
	./$(TARGET)
