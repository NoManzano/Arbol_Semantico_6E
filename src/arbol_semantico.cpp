#include "arbol_semantico.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace {

std::string minusculas(const std::string& valor) {
    std::string resultado = valor;
    for (size_t i = 0; i < resultado.size(); ++i)
        resultado[i] = static_cast<char>(
            std::tolower(static_cast<unsigned char>(resultado[i])));
    return resultado;
}

std::string quitarEspacios(const std::string& valor) {
    size_t inicio = valor.find_first_not_of(" \t\r\n");
    if (inicio == std::string::npos) return "";
    size_t final = valor.find_last_not_of(" \t\r\n.,;");
    return valor.substr(inicio, final - inicio + 1);
}

bool comienzaCon(const std::string& valor, const std::string& inicio) {
    return valor.size() >= inicio.size() &&
           valor.compare(0, inicio.size(), inicio) == 0;
}

std::string reemplazarTodas(std::string texto, const std::string& viejo,
                            const std::string& nuevo) {
    size_t posicion = texto.find(viejo);
    while (posicion != std::string::npos) {
        texto.replace(posicion, viejo.size(), nuevo);
        posicion = texto.find(viejo, posicion + nuevo.size());
    }
    return texto;
}

std::string aplicarDiccionario(const std::string& original,
                               const Diccionario& diccionario) {
    std::string texto = minusculas(original);
    for (std::map<std::string, std::string>::const_iterator palabra =
             diccionario.palabras.begin();
         palabra != diccionario.palabras.end(); ++palabra) {
        texto = reemplazarTodas(texto, palabra->first, palabra->second);
    }
    return texto;
}

Formula* crearAtomo(const std::string& nombre) {
    return new Formula(TipoFormula::Atomo, quitarEspacios(nombre));
}

Formula* analizarTexto(const std::string& original, std::vector<std::string>& atomos,
                       const Diccionario& diccionario) {
    std::string texto = quitarEspacios(aplicarDiccionario(original, diccionario));
    if (texto.empty()) return crearAtomo("proposicion vacia");

    if (comienzaCon(texto, "si ") && texto.find(" entonces ") != std::string::npos) {
        size_t posicion = texto.find(" entonces ");
        Formula* primera = analizarTexto(texto.substr(3, posicion - 3), atomos, diccionario);
        Formula* segunda = analizarTexto(texto.substr(posicion + 10), atomos, diccionario);
        return new Formula(TipoFormula::Implicacion, "", primera, segunda);
    }

    size_t posicion = texto.find(" si y solo si ");
    if (posicion != std::string::npos) {
        Formula* primera = analizarTexto(texto.substr(0, posicion), atomos, diccionario);
        Formula* segunda = analizarTexto(texto.substr(posicion + 14), atomos, diccionario);
        return new Formula(TipoFormula::Equivalencia, "", primera, segunda);
    }

    posicion = texto.find(" o ");
    if (posicion != std::string::npos) {
        Formula* primera = analizarTexto(texto.substr(0, posicion), atomos, diccionario);
        Formula* segunda = analizarTexto(texto.substr(posicion + 3), atomos, diccionario);
        return new Formula(TipoFormula::Disyuncion, "", primera, segunda);
    }

    posicion = texto.find(" y ");
    if (posicion != std::string::npos) {
        Formula* primera = analizarTexto(texto.substr(0, posicion), atomos, diccionario);
        Formula* segunda = analizarTexto(texto.substr(posicion + 3), atomos, diccionario);
        return new Formula(TipoFormula::Conjuncion, "", primera, segunda);
    }

    if (comienzaCon(texto, "no "))
        return new Formula(TipoFormula::Negacion, "",
                           analizarTexto(texto.substr(3), atomos, diccionario));

    std::string atomo = quitarEspacios(texto);
    if (std::find(atomos.begin(), atomos.end(), atomo) == atomos.end())
        atomos.push_back(atomo);
    return crearAtomo(atomo);
}

void cambiarNombres(Formula* formula, const std::map<std::string, std::string>& nombres) {
    if (!formula) return;
    if (formula->tipo == TipoFormula::Atomo) {
        formula->texto = nombres.find(formula->texto)->second;
        return;
    }
    cambiarNombres(formula->izquierda, nombres);
    cambiarNombres(formula->derecha, nombres);
}

