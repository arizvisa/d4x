          Často pokládané otázky FAQ pro Downloader pro X

Autoři tohoto dokumentu FAQ:
Koshelev Maxim (mdem@chat.ru)

Úvodní informace

1.1 Co je to "Downloader pro X"
1.2 Kde mohu najít "Downloader pro X"
1.3 Jakou má distribuční licenci "Downloader pro X"?
1.4 Jaká verze knihovny libc je třeba pro kompilaci programu
	"Downloader pro X"?
1.5 Zkompiloval jsem "Downloader pro X", ale ten způsobil segfault ihned
	po spuštění!

Základní otázky

2.1 Jak mohu přidat novou adresu URL pro stahování?
2.2 Chci zastavit dvě stahování, jak to mohu udělat?
2.3 Jak mohu stáhnout adresář ze serveru ftp?
2.4 Jak mohu stáhnout jen soubory jpeg ze serveru ftp?
2.5 Hlavní okno programu "Downloader pro X" obsahuje pro mne příliš mnoho
	informací.
2.6 V okně "Downloader pro X" zůstává mnoho dokončených stahování. Jak je
    mohu odstranit?

Další otázky

3.1 Může "Downloader pro X" omezovat rychlost pro stahování?
3.2 Může "Downloader pro X" stahovat adresáře HTTP?
3.3 Potřebuji omezit počet spojení k serveru ftp.blahblahblah.org.
    Jak to mohu provést v programu "Downloader pro X"?
3.4 Chci zapsat hlavní záznam do souboru, umí to "Downloader pro X"?
3.5 Můj proxy server vrací nesmysly před přerušením spojení. Co mohu
    udělat pro úspěšné stahování?
3.6 Našel jsem adresu URL na konzoli, jak ji mohu přidat do Downloaderu?
3.7 V okně záznamu vidím nesmysly namísto čitelného textu!
3.8 Když stahuji soubory, Downloader vytváří celou adresářovou strukturu!
	Jak mohu tuto vlastnost vypnout?

Odpovědi:

*******************************************************************************
Úvodní informace

1.1 Co je to "Downloader pro X"

"Downloader pro X" je nástroj pro stahování souborů z internetu. Podporuje
oba protokoly ftp a http, automatické obnovení spojení po jeho přerušení,
pokračování ve stahování, rekurzi pomocí http a ftp, oba typy proxy
serverů: ftp (wingate) a http (squid) a mnoho dalších vlastností.
Tento program bude velmi užitečný pro stahování souborů na pomalých
linkách, protože "Downloader pro X" se bude pokoušet stáhnout soubor znovu
a znovu.

1.2 Kde mohu najít "Downloader pro X"

http://www.krasu.ru/soft/chuchelo - domácí stránka programu
"Downloader pro X"

1.3 Jakou má distribuční licenci "Downloader pro X"?

Tento program je zdarma a dokonce poskytuje otevřené zdrojové texty, ale
není to GPL! Můžete distribuovat pouze nezměněné zdrojové texty. Pokud
učiníte nějaké změny a chcete je veřejně vydat, MUSÍTE kontaktovat autora.

1.4 Jaká verze knihovny libc je třeba pro kompilaci programu
	"Downloader pro X"?

"Downloader pro X" potřebuje pro kompilaci libc6 (nebo glibc2.0) a vyšší.
Je mi líto, ale tento program nepodporuje libc5.

1.5 Zkompiloval jsem "Downloader pro X", ale ten způsobil segfault ihned
	po spuštění!

Pokud vím, tak "Downloader pro X" nefunguje dobře na nestabilních verzích
libc a dalších knihoven. Měli byste se ujistit, že máte stabilní
verze libc, gtk+ a pthreads! Mohou zde také být nějaké problémy ve vztahu
k dvěma nebo více odlišným verzím jedné knihovny nainstalované na vašem
systému současně. Počínaje verzí 1.06 "Downloader pro X" nepracuje
s knihovnou libpthreads, která je starší než verze 0.8.

*******************************************************************************
Základní otázky

2.1 Jak mohu přidat novou adresu URL pro stahování?

První způsob: stisknete Ctrl+N a zadáte adresu URL v otevřeném dialogu.
Druhý způsob: kliknete na první tlačítko v tlačítkové liště a zadáte
adresu URL.
Třetí způsob: přetáhnete odkaz z netscape a pustíte jej na program
"Downloader pro X"
Čtvrtý způsob: přetáhnete odkaz netscape a pustíte jen ikonu, která
je vždy navrchu, jakmile spustíte program "Downloader pro X".

2.2 Chci zastavit dvě stahování, jak to mohu udělat?

Vyberte stahování kliknutím myši na prvním a kliknutím se stisknutou
klávesou 'Ctrl' na druhém, pak stiskněte Alt+h nebo stiskněte tlačítko
s černým křížkem v červeném kolečku na tlačítkové liště.

2.3 Jak mohu stáhnout adresář ze serveru ftp?

