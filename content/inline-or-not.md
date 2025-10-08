---
title: inline or not 
---

### [[index|alitokur dot com]]

<h1>
    should you use inline?
</h1>


Do you think about the overhead of a function on some long nights, or are you just normal.
This night we are going to crack some cpp codes while we 
are listening [Pilli Bebek](https://open.spotify.com/track/0LK1eyxnHTeTgDAXl02G0s?si=018dfddeef7b47d7), 
and we are going to talk about some things that people don't usually want to discuss. 

And then we'll decide: **should we use inline or not in C++?**

If you are here, probably you know that, what c++ inline functions are. Just a quick reminder: when we declare 
a function inline, we are trying to remove the overhead of a function call, by getting the code from the 
function and putting it to the point that we call the function. Well in this point i will be honest for you, i use this 
definitions for my all life if the topic is inline. But one time, a guy ask me a follow up question, so what is this overhead? 
And i was like, you know this, this, eeh this is overhead man. That was the moment i say myself, dont be a d#ck, if you dont 
know something, just stay humbe and give yourself some time to learn what you wanna talk. Now, lets look at a bit deeper 
this OVERHEAD. When we call a function, in the background, we are making some jumps, loading parameters, 
saving return adress, and then popping parameters. All these are just a general view of this overhead. 
And no suprise, modern compilers and cpu's are realy smart when they making optimizations on them. But optimizations 
of them not means that we should aware of them.


Here is a simple cpp example of a function compiled with the flags: 
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
Without inline it should be something like that:

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
As you can see have much more operation, much more cycles like [Agner Fog says:](https://www.agner.org/optimize/optimizing_cpp.pdf):

> The function call makes the microprocessor jump to different code address and back again. This may take
up to 4 clock cycles. In most cases, the microprocessor is able to overlap the call and return operations
with other calculations to save time.

So, i think we have a clear and basic understanding what is our problem. And the most asking question
is coming. If inline is wonderfull optimization tool why we dont just make all function inline? The answer is simple and 
you probably know that, it us just a hint. Compiler is absolutely free to ignore it. And some cases gozunun yasina bile bakmiycak:

- Recursive functions
- Virtual functions
- If its too big. i know, i know, you asking what is the big? But believe me i dont have any idea. They are all 
compiler depented.
- If multiple inline are nested. Like inline Foo call inline Far, and inline Far calls inline Tar and .etc. There are
some depth limits for this kind of cases.

So of course every compiler have some diffrent acts for this cases. Me or anyone can give any guarente. 
But this is not means, our inline keyword has no meaning. This is compiletely wrong. For compilers inline keyword has a meaning still. 
I read a great article from Sy Brand, and you have to check it. In nutshell:


```cpp
 // Adjust the threshold based on inlinehint attribute and profile based
  // hotness information if the caller does not have MinSize attribute.
  if (!Caller->hasMinSize()) {
    if (Callee.hasFnAttribute(Attribute::InlineHint))
      Threshold = MaxIfValid(Threshold, Params.HintThreshold);
 
    // FIXME: After switching to the new passmanager, simplify the logic below
    // by checking only the callsite hotness/coldness as we will reliably
    // have local profile information.
    //
    // Callsite hotness and coldness can be determined if sample profile is
    // used (which adds hotness metadata to calls) or if caller's
    // BlockFrequencyInfo is available.
    BlockFrequencyInfo *CallerBFI = GetBFI ? &(GetBFI(*Caller)) : nullptr;
    auto HotCallSiteThreshold = getHotCallSiteThreshold(Call, CallerBFI);
    if (!Caller->hasOptSize() && HotCallSiteThreshold) {
      LLVM_DEBUG(dbgs() << "Hot callsite.\n");
      // FIXME: This should update the threshold only if it exceeds the
      // current threshold, but AutoFDO + ThinLTO currently relies on this
      // behavior to prevent inlining of hot callsites during ThinLTO
      // compile phase.
      Threshold = *HotCallSiteThreshold;
    } else if (isColdCallSite(Call, CallerBFI)) {
      LLVM_DEBUG(dbgs() << "Cold callsite.\n");
      // Do not apply bonuses for a cold callsite including the
      // LastCallToStatic bonus. While this bonus might result in code size
      // reduction, it can cause the size of a non-cold caller to increase
      // preventing it from being inlined.
      DisallowAllBonuses();
      Threshold = MinIfValid(Threshold, Params.ColdCallSiteThreshold);
    } else if (PSI) {
      // Use callee's global profile information only if we have no way of
      // determining this via callsite information.
      if (PSI->isFunctionEntryHot(&Callee)) {
        LLVM_DEBUG(dbgs() << "Hot callee.\n");
        // If callsite hotness can not be determined, we may still know
        // that the callee is hot and treat it as a weaker hint for threshold
        // increase.
        Threshold = MaxIfValid(Threshold, Params.HintThreshold);
      } else if (PSI->isFunctionEntryCold(&Callee)) {
        LLVM_DEBUG(dbgs() << "Cold callee.\n");
        // Do not apply bonuses for a cold callee including the
        // LastCallToStatic bonus. While this bonus might result in code size
        // reduction, it can cause the size of a non-cold caller to increase
        // preventing it from being inlined.
        DisallowAllBonuses();
        Threshold = MinIfValid(Threshold, Params.ColdThreshold);
      }
    }
```

So, the part i shared and Sy Brand blog a bit different, LLVM's logic is growing, 
there are some new checks for callsite hotness/coldness but im not the guy who can 
explain this cool things. I can say that Clang still looking for your inline keyword here:

```cpp
 if (Callee.hasFnAttribute(Attribute::InlineHint))
      Threshold = MaxIfValid(Threshold, Params.HintThreshold);
```

But i want to show another trick that i saw from Jason Turner. Lets open Compiler Explorer's optimization 
remarks panel. 
Just created a sum function and this is the result of it:

![Screenshot](https://github.com/alitokur/alitokur.github.io/blob/master/src/inline-or-not/ss/noinline.png?raw=true)

See you that this tiny cost and threshold. It says that my function cost is 35 and the calculated
threshold for it just 337. We are so far from this limit and it says, no woried frined, i inlined it for you.
And Then i add inline keyword to my sum function.

here is the result:

![Screenshot](https://github.com/alitokur/alitokur.github.io/blob/master/src/inline-or-not/ss/inline.png?raw=true)


Can you see that. cost is same which we expted it. but the threshold is changed. Holy Bjorn. This is how inline can effect the optimixer.
The clang gives us a somewhat unique advantage here. The all arguments about 
inline has no effects on compilers are just a lie. We strongly say that inliners has some tiny effects. 


But, but, but...
Do we have on effect on compilers, means that we should use inline? Now you are welcome to new war. This is between
inliners and non-inliners. You can choise your side after this article. Let me explain what i knows.

The Core guideline says

> Some optimizers are good at inlining without hints from the programmer, 
but don’t rely on it. Measure! Over the last 40 years or so, we have been promised 
compilers that can inline better than humans without hints from humans. 
We are still waiting. Specifying inline (explicitly, or implicitly when writing 
member functions inside a class definition) encourages the compiler to do a better job.

Yes, there is advice on measurement, but the critic about the compilers, feels that like 
they are close the inlining. 

The other side says: 

The inline should be used when you need to deal with linking (we are not mention about it but its also 
too for multiple definitions error). They dont see inline as a 
performance tool. Its mainly linkage tool. Because compiler, made all kind of caclulations eithout your input
at all, and there is a good change that by modify this thresghold you are actually doing something that will perform worse.
The compiler has a pretty good reason for why it set the threshold this value. So they say its something that you should pretty
much rely on you almost certaily dontw want to mess with this. 














