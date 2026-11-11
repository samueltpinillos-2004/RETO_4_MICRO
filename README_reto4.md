# 🔒 Sistema de Control de Acceso Seguro (Caja Fuerte Inteligente)
**Proyecto Académico:** Reto 4 - Sistemas Embebidos y Tiempo Real  
**Autor:** Samuel Tangarife Pinillos  
**Plataforma:** STM32F407VET6 con FreeRTOS (CMSIS-RTOS v2)  

---

## 1. Nombre, Objetivo y Descripción del Sistema

* **Nombre del Sistema:** Caja Fuerte Electrónica con Control de Acceso Dual, Matriz 4x4 y FreeRTOS.
* **Objetivo:** Diseñar, programar e implementar un sistema embebido basado en la tarjeta de desarrollo **STM32F407VET6** que gestione el acceso seguro a un compartimento mediante una contraseña digital de 4 dígitos. El sistema integra un selector analógico de modo, actuadores físicos protegidos, retroalimentación visual avanzada por LED RGB y multitarea en tiempo real mediante **FreeRTOS**.
* **Descripción General:** El sistema opera como una caja fuerte inteligente con dos modos de trabajo (Normal y Configuración). Utiliza un teclado matricial 4x4 para el ingreso de contraseñas, un potenciómetro como selector analógico de modo, un servomotor para el cerrojo mecánico, un LED RGB para indicación de estados y un buzzer activo para alertas sonoras ante intentos no autorizados.

---

## 2. Requerimientos Funcionales y Alcance

### Requerimientos Funcionales (RF):
* **RF-01 (Validación de Acceso):** El sistema debe permitir desbloquear la caja fuerte al ingresar una clave de 4 dígitos en el teclado matricial.
* **RF-02 (Selección de Modo Analógico):** Debe conmutar entre Modo Normal y Modo Configuración evaluando la tensión leída en el pin `PA1` mediante el ADC.
* **RF-03 (Cambio de Clave Master):** En Modo Configuración, debe permitir guardar una nueva combinación de 4 dígitos y confirmar la operación con un destello amarillo en el LED RGB.
* **RF-04 (Cierre Seguro):** En estado abierto, el sistema debe cerrarse únicamente al ingresar la secuencia fija `0000`. Si se cometen 3 intentos fallidos de cierre, el cerrojo se bloquea automáticamente por seguridad.
* **RF-05 (Gestión de Errores):** Ante un intento de apertura fallido, se debe encender el LED en color Rojo y activar el buzzer activo por 2 segundos.
* **RF-06 (Control Mecánico):** El accionamiento del cerrojo se debe ejecutar mediante modulación por ancho de pulso (PWM) al servomotor SG90.

### Alcance:
El alcance abarca el diseño del firmware bajo una arquitectura multitarea determinista, el acondicionamiento eléctrico en protoboard con plano de masa común (GND) y alimentación externa de 5V para actuadores de potencia, y la prueba integral de estabilidad del sistema.

---

## 3. Diagrama de Bloques de Hardware

```
                      +-----------------------------+
                      |   Fuente Externa (5V / GND) |
                      +--------------+--------------+
                                     | (Potencia)
                                     v
+------------------+        +--------+--------+        +-------------------+
| Teclado 4x4      |------> |                 | -----> | Servomotor SG90   |
| (PD0 - PD7)      |        |                 |        | (PD12 - PWM TIM4) |
+------------------+        |   STM32F407VET6 |        +-------------------+
                            |   Microcontroller|
+------------------+        |                 |        +-------------------+
| Potenciómetro    |------> |  (FreeRTOS RTOS)| -----> | LED RGB (KY-016)  |
| (PA1 - ADC1_IN1) |        |                 |        | (PD9, PD10, PD11) |
+------------------+        |                 |        +-------------------+
                            |                 |        +-------------------+
                            |                 | -----> | Buzzer Activo     |
                            +-----------------+        | (PB15 - GPIO Out) |
                                                       +-------------------+
```

---

