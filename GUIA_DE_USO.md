# STEVE ALEXANDER PINED RINCON - 160004726

##  Requisitos del sistema 

- **Sistema Operativo**: Linux (Ubuntu, Debian, Fedora, Arch, etc.)
- **Compilador**: GCC
- **Terminal**: Bash

##  Instalación de GCC

### Ubuntu / Debian

```bash
# Actualizar repositorios
sudo apt update

# Instalar herramientas de desarrollo
sudo apt install build-essential

# Verificar instalación
gcc --version
make --version
```

## Archivos del Proyecto

```
BLOCK_CHAIN/
├── Archivo.txt      
├── hash.h    
├── hash.c    
├── server.c 
├── worker.c        
├── compilar.sh           
├── GUIA_DE_USO.md          
├── DOCUMENTACION_TECNICA.md
```

##  Compilación

### Opción 1: Usando el Script Shell (Recomendado)

```bash
# Dar permisos de ejecución (solo primera vez)
chmod +x compilar.sh

# Compilar
./compilar.sh
```

### Verificar Compilación

Deberías tener estos archivos:

```
servidor
worker
```

Verificar permisos:

```bash
ls -lh servidor worker

# Deberías ver algo como:
# -rwxr-xr-x  1 user user  servidor
# -rwxr-xr-x  1 user user  worker
```


##  Ejecución

### Configuración del Sistema

El simulador necesita:
- **1 servidor** (coordinador)
- **N workers** (clientes) 

### Paso 1: Abrir Terminal para el Servidor

```bash
# Navegar al proyecto
cd ~/BLOCK_CHAIN

# Ejecutar servidor
./servidor
```

### Paso 2: Configurar Parámetros

El servidor te preguntará:

```
Ingrese el texto de relleno (caracteres alfanumericos): AA
Ingrese la cantidad de ceros consecutivos al final del hash: 1
Ingrese el numero de workers (1-10): 2
```

### Paso 3: Abrir Terminales para cada Worker

**Importante**: Necesitas abrir **tantas terminales como workers especificaste**.

#### Cómo abrir múltiples terminales:

**Opción A**: Atajos de teclado
- `Ctrl + Alt + T` - Nueva terminal (Ubuntu/Debian)
- `Ctrl + Shift + T` - Nueva pestaña en terminal actual

**Opción B**: Desde el menú
- Aplicaciones → Terminal
- O buscar "Terminal" en el lanzador

#### En cada terminal de worker:

```bash
cd ~/BLOCK_CHAIN
./worker
```
##  Ejemplo Completo - Paso a Paso

### Escenario: 2 workers

#### Terminal 1 - Servidor
```bash
user@linux:~/BLOCK_CHAIN$ ./servidor

=== SIMULADOR DE BLOCKCHAIN - SERVIDOR ===

Texto original del archivo: Mineria distribuida TCP prueba

Ingrese el texto de relleno: AA
Ingrese la cantidad de ceros: 1
Ingrese el numero de workers: 2

=== CONFIGURACION ===
Texto de relleno: AA (longitud: 2)
Condicion: Hash terminado en 1 ceros
Numero de workers: 2

Total de combinaciones: 3844
Combinaciones por worker: 1922

Worker 1: rango [0 - 1921]
Worker 2: rango [1922 - 3843]

Socket creado.
Bind realizado.
Esperando conexiones en el puerto 8888...
```

#### Terminal 2 - Worker 1
```bash
user@linux:~/BLOCK_CHAIN$ ./worker

=== WORKER - CLIENTE DE MINERIA ===
Conectando al servidor 127.0.0.1:8888...
Conectado al servidor!

=== CONFIGURACION RECIBIDA ===
Texto base: Mineria distribuida TCP prueba
Relleno: AA
Ceros requeridos: 1
Rango asignado: [0 - 1921]
Total de combinaciones a probar: 1922

Iniciando busqueda...

Progreso: 52.08% - Intentos: 10000 - Ultimo hash: 3a5f2e12
```

#### Terminal 3 - Worker 2
```bash
user@linux:~/BLOCK_CHAIN$ ./worker

=== WORKER - CLIENTE DE MINERIA ===
Conectando al servidor 127.0.0.1:8888...
Conectado al servidor!

=== CONFIGURACION RECIBIDA ===
Texto base: Mineria distribuida TCP prueba
Relleno: AA
Ceros requeridos: 1
Rango asignado: [1922 - 3843]
Total de combinaciones a probar: 1922

Iniciando busqueda...
```

### Resultado Esperado

Cuando un worker encuentre la solución:

```
*** SOLUCION ENCONTRADA! ***
Nonce: 2456
Combinacion: Az
Texto completo: Mineria distribuida TCP pruebaAz
Hash: 3a5f2e0

[SERVIDOR] Orden de detencion recibida (otro worker encontro la solucion)
```

En el servidor:

```
*** Worker 2 encontro la solucion! ***
Hash: 3a5f2e0
Nonce: 2456

=== RESULTADO FINAL ===
SOLUCION ENCONTRADA!
Hash: 3a5f2e0
Nonce: 2456
```

### Monitorear Uso de CPU

```bash
# En otra terminal
watch -n 1 'ps aux | grep -E "servidor|worker"'

# O usar top
top -p $(pgrep -d',' -f 'servidor|worker')
```
