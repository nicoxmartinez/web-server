FROM gcc:13 AS builder

WORKDIR /app

COPY . .

RUN make

FROM debian:bookworm-slim

WORKDIR /app

# Instalar dependencias de tiempo de ejecución de OpenSSL y certificados CA
RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# copiar la carpeta con el binario del programa
COPY --from=builder /app/build/bin/main ./main
# Copiar la carpeta con los archivos estaticos
COPY --from=builder /app/src/static ./src/static
# Copiar la carpeta de certificados
COPY --from=builder /app/certs ./certs

# Exponer el puerto del servidor
EXPOSE 8080

# Comando para ejecutar la aplicación
CMD ["./main"]
