//*******************************************************************************
//*******************************************************************************
//Author:  Jamie Levy
//CS 335 Homework #3
//Due: 5/10/02		
//			
//Filename: Cell.h	
//			
//Purpose:		
//	This class represents a single cell or square in order to easily create 
//	mazes.  Each cell is a single square of a maze.				
//										
//										
//Created: 4/28/02								
//Last Date Modified: 5/5/02							
//	Extra Comments added        						
//       This program is free software; you can redistribute it and/or
//       modify it under the terms of the GNU General Public License
//       as published by the Free Software Foundation; either version
//       2 of the License, or (at your option) any later version.
//
//										
//										
//*******************************************************************************
//*******************************************************************************
//Modified 4/28/02:
//	Created setName(), setWall(), getName(), operator<,operator== and operator=.
//Modified 4/29/02:
//	Created print function in order to print out the cells more easily.
//Modified 5/1/02:
//	Modified destroyWall to be a boolean function in order to tell whether
//		the destruction of the wall was successfully completed or not.
//	Also modified the destroyWall function to set the current representations of
//		top, bottom, left, and right to blank in order to print the cells out 
//		more easily.
//Modified 5/2/02:
//	Changed the print function in order to just print out the bottom and right 
//		sides of the cell.  
//	Since the maze is represented essentially as a vector that is folded onto itself
//		the bottom and right sides of adjacent cells will serve as top and left sides
//		of their neighbors.
//	The topmost and leftmost sides of the maze will have to printed out manually.
//		They will, however, not be deleted because of the safeguard in the destroyWall
//		function.


#ifndef _CELL_H_
#define _CELL_H_


#include<iostream>
#include<fstream>

using namespace std;

#define TOP		1
#define LEFT		2
#define BOTTOM		3
#define RIGHT		4

class Cell{
public:
	Cell();
		//default constructor

	bool destroyWall(int wallVal);
		//destroys a given wall of the cell.

	void setName(int newName);
		//sets the name (or set) of the cell to one given by the user

	void setWall(int wallVal, int newVal);
		//sets a given wall to a given value (to set it out of bounds)

	int getName()const;
		//returns the name (or set) of the cell.

	void print (ostream &os);
		//prints out the bottom and right sides of the cell

	void getSides(string &x, string &y);
		//returns the current values of the bottom and right sides

	bool operator <(const Cell &rhs)const;

	bool operator ==(const Cell &rhs)const;

	Cell& operator =(const Cell &rhs);




private:
	int name;			//set that contains this cell

	int topVal,bottomVal,leftVal,rightVal;	
		//values to differentiate between the walls

	string top,bottom,left,right;
		//string value of the cell walls in order to print 

};





#endif
