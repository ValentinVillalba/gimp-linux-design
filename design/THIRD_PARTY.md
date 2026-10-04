# Código y configuración reutilizados

## GIMP

Base GIMP_3_2_6, e101dd19b165f927d3ba0a74658a71537c5661b9. Fuente canónica: https://gitlab.gnome.org/GNOME/gimp; espejo: https://github.com/GNOME/gimp. Las licencias originales están en LICENSE y COPYING; las bibliotecas y los recursos tienen licencias específicas que se conservan.

gimp-data está en fe4ecc0bf70fc8ff3bd929ce6abdd50fb5621072, el commit fijado por upstream. Para obtenerlo en Windows se usó el espejo https://github.com/GNOME/gimp-data porque la ejecución de sh por Git estaba bloqueada en el entorno restringido. Se conservó .gitmodules upstream.

## PhotoGIMP

Autoría: Diolinux y colaboradores. Fuente: https://github.com/Diolinux/PhotoGIMP.
Commit consultado: eca3a8f57b9944c063d043ce7c07524107b5292d.
Licencia: GNU GPL v3; copia íntegra en licenses/PhotoGIMP-COPYING.

- etc/toolrc reutiliza la organización de .config/GIMP/3.0/toolrc.
- etc/shortcutsrc adapta selectivamente asociaciones de teclas y acciones, eliminando repeticiones y evitando asociaciones sin equivalencia clara.
- La disposición de paneles se inspira en PhotoGIMP y reutiliza la estructura del sessionrc de GIMP, con identificadores actuales, sin geometría ni datos de sus monitores.
- No se incorporaron splash, iconos de marca, perfiles ICC, rutas Flatpak ni su instalador.

La consulta local completa está fuera del fork, en ../research/PhotoGIMP. No es una dependencia del programa ni del proceso de compilación.

## Otras referencias consultadas

Se revisó yousei3/gimp-photoshop-layer-workflow (https://github.com/yousei3/gimp-photoshop-layer-workflow), commit e66ca20657af0c80f29c880cf9f9266de9d4b86a, como referencia de agrupar/desagrupar. Su README declara GNU GPL v3; no hay archivo LICENSE en ese checkout. No se copió ni instaló su implementación Python. La operación del fork usa las API nativas y el código existente de GIMP. La referencia local está en testing-loop/research/gimp-layer-workflow; no forma parte del build.

## Candidatos pendientes

GEGL Effects de LinuxBeaver, darktable/RawTherapee y los cambios de PSD/CMYK de GIMP upstream son candidatos para investigación posterior. No se incorporaron y sus licencias/compatibilidad deben revisarse por archivo antes de hacerlo. No hay código de Adobe, Photopea ni bibliotecas propietarias nuevas.
