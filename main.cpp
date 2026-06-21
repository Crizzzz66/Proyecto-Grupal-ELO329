#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <random>
using namespace std;

// ============================================================
// CLASE Curso: Entidad mínima del dominio curricular
// ============================================================
class Curso {
public:
    string nombre;
    int creditos;
    vector<int> requisitos;       // IDs de cursos prerequisito
    vector<int> post_requisitos;  // IDs de cursos que dependen de este
    int semestre_min;
    int semestre_max;

    Curso(const string& nombre, int creditos)
        : nombre(nombre), creditos(creditos), semestre_min(0), semestre_max(0) {}

    string getNombre() const { return nombre; }
    int getCreditos() const { return creditos; }
};

// ============================================================
// CLASE Semestre: Agregación de cursos en un período académico
// ============================================================
class Semestre {
public:
    int id;
    vector<int> cursos_asignados;
    int total_creditos;

    Semestre(int id) : id(id), total_creditos(0) {}

    void agregarCurso(int curso_id, int creditos_curso) {
        cursos_asignados.push_back(curso_id);
        total_creditos += creditos_curso;
    }

    void eliminarCurso(int curso_id, int creditos_curso) {
        for (int k = (int)cursos_asignados.size() - 1; k >= 0; k--)
            if (cursos_asignados[k] == curso_id) {
                cursos_asignados.erase(cursos_asignados.begin() + k);
                break;
            }
        total_creditos -= creditos_curso;
    }

    void calcularTotalCreditos(const vector<Curso>& lista_cursos) {
        total_creditos = 0;
        for (int id_c : cursos_asignados)
            total_creditos += lista_cursos[id_c].creditos;
    }
};

// ============================================================
// Snapshot ligero del estado mutable de la malla (para el AE)
// ============================================================
struct EstadoMalla {
    vector<int> x;
    vector<int> creditos_semestre;
    vector<vector<int>> cursos_semestre;
};

// ============================================================
// CLASE MallaCurricular: Gestor central del dominio
// ============================================================
class MallaCurricular {
public:
    int p, a, b, c, d;
    float creditos_promedio;
    int cursos_promedio;
    vector<Curso> cursos;
    vector<Semestre> semestres;
    vector<int> x;  // x[i] = semestre asignado al curso i

    MallaCurricular(int p, int a, int b, int c, int d)
        : p(p), a(a), b(b), c(c), d(d), creditos_promedio(0.0f), cursos_promedio(0) {
        for (int i = 0; i < p; i++)
            semestres.emplace_back(i);
    }

    // Calcula el MSE de créditos entre semestres
    double evaluarFuncionObjetivo() const {
        double output = 0.0;
        for (int i = 0; i < p; i++) {
            float diff = semestres[i].total_creditos - creditos_promedio;
            output += diff * diff;
        }
        return output;
    }

    // Verifica que la malla cumpla todas las restricciones; retorna 0 si es válida
    int validarRestricciones() const {
        for (int i = 0; i < p; i++) {
            const Semestre& sem = semestres[i];
            if (sem.total_creditos < a || sem.total_creditos > b)
                return i + 1;
            int len = (int)sem.cursos_asignados.size();
            if (len < c || len > d)
                return p + i + 1;
            for (int j = 0; j < len; j++) {
                int cid = sem.cursos_asignados[j];
                for (int req : cursos[cid].requisitos)
                    if (x[req] >= i)
                        return 2 * p + i + 1;
            }
        }
        return 0;
    }

    // Formatea e imprime la malla completa como string
    string imprimirMalla() const {
        string output = "";
        for (int i = 0; i < p; i++) {
            output += "Semestre " + to_string(i + 1) + ":\n";
            int jmax = (int)semestres[i].cursos_asignados.size();
            for (int j = 0; j < jmax; j++) {
                int cid = semestres[i].cursos_asignados[j];
                output += "    " + cursos[cid].nombre + " (" + to_string(cursos[cid].creditos) + ")\n";
            }
            output += "Créditos: " + to_string(semestres[i].total_creditos) + "\n"
                + "Asignaturas: " + to_string(jmax) + "\n";
        }
        return output;
    }

