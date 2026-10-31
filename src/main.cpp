#include "arbol_semantico.hpp"

#include <SFML/Graphics.hpp>
#include <optional>
#include <sstream>

bool cargarFuenteDelSistema(sf::Font& fuente) {
    const char* rutas[] = {
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\calibri.ttf"
    };
    for (size_t i = 0; i < 3; ++i)
        if (fuente.openFromFile(rutas[i])) return true;
    return false;
}

void texto(sf::RenderWindow& ventana, sf::Font& fuente, const std::string& valor,
           unsigned int tamano, float x, float y, sf::Color color) {
    sf::Text dibujo(fuente, valor, tamano);
    dibujo.setPosition({x, y});
    dibujo.setFillColor(color);
    ventana.draw(dibujo);
}

void caja(sf::RenderWindow& ventana, float x, float y, float ancho, float alto,
          sf::Color color, sf::Color borde = sf::Color::Transparent) {
    sf::RectangleShape figura({ancho, alto});
    figura.setPosition({x, y});
    figura.setFillColor(color);
    if (borde != sf::Color::Transparent) {
        figura.setOutlineThickness(2);
        figura.setOutlineColor(borde);
    }
    ventana.draw(figura);
}

void linea(sf::RenderWindow& ventana, float x1, float y1, float x2, float y2,
           sf::Color color) {
    sf::VertexArray segmento(sf::PrimitiveType::Lines, 2);
    segmento[0].position = {x1, y1};
    segmento[1].position = {x2, y2};
    segmento[0].color = color;
    segmento[1].color = color;
    ventana.draw(segmento);
}

void dibujarEntrada(sf::RenderWindow& ventana, sf::Font& fuente,
                    const std::string& entrada) {
    std::vector<std::string> lineas;
    std::stringstream flujo(entrada);
    std::string lineaActual;
    while (std::getline(flujo, lineaActual)) lineas.push_back(lineaActual);
    if (lineas.empty()) lineas.push_back("");

    size_t primera = lineas.size() > 7 ? lineas.size() - 7 : 0;
    float posicionY = 195;
    for (size_t i = primera; i < lineas.size(); ++i) {
        std::string linea = lineas[i];
        if (linea.size() > 32) linea = linea.substr(0, 29) + "...";
        texto(ventana, fuente, linea, 14, 52, posicionY, sf::Color(235, 238, 245));
        posicionY += 24;
    }

    texto(ventana, fuente, "_", 14, 52, posicionY, sf::Color(120, 205, 255));
}

