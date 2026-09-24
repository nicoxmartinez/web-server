FROM gcc:13 AS builder

WORKDIR /app

COPY . .

# Nos movemos a la carpeta 'build' donde vive el Makefile para ejecutar 'make'
WORKDIR /app/build
RUN make

FROM debian:bookworm-slim

WORKDIR /app

# Copiamos el binario generado y la carpeta de estáticos desde la raíz del proyecto
COPY --from=builder /app/build/bin/main ./main
COPY --from=builder /app/src/static ./src/static

EXPOSE 8080

CMD ["./main"]