    // Asigna un curso a un semestre (para construcción inicial)
    void asignarCurso(int curso_id, int semestre_id) {
        semestres[semestre_id].agregarCurso(curso_id, cursos[curso_id].creditos);
        x[curso_id] = semestre_id;
    }

    // Mueve un curso de su semestre actual al semestre destino
    void moverCurso(int curso_id, int semestre_destino) {
        int origen = x[curso_id];
        semestres[origen].eliminarCurso(curso_id, cursos[curso_id].creditos);
        semestres[semestre_destino].agregarCurso(curso_id, cursos[curso_id].creditos);
        x[curso_id] = semestre_destino;
    }

    // Intenta realizar un movimiento de mejora (o aleatorio si !inteligente).
    // Retorna true si no se encontró movimiento (convergencia).
    bool realizarMovimiento(const vector<int>& flip, bool inteligente = true) {
        int cursos_total = (int)cursos.size();
        for (int ii = 0; ii < cursos_total; ii++) {
            int i = flip[ii];
            int this_semestre = x[i];
            int this_creditos = cursos[i].creditos;

            if ((int)semestres[this_semestre].cursos_asignados.size() <= c)
                continue;
            if (semestres[this_semestre].total_creditos - this_creditos < a)
                continue;

            float _creditos = 0.0f, cambio_prev = 0.0f, cambio = -1.0f;
            if (inteligente) {
                _creditos = -creditos_promedio;
                for (int j : semestres[this_semestre].cursos_asignados)
                    if (j != i) _creditos += cursos[j].creditos;
                cambio_prev = _creditos * _creditos - (_creditos + this_creditos) * (_creditos + this_creditos);
            }

            for (int j = cursos[i].semestre_min; j <= cursos[i].semestre_max; j++) {
                if (j == this_semestre) continue;
                if ((int)semestres[j].cursos_asignados.size() >= d) continue;
                if (semestres[j].total_creditos + this_creditos > b) continue;

                bool prereq_ok = true;
                for (int req : cursos[i].requisitos)
                    if (x[req] >= j) { prereq_ok = false; break; }
                if (!prereq_ok) continue;

                bool post_req_conflict = false;
                for (int post : cursos[i].post_requisitos)
                    if (x[post] <= j) { post_req_conflict = true; break; }
                if (post_req_conflict) continue;

                if (inteligente) {
                    _creditos = -creditos_promedio;
                    for (int k : semestres[j].cursos_asignados)
                        _creditos += cursos[k].creditos;
                    cambio = cambio_prev
                        + (_creditos + this_creditos) * (_creditos + this_creditos)
                        - _creditos * _creditos;
                }

                if (cambio < 0) {
                    moverCurso(i, j);
                    return false;
                }
            }
        }
        return true;
    }

    // Construye una solución inicial mediante algoritmo greedy
    void construirGreedy() {
        int cursos_total = (int)cursos.size();
        x.assign(cursos_total, 0);
        for (int i = 0; i < p; i++) {
            semestres[i].cursos_asignados.clear();
            semestres[i].total_creditos = 0;
        }

        int cp = cursos_promedio;
        float ca = creditos_promedio;

        for (int i = 0; i < cursos_total; i++) {
            bool asignado = false;
            for (int j = cursos[i].semestre_min; j <= cursos[i].semestre_max; j++) {
                bool prereq_ok = true;
                for (int req : cursos[i].requisitos)
                    if (x[req] >= j) { prereq_ok = false; break; }
                if (prereq_ok
                    && semestres[j].total_creditos + cursos[i].creditos <= ca
                    && (int)semestres[j].cursos_asignados.size() < cp) {
                    asignarCurso(i, j);
                    asignado = true;
                    break;
                }
            }
            if (!asignado) {
                if (cp < d)          { cp++;  asignado = true; }
                if (ca < (float)b)   { ca++;  asignado = true; }
                if (!asignado) {
                    cout << "No se pudo construir una solución inicial\n";
                    return;
                }
            }
        }
    }

    // Guarda el estado mutable actual (para poblaciones del AE)
    EstadoMalla guardarEstado() const {
        EstadoMalla estado;
        estado.x = x;
        estado.creditos_semestre.resize(p);
        estado.cursos_semestre.resize(p);
        for (int i = 0; i < p; i++) {
            estado.creditos_semestre[i] = semestres[i].total_creditos;
            estado.cursos_semestre[i]   = semestres[i].cursos_asignados;
        }
        return estado;
    }

