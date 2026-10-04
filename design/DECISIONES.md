# Decisiones del fork

Registro iniciado el 2026-10-04. Nombre provisional: GIMP Linux Design.

## Objetivo y límites

Editor nativo para Linux, libre, local y orientado a diseñadores acostumbrados a Photoshop. Windows sirve para editar el código; la plataforma de aceptación es Linux. La semejanza buscada es funcional y de flujo de trabajo. No se incorporarán servicios en línea, cuentas, telemetría, IA, código propietario, iconos ni recursos de Adobe. No se promete equivalencia completa ni rendimiento idéntico en cualquier equipo.

## D001 — Base estable y mantenimiento

Se creó el fork público https://github.com/ValentinVillalba/gimp-linux-design de GNOME/gimp, espejo del repositorio canónico https://gitlab.gnome.org/GNOME/gimp. Se parte de GIMP_3_2_6, commit e101dd19b165f927d3ba0a74658a71537c5661b9, en design/phase-1. La rama master del fork conserva upstream. Los forks públicos de GitHub son públicos; crear el fork no equivale a publicar una versión terminada.

Se mantienen GTK, GEGL, babl, LittleCMS y Meson. No se agrega Electron ni otra interfaz web. Las actualizaciones de seguridad de la rama estable deben revisarse antes de cada entrega. upstream tiene push deshabilitado; origin apunta al fork del usuario. Se conserva el historial y las licencias originales.

## D002 — Investigar antes de implementar

GIMP 3.2 ya ofrece capas vinculadas, vectoriales y filtros no destructivos. Reescribirlas sería trabajo innecesario. Se estudiaron las notas oficiales, el roadmap, los archivos locales de configuración y registro de acciones, y PhotoGIMP. El roadmap menciona trabajo de CMYK, máscaras vectoriales e importación de ajustes PSD; se evaluará su reutilización cuando esté probado, sin confundir trabajo en desarrollo con capacidades estables.

## D003 — Reutilización selectiva de PhotoGIMP

Referencia: https://github.com/Diolinux/PhotoGIMP, commit eca3a8f57b9944c063d043ce7c07524107b5292d, licencia GPL-3.0. Su toolrc permite reutilizar organización de herramientas. No se copia íntegramente su perfil: gimprc contiene datos de monitor/perfiles, rutas Flatpak y cambios del historial; sessionrc fija geometría de varios monitores; shortcutsrc repite acciones. Se conservará atribución y licencia de los fragmentos reutilizados.

## D004 — Primera implementación reversible

Primero se adaptan herramientas, paneles y atajos sobre las funciones existentes. Un lanzador usa GIMP3_DIRECTORY en un directorio de prueba separado, no sobrescribe el perfil personal y registra errores para depuración. No debe reinicializar archivos en cada arranque: los ajustes posteriores del diseñador se conservan.

Los cambios de configuración no alteran el motor de imagen ni agregan procesamiento por fotograma. Se preservan límites automáticos de memoria, configuración de monitores, idioma del sistema e historial de GIMP. No se activa aceleración experimental ni se reduce el historial para aparentar mayor rapidez.

## D005 — Funcionamiento local

La compilación del fork desactiva comprobación de actualizaciones y usa un directorio de configuración propio. El perfil desactiva comprobaciones automáticas y ayuda en línea. Esto todavía no demuestra que todas las rutas de red heredadas estén desactivadas: abrir ubicaciones remotas, acciones de ayuda web y complementos externos deben auditarse antes de llamar al producto completamente desconectado. Ninguna función nueva debe usar red.

## D006 — Licencias y contribuciones

El núcleo conserva GPL-3.0-or-later y las bibliotecas sus licencias LGPL. Se mantienen AUTHORS, COPYING y LICENSE. Cada dependencia reutilizada exige revisar la licencia de los archivos concretos, atribución, compatibilidad y código fuente. Los recursos de Adobe no se copian. Las distribuciones futuras deberán incluir las licencias y el código fuente correspondiente.

El proyecto GIMP indica que no acepta contribuciones generadas con IA en sus repositorios. Este trabajo se realiza en el fork independiente del usuario; no se enviará a repositorios gestionados por GIMP. Que el producto excluya herramientas de IA no significa que su desarrollo no haya usado un asistente.

## D007 — Verificación honesta

La máquina actual tiene Git, Python y Ninja. Meson no aparece en PATH; consultar distribuciones WSL devuelve acceso denegado. Antes de aceptar cambios de núcleo habrá que compilar y probar en Linux con GTK/GEGL y sus dependencias. Las verificaciones estáticas del perfil y los scripts no prueban la interfaz real ni compatibilidad PSD.

Con ejecución autorizada fuera del entorno restringido se encontró Ubuntu 26.04.1 en WSL. Se instalaron dependencias oficiales para compilar y probar (aproximadamente 372 MB en la instalación principal de dependencias). GIMP exige glib-networking y los módulos Python gi/cairo incluso en el build mínimo. Se generó en_US.UTF-8 para su generador de idiomas; no se modificó el idioma del perfil.

