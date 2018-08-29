/*

BUILDING MODELLING.
Computer Graphic Project 2017-2018
Student: Federico Trombetti
ID: 1665803


The aim of this library is to provide an easier access to model building through
functions which make common transformations used to model building easier.

Cardinal directions (plus up and down) are used to orientate the programmer while modelling
the building through the functions.

The heart of this library resides in a structure called ModResult, which is what
most transformation functions will return. This structure lets the programmer access, in 
a simpler way, the new faces which the transformation created.

Function which subdivide faces without actually creating new shapes will instead return a vector
of integers, which represent the indices of the new faces creaeted in a row-major order, starting
from the top left of the face (with top it is meant the highest face in case of faces which span in height,
or the face wich is north-wise most far, for those faces wich span from south to north).

There are three type of functions:

	-Transformation functions:
			they modify the Building and return the proper result to continue working on the Building.

	-Finalization funcionts: 
			they modify the Building and then return nothing. Further modifications after these
			functions have to keep in mind that may produce weird visual mistakes if applied improperly.
			Some faces of a finalized Building cannot be modified correctly with the library itself.

	-Conversion functions:
			they are used to integrate the library with the Yocto one and will transform the Building
			to the desired Yocto structure.





Some conventions shared among the functions of the library:
	
	-All the functions take as first argument the Building to work with and as a second argument
	the face to modify, every other argument usually depends on the function.

	-Some functions which take in input a single face may have a generalization with a vector of
	faces (i.e. pullFace), they basically iterate the desired operation on every given face, check
	all the different overloads of the function to see which suits better your workflow.
	Note that the return value of these generalization function will have a different return value
	of the normal ones.


	
	
Examples are provided for a better understanding on how to use the library, feel free to check them.


*/

#include "../yocto/yocto_gl.h"
using namespace std::literals;


//pre-declare structures
struct Direction;
struct Building;
struct ModResult;
struct Face;


//----struct used for coordinate orientation, instantiated only once and----
//----                    used as a constant value                      ----
struct Direction {
	ygl::vec3f UP = { 0,0,1 };
	ygl::vec3f DOWN = { 0,0,-1 };
	ygl::vec3f NORTH = { 0,1,0 };
	ygl::vec3f SOUTH = { 0,-1,0 };
	ygl::vec3f EAST = { 1,0,0 };
	ygl::vec3f WEST = { -1,0,0 };
};
//--------------------------------------------------------------------------


//struture used to orientate result values after a trasformation
//whenever a transformation wich result in a new shape getting created is used,
//a ModResult is returned, where every value of it cointains the indices of the
//the faces of the new shape in the array of faces of the building.
struct ModResult {
	int up = 0;
	int down = 0;
	int north = 0;
	int south = 0;
	int east = 0;
	int west = 0;
};

struct Face {
	ygl::vec4i v;
	ygl::vec3f orientation;
};


//structure used to moodel buildings, very similar to the shape in the ygl library,
//but with some minor differences on variables for easier use.
//It is additionally free from all those values which are not directly related to the
//modelling of the building itself (i.e. textures)
struct Building {
	std::vector<Face> faces;
	std::vector<ygl::vec3f> points;
};

/* 
  Function used to create a ModResult from a transformation, it takes in input a
  vector of integers and a Building, and returns a ModResult with the values of the
  faces correctly assigned based on the indices and the Bulding given.
  INPUT:
		-faces	: the vector of faces to assign in the ModResult
		-b		: the Buiiding which those faces belongs to
  RETURN VALUE:
		The ModResult of the given faces
*/
ModResult createResult(std::vector<int> faces, Building* b);

/*
  Function used to create the basic shape of a building (which is a parallelepiped
  with the given size), it takes in input a Building b and three integers. The Building
  b values are correctly set to create the structure and the ModResult with the indices
  of the faces is returned.
  INPUT:
		-b		: the pointer to the building where the result wants to be saved
		-w,h,d	: the size values of the building
  RETURN VALUE:
		The ModResult of the new structure created.

*/
ModResult createBuilding(Building* b, float w, float h, float d);

/*
  Function used to divide a given face in four smaller faces. It takes
  in input a Building b and an integer n and divides the face n in
  four faces.
  INPUT:
		-b: the building to modify
		-n: the index of the face to subdivide
  RETURN VALUE:
		The vector with the indices of the new faces, sorted in
		row-major order starting from the top left of the face.
*/
std::vector<int> subdivideFace(Building* b, int n);