    // Restaura el estado mutable desde un snapshot
    void cargarEstado(const EstadoMalla& estado) {
        x = estado.x;
        for (int i = 0; i < p; i++) {
            semestres[i].total_creditos   = estado.creditos_semestre[i];
            semestres[i].cursos_asignados = estado.cursos_semestre[i];
        }
    }

    // Evalúa el MSE de un snapshot sin cargarlo en la malla
    double evaluarEstado(const EstadoMalla& estado) const {
        double output = 0.0;
        for (int i = 0; i < p; i++) {
            float diff = estado.creditos_semestre[i] - creditos_promedio;
            output += diff * diff;
        }
        return output;
    }

    // Factory method: parsea un archivo de instancia y retorna la malla lista
    static MallaCurricular cargarDesdeArchivo(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Archivo no encontrado: " << filename << "\n";
            exit(1);
        }

        int p, a, b, c, d;
        string nn;

        file >> nn >> nn >> nn; nn.pop_back(); p = stoi(nn);
        file >> nn >> nn >> nn; nn.pop_back(); a = stoi(nn);
        file >> nn >> nn >> nn; nn.pop_back(); b = stoi(nn);
        file >> nn >> nn >> nn; nn.pop_back(); c = stoi(nn);
        file >> nn >> nn >> nn; nn.pop_back(); d = stoi(nn);

        MallaCurricular malla(p, a, b, c, d);

        // Nombres de cursos
        file >> nn >> nn >> nn;  // "courses = {"
        while (file >> nn && nn != "};") {
            if (nn.back() == ',') nn.pop_back();
            malla.cursos.emplace_back(nn, 0);
        }
        int cursos_total = (int)malla.cursos.size();
        malla.x.resize(cursos_total, 0);

        // Créditos
        float creditos_total = 0.0f;
        int idx = 0;
        file >> nn >> nn >> nn;  // "credit = ["
        while (file >> nn && nn != "];") {
            if (nn.back() == ',') nn.pop_back();
            int cr = stoi(nn);
            malla.cursos[idx].creditos = cr;
            creditos_total += cr;
            idx++;
        }

        // Pre-requisitos
        string par;
        file >> nn >> nn >> nn;  // "prereq = {"
        while (file >> par && par != "};") {
            par = par.substr(1, par.size() - 2);  // "<MAT023," -> "MAT023"
            for (int i = 0; i < cursos_total; i++) {
                if (malla.cursos[i].nombre == par) {
                    file >> nn;                              // "MAT021>," o "MAT021>;"
                    nn = nn.substr(0, nn.size() - 2);       // -> "MAT021"
                    for (int j = 0; j < cursos_total; j++) {
                        if (malla.cursos[j].nombre == nn) {
                            malla.cursos[i].requisitos.push_back(j);
                            malla.cursos[j].post_requisitos.push_back(i);
                            break;
                        }
                    }
                    break;
                }
            }
        }
        file.close();

        // Calcular semestre mínimo/máximo por dependencias
        for (int i = 0; i < cursos_total; i++) {
            malla.cursos[i].semestre_min = 0;
            malla.cursos[i].semestre_max = p - 1;
        }
        for (int i = 0; i < cursos_total; i++) {
            for (int req : malla.cursos[i].requisitos)
                if (malla.cursos[i].semestre_min <= malla.cursos[req].semestre_min)
                    malla.cursos[i].semestre_min = malla.cursos[req].semestre_min + 1;
            for (int post : malla.cursos[i].post_requisitos)
                if (malla.cursos[i].semestre_max >= malla.cursos[post].semestre_max)
                    malla.cursos[i].semestre_max = malla.cursos[post].semestre_max - 1;
        }

        malla.creditos_promedio = creditos_total / p;
        malla.cursos_promedio   = cursos_total / p;
        return malla;
    }
};

// ============================================================
// CLASE BASE ABSTRACTA AlgoritmoOptimizacion
// ============================================================
class AlgoritmoOptimizacion {
protected:
    MallaCurricular& malla;
public:
    explicit AlgoritmoOptimizacion(MallaCurricular& malla) : malla(malla) {}
    virtual void optimizar() = 0;
    virtual ~AlgoritmoOptimizacion() {}
};

