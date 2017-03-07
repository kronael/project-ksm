#include <iostream>

#include "cityhash.cpp"

bool DEBUGGING_ENABLED = true;

using namespace std;

int main()
{
  cout << hashtag_in("AZE") << endl;
  cout << hashtag_out(hashtag_in("AZE")) << endl;
}
