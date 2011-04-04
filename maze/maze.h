/*******************************************************************************
********************************************************************************
Author:  Jamie Levy							
CS 335 Homework #3															
Due: 5/10/02																	
																
Filename: maze.h															
																				
Purpose:																		
	Main file reqests the dimensions of the maze that is to be created.  Then a 
	maze is created by randomly deleting walls until the first and last elements
	are in the the same set.													
																			
																				
Created: 5/4/02																
Last Date Modified: 5/7/02													
	Extra Comments added        												

    
       This program is free software; you can redistribute it and/or
       modify it under the terms of the GNU General Public License
       as published by the Free Software Foundation; either version
       2 of the License, or (at your option) any later version.
																				
																				
*******************************************************************************
*******************************************************************************/
//Modified 5/5/02:
//	Changed the print function to just print out the current cell and moved the 
//		rest of the code to the maze class.  It made more sense for the maze to 
//		print out a maze than for the DisjSets to do so.
//	Created createGraphic() function in order to print out a second window with 
//		a graphical representation of the maze.
//	Finished the extra credit graphic representation of the assignment.  Having
//		problems printing out the screen dump.
//Modified 5/6/02:
//	Added a linked list in order to keep up with cells that have not yet been added
//		to the zeroth set.  This was in order to cut down on run time.
//	Changed the createGraphic() function to print out a larger display of the maze.
//		The first one was too small to see.
//	Created checkBottom(), checkTop(), checkRight(), and checkLeft() in order to 
//		break up the createMaze() function.
//Modified 5/7/02:
//	Changed the createGraphic() function in order to change the title of the new
//		window to include the dimensions of the maze.


#ifndef _MAZE_H_
#define _MAZE_H_


#include<iostream>
#include<fstream>
#include<list>
#include<vector>
#include "disjsets.h"

using namespace std;

#define SIZE 20			//fudge dimensions to make room for text
#define TOP		1
#define LEFT	2
#define BOTTOM	3
#define RIGHT	4



class Maze{
public:
	//constructors:
	Maze();
	Maze(int x,int y,int size);

	void setUpMaze();
		//sets up the maze by setting all outside values to -1
		//and destroying the outside walls of the entrance and exit

	void createMaze();
		//creates the maze by randomly picking and destroying walls

	bool checkIfMaze();
		//checks whether the first and last cells (entrance and exit)
		//are within the same set- thus indicating that a maze has
		//been created.

	void print(ostream &os);
		//prints out the maze

	void createGraphic();
		//prints out graphical representation of the maze

	


private:
	void checkBottom(int size, int side, int index);	
	void checkRight(int size, int side, int index);	
	void checkTop(int size, int side, int index);
	void checkLeft(int size, int side, int index);


	int myHeight,myWidth;	//height and width of the maze respectively
	int mySize;				//total number of cells in the maze	
	DisjSets mySets;		//array of the sets that make up the maze
	list<int> myList;		//list of all possible cells not already contained
								//within set zero



};


#endif
