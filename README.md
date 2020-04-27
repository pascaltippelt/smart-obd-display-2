# Smart-OBD-Display Version 2
Hier soll dokumentiert werden, wie man ein OBD-Display für den Smart selber bauen kann. Als Basis kommen dazu das TTGO T4-Display mit ESP32 Mikrocontroller sowie die Arduino-Entwicklungsumgebung zum Einsatz.

Die Software-Basis für das Display wird bodmers [TFT_eSPI-Bibliothek](https://github.com/Bodmer/TFT_eSPI).

Die Software-Basis für die Kommunikation wird [PowerBroker2s ELMduino](https://github.com/PowerBroker2/ELMduino).

Als Hauptplatine verwende ich das [TTGO T4 Display](https://github.com/LilyGO/TTGO-T4-DEMO) ([Kaufen](https://www.banggood.com/LILYGO-TTGO-BTC-Ticker-ESP32-Module-Source-Bitcoin-Price-Ticker-Program-4-MB-SPI-Flash-4-MB-Psram-LCD-Display-p-1303223.html?rmmds=buy&cur_warehouse=CN) oder  [hier](https://www.tinytronics.nl/shop/en/platforms/ttgo/lilygo-ttgo-t4-v1.3-esp32-with-2.2-inch-tft-display). )

Zur Kommunikation mit dem Auto verwende ich einen preiswerten [ELM327-Klon mit WLan](https://amzn.to/2YamFiI)*. Die Entscheidung gegen Bluetooth wurde getroffen, da ich mit Bluetooth Probleme hatte die Werte sauber auszulesen. Außerdem macht anscheinend die einmalige Nutzung des Adapters mit der Android-Software Tourque die Konfiguration des Bluetooth Adapters dermaßen zunichte, dass eine Kommunikation mit dem ESP nicht mehr zustande kommt.

*: Achtung Link zu Amazon, gesponsort.
