# GIMP Linux Design

Fork nativo y libre de GIMP para acercar el trabajo cotidiano de diseño a los flujos de Photoshop. Base: GIMP 3.2.6. Nombre provisional; desarrollo inicial, todavía sin paquete final ni paridad funcional.

Leer primero [DECISIONES.md](DECISIONES.md) y [PLAN.md](PLAN.md). [THIRD_PARTY.md](THIRD_PARTY.md) registra el código reutilizado.

## Cambios iniciales

- Herramientas agrupadas con la organización de PhotoGIMP; todas las herramientas de la base siguen disponibles.
- Ventana única, caja de herramientas estrecha a la izquierda y opciones/paneles a la derecha. Capas, canales y trazados mantienen los componentes nativos de GIMP.
- Atajos: V mover, M selección rectangular, L lazo, W selección difusa, C recortar, I cuentagotas, J reparar, B pincel, S clonar, E goma, G degradado, P trazados, T texto, Z zoom, Q máscara rápida. Shift permite acceder a algunas herramientas alternativas.
- Ctrl+T transformación unificada; Ctrl+J capa mediante copiar; Ctrl+D quitar selección; Ctrl+0 encajar imagen; Ctrl+1 tamaño real; Ctrl+Alt+I tamaño de imagen; Ctrl+Alt+C tamaño del lienzo.
- **Capa → Nuevo grupo de ajuste**, también disponible en el menú contextual de capas y con Ctrl+Alt+Shift+A. Crea un grupo vacío Pass through encima de la primera capa seleccionada, en su mismo nivel. Aplicar desde Colores un filtro no destructivo, por ejemplo niveles o curvas, dejando desmarcada la opción de fusionar el filtro. El efecto alcanza las capas inferiores dentro del ámbito de composición. Si se agregan capas al grupo, su comportamiento cambia al de un grupo ordinario con filtros: no usarlo como carpeta de contenido.
- Ctrl+Alt+Shift+D abre la consola de errores.

Ctrl+G ejecuta **Agrupar capas seleccionadas**: mueve las capas a un grupo Pass through, conserva su orden en la pila y selecciona el grupo. Cuando la selección atraviesa carpetas, se usa el ancestro común más cercano. Si están seleccionados un grupo y sus descendientes, se mueve el grupo completo una sola vez. Los bloqueos de posición impiden la operación. Deshacer restaura las posiciones anteriores en un solo paso. Las capas no contiguas quedan juntas; esto cambia su relación con las capas intermedias. Mover contenido fuera de un grupo con máscaras/filtros también puede cambiar la composición, igual que al reorganizarlo manualmente.

Ctrl+J ejecuta **Capa mediante copiar**. Con una selección de píxeles y una sola capa raster copia la región seleccionada a una capa encima de la original, conservando las coordenadas, la forma y los bordes suaves. Duplica la máscara de capa y la recorta de manera alineada; reutiliza la duplicación de filtros permanentes de GIMP y conserva opacidad/modos. El original y el portapapeles no cambian. La selección permanece activa. Sin selección, o al seleccionar varias capas, un grupo o una capa editable de texto, vector o contenido vinculado, duplica la estructura completa sin rasterizarla. «Duplicar capas» sigue disponible por separado. GIMP no duplica algunos filtros temporales o basados en herramientas; no se garantiza equivalencia completa de todas las pilas de efectos con Photoshop.

Ctrl+S conserva XCF y exportar sigue siendo una operación separada. Los perfiles ya creados conservan sus atajos; para probar los nuevos Ctrl+G y Ctrl+J, usar un perfil nuevo o asignarlos desde Preferencias. Ctrl+J debe apuntar a layers-copy-selection para copiar píxeles seleccionados.

## Probar el perfil sin compilar

Requiere una instalación confiable de GIMP 3.2.x y Python 3. El programa se ejecuta en la máquina local. El lanzador no descarga archivos ni accede a servicios.

```powershell
python .\design\launch.py --prepare-only
python .\design\launch.py --executable 'C:\ruta\a\gimp-3.2.exe'
```

```bash
python3 design/launch.py --executable /ruta/a/gimp-3.2 imagen-local.png
```

El perfil predeterminado se guarda en `_design-profile`, separado de la configuración habitual. Cada archivo se copia sólo si falta; los cambios realizados desde GIMP se conservan. `--profile` admite otro directorio vacío o creado por este lanzador. Se rechazan directorios ajenos, enlaces simbólicos y destinos que no sean archivos. Ejecutar una sola instancia por perfil y cerrar GIMP antes de moverlo o compararlo.

Los registros están en `_design-logs`. Pueden incluir rutas de documentos; revisarlos antes de compartirlos. Para volver a GIMP normal, cerrar la instancia y usar el lanzador habitual del sistema. Para probar valores iniciales actualizados sin perder ajustes, usar otro directorio vacío con `--profile`.

## Compilar en Linux

