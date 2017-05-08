

#include <nds.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "testtext.h"
#include "testtext2.h"

//Global variable for keeping track of how many polygons are on the screen
static int totalPolygons = 0;
const int maxPolygons = 4000;
static int testCount = 0;
static int testVision = 0;
static int tookLeft = 0;
static int currentId = 0;
int countTest = 0;
int textureID[3];

const int PI = 3.1415; 

//Declare class
class GameObject;

u32 uv8[] =
{
	TEXTURE_PACK(inttot16(8), 0),
	TEXTURE_PACK(inttot16(8),inttot16(8)),
	TEXTURE_PACK(0, inttot16(8)),
	TEXTURE_PACK(0,0)
};

u32 uv16[] =
{
	TEXTURE_PACK(inttot16(16), 0),
	TEXTURE_PACK(inttot16(16),inttot16(16)),
	TEXTURE_PACK(0, inttot16(16)),
	TEXTURE_PACK(0,0)
};

u32 uv32[] =
{
	TEXTURE_PACK(inttot16(32), 0),
	TEXTURE_PACK(inttot16(32),inttot16(32)),
	TEXTURE_PACK(0, inttot16(32)),
	TEXTURE_PACK(0,0)
};

//texture coordinates
u32 uv64[] =
{
	TEXTURE_PACK(inttot16(65), 0),
	TEXTURE_PACK(inttot16(65),inttot16(65)),
	TEXTURE_PACK(0, inttot16(65)),
	TEXTURE_PACK(0,0)
};

u32 uv128[] =
{
	TEXTURE_PACK(inttot16(128), 0),
	TEXTURE_PACK(inttot16(128),inttot16(128)),
	TEXTURE_PACK(0, inttot16(128)),
	TEXTURE_PACK(0,0)
};

u32 uv256[] =
{
	TEXTURE_PACK(inttot16(256), 0),
	TEXTURE_PACK(inttot16(256),inttot16(128)),
	TEXTURE_PACK(0, inttot16(256)),
	TEXTURE_PACK(0,0)
};

u32 uv512[] =
{
	TEXTURE_PACK(inttot16(256), 0),
	TEXTURE_PACK(inttot16(256),inttot16(256)),
	TEXTURE_PACK(0, inttot16(256)),
	TEXTURE_PACK(0,0)
};

u32 uv1024[] =
{
	TEXTURE_PACK(inttot16(1024), 0),
	TEXTURE_PACK(inttot16(1024),inttot16(1024)),
	TEXTURE_PACK(0, inttot16(1024)),
	TEXTURE_PACK(0,0)
};

struct CameraAngle
{
	public:
		float atx, aty, atz, pointx, pointy, pointz, upx, upy, upz;
		
		CameraAngle()
		{
			atx = 0; 
			aty = 0; 
			atz = 0; 
			pointx = 0;
			pointy = 0; 
			pointz = 0; 
			upx = 0; 
			upy = 0; 
			upz = 0;
		}
		
		CameraAngle(float atx, float aty, float atz, float pointx, float pointy, float pointz, float upx, float upy, float upz)
		{
			this->atx = atx;
			this->aty = aty; 
			this->atz = atz;
			this->pointx = pointx;
			this->pointy = pointy;
			this->pointz = pointz;
			this->upx = upx;
			this->upy = upy;
			this->upz = upz;
		}
		
		void lookAtCurrent()
		{
			gluLookAt(atx, aty, atz, pointx, pointy, pointz, upx, upy, upz);
		}
};

struct Vertex
{
	public: 
		float x,y,z; //When rendered, turned into v16. So range 8 to -9 exclusive. So must scale
		//Scale on own. Use floattov16
		Vertex()
		{
		
		}
		Vertex(float x, float y, float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}
};

struct Face
{
	public: 
		Face()
		{
		
		}
		
		Face(Vertex* v1, Vertex* v2, Vertex* v3)
		{
			numberOfVertices = 3;
			vertices[0] = v1;
			vertices[1] = v2;
			vertices[2] = v3;
			startVertices[0] = v1;
			startVertices[1] = v2;
			startVertices[2] = v3;
			colour[0] = 1;
			colour[1] = 1;
			colour[2] = 1;
		}
		
		void setColour(float col1, float col2, float col3)
		{
			colour[0] = col1;
			colour[1] = col2;
			colour[2] = col3;
		}
		
		int numberOfVertices;
		Vertex* vertices[4];
		Vertex* startVertices[4];
		float colour[3];
};

//Nodes for linked lists of children
struct ObjectNode
{
	public:
		GameObject* content;
		ObjectNode* next;
		ObjectNode* leftNext; //Not used for children. Used for the scene graph
		
		ObjectNode()
		{
			content = NULL;
			next = NULL;
			leftNext = NULL;
		}
};

//Nodes vertices during object creation
struct VertexNode
{
	public:
		Vertex* content;
		VertexNode* next;
		
		VertexNode()
		{
			content = NULL;
			next = NULL;
		}
};

//Nodes Face during object creation
struct FaceNode
{
	public:
		int content[3]; //Ids of the vertice that belongs to this face
		FaceNode* next;
		
		FaceNode()
		{
			next = NULL;
		}
};



//Class for making the game objects
class GameObject  
{
	public:

	//Geometry data
	//Vertices
	int numberOfVertices;
	Vertex** vertices;
	Vertex** startVertices;
	//Polygons
	int numberOfFaces;
	
	Face** faces;//Contain the indinces of the vertexes that are a part of the polygon
	
	int texture;
	int textureSize;
	
	
	//Bounding Box
	Vertex boundingBoxVertex;
	float width; 
	float height; 
	float depth; 
	
	//Location and Rotation ofsets from parent
	float x;
	float y; 
	float z;
	float roll; //X
	float pitch; //y
	float yaw; //z
	
	//Scaling information
	float scaleX;
	float scaleY;
	float scaleZ;
	
	//Reference to the object's parent
	GameObject* parent;
	
	//References to the object's children
	int numberOfChild;
	ObjectNode* children;
	ObjectNode* lastChild;
	
	bool isTop;
	bool isRoot;
	bool inView;
	int id;
	
	public: GameObject()
	{
		numberOfChild = 0;
		parent = NULL;
		isTop = true;
		isRoot = true;
		inView = true;
		id = currentId++;
		texture = -1; //No texture set
		textureSize = 64;
		x = 0;
		y = 0;
		z = 0;
		roll = 0;
		pitch = 0;
		yaw = 0;
		scaleX = 1;
		scaleY = 1;
		scaleZ = 1;
	}
	
	public: ~GameObject() 
	{
		for(int i = 0; i < numberOfVertices; i++)
		{
			delete vertices[i];
			delete startVertices[i];
		}
		
		delete vertices;
		delete startVertices;
		
		for(int i = 0; i < numberOfFaces; i++)
		{
			delete faces[i];
		}
		
		delete faces;
		
		ObjectNode* current = children;
		ObjectNode* previous = children;
		
		if(current != NULL)
		{
			while(current->next != NULL)
			{
				previous = current;
				children = current->next;
				
				previous->content = NULL;
				
				delete previous;
			}
			
			delete current;
		}
	}
	
