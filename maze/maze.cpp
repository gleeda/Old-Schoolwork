/*******************************************************************************
********************************************************************************
Author:  Jamie Levy							
CS 335 Homework #3															
Due: 5/10/02																	
																
Filename: maze.cpp															
																				
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "maze.h"
#include "randgen.h"
#include "canvas.h"

using namespace std;


/**
*Default constructor.
*/
Maze::Maze(){}

/**
*Constructor.
*Precondition: x is the height of the maze.
*y is the width of the maze. size is the 
*total number of cells constructing the maze
*or size of the maze.
*Postcondition: A maze is created.
*/
Maze::Maze(int x,int y, int size):myHeight(x),myWidth(y),
					mySize(size),mySets(size){

	for(int i= 0; i<size; i++)
		myList.push_back(i);	

}//end constructor


/**
*Sets up the maze by destroying the outer walls of
*the first and last index (entrance and exit) and then
*by setting all outer walls as off limits (or -1).
*Precondition: None.
*Postcondition: The maze is initialized by setting up
*the maze by destroying the outer walls of
*the first and last index (entrance and exit) and then
*by setting all outer walls as off limits (or -1).
*/

void Maze::setUpMaze(){
	int last= (mySize-1);

	//destroying the outside walls of the first and last indeces
	mySets.destroyWall(TOP,0);
	mySets.destroyWall(LEFT,0);
	mySets.destroyWall(BOTTOM,last);
	mySets.destroyWall(RIGHT,last);

	for(int i=0; i<mySize; i+=myWidth){
		//going through one row at a time
		for(int m=0,k=i; m < myWidth; k++,m++){
			//going through one cell at a time
			if(i==0)	//the first row
				mySets.setWall(TOP,k,-1);

			if(k%myWidth==0 && i!=0)	//the far left side
				mySets.setWall(LEFT,k,-1);

			if(m==(myWidth-1) && k!= last)	//the far right side
				mySets.setWall(RIGHT,k,-1);

		}//end for

	}//end for

	for(int j= last, p=myWidth; p>0; p--,j--)	//the bottom row
		mySets.setWall(BOTTOM,j,-1);

}//end setUpMaze


/**
*Randomly picks and destroys walls. When a wall
*is destroyed the neighboring cells are then placed
*within the same set.
*Called until a maze is created.
*Precondition: None.
*Postcondition: Randomly picks and destroys walls by generating
*random numbers for the indeces and the walls.  When these are 
*selected, the appropriate function corresponding with the 
*wall that was picked: checkBottom, checkTop, checkLeft, or checkRight.
*If the chosen index is in set zero afterward, it is removed from the 
*list of choices.  
*/
void Maze::createMaze(){
	int temp,side,index;		//index and side to be randomly selected
	int size= mySize-1;			//legal last index
	RandGen gen;				//random number generator

	list<int>::iterator i;		//list iterator
//	if(!myList.empty())
	temp=	gen.RandInt(0,myList.size()-1);	//randomly selecting index
	side=	gen.RandInt(1,4);				//randomly selecting side

	int j=0;
	for(i= myList.begin() ; j<myList.size(); i++,j++)
		if(j==temp){
			index= *i;
			break;
		}//end if and for	

	if(side==BOTTOM)
		checkBottom(size,side,index);

	else if(side==RIGHT)
		checkRight(size,side,index);

	else if(side==TOP)
		checkTop(size,side,index);

	else if(side==LEFT)
		checkLeft(size,side,index);	

	if(mySets.find(index)==mySets.find(0))
		//if index is not in set zero, insert back into list of choices.
		myList.erase(i);			

}//end createMaze


/**
*Precondition: None.
*Postcondition: Returns true if the first and last indeces
*are within the same set. i.e. there is a unique path
*from the entrance and exit. Returns false otherwise.
*/
bool Maze::checkIfMaze(){

	return myList.empty();

}//end checkIfMaze


/**
*Precondition: None.
*Postcondition: Prints out the maze.
*/
void Maze::print(ostream &os){

	os<<"  ";	//there is no top side for index 0

	for(int j=1; j<myWidth;j++)	//printing out top side 
		os<<" "<<"_";

	os<<endl<< " ";			//there is no left side for index 0

	for(int i= 0; i<mySize; i+=myWidth){
		//printing out rest of cells:
		if(i!= 0)
			os<<endl<<"|";		//printing out far left

		for(int m= 0,k=i; m < myWidth; k++,m++)
			mySets.print(os,k);				

	}//end for
	os<<endl;

}//end print


/**
*Prints out the graphical representation of the resulting maze.
*Precondition: None.
*Postcondition: Prints out the graphical representation of the
*maze using the Canvas class as written by Astrachan.  
*/
void Maze::createGraphic(){
	int height= myHeight*10;	//assuming that each cell is of size 10
	int width=  myWidth*10;
	string bottom,right;		//in order to dynamically set the title of 
									//of the maze to its dimensions
	int temp= myHeight;			//taking the values of myHeight and myWidth
	int temp2= myWidth;

	char x[20],y[20];			//temporary variables to get string representations
	string name= "MAZE: ";		//new name of maze to be set

	_ltoa(temp,x,10);			//setting the int values to strings
		
	name+= x;					//concatenating values to the name
	name+= " X ";

	_ltoa(temp2,y,10);			//setting the int values to strings
	name+= y;					//concatenating values to the name

	//Creating canvas, setting title and font color:
	Canvas c(width+SIZE+SIZE,height+SIZE+SIZE,100,100);
	c.SetTitle(name);
	c.SetColor(BLACK);

	c.DrawLine(Point(20,10),Point(width+10,10));		//drawing the top
	c.DrawLine(Point(10,20),Point(10,height+10));		//drawing the left

	for(int i= 0,l=20; i<mySize; i+=myWidth,l+=10){
		//i travels from row to row
		//l is the starting point on the Canvas for each row

		for(int m= 0,k=i,j=10; m < myWidth; k++,m++,j+=10){
			//m visits every cell on the row.
			//k is the current index.
			//j is the current point on the Canvas for each cell
			mySets.getSides(k,bottom,right);

			if(bottom=="_")
				c.DrawLine(Point(j,l),Point(j+10,l));

			if(right=="|")
				c.DrawLine(Point(j+10,l-10),Point(j+10,l));

		}//end for
	}//end for


	c.runUntilEscape();

}//end createGraphic


