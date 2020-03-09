// Dijkstra's Algorithm.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <inttypes.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;
#define Name_Length 15 //Determines maximum length of city name minus one

class City
{
public:
	char* name; //Name of a city
	char* w; //Way from source to a city, shortest or fastest, depends on function
	int d; //Distatnce of the shortest way
	int t; //Time it took to cover fastest way
private:
};

//Global Variables
int n = 0; //Stores number of cities
char* Arr[10000]; //Arr of pointers to strings with names od cities, from what i've seen it takes as much memory as *Arr[1], so I put 10k just to be sure to contain huge number of cities
int** Distance_Array; //Arr of distances between cities
int** Time_Array; //Arr of time of travel between cities
City* City_Array; //Pointer for Arr of cities
int From; //Index of a city from which we start our journey
int To; //Index of a city in which we end our journey
int u, v; //Variables used to store "coordinates" of cells in 2d Arr
int r;
int* Current_arr;//Arr that stores data which cities should be calculated in this turn
int* Next_arr;//Arr that stores data which cities shoudl be calculated in the next turn
char* q; //Variable for inputing cities in Check_Neighbourhood, later used in interface
char c;
int s = 0;
double** Coordinates_Array;

void Clear_Buffer()
{
	while ((c = getchar()) != '\n' && c != EOF)
	{
		getchar();
	}
}

void Check_City(const char* q) //Creates Arr of unique city names
{
	for (int i = 0; i < n; i++)
	{
		if (int((strcmp(q, Arr[i])) == 0))
		{
			return;
		}
	}
	Arr[n] = new char[Name_Length];
	if (Arr[n] == NULL)
	{
		perror("Error allocating memory. Shutting down.");
	}
	strcpy_s(Arr[n], Name_Length, q);
	n++;
}

void Create_Cities() //Create n <cities> structures
{
	City_Array = (City*)new City[n];
	for (int i = 0; i < n; i++)
	{
		City_Array[i].name = (char*)new char[Name_Length];
		strcpy(City_Array[i].name, Arr[i]);
		City_Array[i].w = (char*)new char[((Name_Length + 2) * n)];
		strcpy(City_Array[i].w, "");
		City_Array[i].d = 1000000;//Since this program is not designed for space travel, I guess milion km is close enough to infinity
		City_Array[i].t = 1000000;//See above
	}
}

void Reset_Cities()
{
	for (int i = 0; i < n; i++)
	{
		strcpy(City_Array[i].w, "");
		City_Array[i].d = 1000000;
		City_Array[i].t = 1000000;
	}
}

void Load_Distances() //Program assumes that input is in |City_A City_B Distance Average_Speed| format
{// Function actually reads from file both distances and average speeds, then creates Arrays of distances and time of travel
	FILE* f;
	fopen_s(&f, "Distances.txt", "r");
	if (f == NULL)
	{
		cout << "<<<No such file or directory, please check if program location contains \"Distances.txt.\">>>" << endl;
		s = 4;
		return;
	}
	char* p;//Pointer for strtok
	const char* r;//Pointer for strtok
	char* x;//Pointer for fgets
	char buff[3 * Name_Length]; //Buffer for fgets
	double tmp;
	do
	{
		x = fgets(buff, 3 * Name_Length, (FILE*)f); //Gets single line from a file
		if (x != NULL)
		{
			r = strtok_s(x, " ", &p); //Gets characters from said line, until it encounters " "
			Check_City(r);
			r = strtok_s(NULL, " ", &p);// strtok happens two times, because we know that file is in |City_A City_B Distance Max_Speed| format, so first two entries on each line are cities 
			Check_City(r);
		}
	} while (x != NULL);
	//We have Arr with pointers to strings with unique names of the cities
	Distance_Array = (int**)new int* [n];
	for (int i = 0; i < n; i++)
	{
		Distance_Array[i] = (int*)new int[n]; //Creating 2d Arr of size n*n, while n is equal to number of cities
	}
	Time_Array = (int**)new int* [n];
	for (int i = 0; i < n; i++)
	{
		Time_Array[i] = (int*)new int[n]; //Creating 2d Arr of size n*n, while n is equal to number of cities
	}
	for (int i = 0; i < n; i++)//Setting all values that won't be defined to -1
	{
		for (int j = 0; j < n; j++)
		{
			if (i == j)
			{
				Distance_Array[i][j] = 0; // Both distance, and time from A to A shall be 0
				Time_Array[i][j] = 0;
			}
			else
			{
				Distance_Array[i][j] = -1;
				Time_Array[i][j] = -1;
			}
		}
	}
	rewind(f);
	do //Now getting values into distance and time Arr
	{
		x = fgets(buff, 3 * Name_Length, (FILE*)f); //Gets single line from a file
		if (x != NULL)
		{
			r = strtok_s(x, " ", &p); //Gets characters from said line, until it encounters " "
			for (int i = 0; i < n; i++)
			{
				if (int((strcmp(r, Arr[i])) == 0))
				{
					u = i;
					break;
				}
			}
			r = strtok_s(NULL, " ", &p);
			for (int i = 0; i < n; i++)
			{
				if (int((strcmp(r, Arr[i])) == 0))
				{
					v = i;
					break;
				}
			}
			r = strtok_s(NULL, " ", &p);
			tmp = int(strtoimax(r, NULL, 0));
			Distance_Array[v][u] = tmp;
			Distance_Array[u][v] = Distance_Array[v][u];
			r = strtok_s(NULL, " ", &p);
			tmp = int(strtoimax(r, NULL, 0));
			Time_Array[v][u] = (((Distance_Array[u][v]) / tmp) * 60) + 1; //Travel time is stored in minutes, I could've made Arr of doubles to store minutes including after the decimal point, but I decided against it, hence +1
			Time_Array[u][v] = Time_Array[v][u];
		}
	} while (x != NULL);
	fclose(f);
}

