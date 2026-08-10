#include <iostream>

struct order
{
    int32_t price;
    int32_t qty;

    order() = default;
    order(int x, int y) : price(x), qty(y){}
};

class PoolAllocator
{
public:
    PoolAllocator(size_t pool_size) : pool(pool_size, { order(), true })
    {
        std::cout << "creating pool with size: " << pool_size << std::endl;
        for (int i = 0; i < pool.size(); i++)
        {
            std::printf("the address of pool object [%d] -> %p \n", i, (void *)&pool[i].obj);
        }
    }

    order *allocate(int a, int b)
    {
        auto obj_block = &pool[next_free_node_];
        order *ret = &(obj_block->obj);
        ret = new (ret) order(a, b);
        obj_block->is_free_ = false;
        update_next_free_index();
        return ret;
    }

    void deallocate(order* o) {
        auto index = (reinterpret_cast<object*>(o)-&pool[0]);
        std::cout << "removed index: " << index << std::endl;
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
        std::cout << "update index" << std::endl;
        auto current = next_free_node_;
        while (pool[next_free_node_].is_free_ == false)
        {
            ++next_free_node_;
            if (next_free_node_ % pool.size() == 0)
                next_free_node_ = 0;

            if (current == next_free_node_)
            {
                std::cout << "pool is full" << std::endl;
                break;
            }
        }
    }
};

int main(int argc, char *argv[])
{
    PoolAllocator pool(5);
    auto x = pool.allocate(100, 200);
    auto y = pool.allocate(100, 200);
    auto z = pool.allocate(100, 200);
    pool.allocate(100, 200);
    pool.allocate(100, 200);
    pool.deallocate(x);
    pool.deallocate(y);
    pool.deallocate(z);



    return 0;
}
