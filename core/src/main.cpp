#include <iostream>
#include "Box.h"
#include "Pallet.h"

int main() {
    using namespace std;
    // Create a Box object using the constructor
    Box myBox(10.0, 5.0, 2.0, 3.5);

    // Access and print the dimensions and weight of the box
    cout << "Length: " << myBox.getLength() << std::endl;
    cout << "Width: " << myBox.getWidth() << std::endl;
    cout << "Height: " << myBox.getHeight() << std::endl;
    cout << "Weight: " << myBox.getWeight() << std::endl;

    Pallet pallet(1200, 1000, 1100, 900);

cout << "Pallet Length: "
     << pallet.getLength() << endl;
cout << "Pallet Width: "
        << pallet.getWidth() << endl;
cout << "Pallet Max Load Length: "
        << pallet.getMaxLoadLength() << endl;
cout << "Pallet Max Load Width: "
        << pallet.getMaxLoadWidth() << endl;        
        
    return 0;
}