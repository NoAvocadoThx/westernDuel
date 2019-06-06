#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>



#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include "stb_image.h"
#include "Mesh.h"


using namespace std;



class Model
{
public:
	/*  Model Data */
	
	vector<Mesh> meshes;
	vector<Texture> tex;
	string directory;
	bool gammaCorrection;
	glm::mat4 toWorld;
	std::vector<glm::vec3> boxVertices;
	std::vector<GLfloat> boundingbox;
	//for character's shooting mechanics
	bool dying;
	int duration = 150;
	bool isFired;


	float minx, miny, minz, maxx, maxy, maxz;
	float minX, maxX, minY, maxY, maxZ, minZ;
	float centerx, centery, centerz;
	glm::vec3 viewdir;
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	/*  Functions   */
	// constructor, expects a filepath to a 3D model.
	Model(string const &path, bool gamma = false) : gammaCorrection(gamma)
	{
		toWorld = glm::mat4(1.0f);
		loadModel(path);
		scaleProcess();
	}

	// draws the model, and thus all its meshes
	void Draw(GLuint shader)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shader);
	}

	void scaleProcess()
	{
		
		maxx = -INFINITY;
		maxy = -INFINITY;
		maxz = -INFINITY;
		minx = INFINITY;
		miny = INFINITY;
		minz = INFINITY;
		//scalevalue = 3.0f;

		for (unsigned int i = 0; i < vertices.size(); ++i)
		{
			//find maxx,maxy,maxz,minx,miny,minz
			if (vertices[i].Position.x > maxx)
			{
				maxx = vertices[i].Position.x;
			}
			if (vertices[i].Position.y > maxy)
			{
				maxy = vertices[i].Position.y;
			}
			if (vertices[i].Position.z > maxz)
			{
				maxz = vertices[i].Position.z;
			}
			if (vertices[i].Position.x < minx)
			{
				minx = vertices[i].Position.x;
			}
			if (vertices[i].Position.y < miny)
			{
				miny = vertices[i].Position.y;
			}
			if (vertices[i].Position.z < minz)
			{
				minz = vertices[i].Position.z;
			}
		}

		this->centerx = (maxx + minx) / 2;
		this->centery = (maxy + miny) / 2;
		this->centerz = (maxz + minz) / 2;
		glm::vec3 center = glm::vec3(centerx, centery, centerz);
		for (unsigned int i = 0; i < vertices.size(); i++) {

			vertices[i].Position.x = (vertices[i].Position.x - this->centerx);
			vertices[i].Position.y = (vertices[i].Position.y - this->centery);
			vertices[i].Position.z = (vertices[i].Position.z - this->centerz);

		}
		float dimX, dimY, dimZ;
		dimX = maxx - minx;
		dimY = maxy - miny;
		dimZ = maxz - minz;

		// scale all vertices using the largest dimension, uniform scale
		if (dimX >= dimY && dimX >= dimZ) {
			for (int i = 0; i < vertices.size(); i++) {
				vertices[i].Position = glm::vec3(vertices[i].Position.x / dimX, vertices[i].Position.y / dimX, vertices[i].Position.z / dimX);
			}
		}
		else if (dimY >= dimX && dimY >= dimZ) {
			for (int i = 0; i < vertices.size(); i++) {
				vertices[i].Position = glm::vec3(vertices[i].Position.x / dimY, vertices[i].Position.y / dimY, vertices[i].Position.z / dimY);
			}
		}
		else if (dimZ >= dimX && dimZ >= dimY) {
			for (int i = 0; i < vertices.size(); i++) {
				vertices[i].Position = glm::vec3(vertices[i].Position.x / dimZ, vertices[i].Position.y / dimZ, vertices[i].Position.z / dimZ);
			}
		}


		minX = INFINITY, minY = INFINITY, minZ = INFINITY;   // min vertex
		maxX = -INFINITY, maxY = -INFINITY, maxZ = -INFINITY;   // max vertex

		glm::vec3 minVertX, minVertY, minVertZ;
		glm::vec3 maxVertX, maxVertY, maxVertZ;

		// search for bounding box dimension
		for (int i = 0; i < vertices.size(); i++) {
			if (vertices[i].Position.x < minX) {
				minX = vertices[i].Position.x;
				minVertX = vertices[i].Position;
			}
			if (vertices[i].Position.y < minY) {
				minY = vertices[i].Position.y;
				minVertY = vertices[i].Position;
			}
			if (vertices[i].Position.z < minZ) {
				minZ = vertices[i].Position.z;
				minVertZ = vertices[i].Position;
			}
			if (vertices[i].Position.x > maxX) {
				maxX = vertices[i].Position.x;
				maxVertX = vertices[i].Position;
			}
			if (vertices[i].Position.y > maxY) {
				maxY = vertices[i].Position.y;
				maxVertY = vertices[i].Position;
			}
			if (vertices[i].Position.z > maxZ) {
				maxZ = vertices[i].Position.z;
				maxVertZ = vertices[i].Position;
			}
		}

		boxVertices.push_back(maxVertX);
		boxVertices.push_back(minVertX);
		boxVertices.push_back(maxVertY);
		boxVertices.push_back(minVertY);
		boxVertices.push_back(maxVertZ);
		boxVertices.push_back(minVertZ);

		// bounding box
		boundingbox.push_back(minX);
		boundingbox.push_back(minY);
		boundingbox.push_back(maxZ);


		boundingbox.push_back(maxX);
		boundingbox.push_back(minY);
		boundingbox.push_back(maxZ);


		boundingbox.push_back(maxX);
		boundingbox.push_back(maxY);
		boundingbox.push_back(maxZ);


		boundingbox.push_back(minX);
		boundingbox.push_back(maxY);
		boundingbox.push_back(maxZ);


		boundingbox.push_back(minX);
		boundingbox.push_back(minY);
		boundingbox.push_back(maxZ);

		boundingbox.push_back(minX);
		boundingbox.push_back(minY);
		boundingbox.push_back(minZ);


		boundingbox.push_back(maxX);
		boundingbox.push_back(minY);
		boundingbox.push_back(minZ);


		boundingbox.push_back(maxX);
		boundingbox.push_back(maxY);
		boundingbox.push_back(minZ);


		boundingbox.push_back(minX);
		boundingbox.push_back(maxY);
		boundingbox.push_back(minZ);


		boundingbox.push_back(minX);
		boundingbox.push_back(minY);
		boundingbox.push_back(minZ);

		boundingbox.push_back(minX);
		boundingbox.push_back(maxY);
		boundingbox.push_back(maxZ);
		boundingbox.push_back(minX);
		boundingbox.push_back(maxY);
		boundingbox.push_back(minZ);

		boundingbox.push_back(maxX);
		boundingbox.push_back(minY);
		boundingbox.push_back(maxZ);
		boundingbox.push_back(maxX);
		boundingbox.push_back(minY);
		boundingbox.push_back(minZ);

		boundingbox.push_back(maxX);
		boundingbox.push_back(maxY);
		boundingbox.push_back(maxZ);
		boundingbox.push_back(maxX);
		boundingbox.push_back(maxY);
		boundingbox.push_back(minZ);

	}
	void fire() {
		glm::mat4 translateMat = glm::translate(glm::mat4(1.0f), viewdir*0.05f); //
		//glm::mat4 translateMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		toWorld = translateMat * toWorld;
		duration--;
		isFired = false;
	}