	//Create a cube
	public: GameObject(GameObject* passedParent, float width, float height, float depth, int texture, int textureSize)
	{
		//Give ref of obj to the parent
		passedParent->addChild(this);
		
		//Texture Data
		this->texture = texture;
		this->textureSize = textureSize;
		
		//Location and Rotation ofsets from parent
		Vertex boundCord(-1 * (width/2), -1 * (height/2), -1 * (depth/2));
		x = 0;
		y = 0; 
		z = 0;
		this->roll = 0; //X
		this->pitch = 0; //y
		this->yaw = 0; //z
		scaleX = 1;
		scaleY = 1;
		scaleZ = 1;
		
		isTop = true;
		isRoot = false;
		inView = true;
		id = currentId++;
		
		//Reference to the object's parent
		parent = passedParent;
		
		//References to the object's children
		numberOfChild = 0;
		children = NULL;
		lastChild = NULL;
		
		numberOfVertices = 8;
		vertices = new Vertex*[8]; //TODO memory clean up
		startVertices = new Vertex*[8];
		for(int i = 0; i < numberOfVertices; i++)
		{
			vertices[i] = new Vertex;
			startVertices[i] = new Vertex;
		}
		//Polygons
		numberOfFaces = 6;
		faces = new Face*[6]; //CLEAN UP
		for(int i = 0; i < numberOfFaces; i++)
		{
			faces[i] = new Face;
		}
		
		//Make the vertices and polygons
		vertices[0]->x = (boundCord.x);
		vertices[0]->y = boundCord.y;
		vertices[0]->z = boundCord.z;
		vertices[1]->x = (boundCord.x);
		vertices[1]->y = (boundCord.y);
		vertices[1]->z = (boundCord.z + depth);
		vertices[2]->x = (boundCord.x);
		vertices[2]->y = (boundCord.y + height);
		vertices[2]->z = (boundCord.z);
		vertices[3]->x = (boundCord.x + width);
		vertices[3]->y = (boundCord.y);
		vertices[3]->z = (boundCord.z);
		vertices[4]->x = (boundCord.x);
		vertices[4]->y = (boundCord.y + height);
		vertices[4]->z = (boundCord.z + depth);
		vertices[5]->x = (boundCord.x + width);
		vertices[5]->y = (boundCord.y);
		vertices[5]->z = (boundCord.z + depth);
		vertices[6]->x = (boundCord.x + width);
		vertices[6]->y = (boundCord.y + height);
		vertices[6]->z = (boundCord.z);
		vertices[7]->x = (boundCord.x + width);
		vertices[7]->y = (boundCord.y + height);
		vertices[7]->z = (boundCord.z + depth);
		
		startVertices[0]->x = (boundCord.x);
		startVertices[0]->y = boundCord.y;
		startVertices[0]->z = boundCord.z;
		startVertices[1]->x = (boundCord.x);
		startVertices[1]->y = (boundCord.y);
		startVertices[1]->z = (boundCord.z + depth);
		startVertices[2]->x = (boundCord.x);
		startVertices[2]->y = (boundCord.y + height);
		startVertices[2]->z = (boundCord.z);
		startVertices[3]->x = (boundCord.x + width);
		startVertices[3]->y = (boundCord.y);
		startVertices[3]->z = (boundCord.z);
		startVertices[4]->x = (boundCord.x);
		startVertices[4]->y = (boundCord.y + height);
		startVertices[4]->z = (boundCord.z + depth);
		startVertices[5]->x = (boundCord.x + width);
		startVertices[5]->y = (boundCord.y);
		startVertices[5]->z = (boundCord.z + depth);
		startVertices[6]->x = (boundCord.x + width);
		startVertices[6]->y = (boundCord.y + height);
		startVertices[6]->z = (boundCord.z);
		startVertices[7]->x = (boundCord.x + width);
		startVertices[7]->y = (boundCord.y + height);
		startVertices[7]->z = (boundCord.z + depth);
		
		//Front
		faces[0]->numberOfVertices = 4;
		faces[0]->vertices[0] = vertices[0];
		faces[0]->vertices[1] = vertices[3];
		faces[0]->vertices[2] = vertices[6];
		faces[0]->vertices[3] = vertices[2];
		faces[0]->startVertices[0] = startVertices[0];
		faces[0]->startVertices[1] = startVertices[3];
		faces[0]->startVertices[2] = startVertices[6];
		faces[0]->startVertices[3] = startVertices[2];
		faces[0]->colour[0] = 1;
		faces[0]->colour[1] = 1;
		faces[0]->colour[2] = 1;
		
		//Right
		faces[1]->numberOfVertices = 4;
		faces[1]->vertices[0] = vertices[6];
		faces[1]->vertices[1] = vertices[3];
		faces[1]->vertices[2] = vertices[5];
		faces[1]->vertices[3] = vertices[7];
		faces[1]->startVertices[0] = startVertices[6];
		faces[1]->startVertices[1] = startVertices[3];
		faces[1]->startVertices[2] = startVertices[5];
		faces[1]->startVertices[3] = startVertices[7];
		faces[1]->colour[0] = 1;
		faces[1]->colour[1] = 1;
		faces[1]->colour[2] = 1;
		
		//Back
		faces[2]->numberOfVertices = 4;
		faces[2]->vertices[0] = vertices[7];
		faces[2]->vertices[1] = vertices[5];
		faces[2]->vertices[2] = vertices[1];
		faces[2]->vertices[3] = vertices[4];
		faces[2]->startVertices[0] = startVertices[7];
		faces[2]->startVertices[1] = startVertices[5];
		faces[2]->startVertices[2] = startVertices[1];
		faces[2]->startVertices[3] = startVertices[4];
		faces[2]->colour[0] = 1;
		faces[2]->colour[1] = 1;
		faces[2]->colour[2] = 1;
		
		//Left
		faces[3]->numberOfVertices = 4;
		faces[3]->vertices[0] = vertices[4];
		faces[3]->vertices[1] = vertices[1];
		faces[3]->vertices[2] = vertices[0];
		faces[3]->vertices[3] = vertices[2];
		faces[3]->startVertices[0] = startVertices[4];
		faces[3]->startVertices[1] = startVertices[1];
		faces[3]->startVertices[2] = startVertices[0];
		faces[3]->startVertices[3] = startVertices[2];
		faces[3]->colour[0] = 1;
		faces[3]->colour[1] = 1;
		faces[3]->colour[2] = 1;
		
		//Top
		faces[4]->numberOfVertices = 4;
		faces[4]->vertices[0] = vertices[2];
		faces[4]->vertices[1] = vertices[6];
		faces[4]->vertices[2] = vertices[7];
		faces[4]->vertices[3] = vertices[4];
		faces[4]->startVertices[0] = startVertices[2];
		faces[4]->startVertices[1] = startVertices[6];
		faces[4]->startVertices[2] = startVertices[7];
		faces[4]->startVertices[3] = startVertices[4];
		faces[4]->colour[0] = 1;
		faces[4]->colour[1] = 1;
		faces[4]->colour[2] = 1;
		
		//Bottom
		faces[5]->numberOfVertices = 4;
		faces[5]->vertices[0] = vertices[0];
		faces[5]->vertices[1] = vertices[1];
		faces[5]->vertices[2] = vertices[5];
		faces[5]->vertices[3] = vertices[3];
		faces[5]->startVertices[0] = startVertices[0];
		faces[5]->startVertices[1] = startVertices[1];
		faces[5]->startVertices[2] = startVertices[5];
		faces[5]->startVertices[3] = startVertices[3];
		faces[5]->colour[0] = 1;
		faces[5]->colour[1] = 1;
		faces[5]->colour[2] = 1;
		
		//Find the boundingBox. 
		float maxX = -10; //Unreachable default values to ensure properly set
		float minX = 10;
		float maxY = -10;
		float minY = 10;
		float maxZ = -10;
		float minZ = 10;
		for(int i = 0; i < numberOfFaces; i++)
		{
			for(int j = 0; j < faces[i]->numberOfVertices; j++)
			{
				if(faces[i]->vertices[j]->x < minX)
				{
					minX = faces[i]->vertices[j]->x;
				}
				if(faces[i]->vertices[j]->x > maxX)
				{
					maxX = faces[i]->vertices[j]->x;
				}
				if(faces[i]->vertices[j]->y < minY)
				{
					minY = faces[i]->vertices[j]->y;
				}
				if(faces[i]->vertices[j]->y > maxY)
				{
					maxY = faces[i]->vertices[j]->y;
				}
				if(faces[i]->vertices[j]->z < minZ)
				{
					minZ = faces[i]->vertices[j]->z;
				}
				if(faces[i]->vertices[j]->z > maxZ)
				{
					maxZ = faces[i]->vertices[j]->z;
				}
			}
		}
		boundingBoxVertex.x = minX; 
		boundingBoxVertex.y = minY;
		boundingBoxVertex.z = minZ;
		this->width = maxX - minX; 
		this->height = maxY - minY; 
		this->depth = maxZ - minZ; 
	}
	
	
	
