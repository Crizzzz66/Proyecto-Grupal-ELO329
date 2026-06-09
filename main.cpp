#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
using namespace std;

/**
 * @brief Calcula la desviación cuadrada de los ramos.
 * @param x Cursos asignados a cada semestre.
 * @param creditos Créditos de cada curso.
 * @param p Número de semestres.
 * @param creditos_promedio Créditos promedio por semestre.
 * @return Error cuadrático total.
 */
float evaluar(const vector<vector<int>> &cursos_semestre, const vector<int> &creditos, int p, float creditos_promedio){
    float output = 0;
    for(int i = 0; i < p; i++){
        float _creditos = -creditos_promedio;
        for(int j = cursos_semestre[i].size() - 1; j >= 0; j--)
            _creditos += creditos[cursos_semestre[i][j]];
        output += _creditos * _creditos;
    }
    return output;
}

/**
 * @brief Verifica si una solución es factible
 * @param cursos_semestre Cursos asignados a cada semestre
 * @param creditos_semestre Créditos asignados a cada semestre
 * @param requisito Mapa de los requisitos de cada curso
 * @param x Semestre en que se asigna cada curso
 * @param a Créditos mínimos por semestre
 * @param b Créditos máximos por semestre
 * @param c Cursos mínimos por semestre
 * @param d Cursos máximos por semestre
 * @return true Si la malla es válida, false sino
 */
