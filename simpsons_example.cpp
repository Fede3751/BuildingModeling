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
	auto modStart = createBuilding(build, 15, 8, 10);

	//create the shape of the chimney. this can be done in multiple nested calls:
	//first you divide one of the faces in multiple slices, then you select one, pull it out and
	//pull up the upper face of the resulting shape. In this particular case the face up is first divided in
	//two and then pulled to give a narrowing shape on the higher part.
	pullFace(build, subdivideFace(build, pullFace(build, subdivideFace(build, modStart.west, 5, 1)[2], 1).up, 1, 2)[0], 3);

	//now the east face is first divided with horizonalSlice and then pulled out to create another component
	//of the house on one side. The south face of this result is then pulled down to create a third component.
	auto res1 = pullFace(build, horizontalSlice(build, subdivideFace(build, modStart.east, 2, 1)[1], 0.8)[1], 5.0f);
	auto res2 = pullFace(build, horizontalSlice(build, res1.south, 0.6)[1], 8);

	//create the shape of the window on the first component created, on the east face.
	pullFace(build, subFace(build, subdivideFace(build, res1.east, 1, 2)[0], 0.6, 0.5)[4], -0.3f);
	//create the shape of the garage door on the second component created (looking south)
	pullFace(build, subFace(build, res2.south, 0.8f, 0.8f)[4], -0.4f);

	//create the shape of the roofs on the three components created
	pointFace(build, modStart.up, ygl::vec3f{ 1, 0, 0 }, 2);
	pointFace(build, res1.up, ygl::vec3f{ 1, 0, 0 }, 1);
	pointFace(build, res2.up, ygl::vec3f{ 0, 1, 0 }, 1);

	//subdivide the frontal face of the house (first component) and save the result to perform multiple
	//operations
	auto frontalFace = subdivideFace(build, modStart.south, 1, 2);
	//subdivide again the first face created and use it to create the shape of the windows
	pullFace(build, subFace(build, subdivideFace(build, frontalFace[0], 2, 1), 0.6f, 0.6f, 0, 2), -0.3f);

	//subdivide the bottom face in three faces and pull out the two on the side
	auto faces = subdivideFace(build, frontalFace[1], 3, 1);
	pullFace(build, subFace(build, faces[0], 0.5, 0.95)[4], 1);
	pullFace(build, subFace(build, faces[2], 0.5, 0.95)[4], 1);

	//create the shape of the door
	auto doorSub = subFace(build, faces[1], 0.3, 0.7);
	pullFace(build, doorSub[4], -0.3);
	//create the shape of a small step in front of the door
	pullFace(build, doorSub, 0.5, 6, 9);


	//add the result to the scene and save it
	addBuildingToScene(build, scn, lmat);


	printf("Scene generated, saving...\n");

	auto opts = ygl::save_options();


	ygl::save_scene("scene0.obj", scn, opts);


	return 0;
}
