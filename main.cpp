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
/*
    IMPORTANTE IMPLEMENTAR
    -Probar Greedy con distintos parámetros.
    -Los cursos pueden tener más de un pre-requisito. (Listo)
    -Automatizar pruebas con BATCH (semi-listo)
    -Implementar Algoritmo Genético
*/
/**
 * @brief Calcula la desviación cuadrada de los ramos.
 * @param creditos_semestre Créditos de cada semestre.
 * @param p Número de semestres.
 * @param creditos_promedio Créditos promedio por semestre.
 * @return Error cuadrático total.
 */
float evaluar(const vector<int> &creditos_semestre, int p, float creditos_promedio){
    float output = 0.0;
    for(int i = 0; i < p; i++){
        float _creditos = creditos_semestre[i] - creditos_promedio;
        output += _creditos * _creditos;
    }
    return output;
}
/**
 * @brief Verifica si una solución es factible
 * @param cursos_semestre Cursos asignados a cada semestre
 * @param creditos_semestre Créditos asignados a cada semestre
 * @param x Semestre en que se asigna cada curso
 * @param requisito Mapa de los requisitos de cada curso
 * @param p Número de semestres
 * @param a Créditos mínimos por semestre
 * @param b Créditos máximos por semestre
 * @param c Cursos mínimos por semestre
 * @param d Cursos máximos por semestre
 * @return true Si la malla es válida, false sino
 */
int validar(const vector<vector<int>> &cursos_semestre, const vector<int> &creditos_semestre, const vector<int> &x, const vector<vector<int>> &requisito, int p, int a, int b, int c, int d){
    for(int i = 0; i < p; i++){
        if (creditos_semestre[i] < a || creditos_semestre[i] > b)
            return i + 1;
        int len = cursos_semestre[i].size();
        if (len < c || len > d)
            return p + i + 1;
        for(int j = 0; j < len; j++){
            int _curso = cursos_semestre[i][j];
            for(int k = requisito[_curso].size() - 1; k >= 0; k--)
                if (x[requisito[_curso][k]] >= i)
                    return 2 * p + i + 1;
        }
    }
    return 0;
}
/**
 * @brief Escribe toda la malla creada a la consola
 * @param cursos_semestre Cursos asignados a cada semestre
 * @param cursos Nombre de los cursos
 * @param creditos Cantidad de créditos de cada curso
 * @param extensive Aplicar el formato de entrega o el formato reducido
 */
string print_malla(const vector<vector<int>> &cursos_semestre, const vector<string> cursos, const vector<int> creditos){
    string output = "";
    int imax = cursos_semestre.size();
    for(int i = 0; i < imax; i++){
        output += "Semestre " + to_string(i + 1) + ":\n";
        int jmax = cursos_semestre[i].size(), _creditos = 0;
        for(int j = 0; j < jmax; j++){
            output += "    " + cursos[cursos_semestre[i][j]] + " (" + to_string(creditos[cursos_semestre[i][j]]) + ")\n";
            _creditos += creditos[cursos_semestre[i][j]];
        }
        output += "Créditos: " + to_string(_creditos) + "\n"
            + "Asignaturas: " + to_string(jmax) + "\n";
    }
    return output;
}
/**
 * @brief Mueve una asignatura desde un semestre hasta otro. Se asegura antes de que no rompa restricciones.
 * @param x Semestre en que se asigna cada curso
 * @param creditos_semestre Créditos asignados a cada semestre
 * @param cursos_semestre Cursos asignados a cada semestre
 * @param creditos Créditos de cada asignatura
 * @param curso_semestre_min Semestre mínimo en que un curso puede ser asignado
 * @param curso_semestre_max Semestre máximo en que un curso puede ser asignado
 * @param requisito Mapa de los requisitos de cada curso
 * @param post_requisito Mapa de los post-requisitos de cada curso
 * @param flip vector para cambiar el orden de evaluación
 * @param creditos_promedio Créditos promedio
 * @param a Créditos mínimos por semestre
 * @param b Créditos máximos por semestre
 * @param c Cursos mínimos por semestre
 * @param d Cursos máximos por semestre
 * @return true si logró implementar un movimiento, false sino
 */
