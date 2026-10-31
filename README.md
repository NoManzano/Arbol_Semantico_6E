# Arbol_Semantico

Práctica para la materia de Inteligencia Artificial de la carrera de
Ingeniería Mecatrónica, sexto semestre.

## Estructura del repositorio

```text
assets/    Recursos estáticos del proyecto
bin/       Artefactos generados por la compilación
docs/      Documentación
include/   Interfaces y archivos de cabecera
src/       Código fuente
```

## Compilación con MSYS2 MinGW64

Abre la terminal **MSYS2 MinGW64** e instala el compilador y SFML:

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-sfml
```

Después, desde la carpeta del proyecto:

```bash
make
./bin/arbol_semantico
```

En VS Code, el proyecto ya incluye la configuración de [`.vscode/settings.json`](.vscode/settings.json)
para abrir automáticamente una terminal MSYS2 UCRT64. También puedes usar
`Ctrl+Shift+B` para compilar o ejecutar la tarea **Ejecutar árbol semántico**
desde `Terminal > Ejecutar tarea`.

No necesitas descargar una fuente: la interfaz usa `Arial`, `Segoe UI` o
`Calibri`, que normalmente ya están instaladas en Windows. La interfaz acepta
una regla por línea; presiona `Enter` para agregar una línea y `F5` para
reconstruir el árbol. Usa `y`, `o`, `no`, `si ... entonces` y `si y solo si`.

El núcleo se encuentra en [arbol_semantico.hpp](include/arbol_semantico.hpp) y
[arbol_semantico.cpp](src/arbol_semantico.cpp), para que puedas estudiarlo sin
mezclarlo con la ventana gráfica.

## Diccionario

El diccionario inicial entiende estas palabras adicionales:

```text
tambien, además, ademas, pero -> y
o bien -> o
siempre que -> si
es falso que -> no
```

Puedes agregar más equivalencias en `crearDiccionario()` usando:

```cpp
agregarPalabra(diccionario, "junto con", " y ");
```

La primera palabra es la forma que escribirá el usuario y la segunda es el
conector que entiende el analizador.
