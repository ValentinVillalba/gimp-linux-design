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

Investigación y planificación inicial terminadas. La etapa 1 tiene herramientas/paneles adaptados, 41 asociaciones de teclas y un lanzador de perfil aislado. Se agregó el acceso nativo «Nuevo grupo de ajuste» reutilizando grupos Pass through; no es una implementación completa de las capas de ajuste de Photoshop.

Se compiló en Ubuntu 26.04.1 WSL con GCC 15, Meson/Ninja y dependencias de distribución. Pasaron las 20 pruebas habilitadas en esta configuración, incluidas las dos nuevas del grupo de ajuste, tres de agrupación de capas seleccionadas y dos de capa mediante copiar; también los siete checks del perfil. La suite Meson cuenta ejecutables de prueba; core contiene varios casos internos. Falta aceptación visual, tableta, mediciones de rendimiento y corpus PSD. Ninguna otra función avanzada de la tabla se considera implementada por documentarla.

Ctrl+G agrupa ahora las capas seleccionadas mediante una acción nativa, con orden de pila, ancestro común, protección de bloqueos y undo/redo. Ctrl+Shift+G desagrupa las carpetas seleccionadas sin fusionar sus capas, conservando jerarquía, orden y posiciones de contenido. Ambos están disponibles en los dos menús de capas. Se agregaron tres pruebas internas sobre grupos anidados, máscaras/filtros restaurados por undo, grupos vacíos y bloqueos.

Ctrl+J tiene ahora una acción nativa para copiar píxeles seleccionados, con alfa de selección, coordenadas y máscara de capa recortada, manteniendo filtros permanentes duplicables y propiedades. Se agregaron dos casos de core sobre píxeles/máscara/filtro/offsets/undo y duplicación sin selección. La selección permanece activa; falta contrastar ese detalle con Photoshop y probar pilas de filtros más complejas.

Se restringieron las entradas de archivos del núcleo y la PDB a rutas nativas; se retiró el transporte remoto y se agregaron tres pruebas internas. AUDITORIA_LOCAL.md registra la evidencia y las rutas restantes. No se afirma aislamiento completo de red.

Siguiente trabajo: retirar navegador/correo y fallback de ayuda remota, conservar diagnóstico/ayuda local y verificar conexiones; cerrar la aceptación del perfil. Antes de ampliar capas de ajuste, comprobar la composición y XCF con filtros, máscaras y grupos reales; seguir con estilos, formas y flujos de transformación según la tabla y el código reutilizable disponible.

Se excluyeron mail y web-browser de la compilación y se retiraron los enlaces externos y el panel de descarga de Acerca de. La instalación retira reversiblemente los binarios antiguos. Continúan pendientes la ayuda remota heredada, el diálogo crítico y la auditoría de recursos de documentos.

El diálogo crítico conserva diagnóstico/portapapeles/cierre/reinicio y ya no ofrece gestor de errores ni descargas. Cuatro casos GTK verifican el widget real. La adaptación de ayuda local y la auditoría de recursos de documentos continúan pendientes.

La ayuda ya no ofrece lectura en línea ni usa navegador externo. El parser y los dominios usan rutas nativas. Quedan pendientes las preferencias antiguas, la revisión del visor HTML opcional y su instalación/aceptación visual.

La página de preferencias de ayuda ya no ofrece alternativas de Internet. Conserva idiomas y estado de manuales e informa si falta el visor interno. Sigue pendiente la aceptación visual y revisión del visor HTML opcional.

La prueba nativa de composición del grupo de ajuste comprueba filtro sobre el fondo sin alterar el buffer original y su deshacer/rehacer. Esta evidencia valida reutilizar la composición Pass through; falta ampliar cobertura de máscaras/XCF y crear la operación de ajuste completa.

La prueba de composición detectó tiles obsoletos al cambiar la representación efectiva del grupo. Se corrigió la transición central del grafo para que la proyección se reconstruya; se conserva el render diferido de GEGL y queda pendiente medir el coste en documentos grandes.

Se comprobó el roundtrip XCF del grupo de ajuste con filtro permanente e intensidad 0.5, con/sin compresión. Se conservan capas originales, composición y activación editable. Continúan pendientes máscaras, curvas/niveles, grupos anidados y varios efectos; no se necesita un formato nuevo para este caso.

Se comprobó máscara blanca/negra/parcial, undo/redo de adición, edición después de renderizar y XCF con/sin compresión. Procesar el bucle de eventos en las pruebas permitió retirar la reconstrucción añadida en D021: el código nativo pasa esos casos. Este estado corrige la interpretación anterior. Quedan pendientes offsets/tamaños, grupos anidados y controles de curvas/niveles.

Se verificó Niveles nativo con configuración editable: identidad inicial, modificación, composición enmascarada, guardado/reapertura XCF con/sin compresión y edición posterior. Se reutiliza la sincronización de configuración de GIMP hacia GEGL. El acceso a sus controles desde Nuevo ajuste todavía está pendiente; primero deben cerrarse selección activa, bounds vacíos, offsets y cancelación (D024).

Se verificó la selección parcial en el modelo de ajuste y en XCF: máscara blanca propia del filtro, selección copiada una sola vez al grupo, selección conservada al crear y efecto estable al deseleccionar. Siete casos XCF correctos. Quedan pendientes el acceso de interfaz, bounds/offsets y cancelación; no hace falta borrar/restaurar temporalmente la selección (D025).

## Estimación del avance global

A petición del usuario, informar una estimación global en los avances de nuevas funciones. Referencia inicial al 2026-10-04: aproximadamente 10% (rango orientativo 5–15%). Es una valoración del trabajo restante del fork, no una medición de líneas de código, horas ni del porcentaje de herramientas que GIMP ya trae. El objetivo completo incluye experiencia comparable a Photoshop, flujos avanzados, intercambio de documentos, funcionamiento local y verificación de rendimiento/Linux.

La base, la compilación y algunas adaptaciones de capas están implementadas. Falta aceptación de la experiencia en Linux y la mayor parte de las diferencias avanzadas de la tabla. Las pruebas de Niveles/selección reducen incertidumbre, pero no completan la interfaz de capas de ajuste. No subir el porcentaje por cada test aislado; revisar la estimación cuando se cierre un flujo utilizable y su verificación. Mantener visibles las limitaciones y corregir el valor si la investigación descubre más alcance.
