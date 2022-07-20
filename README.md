# NANO-Frigo
Software de gestión de nevera portátil basada en módulos Peltier TEC1-12760 y Arduino UNO.
Incluye función de termostato pensado para refrigeradores Peltier, e incluye todo lo necesario para garantizar el correcto funcionamiento de la nevera.

## Componentes
Aunque pueda variar, los principales componentes son los siguientes:
```
- 1x Arduino UNO
- 2x Peltier TEC1-12760 (12V 6A 120W)
- 2x Heatsinks Hot Side
- 2x Heatsinks Cold Side
- 1x Fuente CA-CC 220V a 12V 15A
- 1x Fuente CC-CC 12V a 5V 10A
- 1x Relé 2-way con octoacoplador 5V (mín 10A CC)
- 1x Relé 1-way con octoacoplador 5V (mín 10A CC)
- 1x Pantalla LCD 2004
- 1x Botonera de matriz 1x3 (3 keys)
- 1x Sensor temperatura DHT11
- 1x Sensor temperatura DHT22
- 2x Ventilador 12V 200x115x8.5mm Peltier Hot Side
- 2x Ventilador 12V (pequeños) Peltier Cold Side (alimentado a 5V)
- 5x Ventilador 12V Intake y Outflow
- Fusibles (Entrada Arduino: 1A, Peltier 8A, Salida 12V 15A, Salida 5V 10A)
```

## Módulos TEC1-12760
La base del sistema son los dos módulos Peltier TEC1-12760, que serán los encargados de refrigerar la nevera.
Las placas Peltier siguen la siguiente curva:

<img width="739" alt="peltier-curve" src="https://user-images.githubusercontent.com/58596201/179938740-d124977e-db9e-4fd6-9673-f6d52e868615.png">

El comportamiento de estos módulos en tema de consumo va de forma lineal (la intensidad crece linealmente con el aumento de voltaje). Haciendo uso de este datos, el sistema utilizará **tres formas de alimentar las placas: a 12V, a 5V y a 0V (apagadas)**.

Tras probar los módulos a estos voltajes obtenemos los siguientes datos:
- Peltier a **12.8V**: 4.9A (temperatura ambiente)
- Peltier a **4.9V**: 1.5A (temperatura ambiente)

Un factor importante a tener en cuenta es la temperatura del lado caliente (Hot Side). Si el hot side alcanza una temperatura extrema, la temperatura conseguida en el lado frío (Cold Side) se verá impactada negativamente.

Las placas Peltier consiguen su mejor rendimiento cuando la temperatura del hot side es la mínima posible (la más parecida a temperatura ambiente), por ello intentaremos introducir la mayor cantidad de aire en el sistema para conseguir la mayor ventilación posible.

Sin embargo, la ventilación puede no ser suficiente si se lleva la nevera en un sitio cerrado como un maletero de coche, o si hace mucho calor y no se es capaz de disipar dicho calor. En este caso, se ha de implementar medidas que impidan el sobrecalentamiento de los componentes y de la propia nevera.

Para ello se introduce **tres modos de trabajo distintos**, mencionadas anteriormente (**12V, 5V y apagado**). Si la temperatura de la parte caliente donde se ubican los componentes supera un umbral definido en el programa (ver apartado *Seguridad*), se ciclarán entre los distintos modos para evitar mayor calentamiento (12V a 5V, y 5V a apagado si se alcanzan temperaturas extremas).

En distintos tiempos será más interesante utilizar modos de trabajo de menor rendimiento, ya que un modo de mayor rendimiento con temperaturas de trabajo más altas puede llegar a ser contraproducente a la hora de refrigerar.

Esto se puede regular manualmente en las variables del programa dependiendo del uso que se le vaya a dar, o manualmente a través de la interfaz del programa.


## Funcionalidad y variables

### Funcionalidad
- **Interfaz**: El sistema está preparado para trabajar con un LCD 2004 y un keypad que permita el input del usuario para ofrecer distintas funciones e información del funcionamiento. Más en el apartado *Interfaz*.
- **Termostato**: Encargado de alternar entre modos dependiendo de la temperatura de la nevera. Al llegar a 1ºC por debajo de la temperatura especificada se cambiará a un modo de bajo consumo, y al superarla de nuevo en 1ºC se volverá al modo de alto rendimiento.
- **Modos de trabajo**: Capacidad de ajustar manualmente modos de trabajo. Los modos programados ofrecen alternar entre "Alto rendimiento" (todos los módulos a 12V), "Bajo rendimiento" (módulos a 5V) o elegir manualmente a cada Peltier. **Tiene prioridad sobre la función de termostato**.
- **Estadísticas**: La pantalla principal mostrará datos sobre el funcionamiento, como temperatura interior, temperatura del termostato y modo de rendimientos seleccionados. A mayores hay un apartado de "Estadísticas" que aporta más información, como temperatura exterior (hot side), tiempo de encendido, etc.
- **Seguridad**: Aparte de incluir métodos de seguridad en la electrónica de la nevera (fusibles, fuentes de alimentación con corte por sobretensión, etc.), se implementa un corte por software si la parte caliente alcanza una temperatura elevada. Más en el apartado *Seguridad*.

### Variables

- **Work in progress :)**

## Interfaz

- **Work in progress :)**


## Seguridad

- **Work in progress :)**


### Referencias
[Hebei I.T. (Shanghai) Co., Ltd. - Peltier TEC1-12760 Datasheet](https://peltiermodules.com/peltier.datasheet/TEC1-12706.pdf)
- **Work in progress :)**