	//Create a gameobject from a list of vertices and faces
	public: GameObject(GameObject* passedParent, int noVertices, VertexNode* vertexList, int noFaces, FaceNode* faceList, int texture, int textureSize)
	{
		//Give ref of obj to the parent
		passedParent->addChild(this);
		
		//Texture Data
		this->texture = texture;
		this->textureSize = textureSize;
		
		//Location and Rotation ofsets from parent
		x = 0;
		y = 0; 
		z = 0;
		roll = 0; //X
		pitch = 0; //y
		yaw = 0; //z
		scaleX = 1;
		scaleY = 1;
		scaleZ = 1;
		
		isTop = true;
		isRoot = false;
		inView = true;
		id = currentId++;
		
		//Reference to the object's parent
		parent = passedParent;
		
		//References to the object's children
		numberOfChild = 0;
		children = NULL;
		lastChild = NULL;
		
		numberOfVertices = noVertices;
		vertices = new Vertex*[noVertices]; 
		startVertices = new Vertex*[noVertices];
		
		//Polygons
		numberOfFaces = noFaces;
		faces = new Face*[noFaces]; 
		
		//Make the vertices and polygons
		VertexNode* currentVertex = vertexList;
		for(int i = 0; i < numberOfVertices; i++)
		{
			vertices[i] = (currentVertex->content);
			currentVertex = currentVertex->next;
		}
		
		//Time For the polygon faces (Note will be triangle)
		FaceNode* currentFace = faceList;
		for(int i = 0; i < numberOfFaces; i++)
		{
			faces[i] = new Face(vertices[currentFace->content[0]], vertices[currentFace->content[1]], vertices[currentFace->content[2]]);
			
			currentFace = currentFace->next;
		}
		
		//Find the boundingBox.
		float maxX = -10; //Unreachable default values to ensure properly set
		float minX = 10;
		float maxY = -10;
		float minY = 10;
		float maxZ = -10;
		float minZ = 10;
		for(int i = 0; i < numberOfFaces; i++)
		{
			for(int j = 0; j < faces[i]->numberOfVertices; j++)
			{
				if(faces[i]->vertices[j]->x < minX)
				{
					minX = faces[i]->vertices[j]->x;
				}
				if(faces[i]->vertices[j]->x > maxX)
				{
					maxX = faces[i]->vertices[j]->x;
				}
				if(faces[i]->vertices[j]->y < minY)
				{
					minY = faces[i]->vertices[j]->y;
				}
				if(faces[i]->vertices[j]->y > maxY)
				{
					maxY = faces[i]->vertices[j]->y;
				}
				if(faces[i]->vertices[j]->z < minZ)
				{
					minZ = faces[i]->vertices[j]->z;
				}
				if(faces[i]->vertices[j]->z > maxZ)
				{
					maxZ = faces[i]->vertices[j]->z;
				}
			}
		}
		boundingBoxVertex.x = minX; 
		boundingBoxVertex.y = minY;
		boundingBoxVertex.z = minZ;
		width = maxX - minX; 
		height = maxY - minY; 
		depth = maxZ - minZ; 
	}
	
	public: GameObject(GameObject* passedParent, int noVertices, int noFaces, int texture, int textureSize)
	{
		
		//Give ref of obj to the parent
		passedParent->addChild(this);
	
		//Set up a default game object
		
		//Texture Data
		this->texture = texture;
		this->textureSize = textureSize;
		
		//Location and Rotation ofsets from parent
		x = 0;
		y = 0; 
		z = 0;
		roll = 0; //X
		pitch = 0; //y
		yaw = 0; //z
		scaleX = 1;
		scaleY = 1;
		scaleZ = 1;
		
		isTop = true;
		isRoot = false;
		inView = true;
		id = currentId++;
		
		//Reference to the object's parent
		parent = passedParent;
		
		//References to the object's children
		numberOfChild = 0;
		children = NULL;
		lastChild = NULL;
		
		numberOfVertices = noVertices;
		vertices = new Vertex*[noVertices]; 
		startVertices = new Vertex*[noVertices];
		for(int i = 0; i < numberOfVertices; i++)
		{
			vertices[i] = new Vertex;
			startVertices[i] = new Vertex;
		}
		
		//Polygons
		numberOfFaces = noFaces;
		faces = new Face*[noFaces]; 
		for(int i = 0; i < numberOfFaces; i++)
		{
			faces[i] = new Face;
		}
		
		//Make the vertices and polygons
		vertices[0]->x = (-0.5);
		vertices[0]->y = (-0.5);
		vertices[0]->z = (-0.5);
		vertices[1]->x = (-0.5);
		vertices[1]->y = (-0.5);
		vertices[1]->z = (0.5);
		vertices[2]->x = (-0.5);
		vertices[2]->y = (0.5);
		vertices[2]->z = (-0.5);
		vertices[3]->x = (0.5);
		vertices[3]->y = (-0.5);
		vertices[3]->z = (-0.5);
		vertices[4]->x = (-0.5);
		vertices[4]->y = (0.5);
		vertices[4]->z = (0.5);
		vertices[5]->x = (0.5);
		vertices[5]->y = (-0.5);
		vertices[5]->z = (0.5);
		vertices[6]->x = (0.5);
		vertices[6]->y = (0.5);
		vertices[6]->z = (-0.5);
		vertices[7]->x = (0.5);
		vertices[7]->y = (0.5);
		vertices[7]->z = (0.5);
		
		startVertices[0]->x = (-0.5);
		startVertices[0]->y = (-0.5);
		startVertices[0]->z = (-0.5);
		startVertices[1]->x = (-0.5);
		startVertices[1]->y = (-0.5);
		startVertices[1]->z = (0.5);
		startVertices[2]->x = (-0.5);
		startVertices[2]->y = (0.5);
		startVertices[2]->z = (-0.5);
		startVertices[3]->x = (0.5);
		startVertices[3]->y = (-0.5);
		startVertices[3]->z = (-0.5);
		startVertices[4]->x = (-0.5);
		startVertices[4]->y = (0.5);
		startVertices[4]->z = (0.5);
		startVertices[5]->x = (0.5);
		startVertices[5]->y = (-0.5);
		startVertices[5]->z = (0.5);
		startVertices[6]->x = (0.5);
		startVertices[6]->y = (0.5);
		startVertices[6]->z = (-0.5);
		startVertices[7]->x = (0.5);
		startVertices[7]->y = (0.5);
		startVertices[7]->z = (0.5);
		
		//Front
		faces[0]->numberOfVertices = 4;
		faces[0]->vertices[0] = vertices[0];
		faces[0]->vertices[1] = vertices[3];
		faces[0]->vertices[2] = vertices[6];
		faces[0]->vertices[3] = vertices[2];
		faces[0]->startVertices[0] = startVertices[0];
		faces[0]->startVertices[1] = startVertices[3];
		faces[0]->startVertices[2] = startVertices[6];
		faces[0]->startVertices[3] = startVertices[2];
		faces[0]->colour[0] = 1;
		faces[0]->colour[1] = 0;
		faces[0]->colour[2] = 0;
		
		//Right
		faces[1]->numberOfVertices = 4;
		faces[1]->vertices[0] = vertices[6];
		faces[1]->vertices[1] = vertices[3];
		faces[1]->vertices[2] = vertices[5];
		faces[1]->vertices[3] = vertices[7];
		faces[1]->startVertices[0] = startVertices[6];
		faces[1]->startVertices[1] = startVertices[3];
		faces[1]->startVertices[2] = startVertices[5];
		faces[1]->startVertices[3] = startVertices[7];
		faces[1]->colour[0] = 0;
		faces[1]->colour[1] = 1;
		faces[1]->colour[2] = 0;
		
		//Back
		faces[2]->numberOfVertices = 4;
		faces[2]->vertices[0] = vertices[7];
		faces[2]->vertices[1] = vertices[5];
		faces[2]->vertices[2] = vertices[1];
		faces[2]->vertices[3] = vertices[4];
		faces[2]->startVertices[0] = startVertices[7];
		faces[2]->startVertices[1] = startVertices[5];
		faces[2]->startVertices[2] = startVertices[1];
		faces[2]->startVertices[3] = startVertices[4];
		faces[2]->colour[0] = 0;
		faces[2]->colour[1] = 0;
		faces[2]->colour[2] = 1;
		
		//Left
		faces[3]->numberOfVertices = 4;
		faces[3]->vertices[0] = vertices[4];
		faces[3]->vertices[1] = vertices[1];
		faces[3]->vertices[2] = vertices[0];
		faces[3]->vertices[3] = vertices[2];
		faces[3]->startVertices[0] = startVertices[4];
		faces[3]->startVertices[1] = startVertices[1];
		faces[3]->startVertices[2] = startVertices[0];
		faces[3]->startVertices[3] = startVertices[2];
		faces[3]->colour[0] = 1;
		faces[3]->colour[1] = 1;
		faces[3]->colour[2] = 0;
		
		//Top
		faces[4]->numberOfVertices = 4;
		faces[4]->vertices[0] = vertices[2];
		faces[4]->vertices[1] = vertices[6];
		faces[4]->vertices[2] = vertices[7];
		faces[4]->vertices[3] = vertices[4];
		faces[4]->startVertices[0] = startVertices[2];
		faces[4]->startVertices[1] = startVertices[6];
		faces[4]->startVertices[2] = startVertices[7];
		faces[4]->startVertices[3] = startVertices[4];
		faces[4]->colour[0] = 1;
		faces[4]->colour[1] = 0;
		faces[4]->colour[2] = 1;
		
		//Bottom
		faces[5]->numberOfVertices = 4;
		faces[5]->vertices[0] = vertices[0];
		faces[5]->vertices[1] = vertices[1];
		faces[5]->vertices[2] = vertices[5];
		faces[5]->vertices[3] = vertices[3];
		faces[5]->startVertices[0] = startVertices[0];
		faces[5]->startVertices[1] = startVertices[1];
		faces[5]->startVertices[2] = startVertices[5];
		faces[5]->startVertices[3] = startVertices[3];
		faces[5]->colour[0] = 0;
		faces[5]->colour[1] = 1;
		faces[5]->colour[2] = 1;
		
		//Find the boundingBox. 
		float maxX = -10; //Unreachable default values to ensure properly set
		float minX = 10;
		float maxY = -10;
		float minY = 10;
		float maxZ = -10;
		float minZ = 10;
		for(int i = 0; i < numberOfFaces; i++)
		{
			for(int j = 0; j < faces[i]->numberOfVertices; j++)
			{
				if(faces[i]->vertices[j]->x < minX)
				{
					minX = faces[i]->vertices[j]->x;
				}
				if(faces[i]->vertices[j]->x > maxX)
				{
					maxX = faces[i]->vertices[j]->x;
				}
				if(faces[i]->vertices[j]->y < minY)
				{
					minY = faces[i]->vertices[j]->y;
				}
				if(faces[i]->vertices[j]->y > maxY)
				{
					maxY = faces[i]->vertices[j]->y;
				}
				if(faces[i]->vertices[j]->z < minZ)
				{
					minZ = faces[i]->vertices[j]->z;
				}
				if(faces[i]->vertices[j]->z > maxZ)
				{
					maxZ = faces[i]->vertices[j]->z;
				}
			}
		}
		boundingBoxVertex.x = minX; 
		boundingBoxVertex.y = minY;
		boundingBoxVertex.z = minZ;
		width = maxX - minX; 
		height = maxY - minY; 
		depth = maxZ - minZ; 
	}
	