int validar(const vector<vector<int>> &cursos_semestre, const vector<int> &creditos_semestre, const vector<int> &requisito, const vector<int> &x, int a, int b, int c, int d){
    int p = cursos_semestre.size();
    for(int i = 0; i < p; i++){
        if (creditos_semestre[i] < a || creditos_semestre[i] > b){
            cout << "Validación fallida en semestre " << i + 1 << " por créditos" << endl;
            return i + 1;
        }
        int len = cursos_semestre[i].size();
        if (len < c || len > d){
            cout << "Validación fallida en semestre " << i + 1 << " por número de asignaturas" << endl;
            return p + i + 1;
        }
        for(int j = 0; j < len; j++)
            if (requisito[cursos_semestre[i][j]] != -1 && x[requisito[cursos_semestre[i][j]]] >= i){
                cout << "Validación fallida en semestre " << i + 1 << " ramo " << cursos_semestre[i][j] << " por requisitos" << endl;
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
void print_malla(const vector<vector<int>> &cursos_semestre, const vector<string> cursos, const vector<int> creditos, bool extensive){
    int imax = cursos_semestre.size();
    if (extensive){
        for(int i = 0; i < imax; i++){
            cout << "Semestre " << i + 1 << ":" << endl;
            int jmax = cursos_semestre[i].size(), _creditos = 0;
            for(int j = 0; j < jmax; j++){
                cout << "    " << cursos[cursos_semestre[i][j]] << " (" << creditos[cursos_semestre[i][j]] << ")" << endl;
                _creditos += creditos[cursos_semestre[i][j]];
            }
            cout << "Créditos: " << _creditos << endl;
            cout << "Asignaturas: " << jmax << endl << endl;
        }
    }
    else{
        for(int i = 0; i < imax; i++){
            int jmax = cursos_semestre[i].size();
            for(int j = 0; j < jmax; j++)
                cout << cursos[cursos_semestre[i][j]] << " ";
            cout << endl;
        }
    }
}

void print_requisitos(const vector<int> &requisito, const vector<string> &cursos){
    for(int i = requisito.size() - 1; i >= 0; i--)
        if (requisito[i] != -1)
            cout << "<" << cursos[i] << ", " << cursos[requisito[i]] << ">" << endl;
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
        vector<int> requisito = Índice del prerrequisito de la asignatura (-1 sin prerrequisito)
    */
    string filename = "bacp8.txt";
    //cin >> filename;
    ifstream file(filename);
    if (!file.is_open()){
        cerr << "Archivo no encontrado.\n";
        return 1;
    }
    int p, a, b, c, d;
    string nn;
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
    //cout << p << " " << a << " " << b << " " << c << " " << d << endl;
    vector<string> cursos;
    file >> nn >> nn >> nn;
    while (file >> nn && nn != "};"){
        if (nn.back() == ',')
            nn.pop_back();
        cursos.push_back(nn);
    }
    int cursos_total = cursos.size();
    //cout << "Cursos: " << cursos.size() << endl;
    vector<int> creditos;
    int creditos_total = 0;
    file >> nn >> nn >> nn;
    while (file >> nn && nn != "];"){
        if (nn.back() == ',')
            nn.pop_back();
        int _creditos = stoi(nn);
        creditos_total += _creditos;
        creditos.push_back(_creditos);
    }
    //cout << "Creditos: " << creditos.size() << endl;
    string curso;
    vector<int> requisito(cursos_total, -1);
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
                        requisito[i] = j;
                        post_requisito[j].push_back(i);
                        break;
                    }
                break;
            }
    }
    //cout << "Prerequisitos: " << requisito.size() << endl;
    file.close();
    
    //print_requisitos(requisito, cursos);
    
    /*---------------------------------------------------------Solución Inicial-----------------------------------------------------
        Representación x[i] = Semestre en que se dicta la asignatura i
        Greedy:
            Se calcula la cantida promedio de cursos y créditos que debería tener cada semestre.
            Cada ramo se intenta asignar a cada semestre considerando como límites los cursos y créditos PROMEDIO.
            Si no se pudo asignar un curso y los cursos o créditos promedio son menores a los límites rígidos, estos se incrementan.
            Si los cursos o créditos promedio llegan al límite rígido, el Greedy falla en encontrar una solición inicial válida.
        
        Además, se incluyen algunas variables redundantes para aumentar la eficiencia:
            int cursos_total = cursos.len()
            float creditos_promedio = SUM(creditos) / p
                -Util para la evaluación de las soluciones.
            vector<int> creditos_semestre = Total de créditos en cada semestre.
            vector<vector<int>> cursos_semestre = Vector de cursos tomados por cada semestre.
                -Util para la evaluación de las soluciones.
    */
    int cursos_promedio = cursos_total / p;
    float creditos_promedio = creditos_total / p;
    vector<int> x(cursos_total);
    vector<int> creditos_semestre(p, 0);
    vector<vector<int>> cursos_semestre(p);
    auto start = chrono::steady_clock::now();//Iniciar timer
    int jmin = 0;
    for(int i = 0; i < cursos_total; i++){
        bool flag = true;
        for(int j = jmin; j < p; j++){
            if ((requisito[i] == -1 || x[requisito[i]] < j) && creditos_semestre[j] + creditos[i] <= creditos_promedio && cursos_semestre[j].size() < cursos_promedio){
                x[i] = j;
                creditos_semestre[j] += creditos[i];
                cursos_semestre[j].push_back(i);
                flag = false;
                if (creditos_semestre[j] == b || cursos_semestre[j].size() == d)
                    jmin++;
                break;
            }
        }
        if (flag){
            if (cursos_promedio < d)
                cursos_promedio++;
            if (creditos_promedio < b)
                creditos_promedio++;
            else{
                cout << "No se pudo construir una solución inicial" << endl;
                return 1;
            }
        }
    }
    creditos_promedio = creditos_total / p;
    auto end = chrono::steady_clock::now();//Terminar timer
    cout << "Tiempo de generación Greedy: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms" << endl;
    cout << "MSE: " << evaluar(cursos_semestre, creditos, p, creditos_promedio) << endl;
    cout << "Es válido? " << (validar(cursos_semestre, creditos_semestre, requisito, x, a, b, c, d) ? "No" : "Sí") << endl;
    print_malla(cursos_semestre, cursos, creditos, false);
    /*---------------------------------------------------------Métodos de Reparación-------------------------------------------------------------
        Hill CLimbing:
            Movimiento: Mover el curso i al semestre j.
                    -Resultado optimizable
                Total de movimientos: i * j
                !!! Este movimiento no garantiza validez
                    -Revisión optimizable
            Usando Primera Mejora
            Limitando las iteraciones a 100
    */
    int MAX_ITER = 100;
    start = chrono::steady_clock::now();//Iniciar timer
    for(int iter = 0; iter < MAX_ITER; iter++){
        bool stable = true;
        bool flag = false;
        for(int i = 0; i < cursos_total; i++){
            flag = false;
            int this_semestre = x[i];
            if (cursos_semestre[this_semestre].size() <= c) //Evitar quitar una asignatura si el semestre ya tiene el mínimo de ramos
                continue;
            int this_creditos = creditos[i];
            if (creditos_semestre[this_semestre] - this_creditos < a) //Evitar quitar una asignatura si el semestre ya tiene el mínimo de créditos
                continue;
            //region Cambio de la función de evaluación
                float _creditos = -creditos_promedio;
                for(int j = cursos_semestre[this_semestre].size() - 1; j >= 0; j--)
                    if (cursos_semestre[this_semestre][j] != i)
                        _creditos += creditos[cursos_semestre[this_semestre][j]];
                float cambio_prev = _creditos * _creditos - (_creditos + this_creditos) * (_creditos + this_creditos);
            //endregion
            for(int j = 0; j < p; j++){
                if (j == this_semestre) //Evitar el movimiento nulo
                    continue;
                if (cursos_semestre[j].size() >= d) //Evitar agregar una asignatura si el semestre ya tiene el máximo de ramos
                    continue;
                if (creditos_semestre[j] + this_creditos > b) //Evitar agregar una asignatura si el semestre ya tiene el máximo de créditos
                    continue;
                if (requisito[i] != -1 && x[requisito[i]] >= j) //Revisar pre_requisitos
                    continue;
                //region Revisar post_requisito
                    bool flag_2 = false;
                    for(int k = post_requisito[i].size() - 1; k >= 0; k--)
                        if (x[post_requisito[i][k]] <= j){
                            flag_2 = true;
                            break;
                        }
                    if (flag_2)
                        continue;
                //endregion
                //region Calcular el cambio de evaluación por agregar este ramo a su semestre
                    _creditos = -creditos_promedio;
                    for(int k = cursos_semestre[j].size() - 1; k >= 0; k--)
                        _creditos += creditos[cursos_semestre[j][k]];
                    float cambio = cambio_prev + (_creditos + this_creditos) * (_creditos + this_creditos) - _creditos * _creditos;
                //endregion
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
                    flag = true;
                    //cout << "Cambiando " << cursos[i] << " del semestre " << this_semestre << " al " << j << endl;
                    break;
                }
            }
            if (flag)
                break;
        }
        if (stable)
            break;
    }
    end = chrono::steady_clock::now();//Terminar timer
    cout << "Tiempo de ejecución: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms" << endl;
    cout << "MSE: " << evaluar(cursos_semestre, creditos, p, creditos_promedio) << endl;
    cout << "Es válido? " << (validar(cursos_semestre, creditos_semestre, requisito, x, a, b, c, d) ? "No" : "Sí") << endl;
    print_malla(cursos_semestre, cursos, creditos, true);
    return 0;
}