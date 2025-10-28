# UTS-K.A.P

anggota kemmpok : 
fayed yamani : 2022071017
daffa bintang pratama : 2022077002
Razan Aubin Farras : 2022071043



Smart Home IoT – Sistem Monitoring dan Kontrol Rumah
Sistem ini mengintegrasikan ESP32, sensor DHT11 dan Ultrasonik, serta aktuator servo dan relay untuk mengatur pintu dan lampu rumah melalui MQTT dashboard berbasis web.


 
tools hardware :
ESP32 DevKit v1
Sensor DHT11 (Suhu & Kelembapan)
Sensor Ultrasonik HC-SR04 (Jarak / Gerbang Garasi)
Relay 1 Channel (Lampu)
Servo SG90 (Pintu Otomatis)
LCD I2C 16x2
Broker MQTT: test.mosquitto.org
Dashboard: dash.html (Web MQTT)

wiring table :
| Komponen     | Pin ESP32 | Keterangan     |
| ------------ | --------- | -------------- |
| DHT11        | GPIO 25   | Data           |
| HC-SR04 Trig | GPIO 19   | Output         |
| HC-SR04 Echo | GPIO 18   | Input          |
| Relay        | GPIO 26   | Aktuator lampu |
| Servo        | GPIO 13   | Aktuator pintu |
| LCD SDA      | GPIO 21   | I2C            |
| LCD SCL      | GPIO 22   | I2C            |


Arsitektur Sistem :
ESP32 kirim data sensor → Broker MQTT
Dashboard Web terima & tampilkan data
Dashboard kirim perintah → Broker MQTT → ESP32 jalankan aksi


topik mosquito :
| Topik                           | Arah      | Deskripsi                | Payload Contoh |
| ------------------------------- | --------- | ------------------------ | -------------- |
| `/smart/home/data/suhu_lembap`  | Publish   | Data suhu dan kelembapan | `28.5,70`      |
| `/smart/home/data/garasi/jarak` | Publish   | Data jarak ultrasonik    | `125`          |
| `/smart/home/kontrol/lampu`     | Subscribe | Kontrol relay lampu      | `ON` / `OFF`   |
| `/smart/home/kontrol/pintu`     | Subscribe | Kontrol servo pintu      | `0`–`180`      |



Cara Pengoperasian :
Dashboard menampilkan suhu, kelembapan, dan jarak.

Klik tombol “Lampu ON/OFF” untuk menyalakan/mematikan relay.

Atur slider untuk membuka pintu (servo).

Data real-time ditampilkan di LCD & dashboard.


Uji Coba Akhir
Pastikan semua fungsi berikut benar-benar jalan:
 Sensor suhu & kelembapan publish data
 Sensor ultrasonik publish data
 Relay bisa dikontrol dari dashboard
 Servo bisa dikontrol dari dashboard
 LCD menampilkan suhu, kelembapan, jarak, status lampu/pintu
 Dashboard tampil real-time
