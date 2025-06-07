#include <iostream>
#include "Canvas.hpp"

int main() {
    Canvas canvas{10, 20};
    std::cout << "Hello, World!" << std::endl;
    std::cout << canvas.width << " , " << canvas.height << "\n";
    const std::string output{"outfile.ppm"};
    auto retval = canvas.write(output);
    if (retval.has_value()) {
        std::cout << "Data written to " << output << "\n";
    }
    else if (retval.error() == Canvas::CanvasError::invalid_path){
        std::cout << "Cannot write file " << output << " - invalid path\n";
    }
    return 0;
}
