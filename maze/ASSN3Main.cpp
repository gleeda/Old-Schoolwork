//*******************************************************************************
//*******************************************************************************
//Author:  Jamie Levy
//CS 335 Homework #3					
//Due: 5/10/02						
//							
//Filename: ASSN#3Main.cpp				
//								
//Purpose:							
//	Main file reqests the dimensions of the maze that is to be created.  Then a
//	maze is created by randomly deleting walls until the first and last elements
//	are in the the same set.							
//											
//											
//Created: 4/28/02									
//Last Date Modified: 5/8/02								
//	Extra Comments added        							
//											
//       This program is free software; you can redistribute it and/or
//       modify it under the terms of the GNU General Public License
//       as published by the Free Software Foundation; either version
//       2 of the License, or (at your option) any later version.
//
//											
//*******************************************************************************
//*******************************************************************************
//Modified 4/29/02:
//	Created functions createMaze and setUpMaze in order to test the DisjSets class.
//	Have encountered various problems.  The unionSets() function of the DisjSets
//		algorithm is suspect.  Names that are larger than the available indeces
//		are being created leading to unauthorized memory accesses.
//Modified 5/1/02:
//	Have changed the limits of the indeces to be accessed during a unionSets()
//		function call.  Hopefully this will alleviate the problem mentioned beforehand.
//	Unfortunately it has not.  Various trials and errors have occured here.
//Modified 5/2/02:
//	Have rewritten the unionSets() function in the DisjSets class and now there are no 
//		more errors.  Some sets are added into other sets more than once, however...
//Modified 5/4/02:
//	Have added extra check statements in order to check whether or not the proposed
//		sets for the unionSets() function call are not already within the same set.
//	Have taken the createMaze and setUpMaze functions out of this main file and made 
//		them part of the new Maze class.
//Modified 5/5/02:
//	Made a new function: generateMazes that will continue to generate multiple
//		mazes until the user has tired of them.
//Modified 5/7/02:
//	Created a new function: getMazeInfo() in order to make the generateMazes() function
//		less lengthy.
//Modified 5/8/02:
//	Created a new function: checkIfNum() in order to make sure that the dimensions entered
//		for the maze are indeed integers.
//	Created reset() in order to reset the character arrays to '\0' so that no previous 
//		will be confused with current.

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <ctype.h>
#include "Cell.h"
#include "DisjSets.h"
#include "RandGen.h"
#include "maze.h"
#include "canvas.h"

using namespace std;


#define SIZE 20

//prototypes:
void generateMazes(int height, int width, int size,char filename[]);
void getMazeInfo();
bool checkIfValid(int x,int y);
bool checkIfNum(char temp[]);
void reset(char temp[]);


/**
*Repeatedly generates mazes unless the user has grown 
*tired of doing so.
*Preconditions: None.
*Postconditions: Repeatedly generates mazes until
*requested to stop
*/
int main(){	
	string resp;

	while(true){

		getMazeInfo();

		cout<<"To quit press 'q', otherwise press a key"<<endl;
		cin>>resp;
		
		if(resp=="q")
			break;

	}//end while

	return 0;

}//end main

/**
*Gets the information for the maze to be created and
*then calls on generateMazes in order to create the maze.
*Preconditions: None.
*Postcondition: After obtaining the dimensions of the maze
*and the filename to which the maze will be printed, it then
*calls on generateMazes in order set up, generate and print
*out the maze.
*/
void getMazeInfo(){
	int x, y,size;
	char filename[SIZE],height[SIZE],width[SIZE];	
	reset(height); reset(width);

	do{		
		//getting dimensions:
		cout<<"Enter dimensions of maze: "<<endl;
		cout<<"Height: "<<endl;
		cin>>height;
		cout<<"Width: "<<endl;
		cin>>width;
		if(checkIfNum(height)&&checkIfNum(width)){
			x= atoi(height);
			y= atoi(width);
			if(checkIfValid(x,y))
				break;

		}//end if		

	}while(true);

	//getting filename in order to print:
	cout<<"Enter filename to print to: "<<endl;
	cin>>filename;
	
	size= x*y;				//calculating size

	cout<<"Wait for graphical representation: "<<endl;
	cout<<"Click graphical window with mouse in order to see maze."<<endl;
	cout<<"Hit escape to return to program from graphic window."<<endl<<endl;

	generateMazes(x,y,size,filename);	//creating maze 


}//end getMazeInfo


/**
*Takes in the dimensions of the maze and filename as
*given by the user and creates a maze with those 
*dimensions.  Then prints out the resulting maze.
*Preconditions: height is the height of the maze to be,
*width is the width of the maze, size is the total size
*of the maze and filename is the file to which the maze is
*to be written.
*Postcondition: Creates and prints out a maze.
*/
void generateMazes(int height, int width, int size,char filename[]){

	ofstream output(filename);	//file to which the maze will be printed

	//initializing maze:
	Maze theMaze(height,width,size);
	theMaze.setUpMaze();	

	while(!theMaze.checkIfMaze())
		//keep deleting walls until a maze is created
		theMaze.createMaze();	

	theMaze.print(output);	//printing out maze
	theMaze.createGraphic();

	output.close();			//closing the output file	

}//end generateMazes


/**
*Checks whether the requested dimensions are legal. 
*Returns true if they are within the limits and false otherwise.
*Preconditions: x and y are the dimensions to be checked.
*Postcondition: Returns true if the dimensions are legal and
*false otherwise.
*/
bool checkIfValid(int x, int y){
	bool isValid= true;
	if(x <= 2 || y <= 2 ){
		cout<<"Invalid dimensions: "<<x<<" "<<y<<endl;
		isValid= false;

	}else if(y > 50 || x >60){
		cout<<"Dimensions chosen are too large: "<<x<<" "<<y<<endl;
		isValid= false;

	}//end if
	
	return isValid;

}//end checkIfValid


/**
*Checks to make sure that the input is an integer.
*Precondition: temp is the char array that to be tested whether
*or not it is an integer.
*Postcondition: Returns true if the array is indeed an integer
*false otherwise.
*/
bool checkIfNum(char temp[]){
	
	bool isValid= true;
	for(int i=0; i<SIZE&&temp[i]!='\0'; i++){
		if(isdigit(temp[i])||temp[0]=='-'){
			;			

		}//end if
		else{

			isValid= false;
			cout<<"Invalid response: "<<temp<<endl;
			break;

		}//end else
	}//end for

	return isValid;

}//end checkIfNum


/*
*Resets the array to empty in order to keep info from a previous
*access from being mixed up with the new info.
*Precondition: temp is the array to be "reset"
*Postcondition: The array temp is "reset" to '\0'
*/
void reset(char temp[]){
	
	for(int i=0;i<SIZE;i++)
		temp[i]='\0';

}//end reset