	void addChild(GameObject* newChild)
	{
		if(numberOfChild == 0)
		{
			children = new ObjectNode;
			lastChild = children;
			children->content = newChild;
			newChild->parent = this;
		}
		else
		{
			lastChild->next = new ObjectNode;
			lastChild = lastChild->next;
			lastChild->content = newChild;
			newChild->parent = this;
		}
		
		newChild->isTop = false;
		
		numberOfChild++;
	}
	
	void removeFromChain()
	{
		if(isTop)
		{
			return;
		}
		
		isTop = true;
		parent->numberOfChild--;
		
		
		
		parent->removeChildFromParent(this);
	}
	
	void removeChildFromParent(GameObject* childToRemove)
	{
		if(numberOfChild == 0)
		{
			children = NULL;
			lastChild = NULL;
			return;
		}
	
		ObjectNode* currentObject = children;
		ObjectNode* childBefore = NULL;
		
		while(currentObject->next != NULL)
		{
			childBefore = currentObject;
			currentObject = currentObject->next;
			
			if(currentObject->content->id == childToRemove->id)
			{
				break;
			}
		}
		
		//Remove it
		ObjectNode* tempHold = childBefore->next;
		childBefore->next = currentObject->next;
		delete tempHold;
		
		
		while(currentObject->next != NULL)
		{
			currentObject = currentObject->next;
		}
		
		lastChild = currentObject;
	}
	
	GameObject* findTopParent()
	{
		
		if(isTop)
		{
			return this;
		}
		else
		{
			return parent->findTopParent();
		}
	}
	
	void findBoundingBox()
	{
		//Find the boundingBox. 
		float maxX = -10; //Unreachable default values to ensure properly set
		float minX = 10;
		float maxY = -10;
		float minY = 10;
		float maxZ = -10;
		float minZ = 10;
		for(int i = 0; i < numberOfFaces; i++)
		{
			for(int j = 0; j < faces[i]->numberOfVertices; j++)
			{
				if(faces[i]->vertices[j]->x < minX)
				{
					minX = faces[i]->vertices[j]->x;
				}
				if(faces[i]->vertices[j]->x > maxX)
				{
					maxX = faces[i]->vertices[j]->x;
				}
				if(faces[i]->vertices[j]->y < minY)
				{
					minY = faces[i]->vertices[j]->y;
				}
				if(faces[i]->vertices[j]->y > maxY)
				{
					maxY = faces[i]->vertices[j]->y;
				}
				if(faces[i]->vertices[j]->z < minZ)
				{
					minZ = faces[i]->vertices[j]->z;
				}
				if(faces[i]->vertices[j]->z > maxZ)
				{
					maxZ = faces[i]->vertices[j]->z;
				}
			}
		}
		
		//Also check its children's vertices to find the bounding box of the whole combiniation
		ObjectNode* focusChild = children;
		if(focusChild != NULL)
		{
			focusChild->content->findBoundingBox(&maxX, &minX, &maxY, &minY, &maxZ, &minZ);
		
			while(focusChild->next != NULL)
			{
				focusChild = focusChild->next;
				focusChild->content->findBoundingBox(&maxX, &minX, &maxY, &minY, &maxZ, &minZ);
			}
		}
		
		boundingBoxVertex.x = minX; 
		boundingBoxVertex.y = minY;
		boundingBoxVertex.z = minZ;
		width = maxX - minX; 
		height = maxY - minY; 
		depth = maxZ - minZ; 
	}
	
	void findBoundingBox(float* maxX, float* minX, float* maxY, float* minY, float* maxZ, float* minZ)
	{
		for(int i = 0; i < numberOfFaces; i++)
		{
			for(int j = 0; j < faces[i]->numberOfVertices; j++)
			{
				if(faces[i]->vertices[j]->x < *minX)
				{
					*minX = faces[i]->vertices[j]->x;
				}
				if(faces[i]->vertices[j]->x > *maxX)
				{
					*maxX = faces[i]->vertices[j]->x;
				}
				if(faces[i]->vertices[j]->y < *minY)
				{
					*minY = faces[i]->vertices[j]->y;
				}
				if(faces[i]->vertices[j]->y > *maxY)
				{
					*maxY = faces[i]->vertices[j]->y;
				}
				if(faces[i]->vertices[j]->z < *minZ)
				{
					*minZ = faces[i]->vertices[j]->z;
				}
				if(faces[i]->vertices[j]->z > *maxZ)
				{
					*maxZ = faces[i]->vertices[j]->z;
				}
			}
		}
	}
	
	void move(float x, float y, float z)
	{
		//DS has boundardies of 7 and -8. Check if the movement will send it out of bounds. Should be bounding box actually
		
		
		//Check if movement causing bounding box to left the max size
		if(x!=0)
		{
			if(x < 0 && boundingBoxVertex.x + x < -7)
			{
				return;
			}
			else if(x > 0 && boundingBoxVertex.x + width + x > 6)
			{
				return;
			}
		}
		if(y!=0)
		{
			if(y < 0 && boundingBoxVertex.y + y < -7)
			{
				return;
			}
			else if(y > 0 && boundingBoxVertex.y + height + y > 6)
			{
				return;
			}
		}
		if(z!=0)
		{
			if(z < 0 && boundingBoxVertex.z + z < -7)
			{
				return;
			}
			else if(z > 0 && boundingBoxVertex.z + depth + z > 6)
			{
				return;
			}
		}
		
		//Commit the movement
		//Shift the local coordinates
		this->x += x;
		this->y += y;
		this->z += z;
		
	}
	
