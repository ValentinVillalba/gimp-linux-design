# Verificación de la primera etapa

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
