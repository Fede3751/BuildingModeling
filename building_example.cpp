#include "../yocto/yocto_gl.h"
using namespace std::literals;
#include "build.h"


int main(int argc, char* argv[])
{

	printf("Generating scene....\n");

	//crate scene
	auto scn = new ygl::scene();

	//create material for the floor
	auto mat = new ygl::material{ "floor" };
	mat->kd = { 0.2f, 0.2f, 0.2f };
	scn->materials.push_back(mat);


	//create material for the building
	auto lmat = new ygl::material{ "building_mat" };
	lmat->kd = { 0.6f, 0.6f, 0.6f };
	scn->materials.push_back(lmat);

	//start working on the building
	auto build = new Building{};

	//create the first shape of the building
	auto modStart = createBuilding(build, 10.0f, 20.0f, 10.0f);

	//create the shape of the balcony on the roof:
	//you can crete it by using subFace on the top face of the building...
	auto roof = subFace(build, modStart.up, 0.95, 0.95);
	//...and then by pulling all the resulting faces (except the central one) up.
	pullFace(build, roof, 0.9, 0, 4);
	pullFace(build, roof, 0.9, 5, 9);

	//we then pull some random spots on the roof to create some shapes on top of it
	auto topFace = subdivideFace(build, roof[4], 10, 10);
	pullFace(build, topFace[12], 0.8);
	pullFace(build, topFace[64], 0.8);
	pullFace(build, topFace[88], 0.8);

	//subdivide the face where the door wants to be put,
	//results has to be saved on a variable because we will perform two different operations
	//on the resulting faces:
	auto doorFace = subdivideFace(build, modStart.south, 5, 10);
	//first we create the windows (by using subfaces and then pull with a negative value)
	//note that with the right indexes we can ignore the last row of windows
	pullFace(build, subFace(build, doorFace, 0.8, 0.8, 0, 45), -0.3, 0, 45);
	//and then we create the shape of the door on the bottom row
	pullFace(build, subFace(build, doorFace[48], 0.4, 0.8, 0, 0.2)[4], -0.3);

	//we then itereate the creation of the windows on every remaining wall
	pullFace(build, subFace(build, subdivideFace(build, modStart.north, 5, 10), 0.8, 0.8), -0.3, 0, 45);
	pullFace(build, subFace(build, subdivideFace(build, modStart.east, 5, 10), 0.8, 0.8), -0.3, 0, 45);
	pullFace(build, subFace(build, subdivideFace(build, modStart.west, 5, 10), 0.8, 0.8), -0.3, 0, 45);
	

	//and then we can add the building to the scene
	addBuildingToScene(build, scn, lmat);


	printf("Scene generated, saving...\n");

	auto opts = ygl::save_options();


	ygl::save_scene("scene0.obj", scn, opts);

	return 0;
}