/**
*Called by the createMaze() function in order to check the bottom 
*side of the cell at the requested index.
*Precondition: size is the last legal index of the DisjSets mySets.
*side is the integer representation of the side to check and index 
*is the index of the cell whose side is to be checked.
*Postcondition: Checks to make sure that the index is valid then makes
*sure that the cell and its neighbor are not already within the same set
*and if they are not it tries to destroy the wall seperating them.  If 
*all of this is successful, the two neighbors are put into the same set
*using the unionSets() function and then if the neighbor is in set zero
*it is removed from the list of choices.
*/
void Maze::checkBottom(int size,int side, int index){
	list<int>::iterator i;
	if((side==BOTTOM) && (index < size-myWidth)){
		//"size - myY" means that the index is at least as 
		//high as the next to last row.
		if((mySets.find(index)!=mySets.find(index+myWidth))&&
				mySets.destroyWall(side,index)){	
			//make sure that the proposed indeces are not already in the 
			//same set and that the wall was destroyed.
			
			mySets.unionSets(index,index+myWidth);	//place cells at indeces 
												//into same set
			if(mySets.find(index+myWidth)==0){
				i= find(myList.begin(),myList.end(),index+myWidth);
				if(i!=myList.end())
					myList.erase(i);

			}//end if

		}//end if
	}//end if

}//end checkBottom


/**
*Called by the createMaze() function in order to check the bottom 
*side of the cell at the requested index.
*Precondition: size is the last legal index of the DisjSets mySets.
*side is the integer representation of the side to check and index 
*is the index of the cell whose side is to be checked.
*Postcondition: Checks to make sure that the index is valid then makes
*sure that the cell and its neighbor are not already within the same set
*and if they are not it tries to destroy the wall seperating them.  If 
*all of this is successful, the two neighbors are put into the same set
*using the unionSets() function and then if the neighbor is in set zero
*it is removed from the list of choices.
*/
void Maze::checkRight(int size,int side, int index){
	list<int>::iterator i;
	if((side==RIGHT)&&(index<size)){
		//make sure that index is not the last index
		if((mySets.find(index)!=mySets.find(index+1))&&
				mySets.destroyWall(side,index)){
			//make sure that the proposed indeces are not already in the 
			//same set and that the wall was destroyed.

			mySets.unionSets(index,index+1);	//place cells at indeces 
												//into same set
			if(mySets.find(index+1)==0){
				i= find(myList.begin(),myList.end(),index+1);
				if(i!=myList.end())
					myList.erase(i);

			}//end if
		}//end if
	}//end if

}//end checkRight


/**
*Called by the createMaze() function in order to check the bottom 
*side of the cell at the requested index.
*Precondition: size is the last legal index of the DisjSets mySets.
*side is the integer representation of the side to check and index 
*is the index of the cell whose side is to be checked.
*Postcondition: Checks to make sure that the index is valid then makes
*sure that the cell and its neighbor are not already within the same set
*and if they are not it tries to destroy the wall seperating them.  If 
*all of this is successful, the two neighbors are put into the same set
*using the unionSets() function and then if the neighbor is in set zero
*it is removed from the list of choices.
*/
void Maze::checkTop(int size,int side, int index){
	list<int>::iterator i;
	if((side== TOP) && (index >= myWidth)){
		//"index >= myY" means that the index is at least in the
		//second row.
		if((mySets.find(index)!=mySets.find(index-myWidth))&&
				mySets.destroyWall(BOTTOM,index-myWidth)){
			//make sure that the proposed indeces are not already in the 
			//same set and that the wall was destroyed.

			mySets.unionSets(index-myWidth,index);	//place cells at indeces 
												//into same set
			if(mySets.find(index-myWidth)==0){
				i= find(myList.begin(),myList.end(),index-myWidth);
				if(i!=myList.end())
					myList.erase(i);

			}//end if

		}//end if
	}//end if
}//end checkTop


/**
*Called by the createMaze() function in order to check the bottom 
*side of the cell at the requested index.
*Precondition: size is the last legal index of the DisjSets mySets.
*side is the integer representation of the side to check and index 
*is the index of the cell whose side is to be checked.
*Postcondition: Checks to make sure that the index is valid then makes
*sure that the cell and its neighbor are not already within the same set
*and if they are not it tries to destroy the wall seperating them.  If 
*all of this is successful, the two neighbors are put into the same set
*using the unionSets() function and then if the neighbor is in set zero
*it is removed from the list of choices.
*/
void Maze::checkLeft(int size, int side, int index){
	list<int>::iterator i;
	if((side== LEFT) && (index > 0)){
		//make sure that index is not the first one
		if((mySets.find(index)!=mySets.find(index-1))&&
				mySets.destroyWall(RIGHT,index-1)){
			//make sure that the proposed indeces are not already in the 
			//same set and that the wall was destroyed.

			mySets.unionSets(index-1,index);	//place cells at indeces 
												//into same set
			if(mySets.find(index-1)==0){
				i= find(myList.begin(),myList.end(),index-1);
				if(i!=myList.end())
					myList.erase(i);

			}//end if
		}//end if
	}//end if	

}//end checkLeft



