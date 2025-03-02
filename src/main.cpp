#include <iostream>
#include <string>

#include "Game.hpp"
#include "Log.hpp"
#include "Utils/getopt.h"

void printHelp(const std::string& progName) {
    std::cout << "Usage:\n"
              << "    " << progName << '\n'
              << "Options:\n"
              << "    -l        logging level (0: errors only, 3: all)" << std::endl;
}

int main(int argc, char* argv[]) {
#ifdef IS_DEBUG
    LogLevel loggingLevel = LogLevel::DEBUG;
#else
    LogLevel loggingLevel = LogLevel::INFO;
#endif

    int c;
    while((c = getopt(argc, argv, "hl:")) != -1) {
        switch(c) {
            case '?':
            case 'l': {
                try {
                    loggingLevel = static_cast<LogLevel>(std::stoi(optarg));
                } catch(const std::invalid_argument& e) {
                    std::cerr << "Invalid logging level: " << e.what() << std::endl;
                }
                break;
            }
            case 'h':
            default:
                printHelp(argv[0]);
                return 0;
                break;

            case -1:
                break;
        }
    }

    Log::setLevel(loggingLevel);
    Game game;
    game.start();

    return 0;
}