	void rotate(float roll, float pitch, float yaw)
	{
		//Check if the rotation moves the bounding box outside of the world. //PANCAKES cos(PI * currentAngle / 180.0)???
		if(roll!=0)
		{
			//Check y coord
			if(boundingBoxVertex.y * cos(PI * roll / 180) - boundingBoxVertex.z * sin(PI * roll / 180) < -7)
			{
				return;
			}
			else if(boundingBoxVertex.y * cos(PI * roll / 180) - boundingBoxVertex.z * sin(PI * roll / 180) > 6)
			{
				return;
			}
			else if((boundingBoxVertex.y + height) * cos(PI * roll / 180) - (boundingBoxVertex.z + depth) * sin(PI * roll / 180) < -7)
			{
				return;
			}
			else if((boundingBoxVertex.y + height) * cos(PI * roll / 180) - (boundingBoxVertex.z + depth) * sin(PI * roll / 180) > 6)
			{
				return;
			}
			//Check z coord
			if(boundingBoxVertex.y * sin(PI * roll / 180) + boundingBoxVertex.z * cos(PI * roll / 180) < -7)
			{
				return;
			}
			else if(boundingBoxVertex.y * sin(PI * roll / 180) + boundingBoxVertex.z * cos(PI * roll / 180) > 6)
			{
				return;
			}
			else if((boundingBoxVertex.y + height) * sin(PI * roll / 180) + (boundingBoxVertex.z + depth) * cos(PI * roll / 180) < -7)
			{
				return;
			}
			else if((boundingBoxVertex.y + height) * sin(PI * roll / 180) + (boundingBoxVertex.z + depth) * cos(PI * roll / 180) > 6)
			{
				return;
			}
			
			//Apply rotation
			this->roll += roll;
		}
		
		
		if(pitch!=0)
		{
			//Check x coord
			if(boundingBoxVertex.x * cos(PI * pitch / 180) + boundingBoxVertex.z * sin(PI * pitch / 180) < -7)
			{
				return;
			}
			else if(boundingBoxVertex.x * cos(PI * pitch / 180) + boundingBoxVertex.z * sin(PI * pitch / 180) > 6)
			{
				return;
			}
			else if((boundingBoxVertex.x + width) * cos(PI * pitch / 180) + (boundingBoxVertex.z + depth) * sin(PI * pitch / 180) < -7)
			{
				return;
			}
			else if((boundingBoxVertex.x + width) * cos(PI * pitch / 180) + (boundingBoxVertex.z + depth) * sin(PI * pitch / 180) > 6)
			{
				return;
			}
			//Check z coord
			if(-1 * boundingBoxVertex.x * sin(PI * pitch / 180) + boundingBoxVertex.z * cos(PI * pitch / 180) < -7)
			{
				return;
			}
			else if(-1 * boundingBoxVertex.x * sin(PI * pitch / 180) + boundingBoxVertex.z * cos(PI * pitch / 180) > 6)
			{
				return;
			}
			else if(-1 * (boundingBoxVertex.x + width) * sin(PI * pitch / 180) + (boundingBoxVertex.z + depth) * cos(PI * pitch / 180) < -7)
			{
				return;
			}
			else if(-1 * (boundingBoxVertex.x + width) * sin(PI * pitch / 180) + (boundingBoxVertex.z + depth) * cos(PI * pitch / 180) > 6)
			{
				return;
			}
			
			//Apply rotation
			this->pitch += pitch;
		}
		
		
		
		if(yaw!=0)
		{
			//Check x coord
			if(boundingBoxVertex.x * cos(PI * yaw / 180) - boundingBoxVertex.y * sin(PI * yaw / 180) < -7)
			{
				return;
			}
			else if(boundingBoxVertex.x * cos(PI * yaw / 180) - boundingBoxVertex.y * sin(PI * yaw / 180) > 6)
			{
				return;
			}
			else if((boundingBoxVertex.x + width) * cos(PI * yaw / 180) - (boundingBoxVertex.y + height) * sin(PI * yaw / 180) < -7)
			{
				return;
			}
			else if((boundingBoxVertex.x + width) * cos(PI * yaw / 180) - (boundingBoxVertex.y + height) * sin(PI * yaw / 180) > 6)
			{
				return;
			}
			//Check y coord
			if(boundingBoxVertex.x * sin(PI * yaw / 180) + boundingBoxVertex.y * cos(PI * yaw / 180) < -7)
			{
				return;
			}
			else if(boundingBoxVertex.x * sin(PI * yaw / 180) + boundingBoxVertex.y * cos(PI * yaw / 180) > 6)
			{
				return;
			}
			else if((boundingBoxVertex.x + width) * sin(PI * yaw / 180) + (boundingBoxVertex.y + height) * cos(PI * yaw / 180) < -7)
			{
				return;
			}
			else if((boundingBoxVertex.x + width) * sin(PI * yaw / 180) + (boundingBoxVertex.y + height) * cos(PI * yaw / 180) > 6)
			{
				return;
			}
			
			//Apply rotation
			this->yaw += yaw;
			
		}
	}
	
	void scale(float x, float y, float z)
	{
		//Check that none of the vertices go out of bounds
		if (x != 1)
		{
			if (x < 0 && boundingBoxVertex.x * (scaleX + x) < -7)
			{
				return;
			}
			else if (x > 0 && (boundingBoxVertex.x + width) * (scaleX * x) > 6)
			{
				return;
			}
		}
		if (y != 1)
		{
			if (y < 0 && boundingBoxVertex.y * (y * scaleY) < -7)
			{
				return;
			}
			else if (y > 0 && (boundingBoxVertex.y + height) * (y * scaleY) > 6)
			{
				return;
			}
		}
		if (z != 1)
		{
			if (z < 0 && boundingBoxVertex.z * (z * scaleZ) < -7)
			{
				return;
			}
			else if (z > 0 && (boundingBoxVertex.z + depth) * (z * scaleZ) > 6)
			{
				return;
			}
		}

		//Commit the scaling
		scaleX += x;
		scaleY += y;
		scaleZ += z;
	}
	
	void applyTransformations()
	{
		GameObject* currentObject = this;
		
		//Build the list of objects
		ObjectNode* family = new ObjectNode;
		family->content = this;
		family->next = NULL;
		
		ObjectNode* lastAddition = family;
		ObjectNode* tempNode = family;
		
		while(!currentObject->isTop)
		{
			//Next iteration
			currentObject = currentObject->parent;
			
			tempNode = lastAddition;
			
			lastAddition = new ObjectNode;
			lastAddition->content = currentObject;
			
			lastAddition->next = tempNode;
			
			countTest++;
		}
		
		tempNode = lastAddition;
			
		lastAddition = new ObjectNode;
		
		lastAddition->next = tempNode;
		
		//Reset the vertices
		for(int j = 0; j < numberOfVertices; j++)
		{
			vertices[j]->x = startVertices[j]->x;
			vertices[j]->y = startVertices[j]->y;
			vertices[j]->z = startVertices[j]->z;
		}
		
		ObjectNode* findPosPointer = lastAddition;
		Vertex totalPos(0,0,0);
		Vertex totalRot(0,0,0);
		Vertex previousOffset(0, 0, 0);
		
		
		while(findPosPointer->next != NULL)
		{
			findPosPointer = findPosPointer->next;
			
			float lastx = 0;
			float lasty = 0;
			float lastz = 0;
			
			for(int j = 0; j < numberOfVertices; j++)
			{
				
				
				//Translate to the origin to get proper rotation
				
				vertices[j]->x = vertices[j]->x - totalPos.x;
				vertices[j]->y = vertices[j]->y - totalPos.y;
				vertices[j]->z = vertices[j]->z - totalPos.z;
				
				
				
				float tempx = vertices[j]->x;
				float tempy = vertices[j]->y;
				float tempz = vertices[j]->z;
				
				vertices[j]->y = tempy * cos(PI * findPosPointer->content->roll / 180) - tempz * sin(PI * findPosPointer->content->roll / 180);
				vertices[j]->z = tempy * sin(PI * findPosPointer->content->roll / 180) + tempz * cos(PI * findPosPointer->content->roll / 180);
				
				tempy = vertices[j]->y;
				tempz = vertices[j]->z;
				
				vertices[j]->x = tempx * cos(PI * findPosPointer->content->pitch / 180) + tempz * sin(PI * findPosPointer->content->pitch / 180);
				vertices[j]->z = -1 * tempx * sin(PI * findPosPointer->content->pitch / 180) + tempz * cos(PI * findPosPointer->content->pitch / 180);
				
				tempx = vertices[j]->x;
				tempz = vertices[j]->z;
				
				vertices[j]->x = tempx * cos(PI * findPosPointer->content->yaw / 180) - tempy * sin(PI * findPosPointer->content->yaw / 180);
				vertices[j]->y = tempx * sin(PI * findPosPointer->content->yaw / 180) + tempy * cos(PI * findPosPointer->content->yaw / 180);
				
				tempx = vertices[j]->x;
				tempy = vertices[j]->y;
				
				//Scale the object while its still at the centre
				vertices[j]->x *= scaleX;
				vertices[j]->y *= scaleY;
				vertices[j]->z *= scaleZ;
				
				//Move back
				vertices[j]->x = vertices[j]->x + totalPos.x;
				vertices[j]->y = vertices[j]->y + totalPos.y;
				vertices[j]->z = vertices[j]->z + totalPos.z;
				
				//Move its offset based on the total rotation done up to this point
				lastx = findPosPointer->content->x;
				lasty = findPosPointer->content->y;
				lastz = findPosPointer->content->z;
				tempx = findPosPointer->content->x;
				tempy = findPosPointer->content->y;
				tempz = findPosPointer->content->z;
				
				lasty = tempy * cos(PI * totalRot.x / 180) - tempz * sin(PI * totalRot.x / 180);
				lastz = tempy * sin(PI * totalRot.x / 180) + tempz * cos(PI * totalRot.x / 180);
				
				tempy = lasty;
				tempz = lastz;
				
				lastx = tempx * cos(PI * totalRot.y / 180) + tempz * sin(PI * totalRot.y / 180);
				lastz = -1 * tempx * sin(PI * totalRot.y / 180) + tempz * cos(PI * totalRot.y / 180);
				
				tempx = lastx;
				tempz = lastz;
				
				lastx = tempx * cos(PI * totalRot.z / 180) - tempy * sin(PI * totalRot.z / 180);
				lasty = tempx * sin(PI * totalRot.z / 180) + tempy * cos(PI * totalRot.z / 180);
				
				//Make the movement
				vertices[j]->x = vertices[j]->x + lastx; //CHRIS
				vertices[j]->y = vertices[j]->y + lasty; 
				vertices[j]->z = vertices[j]->z + lastz; 
				
				
			}
			
			//Set up next object
			totalPos.x += lastx;
			totalPos.y += lasty;
			totalPos.z += lastz;
			
			totalRot.x += findPosPointer->content->roll;
			totalRot.y += findPosPointer->content->pitch;
			totalRot.z += findPosPointer->content->yaw;
				
		}
		
		//Delete objectNode created in this function
		ObjectNode* current = lastAddition;
		ObjectNode* previous = lastAddition;
		
		if(current != NULL)
		{
			while(current->next != NULL)
			{
				previous = current;
				current = current->next;
				
				delete previous;
			}
			
			delete current;
		}
		
		//Move its children
		ObjectNode* focusChild;
		focusChild = children;
		for(int i = 0; i < numberOfChild; i++)
		{
			focusChild->content->applyTransformations();
			focusChild = focusChild->next;
		}
		
		findBoundingBox();
	}
	
