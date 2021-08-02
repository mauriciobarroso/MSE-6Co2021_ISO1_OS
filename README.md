# MSE-6Co2021_ISO1_OS
TP para la materia Implementación de Sistemas Operativos I de la Maestría en Sistemas Embebidos 6ta cohorte

## Requerimientos

1. [x] El sistema operativo (de aquí en más nombrado como OS) será del tipo estático,
entendiéndose por este término que la totalidad de memoria asociada a cada tarea y al
kernel será definida en tiempo de compilación.
2. [x] La cantidad de tareas que soportara el OS será ocho (8).
3. [ ] El OS debe administrar las IRQ del hardware.
4. El kernel debe poseer una estructura de control la cual contenga como mínimo los
siguientes campos:
- [x] Último error ocurrido.
- [x] Estado de sistema operativo, por ejemplo: Reset, corriendo normal, interrupción,
etc.
- [x] Bandera que indique la necesidad de ejecutar un scheduling al salir de una IRQ.
- [x] Puntero a la tarea en ejecución.
- [x] Puntero a la siguiente tarea a ejecutar.
5. Cada tarea tendrá asociada una estructura de control que, como mínimo, tendrá los
siguientes campos:
- [x] Stack (array).
- [x] Stack Pointer.
- [x] Punto de entrada (usualmente llamado entryPoint).
- [x] Estado de ejecución.
- [x] Prioridad.
- [x] Número de ID.
- [x] Ticks bloqueada.
6. Los estados de ejecución de una tarea serán los siguientes:
- [x] Corriendo (Running).
- [x] Lista para ejecución (Ready).
- [x] Bloqueada (Blocked).
- [ ] Suspendida (Suspended) - Opcional
7. [x] El tamaño de stack para cada tarea será de 256 bytes.
8. [x] La implementación de prioridades será de 4 niveles, donde el nivel cero (0) será el de más alta prioridad y tres (3) será el nivel de menor prioridad.
9. [x] La política de scheduling entre tareas de la misma prioridad será del tipo Round-Robin.
10. [x] El tick del sistema será de 1 ms.
11. El OS debe tener hooks definidos como funciones WEAK para la ejecución de código en las siguientes condiciones:
- [x] Tick del sistema (tickHook).
- [x] Ejecución de código en segundo plano (taskIdle).
- [x] Error y retorno de una de las tareas (returnHook).
- [x] Error del OS (errorHook).
12. El OS debe poseer una API que contenga como mínimo las siguientes funciones:
- [x] Función de retardos (delay).
- [ ] Semáforos binarios.
- [ ] Colas (queue).
- [ ] Secciones críticas.
- [ ] Forzado de Scheduling (cpu yield).