bool sonOpuestos(const Formula* primera, const Formula* segunda) {
    return (primera->tipo == TipoFormula::Atomo &&
            segunda->tipo == TipoFormula::Negacion &&
            segunda->izquierda->tipo == TipoFormula::Atomo &&
            primera->texto == segunda->izquierda->texto) ||
           (segunda->tipo == TipoFormula::Atomo &&
            primera->tipo == TipoFormula::Negacion &&
            primera->izquierda->tipo == TipoFormula::Atomo &&
            segunda->texto == primera->izquierda->texto);
}

void cerrarRama(Rama& rama) {
    for (size_t i = 0; i < rama.formulas.size(); ++i) {
        for (size_t j = i + 1; j < rama.formulas.size(); ++j) {
            if (sonOpuestos(rama.formulas[i], rama.formulas[j])) {
                rama.cerrada = true;
                rama.razon = "contiene un atomo y su negacion";
                return;
            }
        }
    }
}

bool expandirUna(std::vector<Rama>& ramas) {
    for (size_t numeroRama = 0; numeroRama < ramas.size(); ++numeroRama) {
        if (ramas[numeroRama].cerrada) continue;
        for (size_t numeroFormula = 0;
             numeroFormula < ramas[numeroRama].formulas.size(); ++numeroFormula) {
            Formula* formula = ramas[numeroRama].formulas[numeroFormula];
            if (formula->tipo == TipoFormula::Atomo ||
                (formula->tipo == TipoFormula::Negacion &&
                 formula->izquierda->tipo == TipoFormula::Atomo))
                continue;

            Rama original = ramas[numeroRama];
            original.formulas.erase(original.formulas.begin() + numeroFormula);
            if (formula->tipo == TipoFormula::Conjuncion) {
                original.formulas.push_back(formula->izquierda);
                original.formulas.push_back(formula->derecha);
                ramas[numeroRama] = original;
                cerrarRama(ramas[numeroRama]);
            } else if (formula->tipo == TipoFormula::Negacion &&
                       formula->izquierda->tipo == TipoFormula::Negacion) {
                original.formulas.push_back(formula->izquierda->izquierda);
                ramas[numeroRama] = original;
                cerrarRama(ramas[numeroRama]);
            } else if (formula->tipo == TipoFormula::Negacion &&
                       (formula->izquierda->tipo == TipoFormula::Disyuncion ||
                        formula->izquierda->tipo == TipoFormula::Implicacion)) {
                if (formula->izquierda->tipo == TipoFormula::Disyuncion) {
                    original.formulas.push_back(new Formula(
                        TipoFormula::Negacion, "", formula->izquierda->izquierda));
                    original.formulas.push_back(new Formula(
                        TipoFormula::Negacion, "", formula->izquierda->derecha));
                } else {
                    original.formulas.push_back(formula->izquierda->izquierda);
                    original.formulas.push_back(new Formula(
                        TipoFormula::Negacion, "", formula->izquierda->derecha));
                }
                ramas[numeroRama] = original;
                cerrarRama(ramas[numeroRama]);
            } else {
                Rama izquierda = original;
                Rama derecha = original;
                if (formula->tipo == TipoFormula::Disyuncion) {
                    izquierda.formulas.push_back(formula->izquierda);
                    derecha.formulas.push_back(formula->derecha);
                } else if (formula->tipo == TipoFormula::Implicacion) {
                    izquierda.formulas.push_back(new Formula(
                        TipoFormula::Negacion, "", formula->izquierda));
                    derecha.formulas.push_back(formula->derecha);
                } else if (formula->tipo == TipoFormula::Equivalencia) {
                    izquierda.formulas.push_back(formula->izquierda);
                    izquierda.formulas.push_back(formula->derecha);
                    derecha.formulas.push_back(new Formula(
                        TipoFormula::Negacion, "", formula->izquierda));
                    derecha.formulas.push_back(new Formula(
                        TipoFormula::Negacion, "", formula->derecha));
                } else if (formula->tipo == TipoFormula::Negacion &&
                           formula->izquierda->tipo == TipoFormula::Conjuncion) {
                    izquierda.formulas.push_back(new Formula(
                        TipoFormula::Negacion, "", formula->izquierda->izquierda));
                    derecha.formulas.push_back(new Formula(
                        TipoFormula::Negacion, "", formula->izquierda->derecha));
                } else if (formula->tipo == TipoFormula::Negacion &&
                           formula->izquierda->tipo == TipoFormula::Equivalencia) {
                    izquierda.formulas.push_back(formula->izquierda->izquierda);
                    izquierda.formulas.push_back(new Formula(
                        TipoFormula::Negacion, "", formula->izquierda->derecha));
                    derecha.formulas.push_back(new Formula(
                        TipoFormula::Negacion, "", formula->izquierda->izquierda));
                    derecha.formulas.push_back(formula->izquierda->derecha);
                }
                ramas[numeroRama] = izquierda;
                ramas.insert(ramas.begin() + numeroRama + 1, derecha);
                cerrarRama(ramas[numeroRama]);
                cerrarRama(ramas[numeroRama + 1]);
            }
            return true;
        }
    }
    return false;
}

} // namespace