	//Function for drawing the game object
	void drawObject()
	{
		//If the object is the root node dont draw anything
		if(parent == NULL)
		{
			return;
		}
	
		for(int i = 0; i < numberOfFaces; i++)
		{
			if(faces[i]->numberOfVertices == 4)
			{
				glBegin(GL_QUADS);
			}
			else if(faces[i]->numberOfVertices == 3)
			{
				glBegin(GL_TRIANGLES);
			}
			
			if(textureSize > 0)
			{
				glColor3f(1, 1, 1);
			}
			else
			{
				glColor3f(faces[i]->colour[0], faces[i]->colour[1], faces[i]->colour[2]);
			}
			
			for(int j = 0; j < faces[i]->numberOfVertices; j++)
			{	
				if(textureSize > 0)
				{
					glBindTexture(0, textureID[texture]);
					
					if(textureSize == 64)
					{
						GFX_TEX_COORD = (uv64[j%4]); 
					}
					else if(textureSize == 128)
					{
						GFX_TEX_COORD = (uv128[j%4]);
					}
					else if(textureSize == 8)
					{
						GFX_TEX_COORD = (uv8[j%4]);
					}
					else if(textureSize == 16)
					{
						GFX_TEX_COORD = (uv16[j%4]);
					}
					else if(textureSize == 32)
					{
						GFX_TEX_COORD = (uv32[j%4]);
					}
					else if(textureSize == 256)
					{
						GFX_TEX_COORD = (uv256[j%4]);
					}
					else if(textureSize == 512)
					{
						GFX_TEX_COORD = (uv512[j%4]);
					}
					else if(textureSize == 1024)
					{
						GFX_TEX_COORD = (uv1024[j%4]);
					}
				}
				
				glVertex3v16(floattov16(faces[i]->vertices[j]->x), floattov16(faces[i]->vertices[j]->y), floattov16(faces[i]->vertices[j]->z));
			}

			totalPolygons++;
			glEnd();
		}
		
		//Do the same for all the object's children
		ObjectNode* currentChild = children;
		if(currentChild != NULL)
		{
			currentChild->content->drawObject();
			while(currentChild->next != NULL)
			{
				currentChild = currentChild->next;
				currentChild->content->drawObject();
			}
		}
		
		
	}
	
	float getXOffset()
	{
		//Int total position
		float collectedPos = 0;
	
		//If its the root node, break recursion
		if(parent == NULL)
		{
			return x;
		}
		
		//Recursively get the parents offset
		collectedPos += parent->getXOffset();
		
		//Add object's own offset
		collectedPos += x;
		
		return collectedPos;
	}
	float getYOffset()
	{
		//Int total position
		float collectedPos = 0;
	
		//If its the root node, break recursion
		if(parent == NULL)
		{
			return y;
		}
		
		//Recursively get the parents offset
		collectedPos += parent->getYOffset();
		
		//Add object's own offset
		collectedPos += y;
		
		return collectedPos;
	}
	float getZOffset()
	{
		//Int total position
		float collectedPos = 0;
	
		//If its the root node, break recursion
		if(parent == NULL)
		{
			return z;
		}
		
		//Recursively get the parents offset
		collectedPos += parent->getZOffset();
		
		//Add object's own offset
		collectedPos += z;
		
		return collectedPos;
	}
	
	float getRollOffset()
	{
		//Int total position
		float collectedPos = 0;
	
		//If its the root node, break recursion
		if(parent == NULL)
		{
			return roll;
		}
		
		//Recursively get the parents offset
		collectedPos += parent->getRollOffset();
		
		//Add object's own offset
		collectedPos += roll;
		
		return collectedPos;
	}
	float getPitchOffset()
	{
		//Int total position
		float collectedPos = 0;
	
		//If its the root node, break recursion
		if(parent == NULL)
		{
			return pitch;
		}
		
		//Recursively get the parents offset
		collectedPos += parent->getPitchOffset();
		
		//Add object's own offset
		collectedPos += pitch;
		
		return collectedPos;
	}
	float getYawOffset()
	{
		//Int total position
		float collectedPos = 0;
	
		//If its the root node, break recursion
		if(parent == NULL)
		{
			return yaw;
		}
		
		//Recursively get the parents offset
		collectedPos += parent->getYawOffset();
		
		//Add object's own offset
		collectedPos += yaw;
		
		return collectedPos;
	}
	
	void setPosition(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	void setRotation(float x, float y, float z)
	{
		roll = x;
		pitch = y;
		yaw = z;
	}

	void setScale(float x, float y, float z)
	{
		scaleX = x;
		scaleY = y;
		scaleZ = z;
	}
};

class SceneGraph
{
	public:
		GameObject rootNode;
		ObjectNode sceneGraph;
		ObjectNode objectList;
		ObjectNode* lastObject;
		
		ObjectNode topObjectList;
		ObjectNode* lastTopObject;
		
		CameraAngle mainCamera;
		int numberOfObjects;
	
		
	SceneGraph()
	{
		numberOfObjects = 0;
		lastObject = &objectList;
		lastTopObject = &objectList;
		
		objectList.content = &rootNode;
		objectList.next = new ObjectNode;
		lastObject = objectList.next;
		
		topObjectList.content = objectList.content;
		topObjectList.next = new ObjectNode;
		lastTopObject = topObjectList.next;
	}
	
	~SceneGraph()
	{
		//Skip the root because not dynamically allocated
		ObjectNode* current = topObjectList.next;
		ObjectNode* previous = topObjectList.next;
		
		if(current != NULL)
		{
			while(current->next != NULL)
			{
				previous = current;
				current = current->next;
				
				delete previous;
			}
			
			delete current;
		}
		
		current = objectList.next;
		previous = objectList.next;
		
		if(current != NULL)
		{
			while(current->next != NULL)
			{
				previous = current;
				current = current->next;
				
				//ALso delete the object itself
				delete previous->content;
				delete previous;
			}
			
			delete current;
		}
	}
	
	
	void testInit() //Take out when finishedd
	{
		
		
		//Add test objs to the object list
		objectList.content = &rootNode;
		objectList.next = new ObjectNode;
		lastObject = objectList.next;
	
		lastObject->content = new GameObject(&rootNode, 8, 6, 0, 64);
		
		lastObject->next = new ObjectNode;
	
		lastObject->next->content = new GameObject(lastObject->content, 8, 6, 0, 64);
		lastObject->next->content->isTop = false;
		lastObject = lastObject->next;
		
		//Test topObjectlist
		topObjectList.content = objectList.content;
		topObjectList.next = new ObjectNode;
		topObjectList.next->content = objectList.next->content;
		lastTopObject = topObjectList.next;
		
		//Test finding the parent of the 2 boxes bounding box with the 2 boxes
		objectList.next->content->findBoundingBox();
	}
	
