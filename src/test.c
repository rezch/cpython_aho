#include "aho.h"

#include <assert.h>
#include <stdio.h>


int main()
{
    dynamic_aho_t* aho = dynamic_aho_init(5);

    insert(aho, "hello", 2);
    insert(aho, "abc", 1);
    int32_t ans = request(aho, "hello world aabc");
    assert(ans == 3);

    insert(aho, "a", 1);
    ans = request(aho, "hello world aabc");
    assert(ans == 5);

    insert(aho, "a", -1);
    ans = request(aho, "hello world aabc");
    assert(ans == 3);

    assert(aho->size == 5);
    resize(aho, 15);
    assert(aho->size == 15);

    printf("Ok\n");
}