Diccionario crearDiccionario() {
    Diccionario diccionario;
    agregarPalabra(diccionario, "tambien", " y ");
    agregarPalabra(diccionario, "ademas", " y ");
    agregarPalabra(diccionario, "además", " y ");
    agregarPalabra(diccionario, "pero", " y ");
    agregarPalabra(diccionario, "o bien", " o ");
    agregarPalabra(diccionario, "siempre que", "si ");
    agregarPalabra(diccionario, "es falso que", "no ");
    return diccionario;
}

void agregarPalabra(Diccionario& diccionario, const std::string& palabra,
                    const std::string& significado) {
    diccionario.palabras[palabra] = significado;
}

BaseConocimiento construirBase(const std::vector<std::string>& oraciones,
                               const Diccionario& diccionario) {
    BaseConocimiento base;
    base.diccionario = diccionario;
    for (size_t i = 0; i < oraciones.size(); ++i) {
        if (quitarEspacios(oraciones[i]).empty()) continue;
        Regla regla;
        regla.original = quitarEspacios(oraciones[i]);
        regla.formula = analizarTexto(regla.original, base.atomos, diccionario);
        base.reglas.push_back(regla);
    }

    std::map<std::string, std::string> nombres;
    for (size_t i = 0; i < base.atomos.size(); ++i)
        nombres[base.atomos[i]] = std::string(1, static_cast<char>('p' + i));
    for (size_t i = 0; i < base.reglas.size(); ++i)
        cambiarNombres(base.reglas[i].formula, nombres);

    Rama inicial;
    inicial.cerrada = false;
    for (size_t i = 0; i < base.reglas.size(); ++i)
        inicial.formulas.push_back(base.reglas[i].formula);
    base.ramas.push_back(inicial);
    cerrarRama(base.ramas[0]);
    while (expandirUna(base.ramas)) {}

    bool hayRamaAbierta = false;
    for (size_t i = 0; i < base.ramas.size(); ++i)
        if (!base.ramas[i].cerrada) hayRamaAbierta = true;
    base.mensaje = hayRamaAbierta
        ? "La base de conocimiento es satisfacible."
        : "La base de conocimiento es inconsistente.";
    return base;
}

std::string formulaComoTexto(const Formula* formula) {
    if (!formula) return "";
    if (formula->tipo == TipoFormula::Atomo) return formula->texto;
    if (formula->tipo == TipoFormula::Negacion)
        return "no " + formulaComoTexto(formula->izquierda);
    std::string conector = " ? ";
    if (formula->tipo == TipoFormula::Conjuncion) conector = " y ";
    if (formula->tipo == TipoFormula::Disyuncion) conector = " o ";
    if (formula->tipo == TipoFormula::Implicacion) conector = " -> ";
    if (formula->tipo == TipoFormula::Equivalencia) conector = " <-> ";
    return "(" + formulaComoTexto(formula->izquierda) + conector +
           formulaComoTexto(formula->derecha) + ")";
}

std::string tablaAtomos(const BaseConocimiento& base) {
    std::ostringstream salida;
    salida << "ATOMOS\n";
    for (size_t i = 0; i < base.atomos.size(); ++i)
        salida << static_cast<char>('p' + i) << " = " << base.atomos[i] << "\n";
    return salida.str();
}

std::string ramaComoTexto(const Rama& rama) {
    std::ostringstream salida;
    salida << (rama.cerrada ? "[CERRADA X] " : "[ABIERTA] ");
    if (rama.cerrada) return salida.str() + rama.razon;
    for (size_t i = 0; i < rama.formulas.size(); ++i) {
        if (i > 0) salida << ", ";
        salida << formulaComoTexto(rama.formulas[i]);
    }
    return salida.str();
}

void destruirFormula(Formula* formula) {
    if (!formula) return;
    if (formula->tipo != TipoFormula::Atomo) {
        destruirFormula(formula->izquierda);
        if (formula->tipo != TipoFormula::Negacion)
            destruirFormula(formula->derecha);
    }
    delete formula;
}

void destruirBase(BaseConocimiento& base) {
    for (size_t i = 0; i < base.reglas.size(); ++i)
        destruirFormula(base.reglas[i].formula);
    base.reglas.clear();
}
