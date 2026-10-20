# Halo One

Halo One to bardzo kompaktowy i lekki headset DIY PCVR, który oferuje:
- Podwójne ekrany LCD 1600x1600 90Hz,
- 6DoF z użyciem kamer PS3 Move(wkrótce),
- Soczewki Fresnela,
- Kompatybilność z  SteamVR,
- Raspberry Pi Pico 2 oraz płytkę IMU LSM6DS3,
- Komfortowy pasek halo,
- Bardzo prosta i przyjazna konfiguracja softu.

![1](Images/headset1.jpg)
Sama złożona obudowa, w celach ilustracyjnych.

> [!NOTE]
> ### Konfiguracja Softu
> By użyć Halo One, musisz zainstalować mój, oficjalny Halo Launcher w najnowszej wersji, launcher automatycznie zainstaluje i skonfiguruje większosć rzeczy.
> 
> Żeby skonfigurować mikrokontroler musisz:
> 1. Pobrać folder `Pico2_Firmware` z mojego repozytorium.
> 2. Zainstaluj najnowszą wersję **Arduino IDE**.
> 3. Otwórz Arduino IDE, w zakladce **Boards Manager**, zainstaluj paczkę **"Raspberry Pi Pico/RP2040/RP2350"**  stworzoną przez Earle F. Philhower, III.
> 4. W zakładce **Library Manager**, wyszukaj **FastIMU** stworzone przez LiquidCGS, i zainstaluj je.
> 5. **Bardzo ważne! (Włączenie trybu bootloadera):** Naciśnij i przytrzymaj fizyczny przycisk **BOOTSEL** na twoim Raspberry Pi Pico 2, w trakcie tego, podłącz Pico 2 do swojego komputera poprzez USB. następnie póść przycisk BOOTSEL.
> 6. Otwórz plik `Pico2_Firmware.ino` w Arduino IDE (**File >> Open** lub `CTRL+O`).
> 7. Wybierz **Raspberry Pi Pico 2** jako płytkę docelową w rozwijanej liscie w lewym górnym rogu  (lub za pomocą Tools >> Board).
> 8. Naciśnij przycisk **Upload**. Wszystko powinno się udać!

> [!TIP]
> Obecnie, ten headset ma dość mały FOV, będę pracować nad aktualizacją w przyszłej wersji headsetu. Jeśli chcesz mieć duży FOV, poczekaj na przyszłą akutalizacje!

## Lista częśći

- 1x Raspberry Pi Pico 2
- 1x Płytka IMU LSM6DS3
- 2x Soczewki Fresnela (d40mm, fl28mm)
- 2x Ekrany 1600x1600 @ 90Hz
- 1x Display Driver Board
- 25x M2 (długość: 6mm) samogwintujące śubki
- Wszystkie części z drukarki 3D (pobierz z `/3D Models`)
- Taśma dwustronna
- Stockowa gumowa opaska z questa 2, bez górnej części.

## Komponenty i uznanie stron trzecich

Projekt ten uwzględnia i rozwija zasoby open-source stworzone przez społeczność zajmującą się drukiem 3D:

* **Halo Mount:** Na podstawie Bigscreen Beyond 2 halo mount autorstwa **EloyCabañero** ([Printables](https://www.printables.com/model/1602107-halo-mount/collections)) — *Public Domain / CC0*
* **Strap Adapters:** Modified Quest 2 adapters autorstwa **Val's Virtuals**, originally based on *Bigscreen Beyond strap Hook* by **Val's Virtuals** ([Printables](https://www.printables.com/model/585216-bigscreen-beyond-strap-hook)) — *CC BY 4.0*
 