bool movimiento(vector<int> &x, vector<int> &creditos_semestre, vector<vector<int>> &cursos_semestre, const vector<int> &creditos, const vector<int> &curso_semestre_min, const vector<int> &curso_semestre_max, const vector<vector<int>> &requisito, const vector<vector<int>> &post_requisito, const vector<int> &flip, float creditos_promedio, int cursos_total, int p, int a, int b, int c, int d, bool inteligente = true){
    for(int ii = 0; ii < cursos_total; ii++){
        int i = flip[ii], this_semestre = x[i];
        //Evitar quitar una asignatura si el semestre ya tiene el mínimo de ramos
        if (cursos_semestre[this_semestre].size() <= c)
            continue;
        //Evitar quitar una asignatura si el semestre ya tiene el mínimo de créditos
        int this_creditos = creditos[i];
        if (creditos_semestre[this_semestre] - this_creditos < a)
            continue;
        //Cambio de la función de evaluación
        float _creditos, cambio_prev, cambio = -1.0;
        if (inteligente){
            _creditos = -creditos_promedio;
            for(int j = cursos_semestre[this_semestre].size() - 1; j >= 0; j--)
                if (cursos_semestre[this_semestre][j] != i)
                    _creditos += creditos[cursos_semestre[this_semestre][j]];
            cambio_prev = _creditos * _creditos - (_creditos + this_creditos) * (_creditos + this_creditos);
        }
        for(int j = curso_semestre_min[i]; j <= curso_semestre_max[i]; j++){
            //Evitar el movimiento nulo
            if (j == this_semestre)
                continue;
            //Evitar agregar una asignatura si el semestre ya tiene el máximo de ramos
            if (cursos_semestre[j].size() >= d)
                continue;
            //Evitar agregar una asignatura si el semestre ya tiene el máximo de créditos
            if (creditos_semestre[j] + this_creditos > b)
                continue;
            //Revisar pre_requisitos
            bool flag2 = true;
            for(int k = requisito[i].size() - 1; k >= 0; k--)
                if (x[requisito[i][k]] >= j){
                    flag2 = false;
                    break;
                }
            if (not flag2)
                continue;
            //Revisar post_requisito
            bool flag_2 = false;
            for(int k = post_requisito[i].size() - 1; k >= 0; k--)
                if (x[post_requisito[i][k]] <= j){
                    flag_2 = true;
                    break;
                }
            if (flag_2)
                continue;
            //Calcular el cambio de evaluación por agregar este ramo a su semestre
            if (inteligente){
                _creditos = -creditos_promedio;
                for(int k = cursos_semestre[j].size() - 1; k >= 0; k--)
                    _creditos += creditos[cursos_semestre[j][k]];
                cambio = cambio_prev + (_creditos + this_creditos) * (_creditos + this_creditos) - _creditos * _creditos;
            }
            //APLICAR CAMBIO
            if (cambio < 0){
                x[i] = j;
                for(int k = cursos_semestre[this_semestre].size() - 1; k >= 0; k--)
                    if (cursos_semestre[this_semestre][k] == i){
                        cursos_semestre[this_semestre].erase(cursos_semestre[this_semestre].begin() + k);
                        break;
                    }
                cursos_semestre[j].push_back(i);
                creditos_semestre[this_semestre] -= this_creditos;
                creditos_semestre[j] += this_creditos;
                return false;
            }
        }
    }
    return true;
}
/**
 * @brief Escoge aleatoriamente un índice de un arreglo en base a su peso
 * @param peso Arreglo de pesos
 * @param valor_total suma de todos los pesos pre-calculados
 */
