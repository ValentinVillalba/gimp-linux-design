# Funcionamiento local: auditoría de rutas heredadas

Fecha: 2026-10-04. Estado: auditoría en curso. Este documento no certifica aislamiento completo de red.

El objetivo es que el editor use archivos y herramientas locales, sin servicios, cuentas, IA ni funciones en línea. Desactivar actualizaciones y ayuda en un perfil no basta: se deben retirar las entradas nativas, los complementos y los lanzamientos de navegador que todavía permiten acceso remoto.

## Restricciones implementadas

La política interna `file_utils_require_native` comprueba `g_file_is_native` antes de buscar cargadores, consultar metadatos o ejecutar procedimientos. La comprobación no hace I/O bloqueante. Se aplica al parser de nombres/URI, apertura, guardado, búsqueda de procedimientos de archivo y argumentos GFile de la PDB. Esta última entrada también protege las llamadas directas a cargadores, incluido XCF, antes de la validación normal del parámetro y aunque la bandera de validación estuviera desactivada.

`file-remote.c` ya no contiene transporte GIO/GVfs ni NSURLSession. Se mantienen sus cuatro funciones internas para devolver G_IO_ERROR_NOT_SUPPORTED a cualquier llamada antigua; no montan volúmenes, descargan, suben ni preparan archivos temporales. Los selectores GTK se configuran sólo para rutas locales/nativas y se retiró «Abrir ubicación» del menú principal. Abrir un archivo normal y escribir una ruta en el diálogo Abrir siguen disponibles.

Se conservan rutas de plataforma y URI file:// nativas. Se rechazan URI no nativas, incluidas HTTP(S), FTP, SFTP, SMB y trash://. No hay un interruptor para volver a habilitar el transporte remoto en el fork.

Una ruta nativa puede apuntar a un sistema de archivos de red montado por el sistema operativo. Esta política no distingue esos montajes ni bloquea sockets del proceso o de terceros. Los complementos Python/C pueden ejecutar su propio código; una PDB restringida no constituye una sandbox para ellos. Estos límites deben considerarse al preparar la distribución final.

## Inventario y trabajo pendiente

| Superficie | Evidencia en el código | Estado |
| --- | --- | --- |
| Abrir/guardar URI remotas | app/file/{file-open,file-save,file-utils}.c | Restringido a rutas nativas |
| Descarga/subida/montaje interno | app/file/file-remote.c | Transporte retirado; errores explícitos |
| Cargadores llamados directamente | app/pdb/gimpprocedure.c y app/plug-in/gimppluginmanager-file.c | Argumentos GFile remotos rechazados antes de invocar |
| Selector de archivos | app/widgets/gimpfiledialog.c | local-only activado; falta aceptación visual |
| Actualizaciones | app/gimp-update.c, meson_options.txt | Desactivadas en el build actual; revisar eliminación del código/controles heredados |
| Ayuda remota/fallback de navegador | app/widgets/gimphelp.c, preferencias, plug-ins/help | Desactivada por el perfil; pendiente retirar fallback y asegurar ayuda local |
| Navegador web | plug-ins/common/web-browser.c y su entrada Meson | Excluido de Meson; binarios antiguos retirados reversiblemente; falta adaptar llamadores de ayuda |
| Envío por correo | plug-ins/common/mail.c y su entrada Meson | Excluido de Meson; binarios antiguos retirados fuera del árbol de búsqueda |
| Enlaces del diálogo Acerca de | app/dialogs/about-dialog.c | Enlaces manejados sin lanzamiento externo; panel de descarga retirado; pendiente aceptación visual |
| Enlaces del diálogo de error crítico | app/widgets/gimpcriticaldialog.c | Puede abrir navegador para bugs/descargas; pendiente mantener diagnóstico local |
| Extensiones y recursos externos de formatos | app/core/gimpextension.c, cargadores SVG/PDF y complementos | Revisar URI, referencias incrustadas y comportamiento de bibliotecas; no auditado por completo |
| Herramientas externas y complementos añadidos por el usuario | sistema de plug-ins, RAW y scripts | Requieren delimitar soporte y comprobar ejecución; no se afirma aislamiento |

Los URL de licencia y documentación en comentarios no son conexiones por sí mismos. Se conservan la procedencia, las licencias y el acceso al código fuente. La descarga de dependencias durante el desarrollo tampoco es una función del editor final.

## Verificación

Tres casos nativos cubren rutas nativas/URI file:// y el rechazo de varios esquemas; apertura y guardado sin modificar el archivo asociado de la imagen; los cuatro antiguos transportes; y llamadas PDB gimp-file-load y gimp-xcf-load. La suite de guardar/exportar y XCF sigue pasando con archivos locales.

Faltan trazas de red del programa instalado y de sus procesos hijos, pruebas de los diálogos y recursos referenciados por documentos. Una búsqueda de símbolos o la ausencia de errores en el arranque no prueba aislamiento completo.

Fuente primaria consultada: https://docs.gtk.org/gio/method.File.is_native.html. Su contrato distingue ruta nativa de almacenamiento necesariamente local y especifica que la comprobación no hace I/O bloqueante.
