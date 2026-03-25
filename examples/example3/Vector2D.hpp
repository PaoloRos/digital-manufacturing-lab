

class Vector2D {
public:
  Vector2D();
  Vector2D(double x, double y);
  double x() const;
  double y() const;
  double norm() const;
  double distanceTo(const Vector2D& other) const;
  Vector2D operator+(const Vector2D& other) const;
  Vector2D operator-(const Vector2D& other) const;
  Vector2D operator*(double scalar) const;
private:
  double x_;
  double y_;
};


std::ostream& operator<<(std::ostream& os, const Vector2D& v);