## 4. Tabla de Conexiones entre el STM32 y Periféricos

| Periférico / Componente | Pin STM32F407VET6 | Tipo de Pin / Canal | Descripción / Configuración |
| :--- | :--- | :--- | :--- |
| **Teclado (Filas 1-4)** | `PD0`, `PD1`, `PD2`, `PD3` | GPIO Output | Salidas digitales para escaneo |
| **Teclado (Columnas 1-4)**| `PD4`, `PD5`, `PD6`, `PD7` | GPIO Input (Pull-Up) | Entradas con debounce por software (20ms) |
| **Potenciómetro** | `PA1` | `ADC1_IN1` | Entrada analógica (0 - 3.3V) |
| **Servomotor SG90** | `PD12` | `TIM4_CH1` (PWM) | Salida PWM a 50Hz (período 20ms) |
| **LED RGB (Rojo)** | `PD9` | GPIO Output (Push-Pull) | Salida digital |
| **LED RGB (Verde)** | `PD10` | GPIO Output (Push-Pull) | Salida digital |
| **LED RGB (Azul)** | `PD11` | GPIO Output (Push-Pull) | Salida digital |
| **Buzzer Activo (MH-FMG)**| `PB15` | GPIO Output (Push-Pull) | Salida digital por nivel alto (High-Level Trigger) |
| **Alimentación Externa**| `GND` | Common Ground | Masa compartida entre STM32 y 5V externos |

---

## 5. Diagrama de Estados de la Aplicación

```
                      +-------------------+
                      |   ESTADO_ESPERA   |  <--- (Inicio / Reposo - LED Azul)
                      +---------+---------+
                                |
             +------------------+------------------+
             | (Tipeo en Modo   | (Potenciómetro  | (Potenciómetro
             |  Normal)         |  > 2100)        |  < 1900)
             v                  v                 v
   +-------------------+  +---------------+  +-------------------+
   | ESTADO_INGRESANDO |  |  MODO_CONFIG  |  | Restablece a      |
   | (LED Morado)      |  | (LED Cian)    |  | ESTADO_ESPERA     |
   +---------+---------+  +-------+-------+  +-------------------+
             |                    |
             | Clave Validada     | Clave Guardada
             +----------+         | (LED Amarillo 1.5s)
                        v         v
             +--------------------+
             |   Evaluación       |
             +---+------------+---+
                 |            |
   Clave Correcta|            |Clave Incorrecta
                 v            v
      +--------------+    +--------------+
      | ESTADO_ABIERTO|   | ESTADO_ERROR |
      | (LED Verde / |    | (LED Rojo /  |
      | Servo Giro)  |    | Buzzer 2s)   |
      +--------------+    +--------------+
```

---

## 6. Tabla de Tareas FreeRTOS

| Nombre Tarea | Función de Entrada | Prioridad | Tamaño Pila (*Stack*) | Período / Evento | Recursos Compartidos |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`vTaskTeclado`** | `StartTask_Teclado` | `osPriorityHigh` | 128 words (512 B) | Periódico (30 ms) | Buffer `claveingresada`, `indice_tecla` |
| **`vTaskADC`** | `StartTask_ADC` | `osPriorityNormal` | 128 words (512 B) | Periódico (200 ms) | Variable global `modoConfig` |
| **`vTaskLogica`** | `StartTask_Logica` | `osPriorityNormal` | 256 words (1024 B) | Periódico (50 ms) | `claveactual`, `claveingresada`, `estado_actual` |
| **`vTaskActuadores`**| `StartTask_Actuadores` | `osPriorityLow` | 128 words (512 B) | Periódico (100 ms) | `estado_actual`, `modoConfig` |

---

## 7. Comunicación y Sincronización entre Tareas

La arquitectura de software se basa en el intercambio de información mediante **variables de estado y arreglos globales compartidos** administrados de forma asíncrona no bloqueante:

