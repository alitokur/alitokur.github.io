#include <iostream>
#include <iterator>
#include <vector>
#include <chrono>

struct order
{
    int32_t price;
    int32_t qty;

    order() = default;
    order(int x, int y) : price(x), qty(y) {}
};

class PoolAllocator
{
public:
    PoolAllocator(size_t pool_size) : pool(pool_size, { order(), true })
    {
        std::cout << "creating pool with size: " << pool_size << std::endl;
        for (int i = 0; i < pool.size(); i++)
        {
            // std::printf("the address of pool object [%d] -> %p \n", i, (void*)&pool[i].obj);
        }
    }

    order* allocate(int a, int b)
    {
        auto& slot = pool[next_free_node_];
        auto* o = &slot.obj;
        o = new (o) order(a, b);
        slot.is_free_ = false;
        update_next_free_index();
        return o;
    }

    void deallocate(order* o) {
        size_t index = reinterpret_cast<object*>(o) - &pool[0];
        pool[index].is_free_ = true;
    }

private:
    struct object
    {
        order obj;
        bool is_free_ = true;
    };
    std::vector<object> pool;
    size_t next_free_node_ = 0;
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
};

class RawAllocator
{
public:
    order* allocate(int a, int b)
    {
        order* o = new order(a,b);
        return o;
    }

    void deallocate(order* o)
    {
        delete o;
    }

};

constexpr size_t SIZE = 5000000;
int main(int argc, char* argv[])
{
    std::vector<order*> pool_orders;
    std::vector<order*> raw_orders;
    pool_orders.reserve(SIZE);
    raw_orders.reserve(SIZE);
    PoolAllocator pool(SIZE);
   
    std::cout << "testing wiht pool" << std::endl;
    auto start = std::chrono::steady_clock::now();

    /// TODO:: add warm up

    for (int i = 0; i < SIZE-1; i++)
    {
        pool_orders.push_back(pool.allocate( i*2,  i*2));
    }
    for (int i = 0; i < SIZE-1; i++)
    {
        pool.deallocate(pool_orders[i]);
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start);
    std::cout << "time: " << duration.count() / double(SIZE) << std::endl;
    
    std::cout << "testing wiht new/delete" << std::endl;
    RawAllocator raw;
    start = std::chrono::steady_clock::now();
    
    for (int i = 0; i < SIZE-1; i++)
    {
        raw_orders.push_back(raw.allocate( i*2,  i*2));
    }
    for (int i = 0; i < SIZE-1; i++)
    {
        raw.deallocate(raw_orders[i]);
    }
    end = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start);
    std::cout << "time: " << duration.count() / double(SIZE) << std::endl;
    

    return 0;
}
