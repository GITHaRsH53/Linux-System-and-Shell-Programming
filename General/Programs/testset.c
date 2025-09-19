// Test and Set Pseudocode –
// Shared variable lock initialized to false
#include <stdio.h>
#include <stdbool.h>
bool TestAndSet(bool);
int main()
{
    bool lock;

    while (1)
    {
        while (TestAndSet(lock))
            ;
        // critical section
        lock = false;
        // remainder section
    }
}

bool TestAndSet(bool target)
{
    bool rv = target;
    target = true;
    return rv;
}