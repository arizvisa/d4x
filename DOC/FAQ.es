               Preguntas M�s Frecuentes sobre Downloader for X

Autores de este documento :
Koshelev Maxim (mdem@chat.ru)

Traducci�n al castellano (6/1/2000, nt 1.0.9) :
Vicente Aguilar <vaguilar@linuxfreak.com> 


Informaci�n inicial

1.1 �Qu� es "Downlader for X"?
1.2 �D�nde se puede obtener "Downloader for X"?
1.3 Licencia de distribuci�n de "Downloader for X"
1.4 �Qu� versi�n de libc se necesita para compilar "Downloader for X"?
1.5 �He compilado "Downloader for X" pero da un segfault al ejecutarlo!

Preguntas b�sicas

2.1 �C�mo a�adir una nueva descarga?
2.2 Quiero detener dos descargas, �c�mo puedo hacerlo?
2.3 �C�mo puedo descargar todo un directorio de un servidor FTP?
2.4 �C�mo puedo descargar s�lo los ficheros jpeg de un servidor FTP?
2.5 La ventana principal contiene demasiada informaci�n para mi.
2.6 Hay demasiadas descargas ya finalizadas en la ventana principal de
    "Downloader for X". �C�mo las borro?

M�s preguntas

3.1 �Puede "Downloader for X" limitar la velocidad de descarga?
3.2 �Puede "Downloader for X" descargar directorios mediante HTTP?
3.3 Necesito limitar la cantidad de conexiones al servidor ftp.blabla.org.
    �C�mo puedo hacerlo?
3.4 Quiero guardar el cuaderno de bit�cora en un fichero, �c�mo lo hago?
3.5 Mi servidor proxy devuelve basura tras una desconexi�n. �C�mo hago para
    que no afecte a mis descargas?
3.6 He encontrado una URL mientras estaba en la consola. �C�mo puedo
    a�adirla en el Downloader?

*******************************************************************************
Respuestas:

1.1 �Qu� es "Downlader for X"?

"Downloader for X" es una herramienta para descargar ficheros de internet.
Soporta los protocolos FTP y HTTP, reconexiones autom�ticas despu�s de una
desconexi�n, reanudaci�n de descargas en el punto que se interrumpieron,
recursividad tanto en FTP como HTTP, ambos tipos de servidores proxy (FTP
wingate y HTTP squid) y muchas m�s caracter�sticas. Este programa resulta
muy �til para descargar ficheros usando conexiones lentas ya que el programa
las intentar� descargar una y otra vez hasta conseguirlo.

1.2 �D�nde se puede obtener "Downloader for X"?

http://www.krasu.ru/soft/chuchelo - P�gina web del "Downloader for X"

1.3 Licencia de distribuci�n de "Downloader for X"

Este programa es gratuito y "open source", pero a pesar de ello �NO ES GPL!
Usted �nicamente puede distribuir las fuentes sin modificar. Si realiza
algunas modificaciones y quiere distribuirlo p�blicamente DEBE contactar con
el autor.

1.4 �Qu� versi�n de libc se necesita para compilar "Downloader for X"?

"Downloader for X" NECESITA libc6 (glibc2.0) o superiores para compilar.
Lo sentimos pero este programa no funciona con libc5.

1.5 �He compilado "Downloader for X" pero da un segfault al ejecutarlo!

Por lo que sabemos, "Downloader for X" no funciona con versiones inestables
(de desarrollo) de libc y otras librer�as. �Aseg�rese de que est� usando
versiones estables de libc, gtk+ y pthreads! Otra posible causa es alguna
incompatibilidad si dispone de dos o m�s versiones distintas de una misma
librer�a en su sistema.

*******************************************************************************
Preguntas b�sicas

2.1 �C�mo a�adir una nueva descarga?

Primera forma : pulse Ctrl+N e introduzca la URL en la ventana de di�logo.
Segunda forma : pulse el primer bot�n por la izquierda de la barra de botones
                e introduzca la URL.
Tercera forma : arrastre un enlace desde el netscape y su�ltelo en la ventana
                principal del "Downloader for X".
