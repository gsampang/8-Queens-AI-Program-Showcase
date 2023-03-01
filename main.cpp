//CS441 Programming Assignment #2
//8-Queens
//Gabriel Sampang
//2/20/2022

#include <iostream>
#include <time.h>
#include <cmath>
#include <list>
#include <bits/stdc++.h>

using namespace std;

//Global Variables
const int N = 8;				//dealing w/ 8-queens
int X[N];						//generates each state
int temp = 0;					//used for random state generation
int R = 0;						//used in cross over function

//More variables for mutation function
int Rm = 0;
int counterM = 1;

//For assignment description
int PopulationSize = 100;	//Number of members. Should be constant from generation to generation
int NumIterations = 100;	//How many times to loop the program - repeat the breeding process
int MutationPct = 20;		//<- as is coded, means 5% chance of mutating. See mutation function

//Node -------------------------
struct node
{
	int state[N];
	int fitnessValue;
	node* location;		//Allows access of the node outside of an iterator

	node()
	{
		for (int i = 0; i < N; i++)
			state[i] = -1;

		fitnessValue = -1;

		location = this;
	}
};

//Global list of all solutions
list<node> solution;

//For plot statistics
struct pStatsNode 
{
	int generation;
	int averageFitness;
	pStatsNode* location;

	pStatsNode()
	{
		generation = -1;
		averageFitness = -1;
		location = this;
	}
};

//Global list of statistics
list<pStatsNode> pStats;


//General Data Structure Functions ------------------------------------------------------------
void printNode(node* curr)
{
	cout << "State: ";
	for (int i = 0; i < N; i++)		//adapted printf
	{
		printf("%d\t", curr->state[i]);
	}
	printf("\n");

	cout << "Fitness Value: " << curr->fitnessValue << endl;

}
void printList(list<node>& ll)
{
	int i = 0;

	//iterate through list and print each node
	for (list<node>::iterator it = ll.begin(); it != ll.end(); it++)
	{
		cout << "\nMember: " << ++i << endl;
		printNode(it->location);
	}
}
void destroyList(list<node>& ll)
{
	//iterate through list and delete each node
	for (list<node>::iterator it = ll.begin(); it != ll.end(); it++)
		delete it->location;

	ll.clear();
}
void sortedInsert(list<node>& ll, node* newNode)						//Also adds all solutions to the solution list.
{
	list<node>::iterator insertAfter = ll.begin();
	for (list<node>::iterator it = ll.begin(); it != ll.end(); it++)
	{
		//find it for largest value smaller than val
		if (it->fitnessValue <= newNode->fitnessValue)
		{
			insertAfter = it;
		}
	}
	list<node>::iterator insertAt = ++insertAfter;
	ll.insert(insertAt, *newNode);

	//Add to solution list if node is a solution
	if (newNode->fitnessValue == 28)
	{
		node* solutionNode = new node;

		for (int i = 0; i < N; i++)
			solutionNode->state[i] = newNode->state[i];

		solutionNode->fitnessValue = newNode->fitnessValue;

		solution.push_back(*solutionNode);
	}
}
void fixList(list<node>& ll)      //deals with a small bug in sortedInsert which misplaces the first in the list
{
	node* outOfPlace = ll.begin()->location;

	ll.pop_front();
	sortedInsert(ll, outOfPlace);
}
int calculateAverageFitness(list<node>& ll)
{
	int total = 0;

	for (list<node>::iterator it = ll.begin(); it != ll.end(); it++)
		total += it->fitnessValue;

	return total / PopulationSize;
}

//Fitness Function ----------------------------------------------------------------------------
void setFitness(int state[], int& fitnessValue)
{
	//Inspired by leftfinger and rightfinger pointers
	int lf = 0;
	int rf = 0;

	//Calculating Fitness
	fitnessValue = 0;

	for (lf = 0; lf < N; lf++)
	{
		for (rf = lf + 1; rf < N; rf++)
		{
			int y1 = lf + 1;
			int y2 = rf + 1;
			int x1 = state[lf];
			int x2 = state[rf];

			if (x1 != x2)                             //Checks if in same row
			{
				if (abs(y2 - y1) != abs(x2 - x1))        //Checks if in same diagonal.
					//derived from slope, where a slope of 1 means the same diagonal.
					//Only possible if delta y and delta x are the same
				{
					fitnessValue++;		//Means a pair of queens are not in the same row, column, or diagonal.
				}
			}
		}
	}
}

//Functions for generating the population ----------------------------------------------------
//Makes the array X of size N populated with random ints w/ a range of 1-N
void make_array()
{
	//populate the array.
	for (int i = 0; i < N; i++)
	{
		srand(time(0));

		temp += abs(rand());
		temp = (temp % N) + 1;

		X[i] = temp;	//now storing to the array.
	}

	temp += 1;      //Allows for 8 possible variations on the random array
}

//Function to find the permutations
void generateState(list<node>& ll)
{
	//Find all possible permutations
	node* newNode;

	int i = 0;

	int counter = 0;
	int limit = PopulationSize / N;

	while (i < PopulationSize) {                //May generate repeats, will generate complete population though

		if (counter == limit)
		{
			make_array();
			counter = 0;
		}
		else
			counter++;

		next_permutation(X, X + N);

		newNode = new node();
		for (int j = 0; j < N; j++)
			newNode->state[j] = X[j];

		setFitness(newNode->state, newNode->fitnessValue);
		sortedInsert(ll, newNode);

		i++;
	}
	fixList(ll);
}

