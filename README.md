# STEVE ALEXANDER PINEDA RINCON - 160004716

Sistema distribuido que simula el proceso de minería en una blockchain utilizando arquitectura cliente-servidor.

## Descripcion General

El sistema consta de:
- **Coordinador (Servidor)**: Coordina el trabajo y divide el espacio de búsqueda
- **Workers (Clientes)**: Realizan la búsqueda de soluciones en paralelo


## Archivos del Proyecto

### Código Fuente Compartido
```
Archivo.txt           # Texto base
hash.h                # Header funciones hash
hash.c                # Implementación hash
server.c              # Servidor (POSIX)
worker.c              # Worker (POSIX)
compilar.sh           # Script compilación Bash
```

### Documentación
```
README.md             
DOCUMENTACION_TECNICA.md
GUIA_DE_USO.md
```

## Caracteristicas Principales

- Algoritmo de hash FNV-1a 64-bit (no SHA-256/MD5)
- Comunicacion por sockets TCP/IP
- Concurrencia multi-threading
- Division automatica del trabajo
- Detencion coordinada al encontrar solucion
- Limite de tiempo por worker (600s)