Cuarta forma :  arrastre un enlace desde el netscape y su�ltelo en el "�cono
                DnD" que aparece al arrancar "Downloader for X" y que siempre
                est� encima de las dem�s ventanas.

2.2 Quiero detener dos descargas, �c�mo puedo hacerlo?

Selecciones las descargas a borrar pulsando con el rat�n sobre la primera y
pulsando sobre los dem�s mientras que mantiene pulsada la tecla 'Ctrl'.
Entonces pulse Alt+C o pulse en el bot�n de la papelera de la barra de
botones.

2.3 �C�mo puedo descargar todo un directorio de un servidor FTP?

S�mplemente a�ada una nueva descarga con la URL del directorio a descargar.
"Downloader for X" entender� que quiere descargar todo el directorio. Otra
forma es introduciendo la nueva descarga como URL/* donde URL es la direcci�n
del directorio FTP que quiere descargar.

2.4 �C�mo puedo descargar s�lo los ficheros jpeg de un servidor FTP?

"Downloader for X" admite m�scaras en las descargas de FTP. Puede introducir
URL/*.jpg o URL/*.j*g como direcci�n a descargar. Hay que advertir que el
esterisco (*) significa �al menos un s�mbolo!

2.5 La ventana principal contiene demasiada informaci�n para mi.

Vaya a la pesta�a "Columnas" del di�logo de Opciones (Ctrl+C) y elija las
columnas que quiere que se muestren.

2.6 Hay demasiadas descargas ya finalizadas en la ventana principal de
    "Downloader for X". �C�mo las borro?

Elija la opci�n 'Borrar finalizada(s)' del men� 'Descargas'. "Downloader for
X" borrar� autom�ticamente las descargas finalizadas si usted lo configura
as� con las opciones.
*******************************************************************************
M�s preguntas

3.1 �Puede "Downloader for X" limitar la velocidad de descarga?

"Downloader for X" tiene opciones para limitar la velocidad. Usted puede
configurar dos l�mites de velocidad en la carpeta 'Velocidad' del cuadro de
Opciones, y elegir entre ellas (o velocidad ilimitada) con los botones de la
barra de botones. Tambi�n puede establecer l�mites de velocidad para cada
descarga de forma individual en la carpeta 'Otros' de las propiedades de
cada descarga (o al a�adirlas).

3.2 �Puede "Downloader for X" descargar directorios mediante HTTP?

Desde la versi�n 1.05, "Downloader for X" puede interpretar ficheros html y
realizar descargas recursivas mediante HTTP. Pero hay que se�alar que
"Downloader for X" no modifica los ficheros html si no que los graba tal y
como son. Adem�s, "Downloader for X" no descargar� los enlaces a direcciones
absolutas que encuentre en los ficheros html.

3.3 Necesito limitar la cantidad de conexiones al servidor ftp.blabla.org.
    �C�mo puedo hacerlo?

"Downloader for X" le permite configurar l�mites en las conexiones para
cualquier servidor por cualquier puerto. Si alguna descarga al servidor a
limitar ya ha sido a�adida a "Downloader for X", s�mplemente pulse con el
bot�n derecho en ella, elija 'l�mites' y all� introduzca la cantidad m�xima
de conexiones permitidas. Puede ver todos los l�mites configurados en la
opci�n 'L�mites' del men� 'Opciones'.

3.4 Quiero guardar el cuaderno de bit�cora en un fichero, �c�mo lo hago?

Vaya al di�logo de Opciones (Ctrl+C) y en la carpeta 'Otros' configure el
modo en el que quiere guardar la bit�cora principal.

3.5 Mi servidor proxy devuelve basura tras una desconexi�n. �C�mo hago para
    que no afecte a mis descargas?

La caracter�stica 'vuelta atr�s' de "Downloader for X" se cre� especialmente
para estos proxys. Configure el valor de vuelta atr�s en el di�logo de
Opciones (Ctrl+C) sobre los 500-2000 bytes.

3.6 He encontrado una URL mientras estaba en la consola. �C�mo puedo
    a�adirla en el Downloader?

Desde la versi�n 1.08 puede ejecutar el downloader en l�nea de comandos con
una URL como par�metro, incluso si el programa ya est� en ejecuci�n.
