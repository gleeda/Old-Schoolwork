//*******************************************************************************
//*******************************************************************************
//Author:  Jamie Levy	(original author Mark Allen Weiss)		
//CS 335 Homework #3							
//Due: 5/10/02								
//									
//Filename: DisjSets.h							
//									
//Purpose:								
//	Makes the creating of mazes much easier by keeping the individual cells of	
//	a maze within an array of sets.  This way we are able to check whether or	
//	the first and last cell of the array are within the same set and therefore	
//	there is a unique path through the maze.					
//											
//											
//Created: 4/28/02									
//Last Date Modified: 5/5/02								
//	Extra Comments added        							
//											
//											
//*******************************************************************************
//*******************************************************************************
//Modified 4/29/02:
//	Created a getSize() function in order to tell the size of the current set array.
//Modified 5/01/02:
//	Created setWall(), destroyWall() and print() in order to make these functions 
//		from the Cell class accessible to outside classes.


#ifndef _DISJ_SETS_H_
#define _DISJ_SETS_H_

// DisjSets class
//
// CONSTRUCTION: with int representing initial number of sets
//
// ******************PUBLIC OPERATIONS*********************
// void union( root1, root2 ) --> Merge two sets
// int find( x )              --> Return set containing x
// ******************ERRORS********************************
// No error checking is performed

#include "Cell.h"
#include <vector>
#include <fstream>

/**
* Disjoint set class.
* Use union by rank and path compression.
* Elements in the set are numbered starting at 0.
*/
class DisjSets
{
	public:
		DisjSets();
        	explicit DisjSets( int numElements );

        	int find( int x );
        	void unionSets( int root1, int root2 );
		int getSize()const;
		void setWall(int wallVal,int index,int newVal);
		bool destroyWall(int wallVal,int index);
		void print(ostream &os,int index);// int x, int y);
		void getSides(int index,string &bottom, string &right);

	private:
             	vector<Cell> s;

};

#endif
