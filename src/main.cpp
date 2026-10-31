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

int main() {
    sf::RenderWindow window(sf::VideoMode({1100, 700}),
                            "Arbol semantico - logica proposicional");
    sf::Font font;
    if (!cargarFuenteDelSistema(font)) {
        return 1;
    }

    std::string input =
        "Si es de día, entonces voy a trabajar\n"
        "Si llueve, entonces no voy a trabajar\n"
        "Es de día";
    Diccionario diccionario = crearDiccionario();
    BaseConocimiento knowledge;
    std::vector<std::string> lines;
    std::stringstream initial(input);
    std::string line;
    while (std::getline(initial, line)) lines.push_back(line);
    knowledge = construirBase(lines, diccionario);

    sf::Text title(font, "Arbol semantico", 24);
    title.setPosition({20, 15});
    sf::Text instructions(font, "Una regla por linea. Enter agrega una linea; F5 recalcula.",
                          16);
    instructions.setPosition({20, 50});
    sf::Text content(font, "", 17);
    content.setPosition({20, 90});
    content.setFillColor(sf::Color::White);

    std::string visibleText = input;
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* textEntered = event->getIf<sf::Event::TextEntered>()) {
                if (textEntered->unicode == '\b' && !visibleText.empty()) {
                    visibleText.erase(visibleText.size() - 1);
                } else if (textEntered->unicode >= 32 &&
                           textEntered->unicode < 127) {
                    visibleText += static_cast<char>(textEntered->unicode);
                }
            }
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                    visibleText += "\n";
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::F5) {
                    std::vector<std::string> currentLines;
                    std::stringstream text(visibleText);
                    while (std::getline(text, line)) currentLines.push_back(line);
                    destruirBase(knowledge);
                    knowledge = construirBase(currentLines, diccionario);
                }
            }
        }

        std::ostringstream output;
        output << "REGLAS Y FORMULAS\n";
        for (size_t i = 0; i < knowledge.reglas.size(); ++i)
            output << "R" << i + 1 << ": "
                   << formulaComoTexto(knowledge.reglas[i].formula) << "\n";
        output << "\n" << tablaAtomos(knowledge) << "\nDICCIONARIO\n";
        for (std::map<std::string, std::string>::const_iterator palabra =
                 knowledge.diccionario.palabras.begin();
             palabra != knowledge.diccionario.palabras.end(); ++palabra)
            output << palabra->first << " -> " << palabra->second << "\n";
        output << "\nRAMAS\n";
        for (size_t i = 0; i < knowledge.ramas.size(); ++i)
            output << "Rama " << i + 1 << ": "
                   << ramaComoTexto(knowledge.ramas[i]) << "\n";
        output << "\n" << knowledge.mensaje << "\n\nENTRADA:\n" << visibleText;
        content.setString(output.str());

        window.clear(sf::Color(30, 35, 45));
        window.draw(title);
        window.draw(instructions);
        window.draw(content);
        window.display();
    }
    destruirBase(knowledge);
    return 0;
}
