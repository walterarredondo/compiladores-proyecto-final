# Compilador FIS-25

Proyecto final de la materia Compiladores (ciclo 26-1) en la Facultad de Ciencias, UNAM. Implementa un compilador que traduce un lenguaje fuente a código intermedio para la máquina virtual FIS-25, siguiendo la especificación técnica y el simulador oficiales.

Autor: Walter Arredondo  
Video de explicación: https://youtu.be/3GdymznSaaU
Link al simulador:  https://amm-gdev.itch.io/fis-25

## Uso rápido

Requisitos: `gcc`, `make`, `flex`, `bison`.

- Compilar el compilador: `make all`
- Probar el ejemplo (Triángulo de Sierpinski): `make example` o `make test`
- Compilar un programa propio: `./build/compiler archivo_entrada.src archivo_salida.asm`
- Ejecutar el `.asm` generado en el simulador FIS-25.