int weigthed_choose(const vector<float> &peso, int valor_total){
    float a = rand() % valor_total;
    for(int b = peso.size() - 1; b >= 0; b--){
        a -= peso[b];
        if (a <= 0)
            return b;
    }
    return -1;
}
/**
 * @brief Genera una instancia inicial con algoritmo greedy
 * @param x semestre asignado a cada ramo
 * @param creditos_semestre creditos de cada semestre
 * @param cursos_semestre arreglo de cursos de cada semestre
 * @param curso_semestre_min semestre mínimo de cada curso
 * @param curso_semestre_max semestre máximo de cada curso
 * @param creditos creditos de cada curso
 */
 void greedy(vector<int> &x, vector<int> &creditos_semestre, vector<vector<int>> &cursos_semestre, const vector<int> &curso_semestre_min, const vector<int> &curso_semestre_max, const vector<int> &creditos, vector<vector<int>> &requisito, float creditos_promedio, int cursos_promedio, int cursos_total, int b, int d){
    for(int i = 0; i < cursos_total; i++){
        bool flag = true;
        for(int j = curso_semestre_min[i]; j <= curso_semestre_max[i]; j++){
            bool flag2 = true;
            for(int k = requisito[i].size() - 1; k >= 0; k--)
                if (x[requisito[i][k]] >= j){
                    flag2 = false;
                    break;
                }
            if (flag2 && creditos_semestre[j] + creditos[i] <= creditos_promedio && cursos_semestre[j].size() < cursos_promedio){
                x[i] = j;
                creditos_semestre[j] += creditos[i];
                cursos_semestre[j].push_back(i);
                flag = false;
                break;
            }
        }
        if (flag){
            if (cursos_promedio < d){
                cursos_promedio++;
                flag = false;
            }
            if (creditos_promedio < b){
                creditos_promedio++;
                flag = false;
            }
            if (flag){
                cout << "No se pudo construir una solución inicial" << endl;
                return;
            }
        }
    }
 }
