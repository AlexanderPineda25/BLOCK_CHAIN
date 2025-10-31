# Simulador de Blockchain

echo "========================================"
echo "Compilando Simulador de Blockchain"
echo "========================================"
echo ""

# Verificar si existe gcc
if ! command -v gcc &> /dev/null; then
    echo "ERROR: gcc no está instalado"
    echo "Instala gcc con uno de estos comandos:"
    echo "  Ubuntu/Debian: sudo apt install build-essential"
    exit 1
fi

echo "GCC encontrado: $(gcc --version | head -n1)"
echo ""

# Compilar metodo de hash
echo "Compilando el metodo hash..."
gcc -c hash.c -o hash.o
if [ $? -ne 0 ]; then
    echo "ERROR al compilar hash.c"
    exit 1
fi
echo "[OK] hash.o"

# Compilar server
echo "Compilando server..."
gcc -c server.c -o server.o -pthread
if [ $? -ne 0 ]; then
    echo "ERROR al compilar server.c"
    exit 1
fi
echo "[OK] server.o"

gcc server.o hash.o -o server -lpthread
if [ $? -ne 0 ]; then
    echo "ERROR al enlazar server"
    exit 1
fi
chmod +x server
echo "[OK] server"

# Compilar worker
echo "Compilando worker..."
gcc -c worker.c -o worker.o -pthread
if [ $? -ne 0 ]; then
    echo "ERROR al compilar worker.c"
    exit 1
fi
echo "[OK] worker.o"

gcc worker.o hash.o -o worker -lpthread
if [ $? -ne 0 ]; then
    echo "ERROR al enlazar worker"
    exit 1
fi
chmod +x worker
echo "[OK] worker"

echo ""
echo "========================================"
echo "Compilacion exitosa!"
echo "========================================"
echo ""
echo "Ejecutables generados:"
echo "  ./server  - Coordinador"
echo "  ./worker    - Cliente"
echo ""
echo "Para ejecutar:"
echo "  Terminal 1: ./server"
echo "  Terminales adicionales: ./worker"
echo ""
echo "Ver GUIA_DE_USO.md para más información"
echo ""
