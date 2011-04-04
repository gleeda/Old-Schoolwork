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
//Last Date Modified: 5/7/02													
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
//	Modified the unionSets() and find() functions in order to accommodate the Cell 
//		class. 
//Modified 5/2/02:
//	Have rewritten the unionSets() function in order to check which sets the two 
//		roots are members of and then to make the one with the larger number a
//		member of the one with the smaller number.
//	When the code was run as originally written there were errors indicating that
//		unauthorized parts of the array had been accessed.  There were names being
//		generated that were too large in other words.
//Modified 5/5/02:
//	Changed the print function to just print out the current cell and moved the 
//		rest of the code to the maze class.  It made more sense for the maze to 
//		print out a maze than for the DisjSets to do so.
//Modified 5/6/02:
//	Created a getSides() function in order to access the current character 
//		representations of the the bottom and right sides of the desired cell.


#include "DisjSets.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

/**
*Default constructor.
*/

DisjSets::DisjSets(){}

/**
* Construct the disjoint sets object.
* numElements is the initial number of disjoint sets.
*/
DisjSets::DisjSets( int numElements ) : s( numElements )
{
	for( int i = 0; i < s.size( ); i++ )
       		s[ i ].setName(-1);
}

/**
* Union two disjoint sets.
* For simplicity, we assume root1 and root2 are distinct
* and represent set names.
* root1 is the root of set 1.
* root2 is the root of set 2.
*/
void DisjSets::unionSets( int root1, int root2 )
{
	int temp= find(root1),temp2= find(root2);

	if(temp2<temp)
		s[temp].setName( temp2 );
		
		else{
			if(s[root2].getName() == s[root1].getName())
				s[temp].setName(s[temp].getName()-1);				

				s[temp2].setName(temp);

		}

}//end unionSets


/**
* Perform a find.
* Error checks omitted again for simplicity.
* Return the set containing x.
*/
//        int DisjSets::find( int x ) const
//      {
//
//				if( s[ x ].getName() < 0 )
//					return x;
//				else
//					return find( s[ x ].getName() );
  //      }//end find


/**
* Perform a find with path compression.
* Error checks omitted again for simplicity.
* Return the set containing x.
*/
int DisjSets::find( int x )
{
	if( s[ x ].getName() < 0 )
                return x;
       	else{
                s[ x ].setName( find( s[ x ].getName() ));
		        return s[ x ].getName();
	}
}//end find

/**
*Precondition: None.
*Postcondition: Returns the size of the private data member
*array s[].
*This is in order to check the size of the sets.
*/
int DisjSets::getSize()const{

	return s.size();

}//end getSize

/**
*Precondition: wallVal is the value of the wall to
*be destroyed and index is the index of the cell within
*the array.
*Postcondition: Returns true if the destroyWall instruction 
*was successful.
*/
bool DisjSets::destroyWall(int wallVal,int index){
	return s[index].destroyWall(wallVal);

}//end destroyWall

/**
*Precondition: wallVal is the value of the wall to be set,
*index is the index of the cell within the array and newVal
*is the newValue of the wallVal.  
*Postcondition: Sets the desired wall to the desired new value.
*This is in order to set some wall values in order to not select
*them when randomly selecting numbers.
*/
void DisjSets::setWall(int wallVal,int index,int newVal){

	s[index].setWall(wallVal,newVal);

}//end setWall


/*Prints out the current set.
*Precondition: os is the output stream requested by the user.
*int x is the height requested by the user and
*int y is the width as requested by the user.
*Postcondition: Prints out the current set.
*/
void DisjSets::print(ostream &os,int index){

	s[index].print(os);


}//end print


/**
*Allows an outside class to access the character representations 
*of the bottom and right sides of the proposed cell at the given index.
*Precondition: index is the index in the vector where the cell is
*residing.  bottom and right are the proposed strings to mutate to 
*the current character values of the bottom and right sides of the cell.
*Postcondition: bottom and right are mutated to the current values of the 
*bottom and right sides of the cell desired cell.
*/
void DisjSets::getSides(int index,string &bottom, string &right){

	s[index].getSides(bottom,right);

}//end getSides

		
