#include <string>   
#include <iostream>
#include <vector>

using namespace std;

class Point
{                                   // Class definition 'Point'
private:                            // Access specifier
    std::string name = "undefined"; // Private member variable to store the name of the point
    double z;

public:             // Access specifier
    double x = 0.0; // Public member variable to store the x-coordinate of the point
    double y{0.0};

    std::vector<double> elements;

    Point() = default;                           // Default constructor for the Point class
    Point(std::string name, double x, double y); // Constructor declaration for the Point class
    void place(double x, double y, double z);

    void print() const;

    void set_name(const std::string name)
    {                      // Method to set the name of the point
        this->name = name; // Assign the new name to the private member variable 'name'
    }
};

int main()
{
    Point p0;
    Point p1("P1", 1.0, 1.0);   // Create an instance of the Point class named p1 with specified name and coordinates
    Point p2("P2", -1.0, -1.0); // Create another instance of the Point class named p2 with specified name and coordinates

    std::cout << p1.x << ", " << p1.y << std::endl; // Output the x and y coordinates of point p1
    std::cout << p2.x << ", " << p2.y << std::endl; // Output the x and y coordinates of point p2

    p1.set_name("first point"); // Set the name of point p1 to "first point" using the set_name method
    p0.print();                 // Call the print method of point p0 to output its coordinates in the format "x, y"
    p1.print();                 // Call the print method of point p1 to output its coordinates in the format "x, y"
    p2.print();                 // Call the print method of point p2 to output its coordinates in the format "x, y"

    p0.place(0.0, 0.0, 0.0);
    p0.print(); // Call the print method of point p0 to output its coordinates in the format "x, y"

    std::vector<int> vecInt;                // Create a vector of integers named vecInt
    vecInt.push_back(0);                    // Add the integer 0 to the end of the vector vecInt
    vecInt.push_back(1);                    // Add the integer 1 to the end of the vector vecInt
    vecInt.push_back(2);                    // Add the integer 2 to the
    std::vector<int> vecInt2({0, 1, 3, 5}); // Create another vector of integers named vecInt2 and initialize it with the values 0, 1, and 2

    for (size_t i = 0; i < vecInt2.size(); i++)
    {
        std::cout << "Index{" << i << "}: " << vecInt2[i] << std::endl; // Output each element of the vector vecInt2 on a new line
    }

    std::vector<Point> points(4); // Create a vector of Point objects named vecPoints
    for (size_t i = 0; i < points.size(); i++)
    {
        points[i].print(); // Call the print method for each Point object in the vector points to output their coordinates
    }

    std::vector<Point> points2(4, Point("-", 1.0, 1.0)); // Create a vector of Point objects named vecPoints2 and initialize it with two Point objects
    std::cout << "First for loop:" << std::endl;
    for (size_t i = 0; i < points2.size(); i++)
    {
        points2[i].set_name("#" + std::to_string(i)); // Set the name of each Point object in the vector points2 to "point i" where i is the index of the Point object
        points2[i].print();                           // Call the print method for each Point object in the vector points2 to output their coordinates
    }

    std::cout << "Second for loop:" << std::endl;     
    for (Point element : points2) // Use a range-based for loop to iterate through each Point object in the vector points2
    {
        element.place(0, 0, 0); // Call the place method for each Point object in the vector points2 to set their coordinates to (0, 0, 0)
        element.print(); // Call the print method for each Point object in the vector points2 to output their coordinates
    }

    // element is a copy of the Point object in the vector points2, so modifying element does not affect the original Point objects in the vector points2
    std::cout << "Third for loop:" << std::endl;
    for (Point element : points2) // Use a range-based for loop with a
    {
        element.print(); // Call the print method for each Point object in the vector points2 to output their coordinates
    }

    // & is used to create a reference to the element in the vector, allowing us to modify the original Point objects in the vector points2
    std::cout << "Fourth for loop:" << std::endl;       
    for (const Point& element : points2) // Use a range-based for loop with a reference to iterate through each Point object in the vector points2
    {
        element.place(10, 10, 10); // Call the place method for each Point object in the vector points2 to set their coordinates to (10, 10, 10)
        element.print(); // Call the print method for each Point object in the vector points2 to output their coordinates
    }

    

    return 0;
}

Point::Point(std::string name, double x, double y)
{

    /*
    this->name = name; // Assign the provided name to the private member variable 'name'
    this->x = x;       // Assign the provided x-coordinate to the public member variable 'x'
    this->y = y;       // Assign the provided y-coordinate to the public member variable 'y'
    */

    this->set_name(name);   // Call the set_name method to set the name of the point
    this->place(x, y, 0.0); // Call the place method to set the coordinates of the point, with z-coordinate defaulting to 0.0
}

void Point::print() const
{                                                                   // Definition of the print method for the Point class
    std::cout << name << ": " << x << ", " << this->y << std::endl; // Method to print the coordinates of the point in the format "x, y"
}

void Point::place(double new_x, double new_y, double new_z)
{              // Definition of the place method for the Point class
    x = new_x; // Assign the provided x-coordinate to the public member variable 'x'
    y = new_y; // Assign the provided y-coordinate to the public member variable 'y'
    z = new_z; // Assign the provided z-coordinate to the private member variable 'z'
}