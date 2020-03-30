                             Downloader dla X

                            PODR�CZNIK INSTALACJI


1.0	Czego potrzebujesz, aby skompilowa� ten program?

Do kompilacji potrzebujesz GTK o wersji nowszej ni� 1.2.0, bibliotek dla
bezpiecznych w�tk�w (tj. libc6) i oczywi�cie �r�d�a programu.
Ja (autor) testowa�em stare wersje tego programu na dystrybucjach RedHat 5.x,
RedHat 6.0 i Slackware 3.6. Ale niestety najnowsze wersje Downloader'a dla X s�
stabilne tylko z libpthread-0.8 i wy�ej.

1.1	Kompilacja programu.

Przejd� do katalogu ze �r�d�ami i wykonaj polecenie 'make install' oczywi�cie
jako root;
Je�li masz problemy z kompilacj�, upewnij si�, czy:
 - posiadasz libc6 lub lepsz�
 - posiadasz pthread-0.8 lub wy�ej
 - posiadasz GTK+ w wersji 1.2.0 lub nowsz� (www.gtk.org)
 - posiadasz gettext (je�li nie chcesz tego to poprostu usu� jedn� lini� z
   Makefile)

Je�li Twoj� dystrybucj� jest Mandrake, RedHat lub SuSE, to mo�esz zainstalowa�
plik RPM, kt�ry mo�esz odnale�� na stronie domowej tego programu
(http://www.krasu.ru/soft/chuchelo).

1.2	Gdzie znajduje si� plik binarny po skompilowaniu?

Plik binarny mo�esz znale�� w katalogu 'main' w strukturze �r�de�. B�dzie mia�
on nazw� 'nt'. Nie pytaj mnie dlaczego on tak zosta� nazwany. Ale ta nazwa jest
kr�tka (�atwa do wpisania) i �atwa do zapami�tania. Tak wi�c my�l�, �e nazwa ta
jest ca�kiem odpowiednia dla u�ytecznego programu.

Wykonanie 'make install' umie�ci wszystkie pliki w /usr/local/bin i
/user/local/share/locale, aby� m�g� uruchomi� go z /usr/local/bin. Je�li nie
potrzebujesz lokalizacji, mo�esz u�ywa� tylko pliku wykonywalnego...

1.3	Jak u�ywa� programu?

Poprostu uruchom go i przygl�dnij si� mu, a zrozumiesz wszystko.
