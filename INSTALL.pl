                             Downloader dla X

                            PODRÊCZNIK INSTALACJI


1.0	Czego potrzebujesz, aby skompilowaæ ten program?

Do kompilacji potrzebujesz GTK o wersji nowszej ni¿ 1.2.0, bibliotek dla
bezpiecznych w±tków (tj. libc6) i oczywi¶cie ¼ród³a programu.
Ja (autor) testowa³em stare wersje tego programu na dystrybucjach RedHat 5.x,
RedHat 6.0 i Slackware 3.6. Ale niestety najnowsze wersje Downloader'a dla X s±
stabilne tylko z libpthread-0.8 i wy¿ej.

1.1	Kompilacja programu.

Przejd¼ do katalogu ze ¼ród³ami i wykonaj polecenie 'make install' oczywi¶cie
jako root;
Je¶li masz problemy z kompilacj±, upewnij siê, czy:
 - posiadasz libc6 lub lepsz±
 - posiadasz pthread-0.8 lub wy¿ej
 - posiadasz GTK+ w wersji 1.2.0 lub nowsz± (www.gtk.org)
 - posiadasz gettext (je¶li nie chcesz tego to poprostu usuñ jedn± liniê z
   Makefile)

Je¶li Twoj± dystrybucj± jest Mandrake, RedHat lub SuSE, to mo¿esz zainstalowaæ
plik RPM, który mo¿esz odnale¼æ na stronie domowej tego programu
(http://www.krasu.ru/soft/chuchelo).

1.2	Gdzie znajduje siê plik binarny po skompilowaniu?

Plik binarny mo¿esz znale¼æ w katalogu 'main' w strukturze ¼róde³. Bêdzie mia³
on nazwê 'nt'. Nie pytaj mnie dlaczego on tak zosta³ nazwany. Ale ta nazwa jest
krótka (³atwa do wpisania) i ³atwa do zapamiêtania. Tak wiêc my¶lê, ¿e nazwa ta
jest ca³kiem odpowiednia dla u¿ytecznego programu.

Wykonanie 'make install' umie¶ci wszystkie pliki w /usr/local/bin i
/user/local/share/locale, aby¶ móg³ uruchomiæ go z /usr/local/bin. Je¶li nie
potrzebujesz lokalizacji, mo¿esz u¿ywaæ tylko pliku wykonywalnego...

1.3	Jak u¿ywaæ programu?

Poprostu uruchom go i przygl±dnij siê mu, a zrozumiesz wszystko.
