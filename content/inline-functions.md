---
title: inline or not 
---

### [[index|alitokur dot com]]

<h1>
    should you use inline in C++?
</h1>


Ever find yourself thinking about  the overhead of a function on those long
nights, or are you one of the normal ones? Tonight we're cracking **"inline"**
while we are listening [Pilli
Bebek](https://open.spotify.com/track/0LK1eyxnHTeTgDAXl02G0s?si=018dfddeef7b47d7),
and we are going to talk about some things that people don't usually like to
discuss. By the end of the night, when the first light of dawn hits the screen,
we'll make our choice: **should we use inline or not in C++?**

And please, please, please, do not forget, i'm not an expert, and writing all
this for you is also a way for me to learn. So, if you find any mistakes, call
me and i can kick your *ss.

Okey, here we go. If you are here, you probably done all your search about
inlining, stackoverflow, chatgpt, reddit... and already know what **inline**
functions are. Btw, we are focusing on the performance aspect of it, not
linkage. So let me give you just a tiny reminder: when we declare a function as
inline, we are trying to reduce the overhead of a function call, by placing the
function's code directly at the call site. And you all might have heard a story
going like this: A function call has overhead, because the program jumps to the
memory location where your function code starts, it needs to save the current
cursor position into the stack, then arguments are being pushed to the stack,
then they are taken from the stack, then a result is written, then 3 apples bla
bla... 

Now, there is no need, but let's write a simple function that sums two integers
inside a loop, and measure the performance difference between inline and
non-inline versions to see that this story is true or not.

> [!NOTE]
> flags: **-O2**


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

I know I could use -O0 to disable inlining, but instead of turning off
optimizations, I’ll keep -O2 and add **__attribute__((noinline))** to block
inlining explicitly. The reason I’m using -O2 here is to show— as you’ll see
later— that the compiler already does most of the heavy lifting for us. And
don’t worry about the ugly volatile.It’s just there to stop the compiler from
optimizing the result away.

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
```
**perf** output for non-inlined version:

```bash
       54.270.085      cycles                                                                
       93.856.882      instructions      #    1,73  insn per cycle            
       30.751.057      branches                                                              

       0,012193552 seconds time elapsed
```
As you can see, main function has this magical **call** instruction.

- ~3 branches/iter makes sense: loop back-edge + call + ret are all counted as
branches.
- Extra instructions are the call/return and arg setup.

And now let compiler make its magic with inline.

```asm

main:
        mov     dword ptr [rsp - 4], 0
        mov     eax, 17
.LBB1_1:
        mov     ecx, dword ptr [rsp - 4]
        add     ecx, eax
        add     ecx, -16
        mov     dword ptr [rsp - 4], ecx
        mov     ecx, dword ptr [rsp - 4]
        ...      #   more operations
        ret

```
- Huge drop in cycles and instructions: the call/ret disappeared.
```bash
       14.141.612      cycles                                                                
       38.742.668      instructions      #    2,74  insn per cycle            
       1.977.227       branches                                                              

       0,003771016 seconds time elapsed
```
We have many more operations, and that means much more cycles.

```bash
Δcycles ≈ 54.270.085 - 14.141.612 = 40.128.473 
TotalIterations = 9,999,999
 4.012 ≈ cycles-per-call
```

That lands right in what: [Agner Fog
says:](https://www.agner.org/optimize/optimizing_cpp.pdf)

> The function call makes the microprocessor jump to different code address 
and back again. This may take up to 4 clock cycles.

Well, i think we have a clear and basic understanding what is our problem. And
here comes most asked question: **"If inline is such a wonderful optimization tool
why do not we just make every function inline?** The answer is simple and you
probably know that: **it is just a hint**. Compiler is absolutely free to ignore
it. And most of the time, it does not need your help. If you check the above example,
if you remove inline from sum function, the compiler will inline it anyway.

And some cases it wont care at all such as:

- Recursive functions
- Virtual functions
- When the function is too big. i know, i know, you are asking, "what is the
  big?" But believe me, i have no idea. Its completely compiler dependent.
- If multiple inline are nested. Ex: inline Foo call inline Far, and inline
  Far calls inline Tar, etc. There are depth limits in such cases.

Well, of course every compiler have some different behavior for this cases. And
no one, not me, not Herb Sutter, not anyone can give a full guarantee. 

Then comes the second most asked question: **"So, inline keyword is useless,
compiler is free to ignore it, why should i use it?"** No our inline keyword is
not meaningless. That would be completely wrong. For compilers (i'm talking
about Clang) it still has meaning. I read a great article by [Sy
Brand](https://tartanllama.xyz/posts/inline-hints/), and you really should
check it. There is a code snippet from clang and in something like this:

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

So, the part I shared is bit different from what **Sy Brand** showed. LLVM's logic
keeps growing, there are now extra checks for callsite hotness/coldness but I
am not the right person to unpack all these cool things. What matters here is
that Clang still looking for your inline keyword,right here:

```cpp
 if (Callee.hasFnAttribute(Attribute::InlineHint))
      Threshold = MaxIfValid(Threshold, Params.HintThreshold);
```

This means that if you add inline to your function, Clang will increase the
threshold for inlining it.And, and, and, i want to show another cool trick that
i saw from **Jason Turner**. Open Compiler Explorer and enable the Optimization
Remarks panel.I created a simple sum function and here's what it showed:

![Screenshot](https://github.com/alitokur/alitokur.github.io/blob/master/src/inline-or-not/ss/noinline.png?raw=true)

Look at this tiny, smol, cost and threshold. It says that my function cost
is 35, and the calculated threshold for it just 337. We are well below that
limit so it says:, "no worries friend, i inlined it for you". Then i added the
**inline** keyword to my sum function.

Here is the result:

![Screenshot](https://github.com/alitokur/alitokur.github.io/blob/master/src/inline-or-not/ss/inline.png?raw=true)


Can you see that? Cost is same(as expected). But the threshold changed. Holy
Bjorn! This is how inline can affect the optimizer. Clang gives us a subtle but
real advantage here. The all arguments claim that "inline has no effect on
compilers" is just wrong. We can clearly say that it reacts to compiler
optimizations, even if slightly.


But, but, but... 
Just because we can influence the compiler, does that mean we should use inline?
You are welcome to new war. This is between inliners and non-inliners. You
can pick your side after this article. 

I want to share two different opinions from experts. First one is from [C++
Core
guideline](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f5-if-a-function-is-very-small-and-time-critical-declare-it-inline):

> Some optimizers are good at inlining without hints from the programmer, 
but don’t rely on it. Measure! Over the last 40 years or so, we have been
promised compilers that can inline better than humans without hints from
humans. We are still waiting. Specifying inline (explicitly, or implicitly when
writing member functions inside a class definition) encourages the compiler to
do a better job.

Yes, the final advice is to measure, but there’s a hint of skepticism here — as
if even the experts feel that compilers aren’t always nailing it yet.

And also [cppreference](https://en.cppreference.com/w/c/language/inline.html) says:

> The intent of the inline specifier is to serve as a hint for the compiler to 
perform optimizations, such as function inlining, which usually require the
definition of a function to be visible at the call site. The compilers can (and
usually do) ignore presence or absence of the inline specifier for the purpose
of optimization.

It seems cppreference is more on the side of "inline is just a hint, and
compilers can ignore it". They don't seem to advocate strongly for using
inline as a performance optimization tool.

The other one is coming from Agner Fog, in his optimization manuals, also
discusses inlining and he says:

> A function is usually inlined if the inline keyword is used or
if it's body is defined inside a class definition. Inlining a function is advantageous if the
function is small or if it is called only from one place in the program. Small
functions are often inlined automatically by the compiler. On the other hand,
the compiler may in some cases ignore a request for inlining a function if the
inlining causes technical problems or performance problems.

As i said before, the first argument is really  debatable. As i showed before,
the compiler performs calculations, and adding the inline keyword will not
always. And threshold for small function is completely compiler dependent.

Here is my rock star, [Jason
Turner](https://www.youtube.com/watch?v=GldFtXZkgYo) says do not use inline for
performance and do not see inline as a performance tool. It's mainly a linkage
tool. The compiler already makes all kinds of calculations without your input,
and all, and there is a change that by modifying this threshold, you might
actually make make performance worse. The compiler has a pretty good reason for
setting the threshold to this value. So they say its something you rely on, you
almost certainly do not want to mess with it. 

As i shared above, Sy Brand's article and most of the answers on SO and Reddit
strongly suggest making measurements before deciding. I think think its the
best way to keep yourself from bias.

**And Baba has some practices**: I use inline functions when i write code at
my company. Because it's a must if you are working in HFT world. But for all
my personal projects, I do not use inline that much. That is not because I do
not like it. I believe that they are useful. However, the criteria for
determinining whether a function should be inlined can be complex and subtle.
So i start with no inline, and when i find a bottleneck in my code, i try
inlining it and then measure.

I have spoken.


### Sources and further reading:
I already shared some links above, and here are some more:

- [A Deeper Look at Inline
  Functions](https://accu.org/journals/overload/9/42/kelly_449/)
- [Function Inlining in C++ | Modern Cpp Series Ep.
  109](https://www.youtube.com/watch?v=c1jyS8h7MlU)
- [Learncpp - Inline functions and
  variables](https://www.learncpp.com/cpp-tutorial/inline-functions-and-variables/https://www.learncpp.com/cpp-tutorial/inline-functions-and-variables/)
- [Inline functions in C++.What is the
  point?](https://softwareengineering.stackexchange.com/questions/35432/inline-functions-in-c-whats-the-point)












