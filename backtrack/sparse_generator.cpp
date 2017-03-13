#include <iostream>
#include <random>
#include <string>
#include <fstream>

using namespace std;


int RandU(int nMin, int nMax)
{
    return nMin + (int)((double)rand() / RAND_MAX * (nMax-nMin+1));
}

string RandomName()
{
    string str = "   ";
    str[0] = 'A' + RandU(0,25);
    str[1] = 'A' + RandU(0,25);
    str[2] = 'A' + RandU(0,25);

    return str;
}

int to_change(int day, int from, int to)
{
    return  day*300*300 + from*300 + to;
}

void from_change(int hashed, int &day, int &from, int &to)
{
    to = hashed % 300;
    from = (hashed / 300) % 300;
    day = hashed / 90000;
}


int main(int argc, char **argv)
{
    if(argc < 2)
    {
      printf("Not enough parameters\n");
      printf("Usage: sparse_generator n_cities rel_density\n");
      exit(1);
    }

    const int n_city = atoi(argv[1]); //POCET MIEST
    double full = atof(argv[2]); //RELATIVNA HUSTOTA

    bool visited[n_city];
    string City[n_city];
    int *paths = new int[27000000];



    int n_path = (int)(n_city-1) * full;
    int n_right = n_path;

    for (int d = 0; d<n_city; ++d)
    {
        for(int i = 0; i<n_city;++i)
        {
            int l = 0;

            for(int j = 0; j< n_path; ++j)
            {
                int ticker = to_change(d,i,l);
                int k = RandU(1, n_city);
                    while(k>0)
                    {
                      l = (l+1) % n_city;
                      ticker = to_change(d,i,l);
                      if(paths[ticker] == 0&&i!=l)
                      {
                            --k;
                      };
                    }
                paths[ticker] = RandU(500,1500);
            }
        }
    }

    int start_city = 0;

    for (int k = 0; k < n_right;++k)
    {
        for(int i = 0; i<n_city;++i)
        {
            visited[i] =false;
        }

        visited[start_city] = true;
        int last = start_city;

        for(int d = 0; d<n_city-1;++d)
        {
            int l = 0;

            int k = RandU(1,n_city);
            while(k>0)
            {
                l = (l+1)% n_city;
                if(!visited[l])
                {
                    --k;
                };
            }
            int ticker = to_change(d,last,l);
            if(paths[ticker] ==0)
            {
                paths[ticker] = RandU(500,1500);
            }

            visited[l] = true;
            last = l;
        }
        int ticker = to_change(n_city-1,last,0);
        if (paths[ticker] == 0)
        {
            paths[ticker] = RandU(500,1500);
        }
    }




    int l = 0;
    while(l<n_city)
    {
        string name = RandomName();
        bool exist = false;
        for(int i=0; i<l;++i)
        {
            if (name == City[i])
            {
                exist = true;
            }
        }
        if (!exist)
        {   City[l] = name;
            ++l;
        }
    }


    char buf[255];
    snprintf(buf, 254, "data_%d_%.0f.txt", n_city, 100 * full);
    std::string oname(buf);
    ofstream output(oname);
    if(output)
    {
        output << City[0] <<"\n";
        for (int d = 0; d<n_city; ++d)
        {
            for(int i = 0; i<n_city; ++i)
            {
                for(int j = 0; j<n_city; ++j)
                {
                    int ticker = to_change(d,i,j);
                    if(paths[ticker]!=0)
                    {
                        output << City[i] << " " << City[j] << " " << d << " " << paths[ticker] <<"\n";
                    }
                }
            }
        }
        output.close();
    }
    else
    {
        cout <<"not opened" <<endl;
    }


/*

        cout << City[0] <<"\n";
        for (int d = 0; d<n_city; ++d)
        {
            for(int i = 0; i<n_city; ++i)
            {
                for(int j = 0; j<n_city; ++j)
                {
                    int ticker = to_change(d,i,j);
                    if(paths[ticker]!=0)
                    {
                        cout << City[i] << " " << City[j] << " " << d << " " << paths[ticker] <<"\n";
                    }
                }
            }
        }

*/
    return 0;
}



