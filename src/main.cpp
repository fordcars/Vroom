#include "getopt.h"
#include <SDL2/SDL.h>
#include <iostream>

void printHelp(const std::string& progName) {
    std::cout
        << "Usage:\n"
        << "    " << progName << " OPTIONS\n"
        << "Options:\n"
        << "    -a        do something!\n"
        << "    -b        do something else!"
        << std::endl;
}

int main(int argc, char* argv[]) {
    for(;;) {
        switch(getopt(argc, argv, "h")) {
        case '?':
        case 'h':
        default:
            printHelp(argv[0]);
            break;

        case -1:
            break;
        }

        break;
    }

    SDL_Init(SDL_INIT_EVERYTHING);
    
    return 0;
}
