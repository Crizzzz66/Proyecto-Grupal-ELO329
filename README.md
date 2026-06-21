# INF295 — Optimización de Malla Curricular

Proyecto de optimización combinatoria que genera una **malla curricular** equilibrada distribuida en semestres, respetando pre-requisitos y restricciones de créditos y cantidad de asignaturas por semestre. Implementa dos metaheurísticas: Hill Climbing (HC) y Algoritmo Evolutivo (AE).

---

## Requisitos

| Componente    | Detalle |
|---------------|---------|
| Sistema operativo | Linux — Ubuntu (ejecutado mediante terminal integrada en VS Code con WSL) |
| Compilador    | g++ 7.0+ con soporte C++17 |
| Build tool    | make |

**Instalación de compilador y make (una sola vez):**

```bash
sudo apt install build-essential
```

---

## Compilación

```bash
make          # compila y genera el ejecutable "programa"
make clean    # elimina el ejecutable generado
```

---

## Ejecución

```bash
./programa
```

El programa lee la lista de instancias configurada directamente en `main.cpp` y escribe los resultados en `output.txt` (se crea o se agrega en la carpeta de trabajo).

---

## Configuración interna (main.cpp)

Al inicio de `main()` se encuentran tres parámetros de control:

```cpp
bool MODELO_HC  = true;   // Ejecutar Hill Climbing
bool MODELO_AE  = true;   // Ejecutar Algoritmo Evolutivo
bool SHOW_MALLA = true;   // Incluir la malla completa en output.txt
```

Y la lista de instancias a procesar (descomentar las que se deseen):

```cpp
vector<string> archivos = {
    "bacp8.txt",
    //"bacp10.txt",
    //"bacp12.txt",
    //"utfsm3.txt",
    ...
};
```

---

## Instancias incluidas

| Archivo        | Semestres | Cursos | Créditos/sem | Cursos/sem |
|----------------|-----------|--------|--------------|------------|
| bacp8.txt      | 8         | 46     | 10 – 24      | 2 – 10     |
| bacp10.txt     | 10        | 42     | 10 – 24      | 2 – 10     |
| bacp12.txt     | 12        | 68     | 10 – 24      | 2 – 10     |
| utfsm3.txt     | 4         | 23     | 20 – 35      | 1 – 8      |
| utfsm2920.txt  | 8         | 46     | 20 – 35      | 1 – 8      |
| utfsm7310.txt  | 10        | 54     | 20 – 35      | 1 – 8      |
| utfsm7313.txt  | 11        | 55     | 20 – 35      | 1 – 8      |
| utfsm2311.txt  | 12        | 60     | 20 – 35      | 1 – 8      |

---

## Formato del archivo de entrada

```
p = 8;            # Número de semestres
a = 10;           # Créditos mínimos por semestre
b = 24;           # Créditos máximos por semestre
c = 2;            # Asignaturas mínimas por semestre
d = 10;           # Asignaturas máximas por semestre

courses = { MAT190, MAT191, ... };
credit = [ 4, 4, ... ];
prereq = { <MAT191, MAT190>, ... };   # <dependiente, prerequisito>
```

---

## Formato del archivo de salida (output.txt)

```
Pruebas <timestamp>
bacp8.txt - HILL CLIMBING
Número de movimientos: 1
Tiempo de ejecución: 0ms
MSE: 11.375
Es válido? Sí

Semestre 1:
    MAT190 (4)
    ...
Créditos: 15
Asignaturas: 5

bacp8.txt - Algoritmo Evolutivo
Tiempo de evolución de 100 instancias con 1000 generaciones: 275ms
Valor mínimo: 13.375
Es válido? Sí
...
```

---

## Estructura del código fuente

```
main.cpp
├── class Curso                       Entidad mínima del dominio
├── class Semestre                    Período académico con cursos asignados
├── struct EstadoMalla                Snapshot del estado mutable (para AE)
├── class MallaCurricular             Gestor central: datos + operaciones
├── class AlgoritmoOptimizacion       Clase base abstracta (polimorfismo)
├── class HillClimbingOpt             Hereda y optimiza por búsqueda local
├── class AlgoritmoEvolutivoOpt       Hereda y optimiza por evolución
└── int main()                        Orquestación: carga, ejecuta, reporta
```

Para una descripción técnica detallada de cada clase, método y decisión de diseño, consultar [DOCUMENTACION.md](DOCUMENTACION.md).
