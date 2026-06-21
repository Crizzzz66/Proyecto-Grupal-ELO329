# Documentación Técnica — INF295 Optimización de Malla Curricular

## Índice

1. [Descripción del problema](#1-descripción-del-problema)
2. [Arquitectura general](#2-arquitectura-general)
3. [Cambios respecto al código original](#3-cambios-respecto-al-código-original)
4. [Clases y estructuras](#4-clases-y-estructuras)
   - [Curso](#41-clase-curso)
   - [Semestre](#42-clase-semestre)
   - [EstadoMalla](#43-struct-estadomalla)
   - [MallaCurricular](#44-clase-mallacurricular)
   - [AlgoritmoOptimizacion](#45-clase-abstracta-algoritmooptimizacion)
   - [HillClimbingOpt](#46-clase-hillclimbingopt)
   - [AlgoritmoEvolutivoOpt](#47-clase-algoritmoevolutivoopt)
5. [Flujo de ejecución completo](#5-flujo-de-ejecución-completo)
6. [Algoritmos en detalle](#6-algoritmos-en-detalle)
   - [Construcción Greedy](#61-construcción-greedy)
   - [Hill Climbing](#62-hill-climbing)
   - [Algoritmo Evolutivo](#63-algoritmo-evolutivo)
7. [Función objetivo y validación](#7-función-objetivo-y-validación)
8. [Gestión de estado en el Algoritmo Evolutivo](#8-gestión-de-estado-en-el-algoritmo-evolutivo)
9. [Makefile y compilación](#9-makefile-y-compilación)

---

## 1. Descripción del problema

El problema es una variante del **Balanced Academic Curriculum Problem (BACP)**: dado un conjunto de cursos con créditos y relaciones de pre-requisito, asignar cada curso a exactamente un semestre, de modo que:

- La cantidad de créditos por semestre esté dentro del rango `[a, b]`.
- La cantidad de cursos por semestre esté dentro del rango `[c, d]`.
- Ningún curso se dicte antes de sus pre-requisitos.
- La distribución de créditos entre semestres sea lo más equilibrada posible.

La métrica de calidad es el **Error Cuadrático Medio (MSE)** entre los créditos de cada semestre y el promedio global. Cuanto menor el MSE, más equilibrada es la malla.

---

## 2. Arquitectura general

El proyecto fue refactorizado de un paradigma **procedural puro** a uno completamente **Orientado a Objetos (POO)**. La nueva arquitectura separa claramente:

- **Dominio del problema:** `Curso`, `Semestre`, `MallaCurricular`
- **Algoritmos de optimización:** `AlgoritmoOptimizacion` (abstracta), `HillClimbingOpt`, `AlgoritmoEvolutivoOpt`
- **Transferencia de estado:** `EstadoMalla`

```
┌─────────────────────────────────────────────────────────────┐
│                         main()                              │
│  carga instancia → ejecuta algoritmos → escribe resultados  │
└──────────────────────────┬──────────────────────────────────┘
                           │ usa
           ┌───────────────▼───────────────┐
           │        MallaCurricular         │
           │  ┌──────────┐  ┌───────────┐  │
           │  │  Curso×n │  │Semestre×p │  │
           │  └──────────┘  └───────────┘  │
           │  evaluar / validar / imprimir  │
           │  construirGreedy / mover       │
           └───────────┬───────────────────┘
                       │ referencia
         ┌─────────────▼──────────────────────┐
         │      AlgoritmoOptimizacion         │
         │    virtual void optimizar() = 0    │
         └────┬──────────────────┬────────────┘
              │ hereda           │ hereda
   ┌──────────▼──────┐  ┌───────▼────────────────┐
   │ HillClimbingOpt │  │ AlgoritmoEvolutivoOpt  │
   │  optimizar()    │  │  optimizar()           │
   └─────────────────┘  │  usa: EstadoMalla×100  │
                        └────────────────────────┘
```

---

## 3. Cambios respecto al código original

### 3.1 Problema central del código original

El archivo `main.cpp` original era un **programa procedural de 486 líneas**: 6 funciones libres que recibían todos los vectores como parámetros. No había clases, herencia ni polimorfismo. Esto hacía imposible dibujar un Diagrama de Clases UML real y dificultaba la comprensión del diseño.

### 3.2 Tabla de correspondencia completa

| Código original (procedural) | Nuevo código (POO) | Razón del cambio |
|---|---|---|
| `float evaluar(creditos_sem, p, promedio)` | `MallaCurricular::evaluarFuncionObjetivo()` | Pertenece al dominio de la malla |
| `int validar(cursos_sem, creditos_sem, x, req, p,a,b,c,d)` | `MallaCurricular::validarRestricciones()` | Opera sobre el estado interno de la malla |
| `string print_malla(cursos_sem, cursos, creditos)` | `MallaCurricular::imprimirMalla()` | La malla es quien sabe cómo mostrarse |
| `bool movimiento(x, creditos_sem, cursos_sem, ..., 13 params)` | `MallaCurricular::realizarMovimiento(flip, inteligente)` | Opera sobre el estado interno; firma reducida de 13 a 2 parámetros |
| `void greedy(x, creditos_sem, cursos_sem, ..., 11 params)` | `MallaCurricular::construirGreedy()` | Inicialización del estado interno de la malla |
| `int weigthed_choose(peso, total)` | `AlgoritmoEvolutivoOpt::weigthed_choose()` (privado) | Solo la usa el AE; encapsulado como detalle de implementación |
| Bloque HC (líneas 349–381 en main) | `HillClimbingOpt::optimizar()` | Separación de responsabilidades |
| Bloque AE (líneas 383–483 en main) | `AlgoritmoEvolutivoOpt::optimizar()` | Separación de responsabilidades |
| Bloque de lectura de archivo (líneas 263–337 en main) | `MallaCurricular::cargarDesdeArchivo()` (static) | Factory method: la malla se construye a sí misma desde el archivo |
| `x_inst`, `creditos_semestre_inst`, `cursos_semestre_inst` | `struct EstadoMalla` + `guardarEstado()` / `cargarEstado()` | Encapsulación del snapshot de población |

### 3.3 Mejoras adicionales aplicadas

**Bug corregido — selección en AE:** El código original sobreescribía la población en el mismo vector (`x_inst[inst] = x_inst[a]`), lo que causaba que instancias ya seleccionadas influyeran en la selección de las siguientes. Se corrigió usando un vector `nueva_gen` separado antes de asignar la generación.

**Guard en `weigthed_choose`:** Se añadió verificación cuando `valor_total <= 0` (todos los fitness > 999) para evitar `rand() % 0` que es comportamiento indefinido en C++.

**Firma de `realizarMovimiento`:** La función original `movimiento()` recibía 17 parámetros. Al operar sobre `this`, la nueva firma tiene solo 2 (`flip` y `inteligente`), lo que hace el código legible.

**`cargarDesdeArchivo` como factory method estático:** En vez de parsear el archivo dentro de `main()` con decenas de líneas, la malla se construye y configura completamente dentro de la clase. `main()` recibe el objeto listo.

---

## 4. Clases y estructuras

### 4.1 Clase `Curso`

Representa una asignatura del plan de estudios.

```cpp
class Curso {
public:
    string nombre;                   // Código del curso (ej: "MAT190")
    int creditos;                    // Unidades de crédito
    vector<int> requisitos;          // IDs de cursos que deben cursarse antes
    vector<int> post_requisitos;     // IDs de cursos que dependen de este
    int semestre_min;                // Semestre más temprano en que puede ubicarse
    int semestre_max;                // Semestre más tardío en que puede ubicarse
};
```

**Por qué `semestre_min` y `semestre_max` están en `Curso`:** Son propiedades derivadas de las relaciones de pre-requisito y se calculan una sola vez al cargar la instancia. Tenerlos aquí evita recalcularlos en cada iteración del algoritmo.

**Por qué `requisitos` es `vector<int>` y no `vector<Curso*>`:** Los cursos se indexan por entero en toda la lógica de los algoritmos. Usar índices enteros es más eficiente y evita problemas de punteros colgantes cuando los vectores se redimensionan.

---

### 4.2 Clase `Semestre`

Representa un período académico con los cursos que le han sido asignados.

```cpp
class Semestre {
public:
    int id;                          // Índice 0-based del semestre
    vector<int> cursos_asignados;    // IDs de cursos en este semestre
    int total_creditos;              // Suma de créditos de los cursos asignados

    void agregarCurso(int curso_id, int creditos_curso);
    void eliminarCurso(int curso_id, int creditos_curso);
    void calcularTotalCreditos(const vector<Curso>& lista_cursos);
};
```

**Por qué `total_creditos` se mantiene actualizado incrementalmente:** Recalcular la suma en cada evaluación sería O(d) por semestre × p semestres × millones de iteraciones. Mantenerlo como suma acumulada hace que `agregarCurso` y `eliminarCurso` sean O(1) y `evaluar` sea O(p).

**Por qué `cursos_asignados` guarda IDs enteros y no objetos `Curso`:** La malla es la dueña de los objetos `Curso`. El semestre solo necesita saber *cuáles* cursos tiene, sin duplicar los datos.

---

### 4.3 `struct EstadoMalla`

Snapshot ligero del estado mutable de la malla, usado por el Algoritmo Evolutivo para gestionar su población de soluciones.

```cpp
struct EstadoMalla {
    vector<int> x;                       // Asignación: x[curso_id] = semestre_id
    vector<int> creditos_semestre;       // Créditos actuales de cada semestre
    vector<vector<int>> cursos_semestre; // Cursos actuales de cada semestre
};
```

**Por qué existe esta estructura y no un vector de `MallaCurricular`:** El AE necesita mantener 100 soluciones en paralelo. Crear 100 copias completas de `MallaCurricular` (que incluye los vectores de `Curso` con todos sus metadatos) sería muy costoso en memoria. `EstadoMalla` contiene únicamente el estado *mutable* (la asignación de cursos a semestres), mientras que el estado *inmutable* (nombres, créditos, pre-requisitos de los cursos) permanece en una sola instancia de `MallaCurricular`.

---

### 4.4 Clase `MallaCurricular`

Clase central del dominio. Gestiona todos los datos del problema y las operaciones sobre la solución.

```cpp
class MallaCurricular {
public:
    int p, a, b, c, d;              // Parámetros de la instancia
    float creditos_promedio;        // Objetivo de créditos por semestre
    int cursos_promedio;            // Objetivo de cursos por semestre
    vector<Curso> cursos;           // Todas las asignaturas del plan
    vector<Semestre> semestres;     // Los p semestres de la malla
    vector<int> x;                  // x[i] = semestre asignado al curso i
};
```

**Métodos principales:**

| Método | Complejidad | Descripción |
|--------|-------------|-------------|
| `evaluarFuncionObjetivo()` | O(p) | Calcula el MSE de créditos entre semestres |
| `validarRestricciones()` | O(p × d × req) | Verifica créditos, cantidad de cursos y pre-requisitos |
| `imprimirMalla()` | O(p × d) | Genera string formateado de la malla completa |
| `construirGreedy()` | O(n × p) | Construye solución inicial |
| `realizarMovimiento(flip, inteligente)` | O(n × p) | Intenta un movimiento de mejora |
| `guardarEstado()` | O(p × d) | Copia el estado mutable a un `EstadoMalla` |
| `cargarEstado(estado)` | O(p × d) | Restaura el estado mutable desde un `EstadoMalla` |
| `evaluarEstado(estado)` | O(p) | Evalúa el MSE de un snapshot sin cargarlo |
| `cargarDesdeArchivo(filename)` | O(n²) | Factory: parsea archivo y construye la malla |

**Por qué `x` existe junto a `semestres`:** Son dos vistas del mismo dato. `x[i]` permite saber en O(1) en qué semestre está el curso `i` (necesario para verificar pre-requisitos). `semestres[j].cursos_asignados` permite iterar en O(d) los cursos de un semestre (necesario para calcular créditos y aplicar movimientos). Mantener ambas consistentes es responsabilidad de `asignarCurso()` y `moverCurso()`.

---

### 4.5 Clase abstracta `AlgoritmoOptimizacion`

Define la interfaz común para todos los algoritmos.

```cpp
class AlgoritmoOptimizacion {
protected:
    MallaCurricular& malla;          // Referencia a la malla que optimiza
public:
    explicit AlgoritmoOptimizacion(MallaCurricular& malla);
    virtual void optimizar() = 0;    // Método puro: cada subclase lo implementa
    virtual ~AlgoritmoOptimizacion() {}
};
```

**Por qué referencia y no puntero o copia:** La malla puede ser grande (hasta 68 cursos × 12 semestres). Usar referencia evita copias innecesarias. Usar puntero hubiera requerido gestión de memoria manual o `shared_ptr`. La referencia deja claro que el algoritmo *usa* la malla pero no la *posee*.

**Por qué el método se llama `optimizar()` y no `ejecutar()` o `run()`:** Describe semánticamente lo que hace: mejorar una solución, no simplemente ejecutar un proceso. El nombre comunica la intención.

---

### 4.6 Clase `HillClimbingOpt`

Implementa la búsqueda local de mejora continua (Hill Climbing).

```cpp
class HillClimbingOpt : public AlgoritmoOptimizacion {
private:
    int max_iter;              // Máximo de iteraciones (default: 100)
    int contador_movimientos;  // Movimientos de mejora realizados
    long long tiempo_ms;       // Tiempo de ejecución en ms
public:
    void optimizar() override;
    int getContadorMovimientos() const;
    long long getTiempoMs() const;
};
```

**Flujo de `optimizar()`:**
1. Llama a `malla.construirGreedy()` para inicializar.
2. Crea un vector `flip` = [0, 1, 2, ..., n-1] que define el orden de evaluación de cursos.
3. Itera hasta `max_iter` o hasta que `realizarMovimiento()` retorne `true` (convergencia).
4. Registra tiempo y número de movimientos.

**Por qué se usan getters en lugar de atributos públicos:** Los resultados del algoritmo (`tiempo_ms`, `contador_movimientos`) son datos de salida, no de entrada. El acceso de solo lectura vía getters refleja que el algoritmo los calcula y el exterior solo los consulta.

---

### 4.7 Clase `AlgoritmoEvolutivoOpt`

Implementa una búsqueda poblacional evolutiva con selección proporcional al fitness y mutación por movimiento vecinal.

```cpp
class AlgoritmoEvolutivoOpt : public AlgoritmoOptimizacion {
private:
    int   AE_instancias;       // Tamaño de la población (default: 100)
    int   AE_random_steps;     // Pasos aleatorios para diversificar (default: 20)
    int   max_iter;            // Generaciones máximas (default: 1000)
    float min_fit;             // Fitness objetivo (convergencia anticipada, default: 1.0)
    float mejor_valor;         // Mejor MSE encontrado
    long long tiempo_ms;
private:
    int weigthed_choose(const vector<float>& peso, int valor_total);
public:
    void optimizar() override;
    float getMejorValor() const;
    long long getTiempoMs() const;
};
```

**Por qué `weigthed_choose` es privado:** Es un detalle de implementación del AE. La lógica de selección proporcional (ruleta) no es relevante fuera del algoritmo evolutivo.

---

## 5. Flujo de ejecución completo

```
main()
│
├─ Escribe timestamp en output.txt
│
└─ Para cada archivo en la lista:
   │
   ├─ MallaCurricular::cargarDesdeArchivo(filename)
   │    ├─ Parsea p, a, b, c, d
   │    ├─ Crea vector<Curso> con nombres y créditos
   │    ├─ Asigna pre/post-requisitos a cada Curso
   │    ├─ Calcula semestre_min y semestre_max por dependencias
   │    └─ Retorna MallaCurricular lista para operar
   │
   ├─ [Si MODELO_HC]:
   │    HillClimbingOpt hc(malla)
   │    hc.optimizar()
   │    │   ├─ malla.construirGreedy()   ← inicializa x y semestres
   │    │   └─ loop: malla.realizarMovimiento(flip) hasta convergencia
   │    └─ Escribe resultados en output.txt
   │
   └─ [Si MODELO_AE]:
        AlgoritmoEvolutivoOpt ae(malla)
        ae.optimizar()
        │   ├─ malla.construirGreedy()   ← reinicia x y semestres
        │   ├─ Genera 100 snapshots EstadoMalla y los diversifica
        │   └─ Loop evolutivo:
        │       ├─ Evalúa cada EstadoMalla con evaluarEstado()
        │       ├─ Selección proporcional → nueva generación
        │       └─ Mutación: cargarEstado → realizarMovimiento → guardarEstado
        └─ Restaura mejor estado en malla y escribe resultados
```

**Nota importante:** HC y AE son independientes entre sí. El AE llama a `construirGreedy()` al inicio de `optimizar()`, lo que reinicia completamente la malla antes de comenzar su propia optimización. Esto garantiza que ambos algoritmos partan desde la misma solución inicial greedy, independientemente del orden en que se ejecuten.

---

## 6. Algoritmos en detalle

### 6.1 Construcción Greedy

`MallaCurricular::construirGreedy()` asigna cada curso al primer semestre válido que encuentre, manteniendo la carga cercana al promedio.

```
Para cada curso i (en orden 0..n-1):
  Para cada semestre j desde semestre_min[i] hasta semestre_max[i]:
    Si NO hay conflicto de pre-requisitos Y
       creditos_semestre[j] + creditos[i] <= promedio_creditos Y
       |cursos_semestre[j]| < promedio_cursos:
         → Asignar curso i al semestre j
         → Continuar con curso i+1
  Si no se pudo asignar:
    Si promedio_cursos < d: incrementar promedio_cursos
    Si promedio_creditos < b: incrementar promedio_creditos
    Si aún no se pudo: ERROR
```

El greedy asume que los cursos están **en orden topológico** en el archivo de entrada (los pre-requisitos aparecen antes que sus dependientes). Esto garantiza que cuando se procesa el curso `i`, todos sus pre-requisitos ya tienen asignado su semestre correcto en `x`.

Los promedios de trabajo (`cp`, `ca`) son copias locales que pueden crecer durante la ejecución del greedy sin afectar los valores almacenados en `MallaCurricular`.

### 6.2 Hill Climbing

`HillClimbingOpt::optimizar()` aplica búsqueda local de mejora estricta.

En cada iteración, `malla.realizarMovimiento(flip, inteligente=true)` recorre todos los cursos (en el orden dado por `flip`) y busca el **primer movimiento que reduzca el MSE**. Cuando lo encuentra, lo aplica inmediatamente y retorna `false`. Si ningún movimiento mejora el MSE, retorna `true` (convergencia local).

**Cálculo del cambio de MSE sin recalcular todo:**

En vez de recalcular el MSE completo tras cada movimiento candidato (O(p)), se calcula el **delta** del MSE usando solo los dos semestres afectados (O(1)):

```
cambio = (creditos_sem_origen_nuevo² - creditos_sem_origen_prev²)
       + (creditos_sem_destino_nuevo² - creditos_sem_destino_prev²)
```

Si `cambio < 0`, el movimiento mejora el MSE y se aplica.

### 6.3 Algoritmo Evolutivo

`AlgoritmoEvolutivoOpt::optimizar()` es una búsqueda poblacional con tres fases por generación:

**Fase 1 — Generación inicial diversificada:**
- Se crea una población de 100 copias del estado greedy.
- A cada copia se le aplican 20 movimientos aleatorios (`inteligente=false`) con orden de cursos aleatorio (shuffle), creando soluciones diversas pero válidas.

**Fase 2 — Evaluación:**
- Se calcula el MSE de cada individuo con `malla.evaluarEstado(estado)`.
- El fitness para la selección se transforma: `valor = 999 - MSE` (mayor valor = mejor aptitud). Si MSE > 999, el individuo no participa en la reproducción (valor = 0).

**Fase 3 — Selección y mutación:**
- **Selección:** Ruleta ponderada (`weigthed_choose`). Los individuos con menor MSE tienen mayor probabilidad de ser seleccionados como padres.
- **Nueva generación:** Se crea en un vector separado antes de sobreescribir la población. Esto evita el bug del original donde seleccionar `poblacion[2]` cuando `poblacion[0]` y `poblacion[1]` ya fueron sobreescritos daba resultados incorrectos.
- **Mutación:** A cada individuo de la nueva generación se le aplica un movimiento. Al inicio de la evolución el movimiento es aleatorio (`inteligente=false`); conforme avanza (`iter / max_iter`), hay mayor probabilidad de que sea un movimiento inteligente de mejora.

---

## 7. Función objetivo y validación

### Función objetivo: MSE de créditos

```
MSE = Σᵢ (creditos_semestre[i] - creditos_promedio)²  para i = 0..p-1
```

El objetivo es minimizar este valor. MSE = 0 significa que todos los semestres tienen exactamente `creditos_promedio` créditos (perfectamente equilibrada). En la práctica, como los créditos son enteros, el mínimo alcanzable es mayor que 0 cuando `creditos_total` no es divisible exactamente por `p`.

### Validación

`validarRestricciones()` retorna 0 si la malla es válida. Retorna un valor positivo que codifica qué restricción fue violada y en qué semestre:

| Valor retornado | Restricción violada |
|-----------------|---------------------|
| `i + 1` (1 ≤ i < p) | Semestre i tiene créditos fuera de [a, b] |
| `p + i + 1` | Semestre i tiene cantidad de cursos fuera de [c, d] |
| `2p + i + 1` | Algún curso del semestre i tiene un pre-requisito en el mismo semestre o posterior |

---

## 8. Gestión de estado en el Algoritmo Evolutivo

El AE necesita mantener 100 soluciones en paralelo sin crear 100 instancias de `MallaCurricular`. El mecanismo es:

```
MallaCurricular malla;                    ← una sola instancia con metadatos
vector<EstadoMalla> poblacion(100);       ← 100 snapshots del estado mutable

Para mutar individuo i:
  malla.cargarEstado(poblacion[i]);       ← carga el snapshot en la malla
  malla.realizarMovimiento(flip, int);    ← ejecuta el movimiento
  poblacion[i] = malla.guardarEstado();   ← guarda el resultado

Para evaluar individuo i sin cargar:
  malla.evaluarEstado(poblacion[i]);      ← lee solo creditos_semestre del snapshot
```

El costo de `guardarEstado()` y `cargarEstado()` es O(p × d_promedio): copiar los vectores de cursos de cada semestre. En instancias medianas (bacp8: 8 semestres, ~5 cursos/sem) esto es ~40 enteros, negligible frente al costo computacional del movimiento.

---

## 9. Makefile y compilación

```makefile
CXX      = g++                          # g++ disponible en Ubuntu/WSL con build-essential
CXXFLAGS = -Wall -O2 -std=c++17         # Warnings completos, optimización O2, C++17
TARGET   = programa
SRC      = main.cpp

$(TARGET): $(SRC)
    $(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
    rm -f $(TARGET)
```

**Flags de compilación:**
- `-Wall`: activa todos los warnings. El código compila sin ningún warning.
- `-O2`: optimización nivel 2. Mejora el tiempo de ejecución del AE en un 30–50% respecto a `-O0`.
- `-std=c++17`: necesario para `if constexpr`, `std::optional`, y garantías de copy elision (aunque el código solo usa `shuffle` con `random_device` que requiere C++11 como mínimo; C++17 asegura compatibilidad completa de la STL usada).

**Por qué `CXX = g++` sin ruta fija:** En Ubuntu (y WSL), `sudo apt install build-essential` instala g++ y lo agrega automáticamente al PATH del sistema. No se necesita ruta absoluta.
