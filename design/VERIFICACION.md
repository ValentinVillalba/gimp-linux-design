# Verificación del desarrollo

Fecha: 2026-10-04. Base: GIMP_3_2_6, e101dd19b165f927d3ba0a74658a71537c5661b9.

## Entorno y resultado

- Ubuntu 26.04.1 LTS sobre WSL, Linux x86_64.
- GCC 15.2.0, Meson 1.10.1, Ninja 1.13.2.
- GTK 3.24.52, GEGL 0.4.70, babl 0.1.124, LittleCMS 2.17.
- Build debugoptimized, check-update=no, gimpdir=GimpLinuxDesign, dependencias opcionales desactivadas y pruebas headless habilitadas.
- Compilación nativa e instalación en `_install`: correctas.
- Suite Meson: **20 correctas, 0 fallos**, tras corregir las rutas de introspección para pruebas después de instalar.
- `design/test_profile.py`: **7 correctas**.
- Sintaxis de build-linux.sh, run-linux.sh y smoke-linux.sh: correcta.
- Arranque GTK en Xvfb/X11 y salida por Script-Fu: correcto, sin errores de parser, conflictos/acciones inexistentes ni mensajes CRITICAL. Se ejecutó con un perfil nuevo.
- git diff --cached --check: correcto antes del commit.

Las dos pruebas nuevas de core verifican grupo de ajuste Pass through, inserción al nivel adecuado, conservación de jerarquía y deshacer/rehacer. Los tests upstream de guardar/exportar y XCF pasaron; no demuestran interoperabilidad PSD completa. Varias pruebas históricas de UI están deshabilitadas en upstream y no se cuentan como ejecutadas.

## Evidencia en esta máquina

- `../linux-build-final.log`: compilación incremental final.
- `../linux-tests-final.log`: resumen de la suite final.
- `../linux-test-details.txt`: informe Meson completo.
- `../linux-install-final.log`: instalación final.
- `_design-logs/linux-smoke.log`: arranque y cierre GTK final.

Los primeros intentos y sus errores también se conservaron en logs separados en la carpeta superior. DECISIONES.md registra correcciones de CRLF, locale, propiedad de tema, rutas babl/GEGL, dispositivo NULL y entorno de pruebas. No se borró evidencia para presentar sólo el resultado correcto.

La caché de compilación está en `/home/boowomp/.cache/gimp-linux-design/build`; los fuentes temporales en la carpeta `source` adyacente. El fork editable sigue en Windows y debe sincronizarse antes de una recompilación.

## Lo que aún no está verificado

No hay aceptación visual humana, prueba de tableta/HiDPI, comparativas de rendimiento, recuperación ante fallos, corpus PSD/PSB ni impresión CMYK. El programa aún hereda rutas web/GIO de upstream que deben auditarse antes de declararlo completamente desconectado. No hay paquete final ni compatibilidad completa con Photoshop. La copia compilada es un build local de desarrollo para Linux.

## Agrupación de capas seleccionadas

El 2026-10-04 se compiló la acción Ctrl+G. Los tres casos nuevos internos de core pasaron: orden de pila y undo/redo con capas no contiguas, ancestro común entre padres distintos, y selección de grupo/descendiente con bloqueo de posición. La suite completa volvió a pasar (20 ejecutables correctos, cero fallos), incluido XCF, y los siete checks del perfil también pasaron.

Evidencia en la carpeta testing-loop: `linux-build-grouping.log`, `linux-tests-grouping-full.log`, `linux-test-grouping-details.txt` y `linux-install-grouping.log`. Estos registros prueban jerarquía y comportamiento nativo; no prueban equivalencia visual de todos los casos de composición con Photoshop.

Se comprobó el arranque instalado con `_design-profile-grouping`, tras corregir la ruta relativa del script smoke (D013). `_design-logs/linux-smoke.log` confirma lectura de los cuatro archivos desde la ruta absoluta del proyecto, sin errores de parser ni CRITICAL. La prueba ahora exige esa lectura, además de la salida correcta.

