# Všeobecné požiadavky

Cieľom semestrálnej práce je vytvoriť sieťovú aplikáciu s využitím socketov. V semestrálnej práci:
* musia byť použité **sockety** na medziprocesnú komunikáciu,
* musia byť použité **vlákna**,
* musí byť riešený **synchronizačný problém**,

Ďalej musí semestrálna práca spĺňať nasledujúce požiadavky:
* implementácia v jazyku C/C++,
* návrh v súlade s **princípmi objektového programovania** (hlavne zapuzdrenie a modularita),
* žiadne úniky pamäte (toto je nutné explicitne ukázať s využitím nástrojov, ktoré sú na to určené),
* kód musí byť logicky rozčlenený do .h a .c/.cpp súborov,
* semestrálna práca musí obsahovať aj **vlastný jednoduchý Makefile**, pomocou ktorého bude možné zostaviť jednotlivé časti semestrálnej práce (napr. klient a server). Makefile musí obsahovať **minimálne nasledujúce ciele:**
  * server,
  * client,
  * all,
  * clean,
* **pri vývoji musí byť využitý verzionovací systém** (preferovane GitLab):
  * v rámci verzionovacieho systému musia byť dokladované aspoň **2 príspevky od každého študenta** a musí byť vykonaný aspoň **1 merge vetví projektu**,
  * pred obhajobou semestrálnej práce je nutné vyučujúcemu, u ktorého budete prácu obhajovať, udeliť prístupové právo **Reporter**,
* so semestrálnou prácou odovzdávate:
  * programátorskú dokumentáciu (podľa zverejnenej šablóny), v ktorej popíšete:
    * štruktúru projektu (s využitím UML),
    * kde a ako ste použili sockety,
    * kde a ako ste použili vlákna,
    * aký synchronizačný problém ste riešili a aké prostriedky ste využili pri jeho riešení,
    * ďalšie kľúčové problémy, ktoré ste v rámci semestrálnej práce riešili,
  * používateľskú dokumentáciu, v ktorej popíšete spustenie a ovládanie jednotlivých častí aplikácie,
  * celá semestrálna práca bude zbalená do jedného .zip súboru, ktorý odovzdáte prostredníctvom aktivity na moodli; v .zip súbore budú **iba**:
    * .h súbory,
    * .c/.cpp súbory,
    * Makefile,
    * **ostatné súbory nevyhnutné pre kompiláciu a spustenie aplikácie (textové, grafické a pod.),**
    * programátorská a používateľská dokumentácia.


# Download manažér.

Klientska aplikácia, ktorá dovolí používateľovi pripojiť sa na vzdialený server a pomocou štandardných protokolov stiahnuť obsah dostupný na serveri. Manažér musí podporovať nasledujúce protokoly:

* http, https, ftp, ftps, 
* bittorent (+ 1 študent),

pričom umožní:
* naplánovať čas, kedy sa má sťahovanie začať,
* ukončiť sťahovanie,
* spravovať lokálne adresáre,
* spravovať históriu sťahovania,
* definovať prioritu sťahovania,
* v prípade súčasného sťahovania viacerých súborov ich naozaj sťahovať súčasne a nie postupne.

Pri protokoloch, ktoré to dovoľujú, aplikácia tiež umožní:
* pozastaviť alebo obnoviť sťahovanie súboru.

Poznámka: aplikácia nesmie byť implementovaná s použitím nástroja Wget a iných podobných nástrojov slúžiacich na sťahovanie internetového obsahu.
