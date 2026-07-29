#include <iostream>
#include "Box.h"
#include "Pallet.h"
#include "Placement.h"

int main() {
    using namespace std;
    // Create a Box object using the constructor
    Box myBox(10.0, 5.0, 2.0, 3.5);

    // Access and print the dimensions and weight of the box
    cout << "Length: " << myBox.getLength() << std::endl;
    cout << "Width: " << myBox.getWidth() << std::endl;
    cout << "Height: " << myBox.getHeight() << std::endl;
    cout << "Weight: " << myBox.getWeight() << std::endl;

    Pallet pallet(1200, 1000, 1100, 980);

cout << "Pallet Length: "
     << pallet.getLength() << endl;
cout << "Pallet Width: "
        << pallet.getWidth() << endl;
cout << "Pallet Max Load Length: "
        << pallet.getMaxLoadLength() << endl;
cout << "Pallet Max Load Width: "
        << pallet.getMaxLoadWidth() << endl;        
        
        Placement p1(
    1,6,0,300,200, 0
);

cout << p1.getX() << endl;
cout << p1.getY() << endl;
    return 0;
}