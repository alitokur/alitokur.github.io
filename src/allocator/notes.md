
perf kurulu ama perf_event_paranoid = 4 ile kilitli. İstersen sen açabilirsin:

! sudo sysctl kernel.perf_event_paranoid=1

O zaman cache-miss sayaçlarını doğrudan görürüz. Şimdilik zamanla ölçelim.

---

Adım 5 — Cache locality: pool'un asıl kazancı

Önce neden önemli olduğunu netleştirelim. Senin makinen:

L1d  32 KB/çekirdek     ~1 ns
L2  512 KB/çekirdek     ~4 ns
L3   16 MB (paylaşımlı) ~15 ns
DRAM                    ~80 ns      ← 5M order buraya sığar, cache'e sığmaz

CPU belleği 64 byte'lık cache line'lar hâlinde okur. Tek bir price alanı istesen bile 64 byte gelir. Yani asıl soru şu: bir cache line'a kaç tane order sığıyor?

pool:        12 byte/nesne  →  64/12 = 5.3 nesne / cache line
new/delete:  32 byte/chunk  →  64/32 = 2.0 nesne / cache line
             (glibc 8 byte'lık isteği 32 byte'lık chunk'a yuvarlıyor)

Sırf yoğunluktan 2.7x daha az cache line. Üstüne bir de bitişiklik var: pool'da adresler ardışık olduğu için donanım prefetcher'ı bir sonraki satırı önceden çekebiliyor. Dağınık heap'te tahmin edemiyor.

Ölçelim:

  Ran 1 shell command

Sonuçlar öğretici — ve bir kısmı beklediğimden farklı. Bir deney daha yapayım, resim tamamlansın: boyutu değiştirip yoğunluğun ne zaman önem kazandığına bakalım (L3'ün 16 MB olduğunu hatırla):

  Ran 2 shell commands

Ölçüm sonuçları

SIRALI ERİŞİM (pointer vektörü üzerinden, ns/nesne)
  pool                        0.85
  new/delete (taze heap)      1.68     2.0x
  new/delete (parçalanmış)    4.51     5.3x   ← asıl fark burada

RASTGELE SIRA
  pool                       11.36
  new/delete (taze)          11.73     1.0x   ← avantaj yok
  new/delete (parçalanmış)   11.46     1.0x

DOĞRUDAN DEPOLAMA ÜZERİNDE GEZİNME (yalnızca pool'da mümkün)
  pool                        0.64

Üç şey öğrendik, biri beklediğim gibi değildi.

---

a) Sıralı erişimde oran ~2.2x, ve boyuttan bağımsız

Boyut taramasında 100 bin ile 8 milyon arasında oran hep 1.9–2.3x çıktı — pool L3'e sığıp new sığmadığı noktada bile bir sıçrama olmadı.

Sebebi şu: sıralı erişimde prefetcher her iki durumda da gecikmeyi gizliyor. Geriye kalan tek fark taşınan byte miktarı — yani iş latency-bound değil, bandwidth-bound. Ve bandwidth farkı doğrudan yoğunluk oranı: 32/12 = 2.67 teorik, 2.2x gözlenen.

Bu, "cache'e sığdırmak" sezgisinden farklı bir mekanizma. Sığdırmak değil, daha az byte taşımak kazandırıyor.

b) Rastgele erişimde avantaj tamamen kayboluyor

Üçü de ~11.4 ns. Rastgele atladığında her erişim zaten cache miss; bellekteki düzenin bir önemi kalmıyor.

Bu, pool'un dürüst sınırı: locality avantajı yalnızca öngörülebilir erişim deseninde var. Emir defteri taraması gibi sıralı işlerde kazanırsın; hash tablosu üzerinden rastgele emir arıyorsan kazanmazsın.

c) Parçalanma, ölçmediğin en büyük fark

Senin benchmark'ın new'i taze heap'te test ediyor — ve taze heap'te malloc da aslında arena'dan ardışık veriyor, yani neredeyse bitişik. Bu, malloc'a fazlasıyla cömert bir senaryo.

Uzun süre çalışan bir sistemde heap parçalanır. Simüle ettim (2N tahsis, her ikincisini serbest bırak, deliklere N tahsis): 4.51 ns — pool'un 5.3 katı.

Pool'un buradaki üstünlüğü performans değil yapısal: bir havuz tanım gereği parçalanamaz. Bu, ölçüme değil tasarıma bağlı bir garanti.

---

Bir de şunu fark ettim: is_free_ sana pahalıya mal oluyor

sizeof(object) = 12   →   cache line başına 5.3 nesne

O bool 1 byte, ama hizalama yüzünden slot'u 8'den 12 byte'a çıkarıyor. Free list'e geçip union kullanırsan:

union slot { order obj; size_t next_free; };   // 8 byte

doğrudan gezinme:
  fat  pool (12 B/slot, 34 MB)   0.63 ns
  slim pool ( 8 B/slot, 22 MB)   0.43 ns     ← 1.5x daha hızlı
  new/delete (32 B/chunk, 91 MB) 1.63 ns     ← slim'in 3.8 katı

Yani free list sadece allocate'i O(1) yapmıyor — is_free_'yi gereksiz kıldığı için cache yoğunluğunu da %50 artırıyor. İki kazanç aynı değişiklikten geliyor.

(Double-free tespitini kaybetmemek için is_free_ bit'lerini ayrı bir vector<uint64_t> bitmap'te tutabilirsin — sıcak veriden çıkmış olur.)

---

Benchmark'ına ne eklemeli

1. Erişim fazı — allocate + deallocate'ten sonra tüm nesneleri bir kez oku ve ayrı ölç. Gerçek kazancın orada.
2. Parçalanmış heap varyantı — yoksa malloc'u en iyi gününde test ediyorsun.
3. Erişim desenini açıkça yaz — "sıralı" mı "rastgele" mi, çünkü sonuç tamamen buna bağlı.

---

Sıradaki adım sence ne olsun: break düzeltmesi mi, free list mi, yoksa benchmark'a bu fazları eklemek mi?
