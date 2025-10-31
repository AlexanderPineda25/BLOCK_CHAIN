# Documentación Técnica - Simulador de Blockchain

## Arquitectura del Sistema

### Componentes Principales

```
┌─────────────────────────────────────────────────────────┐
│                       SERVIDOR                          │
│  - Coordina workers                                     │
│  - Divide espacio de búsqueda                           │
│  - Recibe resultados                                    │
│  - Notifica detención                                   │
└───────────┬─────────────────────────────┬───────────────┘
            │                             │
            │      Socket TCP/IP          │
            │      Puerto 8888            │
            │                             │
    ┌───────▼─────────┐         ┌────────▼────────┐
    │    WORKER 1     │   ...   │    WORKER N     │
    │  Rango: [0-X]   │         │  Rango: [Y-Z]   │
    │  Busca hash     │         │  Busca hash     │
    └─────────────────┘         └─────────────────┘
```

## Algoritmo de Hash (FNV-1a 64 bits)

### Diseño

El hash NO utiliza SHA-256 ni MD5. Implementa un algoritmo propio basado en:

```c
hash = 1469598103934665603       
para cada byte b en el texto:
    hash = hash XOR b
    hash = hash * 1099511628211  
resultado_hex = convertir_a_hexadecimal(hash)

```

### Constantes Utilizadas
```
   OFFSET_BASIS = 1469598103934665603
   FNV_PRIME = 1099511628211
   Tamaño del hash: 64 bits → 16 caracteres hexadecimales
   Formato de salida: hexadecimal, sin prefijo 0x
```
### Características

```
   Determinista: mismo texto → mismo hash
   Rápido: operaciones básicas sobre enteros
   Alta dispersión: pequeños cambios → hashes muy distintos
   Salida compacta: 16 dígitos hexadecimales
   Sin dependencias externas: implementado en C puro
```

### Ejemplo

```
Entrada: "Universidad de los Llanos, sede Barcelona00"
Proceso FNV-1a → Hash: 9af31b47f2b000ff

```

## Generación de Combinaciones

### Sistema Alfanumérico Base-62

Alfabeto: `0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz`

Índice → Combinación (similar a conversión decimal a base-62):

```
Índice 0: "000...0"
Índice 1: "000...1"
...
Índice 61: "000...z"
Índice 62: "000...10"
...
```

### Ejemplo con longitud 3

```
Índice      Combinación
0       →   "000"
1       →   "001"
62      →   "010"
3844    →   "100"
238327  →   "zzz"
```

## Protocolo de Comunicación

### Mensajes Servidor → Worker

1. **CONFIG** (al conectar):
   ```
   CONFIG|texto_base|relleno|num_ceros|rango_inicio|rango_fin
   ```
   Ejemplo:
   ```
   CONFIG|Mineria distribuida TCP prueba|AAA|2|0|61999
   ```

2. **DETENER** (cuando hay solución):
   ```
   DETENER
   ```

### Mensajes Worker → Servidor

1. **SOLUCION** (hash válido encontrado):
   ```
   SOLUCION|hash_hex|nonce
   ```
   Ejemplo:
   ```
   SOLUCION|9af31b47f2b000ff|54291
   ```

2. **NO_ENCONTRADO** (rango completado sin éxito):
   ```
   NO_ENCONTRADO
   ```

3. **PROGRESO** (opcional, cada 10,000 intentos):
   ```
   PROGRESO|porcentaje|intentos
   ```

## Flujo de Ejecución

### Secuencia Completa

```
1. Servidor inicia y lee Archivo.txt
2. Solicita parámetros al usuario
3. Divide el espacio de búsqueda en N rangos
4. Espera conexiones de N workers
5. Envía CONFIG a cada worker
6. Cada worker busca en su rango:
     - Genera combinación (nonce)
     - Calcula hash FNV-1a(texto_base + combinación)
     - Si hash termina con N ceros → envía SOLUCION
7. Servidor recibe primera SOLUCION
8. Envía DETENER a todos los workers
9. Muestra resultado final

```

## Concurrencia y Sincronización

### Threads en Servidor

- **Thread principal**: Acepta conexiones, coordina workers
- **Threads workers**: Un thread por cada worker conectado, escucha mensajes

### Sincronización

```c
CRITICAL_SECTION seccion_critica;

// Proteger variable compartida
EnterCriticalSection(&seccion_critica);
if (!solucion_encontrada) {
    solucion_encontrada = 1;
    // ... procesar solución
}
LeaveCriticalSection(&seccion_critica);
```

### Variables Compartidas Protegidas

- `solucion_encontrada`: Flag global
- `hash_solucion`: Hash ganador
- `nonce_solucion`: Nonce ganador

## Espacio de Búsqueda

### Cálculo de Combinaciones

Total = 62^longitud

| Longitud | Combinaciones | Tamaño Aproximado      |
|----------|---------------|------------------------|
| 1        | 62            | ~62 intentos           |
| 2        | 3,844         | ~4 mil intentos        |
| 3        | 238,328       | ~238 mil intentos      |
| 4        | 14,776,336    | ~15 millones           |
| 5        | 916,132,832   | ~916 millones          |

### División de Trabajo

Para N workers:
```
tamaño_rango = total_combinaciones / N
worker[i].inicio = i * tamaño_rango
worker[i].fin = (i+1) * tamaño_rango - 1

// Último worker toma el resto
worker[N-1].fin = total_combinaciones - 1
```

## Optimizaciones Implementadas

1. **Distribución equitativa**: Rangos balanceados entre workers
2. **Detección temprana**: Stop inmediato al encontrar solución
3. **Tiempo límite**: Previene búsquedas infinitas (600s)
4. **Reporte de progreso**: Cada 10,000 intentos
5. **Hash eficiente**: Operaciones modulares en vez de criptográficas

## Limitaciones Conocidas

1. **No persistence**: Si se cae el servidor, se pierde todo
2. **Sin recuperación**: Workers no pueden reconectarse
3. **Memoria**: Límite de 10 workers simultáneos (configurable)
4. **Puerto fijo**: 8888 (hardcoded)
5. **Sin autenticación**: Cualquiera puede conectarse

## Posibles Mejoras Futuras

1. Guardar estado de búsqueda (checkpoints)
2. Balanceo dinámico de carga
3. Reconexión automática de workers
4. Configuración de puerto por parámetro
5. Sistema de autenticación
6. Métricas y estadísticas detalladas
7. Interfaz gráfica
8. Soporte para múltiples algoritmos de hash

## Consideraciones de Rendimiento

### Factores que Afectan el Rendimiento

1. **Número de workers**: Más workers = búsqueda más rápida (hasta cierto límite)
2. **Dificultad (ceros)**: Más ceros = exponencialmente más difícil
3. **Longitud del relleno**: Mayor longitud = más combinaciones
4. **CPU**: Operaciones modulares intensivas en CPU
5. **Red**: Latencia mínima (localhost), puede afectar en red



