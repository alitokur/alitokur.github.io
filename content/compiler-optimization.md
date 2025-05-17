---
title: compiler optimization
---

### [[index|alitokur dot com]]

<h1>
Lorem Ipsum
</h1>

---
> 
>Lorem Ipsum Lorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem Ipsu
>
> Lorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem Ipsu
---

Lorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem Ipsum
Lorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem Ipsum
Lorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem Ipsum


```cpp
#include <iostream>
#include <thread>
#include <atomic>

int main() {
    std::atomic<int> x = 0;
    std::thread t1([&]() { for (int i = 0; i < 1000000; ++i) x++; });
    std::thread t2([&]() { for (int i = 0; i < 1000000; ++i) x--; });
    t1.join();
    t2.join();
    std::cout << x << std::endl;
}
```
Lorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem IpsumLorem Ipsum

[time complexity](https://en.wikipedia.org/wiki/Time_complexity)
