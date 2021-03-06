#include "Halide.h"

using namespace Halide;

int main(int argc, char **argv) {
    Var x, y;
    Func f, g;

    // Up to about 40MB/image * 2 buffers seems to work on luxosr, when freshly booted
	// 130MB works on 2GB Quadro 4000
    int W = 1024*130, H = 1024;
    
    printf("Defining function...\n");

    f(x, y) = max(x, y);
    g(x, y) = clamp(f(x, y), 20, 100);

    if (use_gpu()) {
        f.root().cudaTile(x, y, 16, 16);
        g.cudaTile(x, y, 16, 16);
    }

    printf("Realizing function...\n");

    Image<int> img = g.realize(W, H);

    for (size_t i = 0; i < W; i++) {
        for (size_t j = 0; j < H; j++) {
            int m = std::max(i,j);
            // printf("img[%d, %d] = %d\n", i, j, img(i, j));
            if (img(i, j) != (m < 20 ? 20 :
                              m > 100 ? 100 :
                              m)) {
                printf("img[%d, %d] = %d\n", i, j, img(i, j));
                return -1;
            }
        }
    }

    printf("Success!\n");
    return 0;
}