void Engage_Short_Dijkstra()
{
	int Sroute = 0;//If algorith is finished it's set to 1, breaking while loop
	int c = 0;//Counter for Next_arr
	Current_arr = (int*)new int[n];
	Next_arr = (int*)new int[n];
	for (int i = 0; i < n; i++)
	{
		Current_arr[i] = -1;
		Next_arr[i] = -1;
	}
	Next_arr[0] = u;
	City_Array[u].d = 0;
	strcpy(City_Array[u].w, Arr[u]);
	strncat(City_Array[u].w, ", ", 5);
	while (Sroute != 1)
	{
		c = 0;
		for (int i = 0; i < n; i++)
		{
			Current_arr[i] = Next_arr[i];
			Next_arr[i] = -1;
		}
		for (int i = 0; i < n; i++)
		{
			if (Current_arr[i] > -1)
			{
				u = Current_arr[i];//Currently calculated city
				for (int j = 0; j < n; j++)
				{
					if (Distance_Array[u][j] > 0)
					{
						if (int(City_Array[u].d + Distance_Array[u][j]) < int(City_Array[j].d))
						{
							City_Array[j].d = City_Array[u].d + Distance_Array[u][j];
							strncpy(City_Array[j].w, City_Array[u].w, (Name_Length * n));
							strncat(City_Array[j].w, Arr[j], (Name_Length * n));
							strncat(City_Array[j].w, ", ", (Name_Length * n));
							Next_arr[c] = j;
							c++;
						}
					}
				}
			}
		}
		if (Next_arr[0] == -1)
		{
			Sroute = 1;
			break;
		}
	}
	cout << City_Array[v].d << " km" << endl;
	cout << "The way leads through: " << City_Array[v].w << endl;
	delete[] Current_arr;
	delete[] Next_arr;
	Reset_Cities();
}

void Engage_Fast_Dijkstra()
{
	int Sroute = 0;//If algorith is finished it's set to 1, breaking while loop
	int c = 0;//Counter for Next_arr
	Current_arr = (int*)new int[n];
	Next_arr = (int*)new int[n];
	for (int i = 0; i < n; i++)
	{
		Current_arr[i] = -1;
		Next_arr[i] = -1;
	}
	Next_arr[0] = u;
	City_Array[u].t = 0;
	strcpy(City_Array[u].w, Arr[u]);
	strncat(City_Array[u].w, ", ", 5);
	while (Sroute != 1)
	{
		c = 0;
		for (int i = 0; i < n; i++)
		{
			Current_arr[i] = Next_arr[i];
			Next_arr[i] = -1;
		}
		for (int i = 0; i < n; i++)
		{
			if (Current_arr[i] > -1)
			{
				u = Current_arr[i];//Currently calculated city
				for (int j = 0; j < n; j++)
				{
					if (Time_Array[u][j] > 0)
					{
						if (int(City_Array[u].t + Time_Array[u][j]) < int(City_Array[j].t))
						{
							City_Array[j].t = City_Array[u].t + Time_Array[u][j];
							strncpy(City_Array[j].w, City_Array[u].w, (Name_Length * n));
							strncat(City_Array[j].w, Arr[j], (Name_Length * n));
							strncat(City_Array[j].w, ", ", (Name_Length * n));
							Next_arr[c] = j;
							c++;
						}
					}
				}
			}
		}
		if (Next_arr[0] == -1)
		{
			Sroute = 1;
			break;
		}
	}
	cout << ((City_Array[v].t) / 60) << ":" << ((City_Array[v].t) % 60) << endl;
	cout << "The way leads through: " << City_Array[v].w << endl;
	delete[] Current_arr;
	delete[] Next_arr;
	Reset_Cities();
}

