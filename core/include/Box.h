#pragma once // only include this file once during compilation 

class Box {
   private: // because only the Box class controls how its data is accessed/changed
   //encapsulation: the data members are private and can only be accessed through public member functions 
   double length;
    double width;
    double height;
    double weight;

    public:
    // constructor to initialize the data members
    Box(double length,
    double width,
    double height,
    double weight);

    double getLength() const;
    double getWidth() const;
    double getHeight() const;
    double getWeight() const;
};

