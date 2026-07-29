#include <iostream>
#include "Box.h"

int main() {
    using namespace std;
    // Create a Box object using the constructor
    Box myBox(10.0, 5.0, 2.0, 3.5);

    // Access and print the dimensions and weight of the box
    cout << "Length: " << myBox.getLength() << std::endl;
    cout << "Width: " << myBox.getWidth() << std::endl;
    cout << "Height: " << myBox.getHeight() << std::endl;
    cout << "Weight: " << myBox.getWeight() << std::endl;

    return 0;
}