	//Add an already created game object to the scene
	void addObject(GameObject* newObject)
	{
		lastObject->next = new ObjectNode;
		lastObject = lastObject->next;
		lastObject->content = newObject;
		
		lastTopObject->next = new ObjectNode;
		lastTopObject = lastTopObject->next;
		lastTopObject->content = newObject;
	}
	

	
	//Create a cube and add it to the scene. Return a reference to the new cube
	GameObject* createCube(float width, float height, float depth, int texture, int textureSize)
	{
		
		if(numberOfObjects != 0)
		{
			lastObject->next = new ObjectNode;
			lastObject = lastObject->next;
		}
		
		lastObject->content = new GameObject(&rootNode, width, height, depth, texture, textureSize);
		
		if(numberOfObjects != 0)
		{
			lastTopObject->next = new ObjectNode;
			lastTopObject = lastTopObject->next;
		}
		lastTopObject->content = lastObject->content;
		
		numberOfObjects++;
		return lastObject->content;
	}
	
	
	void moveObject(int objectId, float amountx, float amounty, float amountz)
	{
		ObjectNode* focusNode = &objectList;
		
		while(focusNode->content->id != objectId)
		{
			if(focusNode->next == NULL)
			{
				return;
			}
		
			focusNode = focusNode->next;
		}
		
		focusNode->content->move(amountx, amounty,amountz);
		
		//Find the parent and tell it to find the new bounding box to account for the new geometery movement
		focusNode->content->findTopParent()->applyTransformations();
		focusNode->content->findBoundingBox(); //Also shift the local bounding box
		focusNode->content->findTopParent()->findBoundingBox();
		
	}
	
	
	
	void drawTopObjectList()
	{
		ObjectNode* currentListNode = &topObjectList;
		
		while(currentListNode->next != NULL)
		{
			currentListNode = currentListNode->next;
			
			if(currentListNode->content->inView)
			{
				testCount++;
				
				//Reached the polygon limit
				if(totalPolygons + currentListNode->content->numberOfFaces >= maxPolygons)
				{
					break;
				}
				currentListNode->content->drawObject();
			}
		}
	}
	
	void drawSceneGraph()
	{
		totalPolygons = 0;
		testCount = 0;
		mainCamera.lookAtCurrent();
		drawTopObjectList();
	}

	
	void checkIfInView()
	{
		testVision = 0;
	
		ObjectNode* currentListNode = &topObjectList;
		GameObject* currentObject;
		
		//Check all the objects and see if they're in the frustrum
		while(currentListNode->next != NULL)
		{
			currentListNode = currentListNode->next;
			currentObject = currentListNode->content;
			
			if(BoxTest(floattov16(currentObject->boundingBoxVertex.x), floattov16(currentObject->boundingBoxVertex.y), floattov16(currentObject->boundingBoxVertex.z), floattov16(currentObject->width), floattov16(currentObject->height), floattov16(currentObject->depth)))
			{
				currentObject->inView = true;
			}
			else
			{
				currentObject->inView = false;
				continue; //Already can't see if why bother checking if its covering something up
			}
			
			
			ObjectNode* currentBoxNode = &topObjectList;
			
			while(currentBoxNode->next != NULL)
			{
				currentBoxNode = currentBoxNode->next;
				
				if(currentBoxNode->content->id == currentListNode->content->id)
				{
					if(currentBoxNode->next != NULL)
					{
						currentBoxNode = currentBoxNode->next;
					}
					else
					{
						break;
					}
					
				}
				
				//If not visible why bother checking
				if(!currentBoxNode->content->inView)
				{
					continue;
				}
				
				bool checkVis[3];
				checkVis[0] = true;
				checkVis[1] = true;
				checkVis[2] = true;
				
				
				if(mainCamera.atz < currentBoxNode->content->boundingBoxVertex.z && currentBoxNode->content->boundingBoxVertex.z < currentListNode->content->boundingBoxVertex.z
				|| mainCamera.atz > currentBoxNode->content->boundingBoxVertex.z && currentBoxNode->content->boundingBoxVertex.z > currentListNode->content->boundingBoxVertex.z)
				{
					//Behind in z plane
					if(((currentBoxNode->content->boundingBoxVertex.x < currentListNode->content->boundingBoxVertex.x
 && currentBoxNode->content->boundingBoxVertex.x + currentBoxNode->content->width > currentListNode->content->boundingBoxVertex.x + currentListNode->content->width)
 && (currentBoxNode->content->boundingBoxVertex.y < currentListNode->content->boundingBoxVertex.y
 && currentBoxNode->content->boundingBoxVertex.y + currentBoxNode->content->height > currentListNode->content->boundingBoxVertex.y + currentListNode->content->height)))
					{
						currentListNode->content->inView = false;
						break;
					}
					else
					{
						currentListNode->content->inView = true;
						
						break;
					}
				}
				
				if(mainCamera.aty < currentBoxNode->content->boundingBoxVertex.y && currentBoxNode->content->boundingBoxVertex.y < currentListNode->content->boundingBoxVertex.y
				|| mainCamera.aty > currentBoxNode->content->boundingBoxVertex.y && currentBoxNode->content->boundingBoxVertex.y > currentListNode->content->boundingBoxVertex.y)
				{
					//Behind in y plane
					if(((currentBoxNode->content->boundingBoxVertex.x < currentListNode->content->boundingBoxVertex.x
 && currentBoxNode->content->boundingBoxVertex.x + currentBoxNode->content->width > currentListNode->content->boundingBoxVertex.x + currentListNode->content->width)
 && (currentBoxNode->content->boundingBoxVertex.z < currentListNode->content->boundingBoxVertex.z
 && currentBoxNode->content->boundingBoxVertex.z + currentBoxNode->content->depth > currentListNode->content->boundingBoxVertex.z + currentListNode->content->depth)))
					{
						currentListNode->content->inView = false;
						break;
					}
					else
					{
						currentListNode->content->inView = true;
						
						break;
					}
				}
				
				if(mainCamera.aty < currentBoxNode->content->boundingBoxVertex.y && currentBoxNode->content->boundingBoxVertex.y < currentListNode->content->boundingBoxVertex.y
				|| mainCamera.aty > currentBoxNode->content->boundingBoxVertex.y && currentBoxNode->content->boundingBoxVertex.y > currentListNode->content->boundingBoxVertex.y)
				{
					//Behind in x plane
					if(((currentBoxNode->content->boundingBoxVertex.z < currentListNode->content->boundingBoxVertex.z
 && currentBoxNode->content->boundingBoxVertex.z + currentBoxNode->content->depth > currentListNode->content->boundingBoxVertex.z + currentListNode->content->depth)
 && (currentBoxNode->content->boundingBoxVertex.y < currentListNode->content->boundingBoxVertex.y
 && currentBoxNode->content->boundingBoxVertex.y + currentBoxNode->content->height > currentListNode->content->boundingBoxVertex.y + currentListNode->content->height)))
					{
						currentListNode->content->inView = false;
						break;
					}
					else
					{
						currentListNode->content->inView = true;
						
						break;
					}
				}
				
				if(!checkVis[0] && !checkVis[1] && !checkVis[2])
				{
					currentListNode->content->inView = false;
					continue;
				}
				
				currentListNode->content->inView = true;
				continue;
				
			}
		}
	}
	
	void changeMainCamera(float atx, float aty, float atz, float pointx, float pointy, float pointz, float upx, float upy, float upz)
	{
		mainCamera.atx = atx;
		mainCamera.aty = aty; 
		mainCamera.atz = atz;
		mainCamera.pointx = pointx;
		mainCamera.pointy = pointy;
		mainCamera.pointz = pointz;
		mainCamera.upx = upx;
		mainCamera.upy = upy;
		mainCamera.upz = upz;
	}
	
	void rotateObject(int objectId, float amountx, float amounty, float amountz)
	{
		ObjectNode* focusNode = &objectList;
		
		while(focusNode->content->id != objectId)
		{
			if(focusNode->next == NULL)
			{
				return;
			}
		
			focusNode = focusNode->next;
			
		}
		
		focusNode->content->rotate(amountx, amounty, amountz);
		
		//Find the parent and tell it to find the new bounding box to account for the new geometery movement
		focusNode->content->findTopParent()->applyTransformations();
		focusNode->content->findBoundingBox(); //Also shift the local bounding box
		focusNode->content->findTopParent()->findBoundingBox();
		
	}
	