// ============================================================
// CLASE HillClimbingOpt: Búsqueda local por Hill Climbing
// ============================================================
class HillClimbingOpt : public AlgoritmoOptimizacion {
private:
    int max_iter;
    int contador_movimientos;
    long long tiempo_ms;

public:
    explicit HillClimbingOpt(MallaCurricular& malla, int max_iter = 100)
        : AlgoritmoOptimizacion(malla), max_iter(max_iter),
          contador_movimientos(0), tiempo_ms(0) {}

    void optimizar() override {
        malla.construirGreedy();
        int cursos_total = (int)malla.cursos.size();
        vector<int> flip(cursos_total);
        for (int i = 0; i < cursos_total; i++) flip[i] = i;

        auto start = chrono::steady_clock::now();
        contador_movimientos = 0;
        for (int iter = 0; iter < max_iter; iter++) {
            if (malla.realizarMovimiento(flip))
                break;
            else
                contador_movimientos++;
        }
        tiempo_ms = chrono::duration_cast<chrono::milliseconds>(
            chrono::steady_clock::now() - start).count();
    }

    int      getContadorMovimientos() const { return contador_movimientos; }
    long long getTiempoMs()           const { return tiempo_ms; }
};

// ============================================================
// CLASE AlgoritmoEvolutivoOpt: Optimización por Algoritmo Evolutivo
// ============================================================
class AlgoritmoEvolutivoOpt : public AlgoritmoOptimizacion {
private:
    int   AE_instancias;
    int   AE_random_steps;
    int   max_iter;
    float min_fit;
    float mejor_valor;
    long long tiempo_ms;

    // Selección aleatoria ponderada
    int weigthed_choose(const vector<float>& peso, int valor_total) {
        if (valor_total <= 0) return rand() % (int)peso.size();
        float a = (float)(rand() % valor_total);
        for (int b = (int)peso.size() - 1; b >= 0; b--) {
            a -= peso[b];
            if (a <= 0) return b;
        }
        return -1;
    }

public:
    explicit AlgoritmoEvolutivoOpt(MallaCurricular& malla,
                                   int instancias    = 100,
                                   int random_steps  = 20,
                                   int max_iter      = 1000,
                                   float min_fit     = 1.0f)
        : AlgoritmoOptimizacion(malla),
          AE_instancias(instancias), AE_random_steps(random_steps),
          max_iter(max_iter), min_fit(min_fit),
          mejor_valor(9999999.0f), tiempo_ms(0) {}

    void optimizar() override {
        malla.construirGreedy();
        int cursos_total = (int)malla.cursos.size();
        vector<int> flip(cursos_total);
        for (int i = 0; i < cursos_total; i++) flip[i] = i;

        random_device rd;
        srand(time(0));

        // Generar y diversificar población inicial
        auto t0 = chrono::steady_clock::now();
        vector<EstadoMalla> poblacion(AE_instancias, malla.guardarEstado());
        for (int inst = 0; inst < AE_instancias; inst++) {
            malla.cargarEstado(poblacion[inst]);
            for (int i = 0; i < AE_random_steps; i++) {
                shuffle(flip.begin(), flip.end(), rd);
                malla.realizarMovimiento(flip, false);
            }
            poblacion[inst] = malla.guardarEstado();
        }
        cout << "Tiempo de generación de " << AE_instancias << " instancias: "
             << chrono::duration_cast<chrono::milliseconds>(
                    chrono::steady_clock::now() - t0).count() << "ms\n";

        // Bucle evolutivo
        auto start = chrono::steady_clock::now();
        mejor_valor = 9999999.0f;

        for (int iter = 0; iter < max_iter; iter++) {
            bool stable = true;
            bool finish = false;
            float valor_total = 0.0f;
            vector<float> valores(AE_instancias, 0.0f);

            // Evaluar toda la población
            for (int inst = 0; inst < AE_instancias; inst++) {
                float fit = (float)malla.evaluarEstado(poblacion[inst]);
                if (fit < mejor_valor) {
                    mejor_valor = fit;
                    cout << iter << ": " << mejor_valor << "\n";
                    if (fit < min_fit || isnan(fit)) {
                        finish = true;
                        cout << "Mejor modelo encontrado\n";
                        break;
                    }
                }
                float val = fit > 999.0f ? 0.0f : 999.0f - fit;
                valores[inst] = val;
                valor_total  += val;
            }
            if (finish) break;

            // Selección: nueva generación (sin sobreescribir la actual)
            vector<EstadoMalla> nueva_gen(AE_instancias);
            for (int inst = 0; inst < AE_instancias; inst++) {
                int sel = weigthed_choose(valores, (int)valor_total);
                nueva_gen[inst] = poblacion[sel >= 0 ? sel : 0];
            }
            poblacion = nueva_gen;

            // Mutación: un movimiento por individuo
            for (int inst = 0; inst < AE_instancias; inst++) {
                malla.cargarEstado(poblacion[inst]);
                shuffle(flip.begin(), flip.end(), rd);
                bool inteligente = (rand() % max_iter < iter);
                if (!malla.realizarMovimiento(flip, inteligente))
                    stable = false;
                poblacion[inst] = malla.guardarEstado();
            }
            if (stable) break;
        }

        tiempo_ms = chrono::duration_cast<chrono::milliseconds>(
            chrono::steady_clock::now() - start).count();

        // Restaurar el mejor individuo en la malla
        float min_val = 9999999.0f;
        int   min_idx = 0;
        for (int inst = 0; inst < AE_instancias; inst++) {
            float fit = (float)malla.evaluarEstado(poblacion[inst]);
            if (fit < min_val) { min_val = fit; min_idx = inst; }
        }
        mejor_valor = min_val;
        malla.cargarEstado(poblacion[min_idx]);

        cout << "Tiempo de ejecución de " << AE_instancias << " instancias: "
             << tiempo_ms << "ms\n";
    }

