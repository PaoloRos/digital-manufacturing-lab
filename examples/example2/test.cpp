// 19/03/2026
// Introduction to std::vector
//

// Riprendi l'esercizio:
//
// vettore di classi

// esercizio:
// un attributo con un solo `element`
// fatti una classe punto 2D --> eredita una classe punto 3D

#include <iostream>
#include <vector>

using namespace std;


int main()
{
  vector<int> vecInt(1);

  vecInt.push_back(1);

  cout << "ok:" << vecInt[0] << endl;


  return 0;
}