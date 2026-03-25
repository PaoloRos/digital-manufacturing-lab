#pragma once

#include <cstddef>
#include <ostream>
#include <string>

class Vector2D {
public:
    Vector2D();
    Vector2D(double x, double y);
    Vector2D(const std::string& label, double x, double y);

    void set_label(std::string label);
    void set(double x, double y);

    double x() const;
    double y() const;
    std::string label() const;

    double norm() const;
    double distance_to(const Vector2D& other) const;

    Vector2D operator+(const Vector2D& other) const;
    Vector2D operator-(const Vector2D& other) const;
    Vector2D operator*(double scalar) const;

    static std::string to_csv_header();
    std::string to_csv_row() const;

private:
    std::string _label;
    double _x;
    double _y;
    static size_t _csv_counter;
};

std::ostream& operator<<(std::ostream& os, const Vector2D& v);

