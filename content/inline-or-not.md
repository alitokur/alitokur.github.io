---
title: inline or not 
---

### [[index|alitokur dot com]]

<h1>
    should you use inline?
</h1>


Do you think about the overhead of a function on some long nights, or are you just normal.
This night will be different my friends. This night we gonna crack some cpp codes
then we are going to talk about some things that people don't usually want to discuss. 
And then we'll decide: **should we use inline or not?**


Here is a quote from [Optimizing Software in C++](https://www.agner.org/optimize/optimizing_cpp.pdf):

> The function call makes the microprocessor jump to different code address and back again. This may take
up to 4 clock cycles. In most cases, the microprocessor is able to overlap the call and retrn operations
with other calculations to save time.

Here is a simple example of a function compiled with the flags: 
**-O2 -masm=intel**


```cpp
__attribute__((noinline))
int sum(int a, int b){
    return a + b;
}

int main (int argc, char *argv[]) {
    volatile int foo = 0;
    for(int i = 0; i < 9999999; i++) {
        foo += sum(i, i+1);
    }
    return 0;
 ```

I know, I can use -O0 to disable inlining, but I'd like to keep things closer to
production setup. So instead of turning off optimization, i will use **__attribute__((noinline))**.
And don't worry this ugly **volatile** here, it just to prevent constant folding.

```asm
sum(int, int):
        lea     eax, [rdi + rsi]
        ret

main:
        push    rbx
        sub     rsp, 16
        mov     dword ptr [rsp + 12], 0
        xor     edi, edi
.LBB1_1:
        lea     ebx, [rdi + 1]
        mov     esi, ebx
        call    sum(int, int)
        add     dword ptr [rsp + 12], eax
        mov     edi, ebx
        cmp     ebx, 9999999
        jne     .LBB1_1
        xor     eax, eax
        add     rsp, 16
        pop     rbx
        ret

sum(int, int):
        lea     eax, [rdi + rsi]
        ret
```
  
without inline it should be something like that:

```asm

sum(int, int):
        lea     eax, [rdi + rsi]
        ret

main:
        mov     dword ptr [rsp - 4], 0
        mov     eax, 17
.LBB1_1:
        mov     ecx, dword ptr [rsp - 4]
        add     ecx, eax
        add     ecx, -16
        mov     dword ptr [rsp - 4], ecx
        mov     ecx, dword ptr [rsp - 4]
        lea     ecx, [rax + rcx - 14]
        mov     dword ptr [rsp - 4], ecx
        mov     ecx, dword ptr [rsp - 4]
        lea     ecx, [rax + rcx - 12]
        mov     dword ptr [rsp - 4], ecx
        mov     ecx, dword ptr [rsp - 4]
        lea     ecx, [rax + rcx - 10]
        mov     dword ptr [rsp - 4], ecx
        mov     ecx, dword ptr [rsp - 4]
        lea     ecx, [rax + rcx - 8]
        mov     dword ptr [rsp - 4], ecx
        mov     ecx, dword ptr [rsp - 4]
        lea     ecx, [rax + rcx - 6]
        mov     dword ptr [rsp - 4], ecx
        mov     ecx, dword ptr [rsp - 4]
        lea     ecx, [rax + rcx - 4]
        mov     dword ptr [rsp - 4], ecx
        mov     ecx, dword ptr [rsp - 4]
        add     ecx, eax
        add     ecx, -2
        mov     dword ptr [rsp - 4], ecx
        add     dword ptr [rsp - 4], eax
        add     eax, 18
        cmp     eax, 20000015
        jne     .LBB1_1
        xor     eax, eax
        ret

```

End then lets check the perf results:

```bash
        54.270.085      cycles                                                                
        93.856.882      instructions                     #    1,73  insn per cycle            
        30.751.057      branches                                                              
            62.489      branch-misses                    #    0,20% of all branches           

       0,012193552 seconds time elapsed

       0,012213000 seconds user
       0,000000000 seconds sys
```

And this is for inline:

```bash
        14.141.612      cycles                                                                
        38.742.668      instructions                     #    2,74  insn per cycle            
         1.977.227      branches                                                              
            60.926      branch-misses                    #    3,08% of all branches           

       0,003771016 seconds time elapsed

       0,003803000 seconds user
       0,000000000 seconds sys

```