1. **ADC -> Lógica/Actuadores:** La tarea `vTaskADC` evalúa la tensión del pin `PA1` cada 200 ms. Para evitar lecturas inestables causadas por el ruido analógico, aplica una lógica de **histéresis** (umbrales en 1900 y 2100). El resultado actualiza la bandera global `modoConfig`.
2. **Teclado -> Lógica:** La tarea `vTaskTeclado` escanea la matriz de botones cada 30 ms aplicando un filtro de antirrebote (*debounce*) de 20 ms. Cuando detecta una tecla, la almacena directamente en el buffer global `claveingresada`.
3. **Lógica -> Actuadores:** La tarea `vTaskLogica` procesa la máquina de estados cada 50 ms comparando `claveingresada` con `claveactual`. Dependiendo del resultado, actualiza la variable global `estado_actual`. `vTaskActuadores` lee este estado cada 100 ms y ajusta la señal PWM en `PD12`, el color en los pines `PD9-PD11` y la alarma en `PB15`.

---

## 8. Justificación de los Recursos Seleccionados

* **GPIO:** Imprescindibles para la matriz 4x4 (ahorro de pines mediante escaneo matricial), el control digital directo del LED RGB y la activación del transistor del buzzer.
* **ADC1 (Canal 1 - PA1):** Permite una selección de modo física e intuitiva mediante un control analógico continuo, transformando variaciones de voltaje en decisiones digitales.
* **PWM (TIM4_CH1 - PD12):** Selección necesaria para el servomotor SG90, ya que requiere un pulso periódico preciso de 50 Hz con anchos de pulso entre 1 ms y 2 ms para posicionar el cerrojo mecánico sin saturar la CPU.
* **FreeRTOS:** Garantiza una ejecución concurrente y determinista. Evita el uso de `HAL_Delay()` bloqueantes, permitiendo que el sistema responda al teclado al mismo tiempo que ejecuta retardos de seguridad o muestreos analógicos.

---

## 9. Procedimiento y Resultados de las Pruebas

### Pruebas Realizadas:
1. **Prueba de Inestabilidad del ADC (Filtrado por Histéresis):**
   * *Procedimiento:* Se varió la posición del potenciómetro cerca del punto medio de transición.
   * *Resultado:* Con una condición `if` simple (>2048), el LED RGB parpadeaba entre Azul y Cian. Al aplicar los umbrales de histéresis (1900 y 2100), el parpadeo se eliminó por completo.
2. **Prueba de Antirrebote en Teclado:**
   * *Procedimiento:* Se presionaron repetidamente las teclas numéricas para verificar si se registraban múltiples caracteres por una sola pulsación.
   * *Resultado:* El filtro de 20 ms garantizó la captura de un solo dígito por pulsación.
3. **Prueba de Cierre Automático por Reintentos:**
   * *Procedimiento:* Con la caja abierta, se ingresaron 3 claves incorrectas de cierre.
   * *Resultado:* Al tercer intento fallido, el sistema activó la alarma sonora y retornó al cerrojo cerrado de forma autónoma.

---

## 10. Limitaciones Conocidas y Posibles Mejoras

### Limitaciones Conocidas:
* **Almacenamiento Volátil:** La clave configurada por el usuario (`claveactual`) se almacena en la memoria RAM, por lo que se reinicia al valor predeterminado si se desconecta la alimentación.
* **Comunicación por Variables Globales:** Aunque es adecuada para el tamaño del proyecto, el uso de variables globales compartidas sin semáforos puede representar un riesgo de condición de carrera (*race condition*) en futuras expansiones del código.

### Posibles Mejoras:
1. **Persistencia de Datos:** Implementar memoria EEPROM externa vía I²C o memoria Flash interna del STM32 para guardar la contraseña de forma permanente.
2. **Uso de Queues de FreeRTOS:** Migrar la comunicación entre la tarea del teclado y la tarea lógica a una **Cola de mensajes (*Queue*)** para lograr un desacoplamiento completo y más robusto entre tareas.
3. **Pantalla LCD / OLED:** Integrar una pantalla mediante bus I²C para proporcionar una interfaz de usuario visual con instrucciones explícitas.