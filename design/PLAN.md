# Plan de desarrollo

## Método

Antes de cada función: definir una tarea de diseño concreta; comprobar la implementación existente y upstream; localizar código libre reutilizable; revisar su licencia; diseñar el cambio mínimo; medir y probar; registrar decisión y limitaciones. Evitar una gran reescritura de la interfaz o del motor de composición.

## Diferencias y posibilidades de reutilización

| Flujo de Photoshop | Base disponible en GIMP 3.2.6 | Trabajo del fork |
| --- | --- | --- |
| Herramientas y atajos familiares | Acciones configurables, grupos de herramientas, ventana única | Perfil selectivo basado en PhotoGIMP; comprobar conflictos y herramientas ocultas |
| Paneles Capas/Canales/Trazados | Dockables nativos | Disposición limpia a la derecha, opciones accesibles, sin rutas de monitor importadas |
| Transformación libre | Transformación unificada y capas vinculadas | Atajo Ctrl+T; evaluar pivote, confirmación y escalado; no llamar a la transformación raster no destructiva |
| Objetos inteligentes | Capas vinculadas conservan una fuente externa | No equivalen a objetos incrustados, pila de transformaciones, composición anidada o Smart Filters completos; investigar representación y XCF antes de ampliar |
| Capas de ajuste | Filtros no destructivos en capas/grupos; grupos vacíos Pass through afectan inferiores | Prototipo de creación y edición de grupos de ajuste reutilizando GimpDrawableFilter; validar máscaras, orden, recorte y guardado |
| Estilos de capa | Filtros GEGL no destructivos | Primero sombras/contornos existentes; evaluar GEGL Effects de LinuxBeaver archivo por archivo antes de incorporar; no descargar binarios sin código/licencia |
| Máscaras de recorte | Máscaras, composición y grupos | No confundir clip-to-backdrop con equivalencia Photoshop; probar pilas con opacidad, grupos y máscaras; diseñar operación reversible |
| Formas editables | Capas vectoriales y trazados | Priorizar rectángulo/elipse reutilizando trazados; revisar herramienta Shape prevista por upstream |
| Texto y tipografía | Texto editable, Pango, fuentes locales | Comparar párrafos, interletrado, estilos, texto en trazados y transformaciones con casos reales |
| PSD/PSB | Importador/exportador nativo en plug-ins/file-psd | Corpus legal de archivos; documentar pérdida de ajustes, estilos, objetos y fuentes; revisar mejoras upstream antes de programar |
| CMYK de producción | Perfiles, prueba de color y soporte parcial por formato | No asumir edición CMYK completa; revisar rama upstream y pruebas de imprenta; prioridad posterior a estabilidad RGB |
| Mesas de trabajo | Capas/grupos y exportación | Modelo multipágina no equivalente; estudiar trabajo upstream antes de crear formato nuevo |
| Acciones grabadas | PDB, Script-Fu, complementos | Reutilizar para automatización local; grabador general requiere infraestructura, está en roadmap |
| Retoque y selección | Clonar, reparar, selección y filtros GEGL | Ajustar interacción; no agregar IA, servicios ni modelos de segmentación |

## Etapas y aceptación

1. **Fundación y experiencia existente.** Fork, rama estable, registro, perfil aislado, herramientas ordenadas, atajos consistentes, paneles. Aceptar sólo tras arranque real, edición, guardado XCF, exportación y reapertura en Linux. Probar 1280×720 y 1920×1080, teclado, tableta y escalado HiDPI.
2. **Compilación Linux repetible y política local.** Usar build/linux y Meson upstream, prefix/config propios, símbolos de depuración, comprobación de actualizaciones desactivada. Compilar, ejecutar meson test y pruebas manuales. Revisar todas las rutas de red heredadas y complementos; preparar paquete sólo después.
3. **Capas y flujo no destructivo.** Operación para grupos de ajuste, formas básicas y estilos reutilizando el motor existente. Cada función requiere undo/redo, serialización XCF, cancelación y pruebas con grupos/máscaras.
4. **Intercambio PSD y diseño avanzado.** Corpus versionado con permisos, reporte de pérdidas visibles, mejoras upstream seleccionadas, objetos incrustados y texto según evidencia. Nunca prometer edición sin pérdidas antes de comprobar ida y vuelta.
5. **Rendimiento y distribución.** Medir inicio, memoria, pincel, transformaciones, filtros, guardado y recuperación. Comparar con GIMP estable en los mismos documentos/equipos. Distribución libre con fuente, licencias y nombre propio.

## Rendimiento y seguridad

Documentos de prueba: composición pequeña 2048²; fotografía 6000×4000 en 8/16 bits; composición con 100 capas y máscaras; documento grande con memoria limitada. Registrar hardware, versión GEGL/babl, RAM máxima, tiempos y fallos. No dar tiempos objetivo sin línea base. Mantener render por regiones/caché GEGL, evitar copias completas y cómputo redundante.

Archivos PSD y otros formatos son entrada no confiable: tamaños, offsets, profundidad y asignaciones necesitan límites y errores claros. No introducir cargadores paralelos sin necesidad. Guardar XCF sigue siendo la vía principal para preservar los datos propios del editor.

## Estado al 2026-10-04

Investigación y planificación inicial terminadas. La etapa 1 tiene herramientas/paneles adaptados, 40 asociaciones de teclas y un lanzador de perfil aislado. Se agregó el acceso nativo «Nuevo grupo de ajuste» reutilizando grupos Pass through; no es una implementación completa de las capas de ajuste de Photoshop.

Se compiló en Ubuntu 26.04.1 WSL con GCC 15, Meson/Ninja y dependencias de distribución. Pasaron las 20 pruebas habilitadas en esta configuración, incluidas las dos nuevas del grupo de ajuste, tres de agrupación de capas seleccionadas y dos de capa mediante copiar; también los siete checks del perfil. La suite Meson cuenta ejecutables de prueba; core contiene varios casos internos. Falta aceptación visual, tableta, mediciones de rendimiento y corpus PSD. Ninguna otra función avanzada de la tabla se considera implementada por documentarla.

Ctrl+G agrupa ahora las capas seleccionadas mediante una acción nativa, con orden de pila, ancestro común, protección de bloqueos y undo/redo. Está disponible en ambos menús de capas. No está implementado todavía desagrupar con Ctrl+Shift+G.

Ctrl+J tiene ahora una acción nativa para copiar píxeles seleccionados, con alfa de selección, coordenadas y máscara de capa recortada, manteniendo filtros permanentes duplicables y propiedades. Se agregaron dos casos de core sobre píxeles/máscara/filtro/offsets/undo y duplicación sin selección. La selección permanece activa; falta contrastar ese detalle con Photoshop y probar pilas de filtros más complejas.

Siguiente trabajo: cerrar la aceptación del perfil y revisar las rutas de red heredadas; desagrupar preservando la jerarquía. Antes de ampliar capas de ajuste, comprobar la composición y XCF con filtros, máscaras y grupos reales.
