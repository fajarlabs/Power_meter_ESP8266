Berikut adalah versi yang sudah dirapikan dan diformat sebagai README GitHub (`README.md`) untuk proyek Arduino + PZEM + Blynk:

---

# Smart Power Monitoring with PZEM-004T v3.0 and Blynk

Sistem ini berfungsi untuk memantau konsumsi daya listrik menggunakan sensor **PZEM-004T v3.0**, dengan tampilan data real-time melalui **Blynk Cloud**.

## 🧰 Library yang Dibutuhkan

Pastikan Anda menginstal library berikut pada Arduino IDE:

* [`ArduinoJson`](https://arduinojson.org/) versi **7.4.2**
* [`PZEM004Tv30`](https://github.com/mandulaj/PZEM-004T-v30) versi **1.2.1**

## ⚡ Alur Sistem

```
Input AC 220V --> Microcontroller (MCU) --> Beban Listrik
```

## 🛠️ Konfigurasi Awal

### 1. Registrasi ke Blynk Cloud

* Buka laman: [https://blynk.cloud/dashboard/814970/global/devices](https://blynk.cloud/dashboard/814970/global/devices)
* Tambahkan device baru dan catat informasi berikut dari Blynk:

```cpp
#define BLYNK_TEMPLATE_ID "TMPL6lx4adcu_"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "I8lNvlXbALwXfXtRwvDoCNQ5Vc7rT3yJ"
```

### 2. Konfigurasi Melalui Serial Monitor

Saat perangkat pertama kali dinyalakan (reboot), buka **Serial Monitor** di Arduino IDE dan tekan **Enter** untuk memulai konfigurasi interaktif:

1. Masukkan **SSID WiFi**, lalu tekan Enter
2. Masukkan **Password WiFi**, lalu tekan Enter
3. Masukkan **Tarif Daya Listrik (misal: 1444)**, lalu tekan Enter
4. Masukkan **Blynk Template ID**, lalu tekan Enter
5. Masukkan **Blynk Template Name**, lalu tekan Enter
6. Masukkan **Blynk Auth Token**, lalu tekan Enter

Setelah selesai, sistem akan mencoba terhubung ke WiFi yang sudah Anda konfigurasi.

### 3. Konfigurasi Melalui Format Langsung Serial

Sebagai alternatif, Anda bisa mengirimkan konfigurasi dengan format berikut ke Serial Monitor:

```text
<ssid:GANTI_DENGAN_SSID_WIFI>
<pass:GANTI_DENGAN_PASSWORD_WIFI>
<tdl:GANTI_DENGAN_TARIF_DASAR_LISTRIK>
<template_id:GANTI_DENGAN_TEMPLATE_ID_BLYNK>
<template_name:GANTI_DENGAN_TEMPLATE_NAME_BLYNK>
<auth:GANTI_DENGAN_AUTH_TOKEN_BLYNK>
```

Contoh:

```text
<ssid:MyWiFi>
<pass:12345678>
<tdl:1444>
<template_id:TMPL6lx4adcu_>
<template_name:Quickstart Template>
<auth:I8lNvlXbALwXfXtRwvDoCNQ5Vc7rT3yJ>
```

## 📡 Blynk Virtual Pin Mapping

| Virtual Pin | Parameter       |
| ----------- | --------------- |
| V0          | Power (Watt)    |
| V1          | Energy (kWh)    |
| V2          | Biaya (Rupiah)  |
| V3          | Tegangan (Volt) |
| V4          | Arus (Ampere)   |
| V5          | Frekuensi (Hz)  |
| V6          | Total Power     |
| V7          | Total Energy    |
| V8          | Total Biaya     |
| V9          | Saklar (0/1)    |

---

Silakan sesuaikan kode dengan kredensial dan tarif listrik sesuai kebutuhan Anda.
Jika ada pertanyaan atau masukan, silakan buat *issue* di repository ini. Terima kasih!

---

Jika kamu ingin, saya juga bisa bantu buatkan `README.md` file siap pakai.