int main(){
    /*---------------------------------------------------------LEER ARCHIVO------------------------------------------------------
        int p = Número de semestres
        int a = Créditos mínimos por semestre
        int b = Créditos máximos por semestre
        int c = Número de asignaturas mínimo por semestre
        int d = Número de asignaturas máximo por semestre
        vector<string> cursos = Nombre de la asignatura
        vector<int> creditos = Créditos de la asignatura
        vector<vector<int>> requisito = Arreglo de los requisitos de una asignatura
    */
    ofstream outFile("output.txt", ios::app);
    if (!outFile.is_open()) {
        cerr << "Error opening file!" << endl;
        return 1;
    }
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
    bool MODELO_HC = true;
    bool MODELO_AE = true;
    bool SHOW_MALLA = true;
    for(int i_a = archivos.size() - 1; i_a >= 0; i_a--){
        string filename = archivos[i_a];
        ifstream file(filename);
        if (!file.is_open()){
            cerr << "Archivo no encontrado.\n";
            return 1;
        }
        int p, a, b, c, d;
        string nn;
        //Parámetros
        file >> nn >> nn >> nn;
        nn.pop_back();
        p = stoi(nn);
        file >> nn >> nn >> nn;
        nn.pop_back();
        a = stoi(nn);
        file >> nn >> nn >> nn;
        nn.pop_back();
        b = stoi(nn);
        file >> nn >> nn >> nn;
        nn.pop_back();
        c = stoi(nn);
        file >> nn >> nn >> nn;
        nn.pop_back();
        d = stoi(nn);
        //Cursos
        vector<string> cursos;
        file >> nn >> nn >> nn;
        while (file >> nn && nn != "};"){
            if (nn.back() == ',')
                nn.pop_back();
            cursos.push_back(nn);
        }
        int cursos_total = cursos.size();
        //Créditos
        vector<int> creditos;
        float creditos_total = 0.0;
        file >> nn >> nn >> nn;
        while (file >> nn && nn != "];"){
            if (nn.back() == ',')
                nn.pop_back();
            int _creditos = stoi(nn);
            creditos_total += _creditos;
            creditos.push_back(_creditos);
        }
        string curso;
        //Pre-requisitos
        vector<vector<int>> requisito(cursos_total);
        vector<vector<int>> post_requisito(cursos_total);
        file >> nn >> nn >> nn;
        while (file >> curso && curso != "};"){
            curso = curso.substr(1, curso.size() - 2);
            for(int i = 0; i < cursos_total; i++)
                if (cursos[i] == curso){
                    file >> nn;
                    nn = nn.substr(0, nn.size() - 2);
                    for(int j = 0; j < cursos_total; j++)
                        if (cursos[j] == nn){
                            requisito[i].push_back(j);
                            post_requisito[j].push_back(i);
                            break;
                        }
                    break;
                }
        }
        file.close();
        vector<int> curso_semestre_min(cursos_total, 0);
        vector<int> curso_semestre_max(cursos_total, p - 1);
        for(int i = 0; i < cursos_total; i++){
            for(int j = requisito[i].size() - 1; j >= 0; j--)
                if (curso_semestre_min[i] <= curso_semestre_min[requisito[i][j]])
                    curso_semestre_min[i] = curso_semestre_min[requisito[i][j]] + 1;
            for(int j = post_requisito[i].size() - 1; j >= 0; j--)
                if (curso_semestre_max[i] >= curso_semestre_max[post_requisito[i][j]])
                    curso_semestre_max[i] = curso_semestre_max[post_requisito[i][j]] - 1;
        }
        /*---------------------------------------------------------Solución Inicial-----------------------------------------------------
        Representación x[i] = Semestre en que se dicta la asignatura i
        Greedy:
            Se calcula la cantida promedio de cursos y créditos que debería tener cada semestre.
            Cada ramo se intenta asignar a cada semestre considerando como límites los cursos y créditos PROMEDIO.
            Si no se pudo asignar un curso y los cursos o créditos promedio son menores a los límites rígidos, estos se incrementan.
            Si los cursos o créditos promedio llegan al límite rígido, el Greedy falla en encontrar una solición inicial válida.
        */
        int cursos_promedio = cursos_total / p;
        float creditos_promedio = creditos_total / p;
        //---------------------------------------------------------Algoritmo Hill CLimbing----------------------------------------------------
        if (MODELO_HC){
            vector<int> x(cursos_total);
            vector<int> creditos_semestre(p, 0);
            vector<vector<int>> cursos_semestre(p);
            greedy(x, creditos_semestre, cursos_semestre, curso_semestre_min, curso_semestre_max, creditos, requisito, creditos_promedio, cursos_promedio, cursos_total, b, d);
            int MAX_ITER = 100, counter = 0;
            bool OUTPUT = false;
            vector<int> flip(cursos_total);
            for(int i = 0; i < cursos_total; i++)
                flip[i] = i;
            auto start = chrono::steady_clock::now();//Iniciar timer
            for(int iter = 0; iter < MAX_ITER; iter++){
                if (movimiento(x, creditos_semestre, cursos_semestre, creditos, curso_semestre_min, curso_semestre_max, requisito, post_requisito, flip, creditos_promedio, cursos_total, p, a, b, c, d))
                    break;
                else
                    counter++;
            }
            auto end = chrono::steady_clock::now();//Terminar timer
            //---------------------------------------------------------Resultados-------------------------------------------------------------
            ofstream outFile("output.txt", ios::app);
            if (!outFile.is_open()) {
                cerr << "Error opening file!" << endl;
                return 1;
            }
            outFile << filename << " - HILL CLIMBING\n"
                << "Número de movimientos: " << counter << "\n"
                << "Tiempo de ejecución: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms\n"
                << "MSE: " << evaluar(creditos_semestre, p, creditos_promedio) << "\n"
                << "Es válido? " << (validar(cursos_semestre, creditos_semestre, x, requisito, p, a, b, c, d) ? "No" : "Sí") << "\n\n";
            if (SHOW_MALLA)
                outFile << print_malla(cursos_semestre, cursos, creditos) << "\n";
            outFile.close();
        }
        //---------------------------------------------------------Algoritmo Evolutivo--------------------------------------------------------
        if (MODELO_AE){
            int AE_instancias = 100;
            int AE_random_steps = 20;
            int MAX_ITER = 1000;
            float MIN_FIT = 1.0;
            auto start = chrono::steady_clock::now(); //Iniciar timer
            vector<int> x(cursos_total);
            vector<int> creditos_semestre(p, 0);
            vector<vector<int>> cursos_semestre(p);
            greedy(x, creditos_semestre, cursos_semestre, curso_semestre_min, curso_semestre_max, creditos, requisito, creditos_promedio, cursos_promedio, cursos_total, b, d);
            srand(time(0));
            vector<vector<int>> x_inst(AE_instancias);
            vector<vector<int>> creditos_semestre_inst(AE_instancias);
            vector<vector<vector<int>>> cursos_semestre_inst(AE_instancias);
            vector<int> flip(cursos_total);
            for(int i = 0; i < cursos_total; i++)
                flip[i] = i;
            random_device rd;
            for(int inst = 0; inst < AE_instancias; inst++){
                x_inst[inst] = x;
                creditos_semestre_inst[inst] = creditos_semestre;
                cursos_semestre_inst[inst] = cursos_semestre;
                for(int i = 0; i < AE_random_steps; i++){
                    mt19937 gen(rd());
                    shuffle(flip.begin(), flip.end(), rd);
                    movimiento(x_inst[inst], creditos_semestre_inst[inst], cursos_semestre_inst[inst], creditos, curso_semestre_min, curso_semestre_max, requisito, post_requisito, flip, creditos_promedio, cursos_total, p, a, b, c, d, false);
                }
            }
            auto end = chrono::steady_clock::now(); //Terminar timer
            cout << "Tiempo de generación de " << AE_instancias << " instancias: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms\n";
            start = chrono::steady_clock::now(); //Iniciar timer
            float best_fit = 999999.9;
            for(int iter = 0; iter < MAX_ITER; iter++){
                bool stable = true;
                bool finish = false;
                float valor_total = 0;
                vector<float> valores(AE_instancias, 0);
                //Evaluar instancias
                for(int inst = 0; inst < AE_instancias; inst++){
                    float a = evaluar(creditos_semestre_inst[inst], p, creditos_promedio);
                    if (a < best_fit){
                        best_fit = a;
                        cout << iter << ": " << best_fit << endl;
                        if (a < MIN_FIT || isnan(a)){
                            finish = true;
                            cout << "Mejor modelo encontrado\n";
                            break;
                        }
                    }
                    a = a > 999 ? 0 : 999 - a;
                    valores[inst] = a;
                    valor_total += a;
                }
                if (finish)
                    break;
                //Elegir la siguiente generación
                for(int inst = 0; inst < AE_instancias; inst++){
                    int a = weigthed_choose(valores, valor_total);
                    x_inst[inst] = x_inst[a];
                    creditos_semestre_inst[inst] = creditos_semestre_inst[a];
                    cursos_semestre_inst[inst] = cursos_semestre_inst[a];
                }
                //Movimientos
                for(int inst = 0; inst < AE_instancias; inst++){
                    mt19937 gen(rd());
                    shuffle(flip.begin(), flip.end(), rd);
                    bool inteligente = (rand() % MAX_ITER < iter);
                    if (not movimiento(x_inst[inst], creditos_semestre_inst[inst], cursos_semestre_inst[inst], creditos, curso_semestre_min, curso_semestre_max, requisito, post_requisito, flip, creditos_promedio, cursos_total, p, a, b, c, d, inteligente))
                        stable = false;
                }
                if (stable)
                    break;
            }
            float min_valor = 9999999;
            int min_model = 0;
            for(int inst = 0; inst < AE_instancias; inst++){
                float a = evaluar(creditos_semestre_inst[inst], p, creditos_promedio);
                if (a < min_valor){
                    min_valor = a;
                    min_model = inst;
                }
            }
            end = chrono::steady_clock::now(); //Terminar timer
            //---------------------------------------------------------Resultados--------------------------------------------------------------------
            cout << "Tiempo de ejecución de " << AE_instancias << " instancias: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms\n";
            ofstream outFile("output.txt", ios::app);
            if (!outFile.is_open()) {
                cerr << "Error opening file!" << endl;
                return 1;
            }
            outFile << filename << " - Algoritmo Evolutivo\n"
                << "Tiempo de evolución de " << AE_instancias << " instancias con " << MAX_ITER << " generaciones: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms\n"
                << "Valor mínimo: " << min_valor << "\n"
                << "Es válido? " << (validar(cursos_semestre_inst[min_model], creditos_semestre_inst[min_model], x_inst[min_model], requisito, p, a, b, c, d) ? "No" : "Sí") << "\n\n";
            if (SHOW_MALLA)
                outFile << print_malla(cursos_semestre_inst[min_model], cursos, creditos) << "\n";
            outFile.close();
            x_inst.clear();
            cursos_semestre_inst.clear();
            creditos_semestre_inst.clear();
        }
    }
    return 0;
}