## D008 — Adaptación de atajos sin cambiar su significado en silencio

Se seleccionaron 39 asociaciones, sin repeticiones, comprobando los identificadores contra el código de 3.2.6. Ctrl+J sigue duplicando capas completas, no copiando solamente los píxeles seleccionados como Photoshop. No se reasignó Ctrl+G a «nuevo grupo», porque GIMP crea un grupo y no mueve las capas seleccionadas dentro. Estas diferencias se documentaron para futuras implementaciones, evitando equivalencias engañosas.

## D009 — Primer acceso directo a ajustes no destructivos

Se añadió una acción nativa para crear un grupo vacío Pass through encima de la primera capa seleccionada y con el mismo padre. Reutiliza gimp_group_layer_new, gimp_layer_set_mode y gimp_image_add_layer; no introduce motores, formatos ni dependencias. Se invoca desde Capa, el menú contextual o Ctrl+Alt+Shift+A. El diseñador aplica después el ajuste de color desde Colores sin fusionar el filtro. Crear el grupo vacío no modifica visualmente la composición por sí solo.

Se excluyen imágenes indexadas, selección flotante y canales activos. Se probarán colocación en jerarquías anidadas y undo/redo en el test de core. No se declara equivalencia completa con capas de ajuste Photoshop ni se incorpora un filtro nuevo.

## D010 — Compilación Windows/Linux

Git para Windows había aplicado core.autocrlf=true; los shebang CRLF fallaban en Linux. Se configuró core.autocrlf=false localmente y .gitattributes fija LF para scripts .py/.sh/.pl. Se preservaron los bytes de archivos upstream que originalmente tenían CRLF; no se necesita una modificación masiva de fuentes.

Compilar directamente sobre /mnt/c era lento. Se usa una copia temporal persistente en /home/boowomp/.cache/gimp-linux-design y se mantienen logs e instalación en el proyecto Windows. /tmp se perdió entre sesiones WSL; no debe usarse para el build incremental persistente. La copia necesita sincronización explícita y el fork Windows sigue siendo el origen de los cambios.

## D011 — Errores encontrados en el arranque real

La primera prueba GTK detectó que prefer-dark-theme aparece en el gimprc.in de referencia, pero ya no es una propiedad válida de la clase de configuración 3.2. Se corrigió el perfil a theme-color-scheme dark y se añadió un check contra las propiedades actuales. Los checks estáticos iniciales habían pasado: por eso se exige probar el parser nativo. Hay ahora siete checks del perfil.

El prefix aislado no encontraba por defecto las extensiones de babl/GEGL de la distribución. run-linux.sh y smoke-linux.sh fijan sus rutas desde pkg-config; evita ejecutar con conversiones de referencia lentas o módulos ausentes. La distribución final necesitará resolver sus dependencias y no debe depender de la caché de esta máquina.

Con GDK_BACKEND=x11, la pantalla Xvfb no tenía dispositivo de entrada seleccionado al cerrar. gimp_devices_save llamaba gimp_device_info_save_tool con NULL y generaba una aserción crítica. Se añadió una comprobación de NULL que preserva el guardado del manager y no cambia el comportamiento cuando hay dispositivo. La prueba smoke ahora también falla ante CRITICAL. Este ajuste no valida tabletas ni sustituye sus pruebas manuales.

Al repetir la suite después de instalar, core y xcf encontraban los complementos de ejemplo instalados, pero no Gimp.typelib ni libgimp del prefix propio. Antes de instalar la suite había pasado. Se reprodujo y comprobó que especificar GI_TYPELIB_PATH/LD_LIBRARY_PATH del build resolvía ambos fallos. Se fijaron esas rutas en el entorno Meson de las pruebas antiguas de app, sin cambiar las rutas del sistema ni ocultar sus errores.

## Fuentes consultadas

- https://www.gimp.org/news/2026/09/10/gimp-3-2-6-released/
- https://www.gimp.org/release-notes/gimp-3.2.html
- https://developer.gimp.org/core/roadmap/
- https://developer.gimp.org/core/setup/build/
- https://developer.gimp.org/core/specifications/layers/
- https://github.com/Diolinux/PhotoGIMP
- https://helpx.adobe.com/uk/photoshop/desktop/create-manage-layers/get-started-layers/layers-overview.html
- https://helpx.adobe.com/uk/photoshop/desktop/get-started/settings-and-preferences/view-keyboard-shortcuts.html
- Código local: LICENSE, etc/{toolrc,sessionrc,gimprc.in}, app/menus/{menus.c,shortcuts-rc.c}, app/widgets/gimpaction.c, app/actions/, app/core/, plug-ins/file-psd/.

Las fuentes se consultaron el 2026-10-04. Las capacidades concretas deben contrastarse con esta versión y archivos de prueba; el roadmap puede cambiar.
