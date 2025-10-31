# STEVE ALEXANDER PINEDA RINCON - 160004726

##  Requisitos del sistema 

- **Sistema Operativo**: Linux (Ubuntu, Debian,etc.)
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
server
worker
```

Verificar permisos:

```bash
ls -lh server worker

# Deberías ver algo como:
# -rwxr-xr-x  1 user user  servr
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
./server
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
Nonce: 6
Combinacion: 06
Texto completo: Mineria distribuida TCP prueba06
Hash: 59763bb0604ec3d0
Tiempo de ejecucion: 0 segundos

[SERVIDOR] Orden de detencion recibida (otro worker encontro la solucion)
```
 o tambien:

```
*** SOLUCION ENCONTRADA! ***
Nonce: 1943
Combinacion: VL
Texto completo: Mineria distribuida TCP pruebaVL
Hash: 801a8db1073b7620
Tiempo de ejecucion: 0 segundos

Worker finalizado.
```


En el servidor:

```
*** Worker 1 encontro la solucion! ***
Hash: 59763bb0604ec3d0
Nonce: 6
Worker 2 conectado desde 127.0.0.1:44196

Todos los workers conectados. Iniciando busqueda...


=== RESULTADO FINAL ===
SOLUCION ENCONTRADA!
Hash: 801a8db1073b7620
Nonce: 1943
```

### Monitorear Uso de CPU

```bash
# En otra terminal
watch -n 1 'ps aux | grep -E "servidor|worker"'

# O usar top
top -p $(pgrep -d',' -f 'servidor|worker')
```
