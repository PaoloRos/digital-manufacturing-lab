# Code styling guide

1) Step 1

* Use CamelCase for class names and namespaces
* Use snake_case for variable names and function names
* put private members at the end of a class
* use `_var_name` with leading underscore for private members

2) Step 2

* guarda foto
* il metodo to_csv_header va static
* we don't need <cmath> in the header -> drop it, just call what I do really need

(foto) _label = "vec_" + std::to_string(_vector_count++);

(foto) std::ostringstream -> salvare una stringa che poi verrà stampata, invece std::ostream -> stampa subito, ma quello si fa ad alto livello, non per parti di utility


Perché si usa static: qualcosa relativo alla classe globale, però è in uno scope della classe, permettendo di non andare a modificare questa variabile involontariamente come saccadrebbe per una globale

`size_t` used to count something
