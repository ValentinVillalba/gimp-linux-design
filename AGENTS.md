# Instrucciones para este fork

- Leer design/DECISIONES.md, design/PLAN.md y design/README.md antes de proponer cambios.
- Investigar la versión estable, upstream y código libre reutilizable antes de implementar cada capacidad. Revisar licencia y atribución por archivo.
- Mantener el núcleo nativo GTK/GEGL/babl y Meson; priorizar cambios pequeños sin nuevas dependencias salvo necesidad demostrada.
- El destino es Linux. Windows es un entorno de desarrollo. Los scripts Linux deben conservar LF.
- No incorporar IA, modelos, servicios en línea, cuentas, telemetría, recursos de Adobe ni código propietario. Las rutas de red heredadas deben revisarse antes de una entrega final.
- Conservar configuración personal, undo/redo, XCF y mecanismos de depuración. Añadir pruebas significativas a cambios de comportamiento del núcleo.
- Registrar decisiones importantes y límites de verificación en design/DECISIONES.md y el estado en design/PLAN.md.
- El fork Windows es la fuente editable; la copia de compilación WSL necesita sincronización. Conservar los directorios de build para iteración incremental.
- No enviar contenido generado con IA a repositorios administrados por GIMP; su política lo excluye. Este es un fork independiente.
- No publicar paquetes, prometer paridad Photoshop ni declarar funcionamiento totalmente offline sin verificar las pruebas correspondientes.