Pouze přidejte nové stahování a adresou URL toho adresáře."Downloader
pro X" pochopí, že chcete automaticky stáhnout adresář (za použití
informací ze serveru).
Další způsob je, že zadáte jako nové stahování adresu URL/*, ve které je
URL ta adresa požadovaného adresáře ftp.

2.4 Jak mohu stáhnout jen soubory jpeg ze serveru ftp?

"Downloader pro X" podporuje masky pro stahování pomocí ftp. Můžete zadat
adresu URL/*.jpg nebo adresu URL/*.j*g, kde URL je adresa serveru ftp.
Všimněte si hvězdičky (*), která znamená alespoň jeden symbol!

2.5 Hlavní okno programu "Downloader pro X" obsahuje pro mne příliš mnoho
	informací.

Jděte do dialogu možností (Ctrl+C) a zvolte, které sloupce chcete vidět
ve složce sloupců.

2.6 V okně "Downloader pro X" zůstává mnoho dokončených stahování. Jak je
    mohu odstranit?

Vyberte 'Smazat dokončené' z menu 'Stahování'. "Downloader pro X" může
automaticky mazat dokončená stahování, pokud jej tak nastavíte
v Možnostech.
VAROVÁNÍ! Nenastavujte automatické odstranění dokončených stahování,
pokud stahujete rekurzivně pomocí HTTP (nebo rekurzivně z FTP pomocí
HTTP proxy). Pokud tuto vlastnost pro rekurzivní stahování HTTP, pak se
Downloader pokusí opakovaně stahovat stejné soubory mnohokrát.

*******************************************************************************
Další otázky

3.1 Může "Downloader pro X" omezovat rychlost pro stahování?

"Downloader pro X" obsahuje sadu vlastností pro omezení rychlosti. Můžete
nastavit dvě omezení rychlosti v menu Možnosti ve složce 'Rychlost' a
zvolit je tlačítky na tlačítkové liště stiskem odpovídajícího tlačítka.
Také můžete nastavit rychlostní omezení pro jedno konkrétní stahování
ve složce 'Jiné' vlastností (nebo, když jej přidáváte).

3.2 Může "Downloader pro X" stahovat adresáře HTTP?

Od verze 1.05 může "Downloader pro X" analyzovat soubory html a stahovat
rekurzivně pomocí HTTP. Uvědomte si ale, že "Downloader pro X" nemění
soubory html a ukládá je takové, jaké jsou. Také v tomto okamžiku
"Downloader pro X" nestahuje globální odkazy zjištěné v html.

3.3 Potřebuji omezit počet spojení k serveru ftp.blahblahblah.org.
    Jak to mohu provést v programu "Downloader pro X"?

"Downloader pro X" umožňuje nastavit omezení pro spojení na jakýkoli
server a jakýkoli port. Pokud je již stahování z požadovaného serveru
přidáno do programu "Downloader pro X", pouze na něm klikněte pravým
tlačítkem myši a zvolte 'omezení', pak zadejte maximální počet spojení
na příslušné místo. Všechna možní omezení můžete vidět v dialogu 'omezení'
z menu 'možnosti'.

3.4 Chci zapsat hlavní záznam do souboru, umí to "Downloader pro X"?

Jděte do dialogu Možnosti (Ctrl+C) a nastavte požadovaný způsob, jak
ukládat hlavní záznam v záložce 'Hlavní záznam'.

3.5 Můj proxy server vrací nesmysly před přerušením spojení. Co mohu
    udělat pro úspěšné stahování?

Vlastnost návrat zpět programu "Downloader pro X" byla vytvořena zvláště
pro takové proxy servery. Nastavte hodnotu návrat zpět v dialogu Možnosti
(Ctrl+C) na 500-2000 bajtů.

3.6 Našel jsem adresu URL na konzoli, jak ji mohu přidat do Downloaderu?

Počínaje verzí 1.08 můžete spustit Downloader s parametrem adresy URL,
i když je již downloader spuštěn.

3.7 V okně záznamu vidím nesmysly namísto čitelného textu!

Za prvé to není chyba programu "Downloader pro X". Můžete ji opravit dvěma
způsoby.
První: nastavte cestu k vašemu preferovanému písmu s pevnou šířkou před
cestu k 'misc' písmům ve vašem souboru XF86Config. Například:
FontPath    "/usr/X11R6/lib/X11/fonts/75dpi:unscaled"
FontPath    "/usr/X11R6/lib/X11/fonts/misc:unscaled"

Za druhé: vypněte vlastnost Downloaderu "Použít pevné písmo v záznamech".

3.8 Když stahuji soubory, Downloader vytváří celou adresářovou strukturu!
	Jak mohu tuto vlastnost vypnout?

Aby se vyhnul přepisování stažených souborů, Downloader vytváří
adresářovou strukturu, když stahuje pomocí protokolu HTTP (t.j. HTTP
proxy). Můžete vypnout rekurzi pro HTTP (a vytváření adresářů) nastavením
"hloubky rekurze" pro HTTP na '1' (bez rekurze).
