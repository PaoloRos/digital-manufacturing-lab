#include "Vector2D.hpp"

#include <iostream>

int main() {
    Vector2D a;
    Vector2D b(3.0, 4.0);
    Vector2D c("p1", 1.5, -2.0);

    c.set_label("point_1");
    c.set(2.0, -1.0);

    Vector2D sum = b + c;
    Vector2D diff = b - c;
    Vector2D scaled = b * 2.0;

    std::cout << "a = " << a << '\n';
    std::cout << "b = " << b << " norm=" << b.norm() << '\n';
    std::cout << "c = " << c << " dist(b,c)=" << b.distance_to(c) << '\n';

    std::cout << "sum = " << sum << '\n';
    std::cout << "diff = " << diff << '\n';
    std::cout << "scaled = " << scaled << '\n';

    std::cout << Vector2D::to_csv_header() << '\n';
    std::cout << b.to_csv_row() << '\n';
    std::cout << c.to_csv_row() << '\n';

    return 0;
}