void Check_Short_Neighbourhood()
{
	cout << "Please input city of departure, then press <Enter>: ";
	r = scanf("%s", q);
	cout << endl;
	u = -1;
	v = -1;
	if (r == 0)
	{
		Clear_Buffer();
		cout << "Invalid input\n" << endl;
		Check_Short_Neighbourhood();
	}
	else
	{
		Clear_Buffer();
	}
	for (int i = 0; i < n; i++)
	{
		if (int((strcmp(q, Arr[i])) == 0))
		{
			u = i;
			break;
		}
	}
	if (u == -1)
	{
		Check_Short_Neighbourhood();
		return;
	}
	cout << "Please input city you wish to reach, then press <Enter>: ";
	r = scanf("%s", q);
	cout << endl;
	if (r == 0)
	{
		Clear_Buffer();
		cout << "Invalid input\n" << endl;
		Check_Short_Neighbourhood();
	}
	else
	{
		Clear_Buffer();
	}
	for (int i = 0; i < n; i++)
	{
		if (int((strcmp(q, Arr[i])) == 0))
		{
			v = i;
			break;
		}
	}
	if (v == -1)
	{
		cout << "Invalid input\n" << endl;
		Check_Short_Neighbourhood();
	}
	if (Distance_Array[u][v] == 0)
	{
		cout << "You are where you want to be!\n" << endl;
	}
	else if (Distance_Array[u][v] > 0)
	{
		cout << "Shortest way from " << Arr[u] << " to " << Arr[v] << " is: " << Distance_Array[u][v] << "km" << endl;
		cout << "There is a direct route between these cities." << endl;
	}
	else
	{
		cout << "Shortest way from " << Arr[u] << " to " << Arr[v] << " is: ";
		Engage_Short_Dijkstra();
	}
}

void Check_Fast_Neighbourhood()
{
	cout << "Please input city of departure, then press <Enter>: ";
	r = scanf("%s", q);
	cout << endl;
	if (r == 0)
	{
		Clear_Buffer();
		cout << "Invalid input" << endl;
		Check_Short_Neighbourhood();
	}
	else
	{
		Clear_Buffer();
	}
	for (int i = 0; i < n; i++)
	{
		if (int((strcmp(q, Arr[i])) == 0))
		{
			u = i;
			break;
		}
	}
	cout << "Please input city you wish to reach, then press <Enter>: ";
	r = scanf("%s", q);
	cout << endl;
	if (r == 0)
	{
		Clear_Buffer();
		cout << "Invalid input\n" << endl;
		Check_Short_Neighbourhood();
	}
	else
	{
		Clear_Buffer();
	}
	for (int i = 0; i < n; i++)
	{
		if (int((strcmp(q, Arr[i])) == 0))
		{
			v = i;
			break;
		}
	}
	if (Distance_Array[u][v] == 0)
	{
		cout << "\nYou are where you want to be!" << endl;
	}
	else if (Distance_Array[u][v] > 0)
	{
		cout << "\nFastest way from " << Arr[u] << " to " << Arr[v] << " it takes: " << ((Time_Array[u][v]) / 60) << ":" << ((Time_Array[u][v]) % 60) << endl;
		cout << "There is a direct route between these cities." << endl;
	}
	else
	{
		cout << "\nFastest way from " << Arr[u] << " to " << Arr[v] << " is ";
		Engage_Fast_Dijkstra();
	}
}

void Print_Cities()
{
	cout << "List of cities:" << endl;
	for (int i = 0; i < n; i++)
	{
		cout << Arr[i] << endl;
	}
}

void Interface()
{
	if (s != 4)
	{
		cout << "Welcome to Dijkstra's Algorithm" << endl;
		cout << "=====================================================" << endl;
	}
	while (s != 4)
	{
		cout << "1. Find shortest way between two cities" << endl;
		cout << "2. Find fastest way between two cities" << endl;
		cout << "3. Show list of possible cities" << endl;
		cout << "4. Exit" << endl;
		cout << "Please choose number related to action you want to preform and press <Enter> to confirm: ";
		r = scanf("%1d", &s);
		cout << endl;
		if (r == 0)
		{
			Clear_Buffer();
			continue;
		}
		else
		{
			Clear_Buffer();
		}
		if (s == 1)
		{
			Check_Short_Neighbourhood();
		}
		else if (s == 2)
		{
			Check_Fast_Neighbourhood();
		}
		else if (s == 3)
		{
			Print_Cities();
		}
		else if (s == 4)
		{
			cout << "You choose to exit the program. Shutting down" << endl;
			break;
		}
		else
		{
			cout << "\nInvalid input. Please try again\n" << endl;
		}
		s = 0;
		cout << "=====================================================" << endl;
	}
}

int main()
{
	Load_Distances();
	Create_Cities();
	q = (char*)new char[Name_Length];
	Interface();
	return 0;
}