El sistema de compilación sigue siendo Meson/Ninja upstream. Consultar la [guía oficial](https://developer.gimp.org/core/setup/build/) y las versiones mínimas en meson.build; los paquetes cambian entre distribuciones.

En Ubuntu 26.04 se prepararon estos paquetes de desarrollo:

```bash
sudo apt-get update
sudo apt-get install --no-install-recommends \
  build-essential meson ninja-build pkg-config gettext gobject-introspection \
  libgirepository1.0-dev libgtk-3-dev libgegl-dev libbabl-dev libexiv2-dev \
  libgexiv2-dev libjson-glib-dev liblcms2-dev libmypaint-dev mypaint-brushes \
  libappstream-dev libarchive-dev libxmu-dev libxfixes-dev libtiff-dev \
  libjpeg-dev libpng-dev liblzma-dev libbz2-dev librsvg2-dev \
  libpoppler-glib-dev poppler-data libunwind-dev xsltproc xvfb xauth \
  python3-gi python3-cairo python3-gi-cairo glib-networking \
  desktop-file-utils appstream-util
git submodule update --init
sudo localedef -i en_US -f UTF-8 en_US.UTF-8
bash design/build-linux.sh
```

La compilación inicial desactiva dependencias opcionales no necesarias para las primeras pruebas. Eso limita formatos/complementos opcionales; no es la configuración definitiva de distribución. Mantiene PSD, JPEG, PNG, XCF y el motor nativo. `glib-networking` es una dependencia exigida por upstream; instalarla para compilar no añade una función en línea del fork. Todavía falta retirar o restringir rutas de red heredadas antes de entregar el producto local final.

El script usa dos trabajos, símbolos de depuración, `_build` y `_install`, ejecuta las pruebas antes de instalar y desactiva comprobación de actualizaciones. Permite `BUILD_DIR`, `INSTALL_PREFIX` y `BUILD_JOBS`. No instalar en /usr ni sobre GIMP del sistema.

El generador de idiomas requiere un locale UTF-8 distinto de C/POSIX. Instalar el paquete `locales` si falta para el comando localedef. `BUILD_LOCALE` permite usar otro locale generado; el valor predeterminado en el build es en_US.UTF-8. Esto no fija el idioma del programa.

```bash
BUILD_JOBS=4 bash design/build-linux.sh
bash design/run-linux.sh
```

El lanzador Linux prepara las rutas de bibliotecas y módulos. El equivalente explícito es:

```bash
LD_LIBRARY_PATH="$PWD/_install/lib/x86_64-linux-gnu:$PWD/_install/lib" \
GI_TYPELIB_PATH="$PWD/_install/lib/x86_64-linux-gnu/girepository-1.0" \
BABL_PATH="$(pkg-config --variable=libdir babl-0.1)/babl-0.1" \
GEGL_PATH="$(pkg-config --variable=libdir gegl-0.4)/gegl-0.4" \
  python3 design/launch.py --executable "$PWD/_install/bin/gimp-3.2"
```

Meson puede elegir otro libdir; comprobarlo en su resumen y ajustar LD_LIBRARY_PATH. Al lanzar desde el directorio de compilación usar `tools/in-build-gimp.py` upstream y sus instrucciones en lugar de mezclar bibliotecas instaladas con las compiladas.

El build local usa GEGL/babl de la distribución. Sus módulos de conversión/procesamiento deben encontrarse en esas rutas, aunque GIMP se instale en un prefix propio. No entregar un paquete sin estos módulos: perderlos puede provocar fallos y conversiones lentas sin SIMD.

## Desarrollo desde Windows con WSL

Git local está configurado con core.autocrlf=false y el repositorio fija LF para scripts. No usar conversiones CRLF sobre scripts ejecutados en Linux. Conservar los permisos de ejecución al clonar directamente en Linux; una copia NTFS puede requerir revisar los permisos.

Para acelerar una compilación WSL, se usó una copia de trabajo en `/home/boowomp/.cache/gimp-linux-design/source` y el build en la carpeta `build` adyacente. El fork editable permanece en Windows. **Sincronizar cambios antes de recompilar:** la caché no se actualiza sola. Si se eliminan/renombran fuentes, recrear una copia de trabajo limpia para no conservar fuentes antiguas.

```bash
rsync -a --exclude=.git --exclude=_build --exclude=_install \
  --exclude='_design-profile*' --exclude=_design-logs \
  /mnt/c/Users/valen/Documents/testing-loop/gimp/ \
  /home/boowomp/.cache/gimp-linux-design/source/
cd /home/boowomp/.cache/gimp-linux-design/source
BUILD_DIR=/home/boowomp/.cache/gimp-linux-design/build \
INSTALL_PREFIX=/mnt/c/Users/valen/Documents/testing-loop/gimp/_install \
BUILD_JOBS=4 bash design/build-linux.sh
```

Las rutas anteriores corresponden a esta máquina; en otro equipo usar su usuario y carpeta. Se conserva el build para compilaciones incrementales. /tmp resultó efímero entre sesiones WSL y no se usa como caché persistente.

## Depuración y aceptación

```bash
python3 design/test_profile.py
meson test -C _build --print-errorlogs
meson test -C _build core --print-errorlogs
bash design/smoke-linux.sh
```

Los siete checks del perfil comprueban preservación de preferencias, rechazo de perfiles ajenos/destinos incorrectos, acciones/atajos, lista de herramientas, delimitadores y propiedades vigentes. Las pruebas nativas nuevas de core comprueban colocación del grupo de ajuste, jerarquía y undo/redo. Los tests de core no prueban fidelidad de color, composición de todos los filtros ni la experiencia con tableta.

Para un fallo nativo: conservar meson-logs/testlog.txt, registro del lanzador, commit, versión de dependencias y pasos reproducibles. Usar gdb sobre la compilación debugoptimized, sin enviar automáticamente informes a terceros.

Aceptación manual pendiente: iniciar en Linux, crear/abrir imagen, editar con pincel, transformar, duplicar, usar ajuste en grupo, deshacer/rehacer, guardar/reabrir XCF, exportar PNG/PSD y comparar. Probar teclado español, HiDPI, tableta y resoluciones 1280×720/1920×1080. Comparar CPU/RAM con upstream usando exactamente los mismos archivos. No presentar el perfil como clon completo ni build de pruebas como paquete listo para distribución.

Los resultados de esta primera etapa están en [VERIFICACION.md](VERIFICACION.md). El mapa para localizar las siguientes funciones está en [MAPA_CODIGO.md](MAPA_CODIGO.md).
