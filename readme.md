# Web Server

Un servidor HTTPS/1.1 estático construido desde cero en **C** utilizando la API nativa de **POSIX Sockets** y la librería **OpenSSL** bajo Linux. Este proyecto aborda el flujo de red cifrado a bajo nivel, gestión de memoria manual, Handshake TLS/SSL, parseo de peticiones HTTP/HTTPS y manejo del sistema de archivos.

---

## Tecnologías y Conceptos

* **Lenguaje:** C (Estándar C11)
* **Redes & OS:** POSIX Sockets (`<sys/socket.h>`), TCP/IP, protocolo HTTP/1.1
* **Compilación:** GCC con flags estrictas (`-Wall -Wextra -std=c11`) y automatización con `GNU Make`
* **Entorno:** Linux (Probado en Linux Mint / Ubuntu)

---

###### comando para probar el servidor localmente:
```bash
make run
```
#### Nota: make run compila el ejecutable y genera automáticamente la pareja de claves SSL (certs/cert.pem y certs/key.pem)


###### comando para construir la imagen en docker:
```bash
docker build -t web-server .
```

###### comando para ejecutar el contenedor:
```bash
docker run -p 8080:8080 --name mi_servidor web-server
```