    float     getMejorValor() const { return mejor_valor; }
    long long getTiempoMs()   const { return tiempo_ms; }
};

// ============================================================
// FUNCIÓN PRINCIPAL: Orquestación
// ============================================================
int main() {
    // Timestamp inicial en output.txt
    ofstream outFile("output.txt", ios::app);
    if (!outFile.is_open()) { cerr << "Error al abrir output.txt\n"; return 1; }
    time_t now = time(nullptr);
    outFile << "Pruebas " << ctime(&now);
    outFile.close();

    vector<string> archivos = {
        "bacp8.txt",
        //"bacp10.txt",
        //"bacp12.txt",
        //"utfsm3.txt",
        //"utfsm2311.txt",
        //"utfsm2920.txt",
        //"utfsm7310.txt",
        //"utfsm7313.txt"
    };

    bool MODELO_HC  = true;
    bool MODELO_AE  = true;
    bool SHOW_MALLA = true;

    for (int i_a = (int)archivos.size() - 1; i_a >= 0; i_a--) {
        const string& filename = archivos[i_a];
        MallaCurricular malla = MallaCurricular::cargarDesdeArchivo(filename);

        // --- Hill Climbing ---
        if (MODELO_HC) {
            HillClimbingOpt hc(malla);
            hc.optimizar();

            ofstream out("output.txt", ios::app);
            out << filename << " - HILL CLIMBING\n"
                << "Número de movimientos: " << hc.getContadorMovimientos() << "\n"
                << "Tiempo de ejecución: "   << hc.getTiempoMs() << "ms\n"
                << "MSE: "      << malla.evaluarFuncionObjetivo() << "\n"
                << "Es válido? " << (malla.validarRestricciones() ? "No" : "Sí") << "\n\n";
            if (SHOW_MALLA) out << malla.imprimirMalla() << "\n";
            out.close();
        }

        // --- Algoritmo Evolutivo ---
        if (MODELO_AE) {
            AlgoritmoEvolutivoOpt ae(malla);
            ae.optimizar();

            ofstream out("output.txt", ios::app);
            out << filename << " - Algoritmo Evolutivo\n"
                << "Tiempo de evolución de 100 instancias con 1000 generaciones: "
                << ae.getTiempoMs() << "ms\n"
                << "Valor mínimo: " << ae.getMejorValor() << "\n"
                << "Es válido? " << (malla.validarRestricciones() ? "No" : "Sí") << "\n\n";
            if (SHOW_MALLA) out << malla.imprimirMalla() << "\n";
            out.close();
        }
    }

    return 0;
}
