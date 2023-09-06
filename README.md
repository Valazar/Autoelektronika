# Parking_senzor
-Projekat iz Autoelektronike - Parking senzor
## Uvod
-Zadatak ovog projekta je realizacija parking senzora, čiji detaljan opis je dat u priloženom pdf-u. Projekat je realizovan uz pomoć RTOS-a u VisalStudio2019. Takođe tokom pisanja koda ispoštovana su pravila MISRA standarda.
## Kratko objašnjenje projekta
-Podaci sa dva senzora (levi i desni) se dobijaju automatski na svakih 200ms sa kanala 0 i 1 koje je potrebno kalibrisati u dvije tačke uz pomoć poruke(npr KALIBRACIJA_200mm_10% KALIBRACIJA_2000mm_10%).

-Procentualne udaljenosti se računaju na osnovu očitavanja senzora i kalibracije

-Sa kanala 2 vršimo slanjeporuka PARK, REVERSE, NEUTRAL i DRIVE čime simuliramo automatski mjenjač i sistem treba da bude aktivan samo za stanje REVERSE.

-Na ledBar-u prva dioda u nultoj koloni predstavlja prekidač, ona se ostavlja da bude ulazna i njenim pritiskanjem vrši se paljenje prve diode u prvoj koloni dok treća kolona predstavlja signalnu i broj upaljenih dioda na njoj određuje procentualnu udaljenost od najbliže detekcije jednog od senzora.

-Zone i procentualne vrednosti sa oba senzora se štampaju na kanalu 2 na svakih 5s.

## Korišćene periferije
-Periferije koje smo koristili su AdvUniCom i LED_bar. 
 
-Pokretanje LED_bar: za ispravno pokretanje ove periferije za naš kod, u terminalu je potrebno uneti komandu LED_bars_plus.exe rRR, da bi nulti stubac bio tasteri (ulazni), a prvi i drugi diode (izlazni).
 
-Pokretanje AdvUniCom: u terminalu je potrebno uneti komande AdvUniCom.exe 0 (ili dvoklikom otvoriti ovaj kanal jer se on automatski pali), AdvUniCom.exe 1, i AdvUniCom.exe 2,da bismo imali sva 3 kanala.

 ## Testiranje sistema
-Prvo što je potrebno je pokrenuti sve periferije na gore pokrenuti način prije pokretanja sistema. Kada se sistem pokrene on je inicijalno u uključenom stanju.

-Prvo je potrebno sistem dovesti u neko od stanja PARK, NEUTRAL i DRIVE. Time se zaustavlja slanje podataka ka serijskoj komunikaciji i sistem se isključuje. Da bi ponovo uključili sistem potrebno je poslati poruku REVERSE preko kanala 2 ili to uraditi uz pomoć diode u nultoj koloni. Ako se sistem iskljuli uz pomoć neke od poruka on se može upaliti samo sa porukom REVERSE.

-Zatim se na kanalu 0 šalje primjer poruke KALIBRACIJA_200mm_10%(gdje vrijednost u milimetrima može ići od 100 do 999 a procenata od 10 do 99). Isto radimo i na kanalu 1.

-Zatim se na kanalu 0 ponovo šalje šalje primjer poruke za kalibraciju u dvije tačke KALIBRACIJA_1000mm_100%(gdje vrijednost u milimetrima može ići od 1000 do 9999 a procente zadržavamo na 100%). Isto radimo i na kanalu 1.

-Nakon toga šaljemo vrijednosti sa senzora 1 i 2 i vidimo da se u zavisnosti od bliže detekcije vrši paljenje signalnih dioda u trećoj koloni led bar-a.

-Takođe na serijskom kanalu 2 vrši se ispis udaljenosti kao i zone detekcije(NEMA_DETEKCIJE ukoliko je udaljenost veća od 100% i KONTAKT_DETEKCIJA ukoliko je udaljenost manja od 20%).
## Korišćeni taskovi
-main_demo:
funkcija u kojoj su inicijalizovane sve periferije, definisane interrupt rutine za serijsku komunikaciju, kreirani semafori, tajmeri, redovi i svi taskovi, i na kraju pozvan vTaskStartScheduler() koji aktivira planer za raspoređivanje taskova.

-Takođe na serijskom kanalu 2 vrši se ispis udaljenosti kao i zone detekcije(NEMA_DETEKCIJE ukoliko je udaljenost veća od 100% i BLISKA_DETEKCIJA ukoliko je udaljenost manja od 20%).

-RXC_isr_0:
funkicija koja očitava koja vrednost je pristigla sa kanala 0, to jest, senzora 1. kao i vrijednosti kalibracije i proslijeđuje ih taskovima za obradu.

-RXC_isr_1:
slično kao prethodna funkcija, samo obrađuje podatke za senzor 2 sa kanala 1.

-PC_Receive_task:
funkcija koja očitava da li je unijeto PARK, REVERSE, DRIVE ili NEUTRAL.

Serial0Send_Task:
funkcija koja šalje "S1" na kanal 0, radi omogućavanja automatskog očitavanja poruke na svakih 200ms.

-Serial1Send_Task:
slično kao prethodna, samo što šalje "S2" na kanal 1.

-Serial2Send_Task:
funkcija koja na kanal 2 štampa vrijednosti sa senzora 1 i 2 kao i tip udaljenosti(NEMA_DETEKCIJE/BLISKA_DETEKCIJA).

-LEDBar_Task:
učitava da li je pritisnut taster prekidač.

-task_ukljuci_iskljuci:
vrši uključivanje/uključivanje sistema u zavisnosti od pritiska tastera/poslate poruke.

-task_obrada_kalibracija1:
vrši obradu podataka za kalibraciju sa kanala 0 za senzor 1.

-task_obrada_kalibracija2:
vrši obradu podataka za kalibraciju sa kanala 1 za senzor 2.

-task_obrada_senzori:
vrši obradu podataka sa senzora 1 i 2 i vrši obradu u zavisnosti od izmjerene udaljenosti.

-diode_on:
uključuje diode u trećoj koloni u zavisnosti od toga kolika je udaljenost izmjerena na najbližem senzoru.
 