	void scaleObject(int objectId, float amountx, float amounty, float amountz)
	{
		ObjectNode* focusNode = &objectList;

		while (focusNode->content->id != objectId)
		{
			if (focusNode->next == NULL)
			{
				return;
			}

			focusNode = focusNode->next;
			
		}

		focusNode->content->scale(amountx, amounty, amountz);

		//Find the parent and tell it to find the new bounding box to account for the new geometery movement
		focusNode->content->findTopParent()->applyTransformations();
		focusNode->content->findBoundingBox(); //Also shift the local bounding box
		focusNode->content->findTopParent()->findBoundingBox();
	}
	
	void separateFromParent(int objectID)
	{
		ObjectNode* focusNode = &objectList;

		while (focusNode->content->id != objectID)
		{
			if (focusNode->next == NULL)
			{
				return;
			}

			focusNode = focusNode->next;	
		}
		
		focusNode->content->removeFromChain();
		
		ObjectNode* focusTop = &topObjectList;
		
		while (true)
		{
			if (focusTop->next == NULL)
			{
				focusTop->next = new ObjectNode;
				lastTopObject = focusTop->next;
				lastTopObject->content = focusNode->content;
				break;
			}

			focusTop = focusTop->next;	
		}
		
	}
	
	void removeObject(int objID)
	{
		ObjectNode* focusNode = &objectList;
		ObjectNode* previousNode = NULL;

		while (focusNode->content->id != objID)
		{
			if (focusNode->next == NULL)
			{
				return;
			}
			previousNode = focusNode;
			focusNode = focusNode->next;	
		}
		
		focusNode->content->removeFromChain();
		
		ObjectNode* currentChild = focusNode->content->children;
		
		//Make all the object's children top objects
		if(currentChild != NULL)
		{
			lastTopObject->next = new ObjectNode;
			lastTopObject = lastTopObject->next;
			lastTopObject->content = currentChild->content;
		
			while(currentChild->next != NULL)
			{
				currentChild = currentChild->next;
				
				lastTopObject->next = new ObjectNode;
				lastTopObject = lastTopObject->next;
				lastTopObject->content = currentChild->content;
			}
		}
		
		//Remove from the object list
		if(previousNode != NULL && focusNode->next != NULL)
		{
			previousNode->next = focusNode->next;
		}
		else if(previousNode != NULL)
		{
			delete previousNode->next->content;
			delete previousNode->next;
			previousNode->next = NULL;
		}
		
		
		ObjectNode* focusTop = &topObjectList;
		ObjectNode* previousTop = NULL;
		
		while (true)
		{
			if (focusTop->next == NULL)
			{
				lastTopObject = focusTop;
				break;
			}
			
			previousTop = focusTop;
			focusTop = focusTop->next;	
			
			//If the object we've removing is in the top object list, get rid of it there too
			if(focusTop->content->id == objID)
			{
				if(focusTop->next != NULL)
				{
					ObjectNode* tempHold = previousTop->next;
					previousTop->next = focusTop->next;
					
					tempHold->content = NULL; //JUMPD
					delete tempHold;
				}
				else
				{
					delete previousTop->next;
					previousTop->next = NULL;
				}
			}
		}
		
		
		
	}
	
	void giveChild(int parentID, int childID)
	{
		ObjectNode* childNode = NULL;
		ObjectNode* parentNode = NULL;
		
		ObjectNode* focusNode = &topObjectList;
		ObjectNode* previousNode = NULL;
		ObjectNode* childsPrevious = NULL;
		bool pfound = false;
		bool cfound = false;

		while (!pfound || !cfound)
		{
			if (focusNode->next == NULL)
			{
				return;
			}
			
			previousNode = focusNode;
			focusNode = focusNode->next;
			
			if(focusNode->content->id == parentID)
			{
				parentNode = focusNode;
				pfound = true;
			}
			else if(focusNode->content->id == childID)
			{
				childsPrevious = previousNode;
				childNode = focusNode;
				cfound = true;
			}
		}
		
		//Now that they've been found add them
		parentNode->content->addChild(childNode->content);
		
		//Remove from the top object list
		if(childNode->next == NULL)
		{
			lastTopObject = childsPrevious;
			delete childsPrevious->next;
			childsPrevious->next = NULL;
		}
		else
		{
			ObjectNode* tempHold = childsPrevious->next;
			childsPrevious->next = childNode->next;
			delete tempHold;
		}
	}
};



//some code for profiling
//---------------------------------------------------------------------------------
u16 startTimer(int timer) {
//---------------------------------------------------------------------------------

	TIMER_CR(timer) = 0;
	TIMER_DATA(0) = 0;
	TIMER_CR(timer) = TIMER_DIV_1 | TIMER_ENABLE;
	return TIMER_DATA(0);
}

#define getTimer(timer) (TIMER_DATA(timer))



//---------------------------------------------------------------------------------
int main() {	
//---------------------------------------------------------------------------------

	touchPosition touchXY;
	
	SceneGraph mainScene;
	
	mainScene.changeMainCamera(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	
	
	//put 3D on top
	lcdMainOnTop();

	//setup the sub screen for basic printing
	consoleDemoInit();

	// Setup the Main screen for 3D 
	videoSetMode(MODE_0_3D);

	// initialize gl
	glInit();
	
	//enable textures
	glEnable(GL_TEXTURE_2D);
	
	// enable antialiasing
	glEnable(GL_ANTIALIAS);
	
	// setup the rear plane
	glClearColor(0,0,0,31); // BG must be opaque for AA to work
	glClearPolyID(63); // BG must have a unique polygon ID for AntiAllisiing to work
	glClearDepth(0x7FFF);
	
	// Set our view port to be the same size as the screen
	glViewport(0,0,255,191);
	
	//Set up textures 
	vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankB(VRAM_B_TEXTURE);
	vramSetBankD(VRAM_D_TEXTURE);
	
	
	glGenTextures(sizeof(textureID) / sizeof(textureID[0]), textureID);
	
	glBindTexture(0, textureID[0]);
	glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_64 , TEXTURE_SIZE_64, 0, TEXGEN_TEXCOORD, (u8*)testtext2Bitmap);

	glBindTexture(0, textureID[1]);
	glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_128 , TEXTURE_SIZE_128, 0, TEXGEN_TEXCOORD, (u8*)testtextBitmap);
	
	
	//camera
	float rotX = 0, rotY = 0;
	float translate = -5;

	//some profiling code
	u16 time;

	//keep track of vertex ram usage
	int polygon_count;
	int vertex_count;

	//object 
	int rx = 50, ry = 15;
	int oldx = 0, oldy = 0;
	
	//Create the objects
	mainScene.createCube(1, 1, 1, 1, 128);
	mainScene.createCube(0.3, 0.3, 0.3, 0, 64);
	mainScene.createCube(0.5, 0.5, 0.5, 0, 64);
	
	
	mainScene.createCube(0.5, 0.5, 0.5, 1, 128);
	mainScene.moveObject(4, 0, 0, -3);
	
	mainScene.giveChild(1,2);
	mainScene.giveChild(1,3);
	
	


	//main loop
	while (1) {

		//process input
		scanKeys();
		
		printf( "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" );

		touchRead(&touchXY);
		
		int held = keysHeld();
		int pressed = keysDown();
		
		if( held & KEY_LEFT)
		{
			mainScene.moveObject(1,-0.1, 0, 0);
		}
		if( held & KEY_RIGHT)
		{
			mainScene.moveObject(2, 0.1, 0, 0);
		}
		if( held & KEY_UP)
		{
			mainScene.moveObject(3, 0, 0.1, 0);
		}
		if( held & KEY_DOWN)
		{
			mainScene.moveObject(1,0.1, 0, 0);
		}
		
		if( held & KEY_L)mainScene.rotateObject(1, 0, 0, 2); //translate += .1;
		if( held & KEY_R) mainScene.scaleObject(1, 0.1, 0, 0);//translate -= .1;

		
		//change ortho vs perspective
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		
		gluPerspective(70, 256.0 / 192.0, 0.1, 15);
		
		
		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_FRONT );

		
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		mainScene.drawSceneGraph();

		swiWaitForVBlank();

		mainScene.checkIfInView();
		
		
		while (GFX_STATUS & (1<<27)); // wait until the geometry engine is not busy

		
		printf("\nObject Count: %i", testCount);
		
		printf("\nPolygon Count: %i", totalPolygons);
		

		// flush to the screen
		glFlush(0);

	}

	return 0;
}