/*
  Function used to divide a given face in w*h smaller faces. It takes
  in input a Building b and three integers f,n,w and divides the face f in
  w*h faces.
  INPUT:
		-b: the building to modify
		-f: the index of the face to subdivide
		-w,h: the number of new faces to create (w*h)
  RETURN VALUE:
		The vector with the indices of the new faces, sorted in
		row-major order starting from the top left of the face.
*/
std::vector<int> subdivideFace(Building* b, int f, int w, int h);




std::vector<int> horizontalSlice(Building* b, int f, float h);



/*
  Function used to subdivide in 3x3 smaller faces with the central one with
  the size relative to the full face, multiplied by the xSub and ySub for every dimension.
  This function is usually used to create the shape of a door or a windows by using it
  in conjuction with pullFace and a negative value.
  xSub and ySub must be strictly smaller than one, otherwise visual errors may appear.
  INPUT:
		-b: the Building to modify
		-f: the face to subdivide
		-xSub, ySub: the sizes of the central face relative to the original face
					(they must be a positive number smaller than one)
  RETURN VALUE:
		The vector of indices of the faces. The central one will be the one in position
		4. The others are returned in case they want to be further modified.
*/
std::vector<int> subFace(Building* b, int f, float xSub, float ySub, float offsetX=0, float offsetY=0);


/*
	Generalization function. Apply subFace to every face in f. Use the values "from"
	and "to" to apply the function only to a slice of the complete vector
	RETURN VALUE:
			The vector of the faces in the middle obtained is returned.
			NOTE: The single function returns every face created, not just the central one.
*/
std::vector<int> subFace(Building* b, std::vector<int> f, float xSub, float ySub, int from=0, int to=0, float offsetX=0, float offsetY=0);


//void Building::subdivideFace(int n, int f) {};


/*
  Function used to pull out a block starting from a face.
  It takes in input a Building b an integer f and a float value and
  modify the Building by pulling the face f of a value amount.
  The ModResult of the transformation is returned.
  INPUT:
		-b: the building to modify
		-f: the index of the face to pull
		-value: how much the face has to be pulled
		(NOTE: the face can also be pushed by giving a negative value, but
		some visual error may appear if you are pushing an entire face inside,
		as connceted faces will overlap exactly on the same position. Push can be 
		done without imperfections if the face doesn't have perpendicular 
		connected faces to the starting one)
  RETURN VALUE:
		The ModResult of the transformation.
*/
ModResult pullFace(Building *b, int f, float value);


/*
	Generalization function. Apply pullFace to every face in f. Use the values "from"
	and "to" to apply the function only to a slice of the complete vector
	RETURN VALUE:
			The vector of the different ModResult(s) is returned.

*/
std::vector<ModResult> pullFace(Building* b, std::vector<int> f, float value, int from = 0, int to = 0);



/*
  Function used to apply pointFace to all the faces pointing up.
  NOTE: The use of this function is discouraged, not all faces which point up should be
  a roof.
  INPUT:
		-b: the building to modify
		-axis: the axis that the roofs top will follow when created
  RETURN VALUE:
		None.

*/
void makeRoofs(Building* b, ygl::vec3f axis, int amnt);


/*
  Function used to make a face wich points up a roof.
  Note that this function is final and won't return anything to work on,
  the roofs are a shape which is not exactly aligned to any cardinal direction
  and cannot be modified anymore by using the library, operations using the
  normals can still be applied (as the vectors of the faces are all sorted in
  clock-wise order).
  You can still apply the same function to point up other faces, as the indices
  of the old faces won't be touched.
  INPUT:
		-b: the building to modify
		-f: the index of face to transform into a roof
		-axis: the axis that the roofs top will follow when created
  RETURN VALUE:
		None.

*/
void pointFace(Building* b, int f, ygl::vec3f axis, int amnt);


/*
  Function used to transform a building to a ygl::shape.
  It takes in input a Building b and a material mat and creates a shape
  with that material.
  INPUT:
		-b: the building to transform
		-mat: the material to apply to the building
  RETURN VALUE:
		The ygl::shape of the building.

*/
ygl::shape* buildingToShape(Building* b, ygl::material* mat);


/*
  Function used to transform a building to a ygl::instance.
  It takes in input a Building b and a material mat and creates an instance
  with that material.
  INPUT:
		-b: the building to transform
		-mat: the material to apply to the building
  RETURN VALUE:
		The ygl::instance of the building.

*/
ygl::instance* buildingToInstance(Building* b, ygl::material* mat);


/*
  Function used to add a Building to given scene. It takes in input a
  Building b, a material mat and a scence scn and add the Building to the 
  scene with the given material applied.
  INPUT:
		-b: the Building to add to the scene
		-scn: the scene to add the Building to
		-mat: the material to apply to the Building.
  RETURN VALUE:
		None.
*/
void addBuildingToScene(Building* b, ygl::scene* scn, ygl::material* mat);