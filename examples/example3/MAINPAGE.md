# Vector2D Exercise

This documentation describes the public API of the Vector2D class used in example3.

## Project dependencies

The following graph summarizes the main dependencies in this project.

\dot
digraph Vector2DDependencies {
	rankdir=LR;
	node [shape=box, style=rounded];

	"main.cpp" -> "Vector2D.hpp";
	"Vector2D.cpp" -> "Vector2D.hpp";
	"vector2d_unit_tests.cpp" -> "Vector2D.hpp";
	"vector2d_smoke_test.cpp" -> "Vector2D.hpp";
}
\enddot

The class models a 2D vector with a label and offers:

- constructors for labeled and unlabeled vectors;
- basic operations (sum, difference, scalar multiplication);
- geometric utilities (norm and distance);
- CSV export and stream output helpers.

Implementation is present in `Vector2D.cpp`.