private:
	/*  Functions   */
	// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(string const &path)
	{
		// read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		// retrieve the directory path of the filepath
		directory = path.substr(0, path.find_last_of('/'));

		// process ASSIMP's root node recursively
		processNode(scene->mRootNode, scene);
	}

	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode *node, const aiScene *scene)
	{
		// process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}

	}
	float xmin = std::numeric_limits<float>::max();
	float xmax = std::numeric_limits<float>::min();
	float ymin = std::numeric_limits<float>::max();
	float ymax = std::numeric_limits<float>::min();
	float zmin = std::numeric_limits<float>::max();
	float zmax = std::numeric_limits<float>::min();


	Mesh processMesh(aiMesh *mesh, const aiScene *scene)
	{
		// data to fill
		
		vector<Texture> textures;
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
							  // positions
			if (mesh->mVertices[i].x < xmin)
				xmin = mesh->mVertices[i].x;
			if (mesh->mVertices[i].x > xmax)
				xmax = mesh->mVertices[i].x;
			if (mesh->mVertices[i].y < ymin)
				xmin = mesh->mVertices[i].y;
			if (mesh->mVertices[i].y > ymax)
				xmax = mesh->mVertices[i].y;
			if (mesh->mVertices[i].z < zmin)
				xmin = mesh->mVertices[i].z;
			if (mesh->mVertices[i].z > zmax)
				xmax = mesh->mVertices[i].z;
		}
		float rmax = 0;
		float ratio = 0;
		float xmid = (xmax + xmin) / 2;
		float ymid = (ymax + ymin) / 2;
		float zmid = (zmax + zmin) / 2;
		float xrange = xmax - xmin;
		float yrange = ymax - ymin;
		float zrange = zmax - zmin;
		float rarray[] = { xrange, yrange, zrange };
		for (int i = 0; i < 3; i++) {
			if (rarray[i] > rmax)
				rmax = rarray[i];
		}
		if (rmax > 0.14) {
			ratio = rmax / 0.14;
			//cout << ratio << endl;
		}
		else {
			ratio = 1;
		}
		// Walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
							  // positions

			vector.x = mesh->mVertices[i].x / ratio;
			vector.y = mesh->mVertices[i].y / ratio;
			vector.z = mesh->mVertices[i].z / ratio;

			vertex.Position = vector;
			// normals
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
			// texture coordinates
			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				glm::vec2 vec;
				// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
				// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
				//cout << "tex coord is: " << vec.x << ", " << vec.y << endl; //
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			// tangent
			if (mesh != nullptr && mesh->mTangents != nullptr) {
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;
			}
			// bitangent
			if (mesh != nullptr && mesh->mBitangents != nullptr) {
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;
			}
			vertices.push_back(vertex);
		}


		// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			// retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		// process materials
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
		// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
		// Same applies to other texture as the following list summarizes:
		// diffuse: texture_diffuseN
		// specular: texture_specularN
		// normal: texture_normalN

		// 1. diffuse maps
		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// 2. specular maps
		vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		// 3. normal maps
		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		// 4. height maps
		std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		// return a mesh object created from the extracted mesh data
		return Mesh(vertices, indices, textures);
	}

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			bool skip = false;
			for (unsigned int j = 0; j < tex.size(); j++)
			{
				if (std::strcmp(tex[j].path.data(), str.C_Str()) == 0)
				{
					textures.push_back(tex[j]);
					skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}
			if (!skip)
			{   // if texture hasn't been loaded already, load it
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory,false);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				tex.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
			}
		}
		return textures;
	}
	unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
	{
		string filename = string(path);
		filename = directory + '/' + filename;

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return textureID;
	}
};




#endif
