               Preguntas Más Frecuentes sobre Downloader for X

Autores de este documento :
Koshelev Maxim (mdem@chat.ru)

Traducción al castellano (6/1/2000, nt 1.0.9) :
Vicente Aguilar <vaguilar@linuxfreak.com> 


Información inicial

1.1 ¿Qué es "Downlader for X"?
1.2 ¿Dónde se puede obtener "Downloader for X"?
1.3 Licencia de distribución de "Downloader for X"
1.4 ¿Qué versión de libc se necesita para compilar "Downloader for X"?
1.5 ¡He compilado "Downloader for X" pero da un segfault al ejecutarlo!

Preguntas básicas

2.1 ¿Cómo añadir una nueva descarga?
2.2 Quiero detener dos descargas, ¿cómo puedo hacerlo?
2.3 ¿Cómo puedo descargar todo un directorio de un servidor FTP?
2.4 ¿Cómo puedo descargar sólo los ficheros jpeg de un servidor FTP?
2.5 La ventana principal contiene demasiada información para mi.
2.6 Hay demasiadas descargas ya finalizadas en la ventana principal de
    "Downloader for X". ¿Cómo las borro?

Más preguntas

3.1 ¿Puede "Downloader for X" limitar la velocidad de descarga?
3.2 ¿Puede "Downloader for X" descargar directorios mediante HTTP?
3.3 Necesito limitar la cantidad de conexiones al servidor ftp.blabla.org.
    ¿Cómo puedo hacerlo?
3.4 Quiero guardar el cuaderno de bitácora en un fichero, ¿cómo lo hago?
3.5 Mi servidor proxy devuelve basura tras una desconexión. ¿Cómo hago para
    que no afecte a mis descargas?
3.6 He encontrado una URL mientras estaba en la consola. ¿Cómo puedo
    añadirla en el Downloader?

*******************************************************************************
Respuestas:

1.1 ¿Qué es "Downlader for X"?

"Downloader for X" es una herramienta para descargar ficheros de internet.
Soporta los protocolos FTP y HTTP, reconexiones automáticas después de una
desconexión, reanudación de descargas en el punto que se interrumpieron,
recursividad tanto en FTP como HTTP, ambos tipos de servidores proxy (FTP
wingate y HTTP squid) y muchas más características. Este programa resulta
muy útil para descargar ficheros usando conexiones lentas ya que el programa
las intentará descargar una y otra vez hasta conseguirlo.

1.2 ¿Dónde se puede obtener "Downloader for X"?

http://www.krasu.ru/soft/chuchelo - Página web del "Downloader for X"

1.3 Licencia de distribución de "Downloader for X"

Este programa es gratuito y "open source", pero a pesar de ello ¡NO ES GPL!
Usted únicamente puede distribuir las fuentes sin modificar. Si realiza
algunas modificaciones y quiere distribuirlo públicamente DEBE contactar con
el autor.

1.4 ¿Qué versión de libc se necesita para compilar "Downloader for X"?

"Downloader for X" NECESITA libc6 (glibc2.0) o superiores para compilar.
Lo sentimos pero este programa no funciona con libc5.

1.5 ¡He compilado "Downloader for X" pero da un segfault al ejecutarlo!

Por lo que sabemos, "Downloader for X" no funciona con versiones inestables
(de desarrollo) de libc y otras librerías. ¡Asegúrese de que está usando
versiones estables de libc, gtk+ y pthreads! Otra posible causa es alguna
incompatibilidad si dispone de dos o más versiones distintas de una misma
librería en su sistema.

*******************************************************************************
Preguntas básicas

2.1 ¿Cómo añadir una nueva descarga?

Primera forma : pulse Ctrl+N e introduzca la URL en la ventana de diálogo.
Segunda forma : pulse el primer botón por la izquierda de la barra de botones
                e introduzca la URL.
Tercera forma : arrastre un enlace desde el netscape y suéltelo en la ventana
                principal del "Downloader for X".
