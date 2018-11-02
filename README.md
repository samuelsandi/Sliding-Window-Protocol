## Sliding-Window-Protocol
Milestone 1 Tugas Besar IF3130 Jaringan Komputer Institut Teknologi Bandung

# Dasar Teori
Komunikasi antar komputer merupakan hal yang sangat penting di abad 21 ini. Hampir semua
aplikasi yang kita gunakan sehari-hari merupakan aplikasi yang berkomunikasi dengan
komputer lain. Akan tetapi, komunikasi antar dua komputer tidak selalu berjalan sesuai yang
diinginkan. Komunikasi antar komputer melalui jaringan rentan mengalami gangguan sehingga
pesan yang dikirim tidak selalu bebas dari error. Untuk itu, perlu dilakukan deteksi terhadap
error pada pesan untuk menjamin tingkat kebenaran pesan.
Untuk mendeteksi error, pihak penerima pesan akan melakukan pemeriksaan checksum pada
setiap paket yang diterima. Jika terdapat error, maka penerima pesan akan meminta pengirim
untuk mengirim ulang paket tersebut. Akan tetapi, jika hal ini dilakukan secara serial (hanya
mengirim dan memeriksa satu paket pada satu waktu) akan menimbulkan masalah kinerja.
Solusi untuk permasalahan ini adalah dengan mengirim lebih dari satu paket dalam satu waktu.
Solusi ini telah lama dikenal dalam dunia jaringan komputer dengan nama Sliding Window
Protocol. Tujuan dari tugas besar ini adalah mempelajari cara kerja Sliding Window Protocol
dengan Selective Repeat Automatic Repeat Request.
Berikut ini adalah penjelasan singkat tentang protokol ini. Misal terdapat sender A dan receiver
B. A pertama-tama akan membaca data yang ingin dikirim dari file eksternal ke suatu buffer.
Misal ukuran window adalah 1 0. Setelah A mengirim paket pertama, A tidak perlu menunggu
ACK dari B untuk mengirim paket kedua sampai kesepuluh. Setiap paket yang dikirim memiliki
time out. Jika dalam durasi time out tersebut A belum mendapat ACK paket tertentu dari B, A
akan mengirim ulang paket tersebut. Demikian pula bila A mendapat ACK paket tertentu, tetapi
isi ACK tersebut menyatakan bahwa terdapat error dari paket yang dikirim, A akan mengirim
ulang paket tersebut. Untuk mengetahui paket mana saja yang sudah di-acknowledge, setiap
paket diberi nomor urut (sequence number). Di sisi penerima B, jika B menerima suatu paket,
maka B akan memeriksa apakah terdapat error dari paket yang diterima. Jika ada, maka B akan
mengirim NAK yang meminta paket tersebut dikirim ulang. Jika tidak ada error, B akan
mengirim ACK yang berisi nomor urut dari paket selanjutnya. Untuk lebih memahami Sliding
Window Protocol, silahkan lihat simulasinya di
http://www.ccs-labs.org/teaching/rn/animations/gbn_sr/. Teori yang dijelaskan pada bagian ini
hanyalah gambaran kasar saja. Peserta diminta untuk mengeksplorasi lebih jauh secara
mandiri.

# Spesifikasi Tugas
Program yang akan dibuat terdiri dari dua file, yaitu sender dan receiver. Implementasi
diwajibkan menggunakan bahasa C/C++ dengan protokol UDP. Program sender akan
membaca suatu file dan mengirimnya ke receiver dengan menggunakan Sliding Window
Protocol. Program receiver akan menerima data yang dikirim dan menuliskan file tersebut ke file
system.
Berikut ini adalah format dari paket (frame) yang dikirim.

| SOH(0x1)  | Sequence Number | Data length | Data | Checksum |
| ------------- | ------------- | ------------- | ------------- | ------------- |
| 1 byte  | 4 byte  | 4 byte  | Max 1024 byte  | 1 byte  |

Berikut ini adalah format dari ACK yang dikirim.

| ACK  | Next Sequence Number | Checksum |
| ------------- | ------------- | ------------- |
| 1 byte  | 4 byte  | 1 byte  |

Untuk mensimulasikan packet loss, anda dapat menggunakan tools pumba yang detailnya
dapat dilihat pada https://github.com/alexei-led/pumba.
Untuk memudahkan penilaian, silahkan gunakan format berikut untuk menjalankan program.
. /sendfile <filename> <windowsize> <buffersize> <destination_ip> <destination_port>
. /recvfile <filename> <windowsize> <buffersize> <port>