## Capa mediante copiar

El 2026-10-04 se compiló la nueva acción de Ctrl+J. Pasaron los dos casos nuevos de core: copia de píxeles con coordenadas negativas del origen, alfa parcial, máscara de capa alineada, filtro permanente, propiedades, portapapeles y undo/redo; y duplicación completa sin selección. Además de contar el filtro, se verificó el píxel renderizado por el filtro de inversión duplicado. La suite completa pasó de nuevo (20 ejecutables, cero fallos), y también los siete checks del perfil.

Evidencia en testing-loop: `linux-build-copy.log`, `linux-tests-copy-full.log`, `linux-test-copy-details.txt` y `linux-install-copy.log`. La comprobación adicional del render del filtro está en `linux-tests-copy-render.log` y `linux-test-copy-render-details.txt`. No se ha validado un corpus de efectos ni equivalencia completa de Layer Via Copy con Photoshop.

La instalación y el arranque GTK/X11 también pasaron con `_design-profile-copy`. `_design-logs/linux-smoke.log` confirma lectura del perfil y atajos desde la ruta esperada, sin acciones inexistentes, errores de parser ni CRITICAL.

## Desagrupar capas

El 2026-10-04 se compiló Ctrl+Shift+G. Pasaron tres casos internos nuevos de core: orden/offsets/buffer y restauración de máscara/filtro/opacidad por undo; desagrupación de grupos anidados y hermanos con selección mixta; y grupos vacíos con bloqueo de un hijo que impide cambios parciales. El menú también verifica los bloqueos de los hijos directos del grupo.

La suite completa volvió a pasar (20 ejecutables, cero fallos), además de los siete checks del perfil. Evidencia en testing-loop: `linux-build-ungroup.log`, `linux-tests-ungroup-full.log`, `linux-test-ungroup-details.txt` y `linux-install-ungroup.log`. No se ha comparado visualmente todo el comportamiento de desagrupar con Photoshop ni probado un corpus de modos y efectos de grupo.

La instalación y el arranque GTK/X11 pasaron con `_design-profile-ungroup`, incluidos los cuatro archivos del perfil y el nuevo atajo. `_design-logs/linux-smoke.log` no contiene errores de parser, acciones inexistentes ni mensajes CRITICAL.

## Archivos nativos y transporte remoto retirado

El 2026-10-04 pasaron tres casos nuevos internos de core: aceptación de rutas/URI file:// y rechazo de HTTP(S), FTP, SFTP, SMB y trash://; guardado remoto rechazado sin cambiar el archivo asociado de la imagen y los cuatro transportes antiguos deshabilitados; y rechazo de llamadas PDB directas a gimp-file-load y gimp-xcf-load antes del cargador. La comprobación central también se aplica antes de la validación normal de parámetros GFile.

La suite completa pasó (20 ejecutables, cero fallos), incluidos guardar/exportar y XCF locales. También pasaron los siete checks del perfil. Evidencia en testing-loop: `linux-build-native-files.log`, `linux-tests-native-files-full.log`, `linux-test-native-files-details.txt` y `linux-install-native-files.log`.

Esto verifica esas entradas de archivos, no ausencia total de conexiones. AUDITORIA_LOCAL.md mantiene el inventario pendiente y explica el límite de montajes nativos y complementos externos. No se hicieron aún trazas de sockets ni pruebas de recursos remotos incrustados en documentos.

La instalación y el arranque GTK/X11 con `_design-profile-native-files` pasaron. El log `_design-logs/linux-smoke.log` confirma lectura de los cuatro archivos de configuración esperados, sin errores de parser, acciones inexistentes ni CRITICAL.

## Retirada de correo, navegador y enlaces externos — 2026-10-04

