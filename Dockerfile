FROM gcc:13 AS builder

WORKDIR /app

COPY . .

RUN make

FROM debian:bookworm-slim

WORKDIR /app

COPY --from=builder /app/build/bin/main ./main
COPY --from=builder /app/src/static ./src/static

# Exponemos el puerto 8080
EXPOSE 8080

CMD ["./main"]