int main() {
    sf::RenderWindow ventana(sf::VideoMode({1200, 760}), "Arbol semantico");
    sf::Font fuente;
    if (!cargarFuenteDelSistema(fuente)) return 1;

    std::string entrada =
        "Si es de dia, entonces voy a trabajar\n"
        "Si llueve, entonces no voy a trabajar\n"
        "Es de dia";
    Diccionario diccionario = crearDiccionario();
    BaseConocimiento base;
    std::vector<std::string> lineas;
    std::stringstream inicial(entrada);
    std::string lineaActual;
    while (std::getline(inicial, lineaActual)) lineas.push_back(lineaActual);
    base = construirBase(lineas, diccionario);

    while (ventana.isOpen()) {
        while (const std::optional evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) ventana.close();
            if (const auto* escrito = evento->getIf<sf::Event::TextEntered>()) {
                if (escrito->unicode >= 32 && escrito->unicode < 127) {
                    entrada += static_cast<char>(escrito->unicode);
                }
            }
            if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
                if (tecla->scancode == sf::Keyboard::Scancode::Enter)
                    entrada += "\n";
                if (tecla->scancode == sf::Keyboard::Scancode::Backspace &&
                    !entrada.empty())
                    entrada.erase(entrada.size() - 1);
                if (tecla->scancode == sf::Keyboard::Scancode::F2)
                    entrada.clear();
                if (tecla->scancode == sf::Keyboard::Scancode::F5) {
                    std::vector<std::string> nuevasLineas;
                    std::stringstream textoNuevo(entrada);
                    while (std::getline(textoNuevo, lineaActual))
                        nuevasLineas.push_back(lineaActual);
                    destruirBase(base);
                    base = construirBase(nuevasLineas, diccionario);
                }
            }
        }

        ventana.clear(sf::Color(22, 27, 38));

        caja(ventana, 0, 0, 1200, 70, sf::Color(35, 44, 61));
        texto(ventana, fuente, "ARBOL SEMANTICO", 25, 28, 16, sf::Color(240, 244, 250));
        texto(ventana, fuente, "F2 limpiar | F5 analizar | ENTER nueva linea", 15,
              350, 25, sf::Color(174, 190, 210));

        caja(ventana, 25, 95, 330, 635, sf::Color(30, 38, 53), sf::Color(67, 87, 116));
        texto(ventana, fuente, "ENTRADA", 19, 45, 115, sf::Color(120, 205, 255));
        texto(ventana, fuente, "Una regla por linea", 14, 45, 145, sf::Color(175, 185, 200));
        caja(ventana, 42, 180, 296, 190, sf::Color(18, 24, 35));
        dibujarEntrada(ventana, fuente, entrada);
        texto(ventana, fuente, "Conectores:", 15, 45, 405, sf::Color(175, 185, 200));
        texto(ventana, fuente, "y   o   no", 15, 45, 435, sf::Color(240, 194, 110));
        texto(ventana, fuente, "si ... entonces", 15, 45, 465, sf::Color(240, 194, 110));
        texto(ventana, fuente, "si y solo si", 15, 45, 495, sf::Color(240, 194, 110));
        caja(ventana, 42, 555, 296, 75,
             base.mensaje.find("satisfacible") != std::string::npos
                 ? sf::Color(31, 91, 75) : sf::Color(112, 52, 57));
        texto(ventana, fuente, "RESULTADO", 14, 57, 570, sf::Color(220, 230, 240));
        texto(ventana, fuente,
              base.mensaje.find("satisfacible") != std::string::npos
                  ? "Hay un modelo posible" : "Hay contradiccion",
              16, 57, 595, sf::Color::White);

        caja(ventana, 380, 95, 795, 125, sf::Color(30, 38, 53), sf::Color(67, 87, 116));
        texto(ventana, fuente, "ATOMOS", 18, 405, 115, sf::Color(120, 205, 255));
        float atomX = 405;
        for (size_t i = 0; i < base.atomos.size(); ++i) {
            std::string nombre(1, static_cast<char>('p' + i));
            caja(ventana, atomX, 155, 220, 42, sf::Color(43, 55, 75));
            texto(ventana, fuente, nombre + "  =  " + base.atomos[i], 14,
                  atomX + 12, 167, sf::Color(240, 244, 250));
            atomX += 235;
            if (atomX > 1080) break;
        }

        caja(ventana, 380, 245, 795, 485, sf::Color(30, 38, 53), sf::Color(67, 87, 116));
        texto(ventana, fuente, "ARBOL DE RAMAS", 18, 405, 265, sf::Color(120, 205, 255));
        float raizX = 778;
        float raizY = 315;
        caja(ventana, raizX - 70, raizY - 22, 140, 45, sf::Color(65, 77, 101));
        texto(ventana, fuente, "REGLAS", 15, raizX - 36, raizY - 8, sf::Color::White);

        float inicioY = 390;
        float separacion = base.ramas.size() > 5 ? 62.0f : 78.0f;
        for (size_t i = 0; i < base.ramas.size(); ++i) {
            const Rama& rama = base.ramas[i];
            float y = inicioY + static_cast<float>(i) * separacion;
            sf::Color color = rama.cerrada ? sf::Color(218, 91, 91) : sf::Color(75, 201, 145);
            linea(ventana, raizX, raizY + 24, 560, y + 22, color);
            caja(ventana, 470, y, 570, 45,
                 rama.cerrada ? sf::Color(75, 42, 54) : sf::Color(36, 76, 67), color);
            sf::CircleShape indicador(10);
            indicador.setPosition({490, y + 12});
            indicador.setFillColor(color);
            ventana.draw(indicador);
            texto(ventana, fuente, "Rama " + std::to_string(i + 1), 14,
                  515, y + 7, sf::Color(230, 235, 242));
            texto(ventana, fuente, ramaComoTexto(rama), 13,
                  515, y + 25, sf::Color(185, 198, 211));
        }

        ventana.display();
    }
    destruirBase(base);
    return 0;
}
