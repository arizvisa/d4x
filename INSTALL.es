MANUAL DE INSTALACIÓN

1.0	¿Qué se necesita para compilar este programa?

Para compilar este programa usted necesita las librerías GTK versión 1.2.0
o posteriores, una librería con threads seguros (safethreads, por ejemplo
libc6/glib2) y por supuesto el código fuente del programa.
El autor ha probado el programa en RedHat 5.x, RedHat 6.0 y Slackware 3.6.
(El traductor, en Debian 2.1)

1.1	Compilar el programa.

Vaya al directorio con los fuentes (main) y escriba 'make install' como
root, por supuesto.
Si tiene problemas compilando, asegúrese de que:
-tiene libc6 (glib2) o posteriores
-tiene GTK+ versión 1.2.0 o posteriores (www.gtk.org)
-tiene GNU gettext (si no, comente la línea correspondiente del Makefile)

1.2	¿Dónde puedo encontrar el ejecutable después de compilar?

En el directorio 'main' de los fuentes. El ejecutable se llama 'nt'.
'make install' también coloca todos los ficheros en /usr/local/bin y
/usr/local/share/locale de forma que usted puede ejecutarlo desde allí.
Si no necesita las traducciones, puede utilizar tan sólo el ejecutable.

1.3	¿Cómo uso el programa?

Ejecútelo, obsérvelo y lo comprenderá.
