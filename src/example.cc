/* This code exists just to give you an idea of how you can use Djinni
   in your own C++ code.  However, please consider using the Python
   libraries instead--they're much, much nicer on end users.  :) 
*/

#include <djinni/djinni.h>

using std::cout;
using std::endl;

int main()
{
	Compressed ca(0.06, 0.0, 0.9999);
	TravelingSalesman tsp("Dumas-1.set");
	Annealer<Compressed, TravelingSalesman>
		CA_TSP(ca, tsp, 0.95, 0.94, 75, 100, 30000);
	CA_TSP.solve();
	cout << CA_TSP << endl;

	return 0;
}

