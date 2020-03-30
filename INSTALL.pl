PODR�CZNIK INSTALACJI

1.0	Czego potrzebujesz, aby skompilowa� ten program?

Do kompilacji potrzebujesz GTK o wersji nowszej ni� 1.2.0,
bibliotek dla bezpiecznych w�tk�w (tj. libc6) i oczywi�cie �r�d�a programu.
Ja (autor) testowa�em ten program na RedHat 5.x, RedHat 6.0 i
Slackware 3.6;

1.1	Kompilacja programu.

Przejd� do katalogu z �r�d�ami i wykonaj polecenie 'make install' oczywi�cie
jako root;
Je�li masz problemy z kompilacj�, upewnij si�, �e:
-posiadasz libc6 lub lepsz�
-posiadasz GTK+ w wersji 1.2.0 lub nowsz� (www.gtk.org)
-posiadasz gettext (je�li nie chcesz tego to poprostu usu� jeden ci�g znak�w
z Makefile)

1.2	Gdzie znajduje si� plik binarny po kompilacji?

W katalogu 'main' w strukturze �r�de�. Znajduje si� pod nazw� 'nt'.
Wykonanie 'make install' umie�ci wszystkie pliki w /usr/local/bin
i /user/local/share/locale, aby� m�g� uruchomi� go z /usr/local/bin.
Je�li nie potrzebujesz lokalizacji, mo�esz u�ywa� tylko pliku
wykonywalnego...

1.3	Jak u�ywa� programu?

Poprostu uruchom go i przygl�dnij si� mu, a zrozumiesz wszystko.
