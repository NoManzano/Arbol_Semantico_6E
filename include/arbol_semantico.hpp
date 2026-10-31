#ifndef ARBOL_SEMANTICO_HPP
#define ARBOL_SEMANTICO_HPP

#include <map>
#include <string>
#include <vector>

enum class TipoFormula { Atomo, Negacion, Conjuncion, Disyuncion, Implicacion, Equivalencia };

struct Formula {
    TipoFormula tipo;
    std::string texto;
    Formula* izquierda;
    Formula* derecha;

    Formula(TipoFormula tipoFormula, const std::string& valor = "",
            Formula* primera = 0, Formula* segunda = 0)
        : tipo(tipoFormula), texto(valor), izquierda(primera), derecha(segunda) {}
};

struct Regla {
    std::string original;
    Formula* formula;
};

struct Rama {
    std::vector<Formula*> formulas;
    bool cerrada;
    std::string razon;
};

struct Diccionario {
    std::map<std::string, std::string> palabras;
};

struct BaseConocimiento {
    std::vector<Regla> reglas;
    std::vector<std::string> atomos;
    std::vector<Rama> ramas;
    Diccionario diccionario;
    std::string mensaje;
};

Diccionario crearDiccionario();
void agregarPalabra(Diccionario& diccionario, const std::string& palabra,
                    const std::string& significado);
BaseConocimiento construirBase(const std::vector<std::string>& oraciones,
                               const Diccionario& diccionario);
std::string formulaComoTexto(const Formula* formula);
std::string tablaAtomos(const BaseConocimiento& base);
std::string ramaComoTexto(const Rama& rama);
void destruirFormula(Formula* formula);
void destruirBase(BaseConocimiento& base);

#endif
