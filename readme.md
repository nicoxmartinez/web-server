# Web Server

Un servidor HTTP/1.1 estático construido desde cero en **C** utilizando la API nativa de **POSIX Sockets** bajo Linux. Este proyecto aborda el flujo de red a bajo nivel, gestión de memoria manual, parseo de peticiones HTTP y manejo del sistema de archivos.

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

###### comando para construir la imagen en docker:
```bash
docker build -t web-server .
```

###### comando para ejecutar el contenedor:
```bash
docker run -p 8080:8080 --name mi_servidor web-server
```