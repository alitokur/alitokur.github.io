
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
    PoolAllocator(size_t pool_size) : pool(pool_size)
    {

    }

    order* allocate(int a, int b)
    {
    }

    void deallocate(order* o)
    {
    }

private:
    struct object
    {
        order obj;
        object* next = nullptr;
    };
    object* _free_head = nullptr;
    std::vector<object> pool;

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

    for (int i = 0; i < SIZE - 1; i++)
    {
        pool_orders.push_back(pool.allocate(i * 2, i * 2));
    }
    for (int i = 0; i < SIZE - 1; i++)
    {
        pool.deallocate(pool_orders[i]);
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "time: " << duration.count() / double(SIZE) << std::endl;

    std::cout << "testing wiht new/delete" << std::endl;
    RawAllocator raw;
    start = std::chrono::steady_clock::now();

    for (int i = 0; i < SIZE - 1; i++)
    {
        raw_orders.push_back(raw.allocate(i * 2, i * 2));
    }
    for (int i = 0; i < SIZE - 1; i++)
    {
        raw.deallocate(raw_orders[i]);
    }
    end = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "time: " << duration.count() / double(SIZE) << std::endl;

    return 0;
}
