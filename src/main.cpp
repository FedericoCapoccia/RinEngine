#include "core/logger.hpp"

#include "core/containers/darray.hpp"

using namespace rin;

int main(void)
{
    log::info("Info %.2f", 3.1235431);
    darray<const char*> arr(false);
    arr.push("Hello");
    arr.push("Hello");
    arr.push("World");

    arr[1] = "Balls";

    for (size_t i = 0; i < arr.len; i++) {
        log::info("%s", arr[i]);
    }

    return 0;
}
