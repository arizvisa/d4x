PODRÊCZNIK INSTALACJI

1.0	Czego potrzebujesz, aby skompilowaæ ten program?

Do kompilacji potrzebujesz GTK o wersji nowszej ni¿ 1.2.0,
bibliotek dla bezpiecznych w±tków (tj. libc6) i oczywi¶cie ¼ród³a programu.
Ja (autor) testowa³em ten program na RedHat 5.x, RedHat 6.0 i
Slackware 3.6;

1.1	Kompilacja programu.

Przejd¼ do katalogu z ¼ród³ami i wykonaj polecenie 'make install' oczywi¶cie
jako root;
Je¶li masz problemy z kompilacj±, upewnij siê, ¿e:
-posiadasz libc6 lub lepsz±
-posiadasz GTK+ w wersji 1.2.0 lub nowsz± (www.gtk.org)
-posiadasz gettext (je¶li nie chcesz tego to poprostu usuñ jeden ci±g znaków
z Makefile)

1.2	Gdzie znajduje siê plik binarny po kompilacji?

W katalogu 'main' w strukturze ¼róde³. Znajduje siê pod nazw± 'nt'.
Wykonanie 'make install' umie¶ci wszystkie pliki w /usr/local/bin
i /user/local/share/locale, aby¶ móg³ uruchomiæ go z /usr/local/bin.
Je¶li nie potrzebujesz lokalizacji, mo¿esz u¿ywaæ tylko pliku
wykonywalnego...

1.3	Jak u¿ywaæ programu?

Poprostu uruchom go i przygl±dnij siê mu, a zrozumiesz wszystko.
