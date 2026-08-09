

#include <iostream>

int main (int argc, char *argv[]) {

    std::allocator<int> allocator; // define an allocator
    int* p = allocator.allocate(5); // allocate space for 3 int
    
    for(int i = 0; i<3; i++){
        std::cout << "calling ctor for " << i << std::endl;
        std::construct_at(&p[i], i*100);
    }

    for(int i = 0; i<3; i++){
        std::cout << "p[" << i << "]" << p[i] << std::endl;
    }

    for(int i=0; i<3; i++){
        std::destroy_at(&p[i]);
    }

    allocator.deallocate(p, 3);


    return 0;
}

