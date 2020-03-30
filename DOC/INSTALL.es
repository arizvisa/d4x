MANUAL DE INSTALACI�N

1.0	�Qu� se necesita para compilar este programa?

Para compilar este programa usted necesita las librer�as GTK versi�n 1.2.0
o posteriores, una librer�a con threads seguros (safethreads, por ejemplo
libc6/glib2) y por supuesto el c�digo fuente del programa.
El autor ha probado el programa en RedHat 5.x, RedHat 6.0 y Slackware 3.6.
(El traductor, en Debian 2.1)

1.1	Compilar el programa.

Vaya al directorio con los fuentes (main) y escriba 'make install' como
root, por supuesto.
Si tiene problemas compilando, aseg�rese de que:
-tiene libc6 (glib2) o posteriores
-tiene GTK+ versi�n 1.2.0 o posteriores (www.gtk.org)
-tiene GNU gettext (si no, comente la l�nea correspondiente del Makefile)

1.2	�D�nde puedo encontrar el ejecutable despu�s de compilar?

En el directorio 'main' de los fuentes. El ejecutable se llama 'nt'.
'make install' tambi�n coloca todos los ficheros en /usr/local/bin y
/usr/local/share/locale de forma que usted puede ejecutarlo desde all�.
Si no necesita las traducciones, puede utilizar tan s�lo el ejecutable.

1.3	�C�mo uso el programa?

Ejec�telo, obs�rvelo y lo comprender�.
