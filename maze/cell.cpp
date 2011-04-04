//*******************************************************************************
//*******************************************************************************
//Author:  Jamie Levy
//CS 335 Homework #3
//Due: 5/10/02		
//			
//Filename: Cell.cpp
//			
//Purpose:		
//	This class represents a single cell or square in order to easily create 
//	mazes.  Each cell is a single square of a maze.				
//										
//										
//Created: 4/28/02								
//Last Date Modified: 5/5/02							
//	Extra Comments added        						
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

#include <iostream>
#include <fstream>
#include <sstream>
#include "Cell.h"

using namespace std;


/**
*Default constructor.  
*Precondition: None.
*Postcondition: topVal, leftVal, bottomVal, rightVal, top, bottom
*left, right, and name are initialized.
*/
Cell::Cell():topVal(TOP),leftVal(LEFT),bottomVal(BOTTOM),rightVal(RIGHT),
	top("_"),bottom("_"),left("|"),right("|"),name(-1){}

/**
*Precondition: wallVal is the value of the wall to be accessed.
*Postcondition: If the wall has either not been destroyed (set to 0)
*or set to -1 (in order to keep them inaccessible) such as outer walls
*the wall will be "destroyed" by being set to 0 and its text value 
*will be set to blank " " in order to print the cell out more 
*efficiently.  The function then returns either true if the was was
*indeed destroyed or false if it was not.
*/
bool Cell::destroyWall(int wallVal){
	bool b= false;
	switch(wallVal){
	case TOP:
		if(topVal==0||topVal==-1)
			b= false;
		else{
			topVal= 0;
			top= " ";
			b= true;
		}
		break;

	case LEFT:
		if(leftVal==0||leftVal==-1)
			b= false;
		else{
			leftVal= 0;
			left= " ";
			b= true;
		}
		break;

	case BOTTOM:
		if(bottomVal==0 || bottomVal==-1)
			b=false;
		else{
			bottomVal= 0;
			bottom= " ";
			b= true;
		}
		break;

	case RIGHT:
		if(rightVal==0 || rightVal==-1)
			b= false;
		else{
			rightVal= 0;
			right= " ";
			b= true;
		}
		break;

	}//end switch

	return b;

}//end destroyWall

/**
*Precondition: wallVal is the value of the wall to be set and 
*newVal is the value to which it is to be set.
*Postcondition: The wall with the corresponding wall value that
*is given is then set to the newVal given by the user.
*/

void Cell::setWall(int wallVal, int newVal){
	switch(wallVal){
	case TOP:
		topVal= newVal;
		break;

	case LEFT:
		leftVal= newVal;
		break;

	case BOTTOM:
		bottomVal= newVal;
		break;

	case RIGHT:
		rightVal= newVal;
		break;

	}//end switch

}//end setWall

/**
*Precondition: newName is the new name to be given
*to the cell.
*Postcondition: The current name of the cell (which 
*corresponds to its current set) is changed to the 
*new value given.
*/
void Cell::setName(int newName){

	name= newName;

}//end setName

/**
*Precondition: None.
*Postcondition: Returns the current name (set)
*of the cell.
*/
int Cell::getName()const{

	return name;

}//end getName

/**
*Precondition: rhs is the Cell that is to which this
*is compared.
*Postcondition: Returns true if this Cell's name is 
*less than the Cell to the right hand side.  False
*otherwise.
*/
bool Cell::operator <(const Cell &rhs)const{

	return (name < rhs.name);
		
}//end operator<

/**
*Precondition: rhs is the Cell that is to which this
*is compared.
*Postcondition: Returns true if this Cell's name is 
*equal to the Cell to the right hand side. False
*otherwise.
*/
bool Cell::operator ==(const Cell &rhs)const{

	return (name == rhs.name);

}//end operator==

/**
*Precondition: rhs is the Cell this is compared
*to and which may be copied by this.
*Postcondition: If this Cell's name differs from that
*of rhs, this Cell copies the name of rhs and this is 
*returned.
*/
Cell& Cell::operator =(const Cell &rhs){

	if(this != &rhs){

		name= rhs.name;

	}//end if

	return *this;

}//end operator= 

/**
*Precondition: os is the output stream designated by
*the user in order to print out the Cell.
*Postcondition: Prints out the bottom and right sides
*of the Cell.
*/
void Cell::print(ostream &os){

	os<< bottom << right;

}//end print


/**
*Allows access to the bottom and right side representations.
*Precondition: x is to be the character representation
*for the bottom side and y is to be the character representation
*of the right side.
*Postcondition: Mutates x and y to the character symbols that
*represent the bottom and right sides.
*/
void Cell::getSides(string &x, string &y){

	x= bottom;
	y= right;

}//end getSides






