---
title: inline or not 
---

### [[index|alitokur dot com]]

<h1>
    should you use inline?
</h1>


here is simple leaf function snippet with flag: 
"-O2 -masm=intel"

```cpp

__attribute__((noinline)) int sum(int x, int y)
{
    return x+y;
}

int main(){
    volatile int a = sum(1,2)
}
```

```assembly
sum(int, int):
        lea     eax, [rdi+rsi]
        ret
main:
        sub     rsp, 16
        mov     esi, 2
        mov     edi, 1
        call    sum(int, int)
        mov     DWORD PTR [rsp+12], eax
        xor     eax, eax
        add     rsp, 16
        ret
```
  
without inline it should be something like that:

```assembly
sum(int, int):
        lea     eax, [rdi+rsi]
        ret
main:
        mov     DWORD PTR [rsp-4], 3
        xor     eax, eax
        ret
```

and here is the non-leaf function:


```cpp
__attribute__((noinline))
int add(int x, int y){
    return x+y;
}

__attribute__((noinline))
int sum_and_bump(int x, int y){
    return add(x,y) + 1;
}

int main(){
    volatile int a = sum_and_bump(1,2);   

```

```assembly
add(int, int):
        lea     eax, [rdi+rsi]
        ret
sum_and_bump(int, int):
        push    rbp
        mov     rbp, rsp
        call    add(int, int)
        pop     rbp
        add     eax, 1
        ret
main:
        push    rbp
        mov     esi, 2
        mov     edi, 1
        mov     rbp, rsp
        sub     rsp, 16
        call    sum_and_bump(int, int)
        mov     DWORD PTR [rbp-4], eax
        xor     eax, eax
        leave
        ret
```

```assembly
and with inline:

add(int, int):
        lea     eax, [rdi+rsi]
        ret
sum_and_bump(int, int):
        lea     eax, [rdi+1+rsi]
        ret
main:
        push    rbp
        xor     eax, eax
        mov     rbp, rsp
        mov     DWORD PTR [rbp-4], 4
        pop     rbp
        ret

```
