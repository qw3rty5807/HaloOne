# Halo One

Halo One is a very compact and lightweight DIY PCVR headset that features:
- Dual 1600x1600 90Hz LCD panels,
- 6DoF tracking using PS3 Move cameras(coming soon),
- Fresnel lenses,
- Compatibility with SteamVR,
- Raspberry Pi Pico 2 microcontroller and LSM6DS3 IMU board,
- Comfortable halo strap,
- Very easy and user friendly software setup.

![1](Images/headset1.jpg)
Only assembled casing, for illustration purposes.

> [!NOTE]
> ### Software Configuration
> To use Halo One, you need to install the latest Halo Launcher release and open it. The launcher will automatically install and configure everything on your PC.
> 
> To configure the microcontroller board:
> 1. Download the `Pico2_Firmware` folder/file from this repository.
> 2. Install the latest version of **Arduino IDE**.
> 3. Open Arduino IDE, go to **Boards Manager**, and install the **"Raspberry Pi Pico/RP2040/RP2350"** board package by Earle F. Philhower, III.
> 4. Go to **Library Manager**, search for **FastIMU** by LiquidCGS, and install it.
> 5. **Important (Entering Bootloader Mode):** Press and hold the physical **BOOTSEL** button on your Raspberry Pi Pico 2, and while holding it, plug the Pico 2 into your computer via USB. Then release the button.
> 6. Open the `Pico2_Firmware.ino` file in Arduino IDE (**File >> Open** or `CTRL+O`).
> 7. Select **Raspberry Pi Pico 2** as your target board in the top-left menu (or via Tools -> Board).
> 8. Click the **Upload** button. Everything should compile and flash successfully!

> [!TIP]
> Currently, this headset setup has a relatively narrow FOV. I am actively working on updated optics for a future revision. If you prefer a wide FOV, please stay tuned for future updates!

## Bill Of Materials

- 1x Raspberry Pi Pico 2
- 1x LSM6DS3 IMU board
- 2x Fresnel Lenses (d40mm, fl28mm)
- 2x 1600x1600 @ 90Hz Displays
- 1x Display Driver Board
- 25x M2 (6mm) self-tapping screws
- All 3D printed parts (available in `/3D Models`)
- Double-sided tape
- Stock Meta Quest 2 head strap (without the top strap)

## Third-Party Components and Credits

This project incorporates and builds upon open-source assets created by the 3D printing community:

* **Halo Mount:** Based on Bigscreen Beyond 2 halo mount by **EloyCabañero** ([Printables](https://www.printables.com/model/1602107-halo-mount/collections)) — *Public Domain / CC0*
* **Strap Adapters:** Modified Quest 2 adapters by **Val's Virtuals**, originally based on *Bigscreen Beyond strap Hook* by **Val's Virtuals** ([Printables](https://www.printables.com/model/585216-bigscreen-beyond-strap-hook)) — *CC BY 4.0*
 