Cuarta forma :  arrastre un enlace desde el netscape y suéltelo en el "ícono
                DnD" que aparece al arrancar "Downloader for X" y que siempre
                está encima de las demás ventanas.

2.2 Quiero detener dos descargas, ¿cómo puedo hacerlo?

Selecciones las descargas a borrar pulsando con el ratón sobre la primera y
pulsando sobre los demás mientras que mantiene pulsada la tecla 'Ctrl'.
Entonces pulse Alt+C o pulse en el botón de la papelera de la barra de
botones.

2.3 ¿Cómo puedo descargar todo un directorio de un servidor FTP?

Símplemente añada una nueva descarga con la URL del directorio a descargar.
"Downloader for X" entenderá que quiere descargar todo el directorio. Otra
forma es introduciendo la nueva descarga como URL/* donde URL es la dirección
del directorio FTP que quiere descargar.

2.4 ¿Cómo puedo descargar sólo los ficheros jpeg de un servidor FTP?

"Downloader for X" admite máscaras en las descargas de FTP. Puede introducir
URL/*.jpg o URL/*.j*g como dirección a descargar. Hay que advertir que el
esterisco (*) significa ¡al menos un símbolo!

2.5 La ventana principal contiene demasiada información para mi.

Vaya a la pestaña "Columnas" del diálogo de Opciones (Ctrl+C) y elija las
columnas que quiere que se muestren.

2.6 Hay demasiadas descargas ya finalizadas en la ventana principal de
    "Downloader for X". ¿Cómo las borro?

Elija la opción 'Borrar finalizada(s)' del menú 'Descargas'. "Downloader for
X" borrará automáticamente las descargas finalizadas si usted lo configura
así con las opciones.
*******************************************************************************
Más preguntas

3.1 ¿Puede "Downloader for X" limitar la velocidad de descarga?

"Downloader for X" tiene opciones para limitar la velocidad. Usted puede
configurar dos límites de velocidad en la carpeta 'Velocidad' del cuadro de
Opciones, y elegir entre ellas (o velocidad ilimitada) con los botones de la
barra de botones. También puede establecer límites de velocidad para cada
descarga de forma individual en la carpeta 'Otros' de las propiedades de
cada descarga (o al añadirlas).

3.2 ¿Puede "Downloader for X" descargar directorios mediante HTTP?

Desde la versión 1.05, "Downloader for X" puede interpretar ficheros html y
realizar descargas recursivas mediante HTTP. Pero hay que señalar que
"Downloader for X" no modifica los ficheros html si no que los graba tal y
como son. Además, "Downloader for X" no descargará los enlaces a direcciones
absolutas que encuentre en los ficheros html.

3.3 Necesito limitar la cantidad de conexiones al servidor ftp.blabla.org.
    ¿Cómo puedo hacerlo?

"Downloader for X" le permite configurar límites en las conexiones para
cualquier servidor por cualquier puerto. Si alguna descarga al servidor a
limitar ya ha sido añadida a "Downloader for X", símplemente pulse con el
botón derecho en ella, elija 'límites' y allí introduzca la cantidad máxima
de conexiones permitidas. Puede ver todos los límites configurados en la
opción 'Límites' del menú 'Opciones'.

3.4 Quiero guardar el cuaderno de bitácora en un fichero, ¿cómo lo hago?

Vaya al diálogo de Opciones (Ctrl+C) y en la carpeta 'Otros' configure el
modo en el que quiere guardar la bitácora principal.

3.5 Mi servidor proxy devuelve basura tras una desconexión. ¿Cómo hago para
    que no afecte a mis descargas?

La característica 'vuelta atrás' de "Downloader for X" se creó especialmente
para estos proxys. Configure el valor de vuelta atrás en el diálogo de
Opciones (Ctrl+C) sobre los 500-2000 bytes.

3.6 He encontrado una URL mientras estaba en la consola. ¿Cómo puedo
    añadirla en el Downloader?

Desde la versión 1.08 puede ejecutar el downloader en línea de comandos con
una URL como parámetro, incluso si el programa ya está en ejecución.
