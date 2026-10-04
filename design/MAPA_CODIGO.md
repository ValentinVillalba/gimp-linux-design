# Mapa del código para continuar el fork

## Interfaz y configuración

- etc/toolrc: orden y grupos de herramientas. app/tools/gimp-tools.c valida su versión, exige las herramientas conocidas y hace fallback a la configuración del sistema.
- etc/sessionrc: paneles iniciales; app/gui/session.c lo lee y guarda. Los identificadores de dockables deben corresponder a app/dialogs/dialogs.c; por ejemplo gimp-path-list, no el antiguo gimp-vectors-list de perfiles externos.
- etc/gimprc.in: valores iniciales configurados por Meson. app/config/gimpguiconfig.c, gimpcoreconfig.c y gimpdisplayoptions.c definen propiedades y valores válidos.
- etc/shortcutsrc: asociaciones iniciales. app/menus/menus.c prioriza el archivo personal y usa el instalado sólo si el personal no existe. app/menus/shortcuts-rc.c usa el parser nativo y detecta conflictos.
- app/widgets/gimpaction.c y gimpactiongroup.c: acciones y valores predeterminados. app/actions/tools-actions.c deriva nombres de acciones desde los identificadores de herramientas; no inventar nombres.
- menus/image-menu.ui.in.in y layers-menu.ui: menú Capa y contextual. Los textos visibles pertenecen a las acciones en app/actions/, con gettext.

## Primer acceso a grupos de ajuste

- app/actions/layers-actions.c: registro, tooltip y condiciones de activación.
- app/actions/layers-commands.c: layers_new_adjustment_group_cmd_callback. Usa únicamente primitivas nativas; no traslada capas ni procesa píxeles al crear el grupo.
- app/core/gimpgrouplayer.c: modo efectivo y optimización strength reduction de grupos Pass through. No cambiar su composición por una aproximación gráfica.
- app/core/gimpdrawablefilter.c y app/gegl/: filtros no destructivos y aplicación GEGL. Revisar estas rutas antes de crear máscaras o estilos nuevos.
- app/tests/test-core.c: pruebas de undo/redo y colocación en grupos anidados.

## Funciones por investigar

- app/core/gimpimage.c y gimpitemtree.c: selección, jerarquía, inserción y reordenamiento de capas. Base para agrupar selecciones; debe respetar orden, grupos, bloqueos y undo.
- app/core/gimp-edit.c y app/actions/edit-commands.c: copiar/pegar selección, buffers y offsets. Base para «capa mediante copiar selección», conservando el portapapeles cuando corresponda.
- app/core/gimplinklayer.c y app/path/gimpvectorlayer.c: fuentes vinculadas y capas vectoriales existentes. Reutilizar antes de diseñar nuevos tipos de capa.
- app/text/ y app/tools/gimptexttool.c: texto editable y UI. Comparar funciones concretas sin sustituir Pango.
- plug-ins/file-psd/: lector/escritor y estructuras PSD. Revisar límites, descriptors y capacidades upstream antes de copiar cambios de master.
- app/xcf/: persistencia XCF. Toda nueva estructura debe guardar/reabrir sin pérdida y tener pruebas de compatibilidad.
- app/paint/, app/operations/ y app/gegl/: procesamiento local. Mantener cachés y cálculo por regiones; evitar motores paralelos.

## Rutas locales/Internet pendientes de auditoría

- app/gimp-update.c y opción Meson check-update: actualización automática, desactivada en este fork.
- app/widgets/gimphelp.c y acciones de ayuda: referencias web y ayuda en línea heredadas. El perfil las desactiva como valores iniciales, pero debe revisarse el código antes de declarar bloqueo total de red.
- app/file/ y GIO: apertura de URI remotas, independientemente de la interfaz.
- plug-ins/ y módulos GEGL externos: extensiones pueden tener capacidades adicionales; una política de producto local exige revisar qué se distribuye y cómo se ejecuta.

## Pruebas

app/tests/meson.build muestra las pruebas realmente habilitadas. Varios tests antiguos de UI están comentados por upstream: que la suite pase no significa que esos tests se hayan ejecutado. design/test_profile.py cubre seguridad/preservación del perfil, no eventos de teclado reales. design/smoke-linux.sh valida arranque GTK en pantalla virtual y lectura del perfil, no calidad visual.