//Breeding Functions --------------------------------------------------------------------------------------
//Crossover function built for anycase, designed from case N = 8.
/*
Generate random number R from 1-6.
Use number as index:
	Child1 will be P1 from index 0-(R-1) and P2 from index R-7.
	Child2 will be P2 from index 0-(R-1) and P1 from index R-7.
Using indexes 1-6 to prevent children clone of parents
*/
void crossover(int P1[], int P2[], int C1[], int C2[])
{
	srand(time(0));

	R += abs(rand());
	R = (R % (N - 1)) + 1;        //random num R in range of 1-6 for N = 8.

	//Create front part of Child 1 and 2
	for (int i = 0; i < R; i++)
	{
		C1[i] = P1[i];
		C2[i] = P2[i];
	}

	//Create back part of Child 1 and 2
	for (int i = R; i < N; i++)
	{
		C1[i] = P2[i];
		C2[i] = P1[i];
	}

	R += 1;     //Move R so next time uses different crossover point
}

/*
Use mutation counter (counterM) that goes from 1 until MutationPct
If counter == MutationPct
	Reset counter
	Mutate state
Else
	note that no mutation was done
0-7 as an index to mutate to a random value from 1-8 (cannot be what’s currently in there).
*/
void mutation(int state[])
{
	if (counterM == MutationPct)     //Mutate the state
	{
		srand(time(0));

		Rm += abs(rand());
		Rm = Rm % N;        //random num R in range of 0-7

		if (state[Rm] != Rm + 1)          //Mutate the state at this index to something different
			state[Rm] = Rm + 1;           //mutate range: 1-8
		else
			state[Rm] = ((Rm + 2) % N) + 1;

		counterM = 1; //reset counter
	}
	else    //don't mutate yet
		counterM++;
}
void breedNextGeneration(list<node>& parentGen, list<node>& childGen)
{
	node* parent1 = NULL;
	node* parent2 = NULL;
	node* child1 = NULL;
	node* child2 = NULL;

	for (list<node>::iterator it = parentGen.begin(); it != parentGen.end(); it++)
	{
		parent1 = it->location;
		it++;

		if (it != parentGen.end())      //Can only breed if there are 2 parents
		{
			child1 = new node;
			child2 = new node;

			parent2 = it->location;

			crossover(parent1->state, parent2->state, child1->state, child2->state);
			mutation(child1->state);
			mutation(child2->state);

			setFitness(child1->state, child1->fitnessValue);
			setFitness(child2->state, child2->fitnessValue);

			sortedInsert(childGen, child1);
			sortedInsert(childGen, child2);
		}
	}

	if (childGen.size() != 0)   //Only fix if there is an actual list
		fixList(childGen);
}



int main()
{
	list<node> list1;
	list<node> list2;

	//Take user input
	cout << "Enter population size: ";
	cin >> PopulationSize;
	cout << "Enter number of times to reproduce: ";
	cin >> NumIterations;

	//Simulation
	make_array();				//Initial random array to generate state permutations from
	generateState(list1);		//Create initial population

	cout << "\nInitial generation: -----------------------------------------------\n";
	printList(list1);

	//Save statistics
	pStatsNode* newPStatsNode = NULL;
	newPStatsNode = new pStatsNode;
	newPStatsNode->generation = 1;
	newPStatsNode->averageFitness = calculateAverageFitness(list1);
	pStats.push_back(*newPStatsNode);

	for (int i = 0; i < NumIterations; i++)
	{
		if ((i % 2) == 0)
		{
			if (list2.size() != 0)
				destroyList(list2);

			breedNextGeneration(list1, list2);      //Here list2 is the child genereation
			cout << "\nPrinting generation " << i + 2 << ": -----------------------------------------------\n";
			printList(list2);
			
			//Save statistics
			newPStatsNode = new pStatsNode;
			newPStatsNode->generation = i + 2;
			newPStatsNode->averageFitness = calculateAverageFitness(list2);
			pStats.push_back(*newPStatsNode);
		}
		else
		{
			if (list1.size() != 0)
				destroyList(list1);
	
			breedNextGeneration(list2, list1);      //Here list1 is the child generation
			cout << "\nPrinting generation " << i + 2 << ": -----------------------------------------------\n";
			printList(list1);
			
			//Save statistics
			newPStatsNode = new pStatsNode;
			newPStatsNode->generation = i + 2;
			newPStatsNode->averageFitness = calculateAverageFitness(list1);
			pStats.push_back(*newPStatsNode);
		}
	}
	//Deallocate any leftover memory from the simulation
	destroyList(list1);
	destroyList(list2);

	//Report Solutions if found:
	cout << "\n\nSimulation results:\n";
	if (solution.size() != 0)
	{
		cout << "Solutions found: (may be redundant) ----------------------------------------------------\n";
		printList(solution);
		destroyList(solution);
	}
	else
		cout << "No solutions found.\n";

	//Print pStats
	cout << "\nStatistics ------------------------------------------------------\n";
	for (list<pStatsNode>::iterator it = pStats.begin(); it != pStats.end(); it++)
	{
		cout << "Generation: " << it->generation << "\t\tAverage Fitness: " << it->averageFitness << endl;
	}

	//Deallocate all pStats memory
	for (list<pStatsNode>::iterator it = pStats.begin(); it != pStats.end(); it++)
		delete it->location;
	pStats.clear();

	return 0;
}
