---
title: allocator
---

### [[index|alitokur dot com]]

<h1>
  what is wrong with new and delete? 
</h1>

It's so hot, and I am reading The Cafe on the Edge of the World. If you are
wondering whether any of thiss has anthing to do with allocators, uhh no. But
if you have some time, I strongly recommend reading this book.

![The Cafe on the Edge of the World book cover](/books/thecafe.jpeg)

Anyway, allocators...

Default memory management is a big sin. Maybe not one of the seven ones, but
might be added in the next patch. It can be slow sometimes, sure but the
dramatic problem is that it is unpredictable. And unpredictablility is the last
thing i want to see in my trading code. I created a place holder for my malloc
post(is malloc broken?). If you are lucky, i already wrote and publish it. The
second problem is: Fragmentation. Objects with different sizes and different
lifetimes can leave holes in memory, which is also something we do not want.
And then there is Cache Locality. I only one fantasy mate. If i use these
object together, please put them together. But the standart allocator can not
guarantee that.

The story begin decades ago. I'm not a Commodore guy, so do not expect a 
full history from me. But back in the days on non-flat memory models,
especially when x86 had this whole near-pointer / far pointer business, STL
needed an abstraction over how containers talked to memory. And allocators
entered the story. The original idea was not exactly efficint allocation strategies
, it was more about abstracting the memory models itself.

Then hardware changed. Flat memory models won, near and far pointers mostly
disappeared, but allocators stated. And their job slowly became something much more 
interesting to us: cusotmizing how caoninters obtain and release memory. 
At the center of this is std::allocator.

It comes with basic contract.

```md
value_type, 
constructor, 
allcoate 
deallocate
```

<!-- TODO: pmr -->

You probably do not write std::allocator explicitly in your daily code. 
But it is already hiding inside many standard containers. 

For example, this is just a vector:
std::vector<int> v;

But actually, its something like that.

std::vector<int, std::allocator<int>> v;

The second template argument is simply defaulted for us.

And if you are wondering, is there any direct usage of it, here its
the boring version that i never used.

```cpp
    std::allocator<int> allocator; // define an allocator
    int* p = allocator.allocate(3); // allocate space for 3 int
    
    for(int i = 0; i<3; i++){
        std::cout << "calling ctor for " << i << std::endl;
        std::construct_at(&p[i], i*100);
    }

    for(int i=0; i<3; i++){
        std::destroy_at(&p[i]); ///cpp17
    }

    allocator.deallocate(p, 3); ///cpp20
```

And of course C++ being C++, there is another layer on top of this, it has
modern helper: allocator_traits. Basically it says:

"well mate, give me the important parts, i can provide defaults for a bunch of
the rest" For more details, read my other (log)[allocator_traits]

Okay, so time for strategies?

Pool allocator or a.k.a memory pool. 
Im starting with a basic one. Not so fancy but it would be great to start. 
And then we can optimize it.

Here is my simple order struct. It just keeps order price and quantity.

```cpp
struct order
{
    int32_t price;
    int32_t qty;

    order() = default;
    order(int x, int y) : price(x), qty(y) {}
};

```

And here is the allocator class. Not template. I hate templates. I ll make it template later. 

```cpp

private:
    struct object
    {
        order obj;
        bool is_free_ = true;
    };
    std::vector<object> pool;
    size_t next_free_node_ = 0;

public:
    PoolAllocator(size_t pool_size) : pool(pool_size, { order(), true })
    {
        std::cout << "creating pool with size: " << pool_size << std::endl;
        for (int i = 0; i < pool.size(); i++)
        {
            std::printf("the address of pool object [%d] -> %p \n", i, (void*)&pool[i].obj);
        }
    }
    
```

Like the other codes, i keep allocate function simple. It just use next free slot. And calling 
in place new to construct the object.

```cpp
    order* allocate(int a, int b)
    {
        auto& slot = pool[next_free_node_];
        auto* o = &slot.obj;
        o = new (o) order(a, b);
        slot.is_free_ = false;
        update_next_free_index();
        return o;
    }
    
    void update_next_free_index()
    {
        /// TODO: use free list
        auto curr = next_free_node_;
        while (!pool[next_free_node_].is_free_)
        {
            next_free_node_++;
            if (next_free_node_ == pool.size())
                next_free_node_ = 0;
            if (next_free_node_ == curr)
            {
                std::cout << " warning: pool is full!" << std::endl;
                std::exit(1);
            }
        }
    }

    void deallocate(order* o) {
        auto index =  reinterpret_cast<object*>(o) - &pool[0];
        pool[index].is_free_ = true;
    }
```

And there is just pointer aritmetic trick for deallocation. I'm using reinterpret_cast for only 
to be seem cool.

```cpp
    void deallocate(order* o) {
        auto index =  reinterpret_cast<object*>(o) - &pool[0];
        pool[index].is_free_ = true;
    }
```

And first results:

```sh
creating pool with size: 5000000
testing wiht pool
time: 5.86106
testing wiht new/delete
time: 20.0248
```

Bu kadar ilkel bir pool ile bile neredeyse 5x improvement. Ve arkasindaki  tek basit fikir
initial bir allocation. Bununla birlikte ilk allocatorimizi yazmis olduk. Dedigim gibi
bu pool cok ilkel ve bir cok noktada gelistirmeye musait. Biraz teorik seylerden bahsetmeden once 
isterseniz bazi ufak improvementleri da yapalim.











