Se recompiló Linux después de eliminar los objetivos mail/web-browser y las rutas externas de Acerca de, incluida la llamada residual gimp_update_refresh. Meson: 20 ejecutables de prueba correctos, 0 fallos. Perfil: 7 pruebas correctas. Retirada reversible: 3 pruebas correctas en Linux, incluido rechazo de symlink exterior antes de mover otros objetivos.

La instalación completa conservaba los antiguos binarios de los complementos: se movieron a _install/_retired-plugins y se verificó su ausencia del árbol de búsqueda. Se actualizó luego sólo el ejecutable principal para incorporar la última retirada de la llamada de actualización. El smoke volvió a pasar con _design-profile-offline-plugins, las cuatro rutas de configuración comprobadas y sin errores GTK/parser detectados. smoke-linux.sh rechaza instalaciones que aún contienen mail/web-browser.

Registros en la carpeta raíz: linux-build-offline-plugins.log, linux-tests-offline-plugins.log, linux-test-offline-plugins-details.txt, linux-install-offline-plugins.log; arranque en _design-logs/linux-smoke.log. Las pruebas no certifican el clic manual de enlaces de Acerca de ni aislamiento completo de red. Ayuda y errores críticos siguen pendientes en AUDITORIA_LOCAL.md.

## Diagnóstico crítico local — 2026-10-04

Compilación Linux correcta del editor y gimp-debug-tool. Meson: 20 pruebas correctas, 0 fallos. design/test-critical-dialog.sh: cuatro casos GTK correctos con/sin versión posterior y error normal/fatal. La prueba compila el widget real, utiliza una cadena de versión de prueba y comprueba respuestas permitidas, contenido de diagnóstico y copia exacta al portapapeles. No ejecuta matar/reiniciar procesos ni valida recuperación efectiva de archivos.

Se actualizaron los ejecutables del prefijo aislado conservando bibliotecas/recursos de la instalación anterior. El smoke GTK pasó con _design-profile-local-diagnostics y comprobó las cuatro rutas de perfil y ausencia de errores detectados. Registros en la carpeta raíz: linux-build-local-diagnostics.log, linux-tests-local-diagnostics.log, linux-test-local-diagnostics-details.txt y linux-test-local-dialog.log; arranque en _design-logs/linux-smoke.log. El funcionamiento sin red sigue pendiente de la auditoría de ayuda y recursos externos.

## Despacho e índice de ayuda local — 2026-10-04

Compilación Linux correcta y suite Meson completa: 20 pruebas correctas, 0 fallos. design/test-local-help.sh compila el parser real con sus auxiliares: rechaza índices HTTPS/SFTP con NOT_SUPPORTED y lee/mapea un índice XML nativo cuyo nombre contiene espacios. Se actualizaron el editor y el complemento help del prefijo aislado. El smoke pasó con _design-profile-local-help, configuración comprobada y salida normal.

Registros raíz: linux-build-local-help.log y linux-tests-local-help.log; arranque en _design-logs/linux-smoke.log. No se prueba navegación visual de manuales, diálogos de idioma, preferencias antiguas ni cargas HTML del visor WebKit opcional, ausente en este build. D019 y AUDITORIA_LOCAL.md mantienen estos límites.

## Preferencias de ayuda local — 2026-10-04

Se recompiló el editor Linux tras retirar selectores de manual en línea/navegador externo y el fallback automático de preferencias. Meson: 20 pruebas correctas, 0 fallos. Se actualizó el ejecutable del prefijo aislado y pasó el smoke con _design-profile-local-help-preferences, lectura comprobada de las cuatro configuraciones y salida normal.

Registros raíz: linux-build-local-help-preferences.log y linux-tests-local-help-preferences.log. Falta aceptación visual de la página con/sin manuales, teclado y escalado; las pruebas anteriores no demuestran esa interacción. La política efectiva de índices locales sigue cubierta por test-local-help.sh y D019.
