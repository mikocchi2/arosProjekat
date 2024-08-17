# Program za Razmenu Tekstualnih Poruka između Procesa

Ovaj program omogućava razmenu tekstualnih poruka između dva procesa koristeći deljenu memoriju (engl. shared memory) i semafore. Program je napisan u programskom jeziku C i omogućava dvosmernu komunikaciju između dva korisnika (terminala).

## Funkcionalnosti

- **Dvosmerna komunikacija:** Dva procesa mogu razmenjivati poruke, pri čemu jedan korisnik piše poruku, a drugi je čita, i obrnuto.
- **Deljena memorija:** Program koristi deljenu memoriju za skladištenje poruka koje procesi razmenjuju.
- **Semafori:** Semafori se koriste za sinhronizaciju pristupa deljenoj memoriji, kako bi se izbegle trke podataka i osigurala pravilna komunikacija između procesa.
- **Nezavisni unos:** Program koristi neblokirajući unos, omogućavajući korisnicima da unose poruke u realnom vremenu dok čekaju na poruku drugog korisnika.

## Kako koristiti program

1. **Kompajliranje programa:**

   Za kompajliranje programa Make koristi komandu
   gcc -o chat chat.c -lrt -lpthread

2. **Upotreba:**

   Potrebno je otvoriti 2 terminala, i u svakom pokrenuti ./chat sa argumentima 1 ili 2
