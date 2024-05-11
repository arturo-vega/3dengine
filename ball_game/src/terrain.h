#pragma once

#include <vector>
#include <map>
#include <glad/glad.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#include "SimplexNoise.h"

struct terrainChunk {
    int posX = 0;
    int posZ = 0;
    int size = 0; 
    int chunkID = 0;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    bool generated = false;
    bool visible = true; // should be set to false later after testing
    bool buffered = false;
    unsigned int numStrips = 0;
    unsigned int numVertsPerStrip = 0;
};

class Terrain {
public:
    int chunksGenerated = 0;
    const int chunkSize = 50; // currently should be a multiple of 6
    std::map<std::pair<int, int>, terrainChunk> chunkMap;
    // constructor, generates the initial area around the player
    Terrain(float chunkHeight, int chunkResolution, float lacunarity, float persistance, int octaves, int chunksVisible) {
        // view distance is measured in number of chunks
        chunksVisible = chunksVisible / 2;
        for (int x = chunksVisible * -1; x < chunksVisible; x++) {
            for (int z = chunksVisible * -1; z < chunksVisible; z++) {
                terrainChunk newChunk;
                newChunk.posX = x * chunkSize;
                newChunk.posZ = z * chunkSize;
                newChunk.size = chunkSize + 1; // I don't know why but if you don't add a 1 here the z axis will be one column short
                newChunk.numStrips = newChunk.size * 3;
                newChunk.numVertsPerStrip = newChunk.size * 3;
                generateChunk(&newChunk, chunkHeight, chunkResolution, lacunarity, persistance, octaves);
                chunkMap[{x, z}] = newChunk;
                newChunk.generated = true;
                newChunk.chunkID = chunksGenerated;
                chunksGenerated += 1;
                //printChunkInfo(newChunk);
            }
        }
    }

    void checkForVisibleChunks(int viewDistance, float playerPosX, float playerPosz) {
        for (int x = viewDistance * -1; x < viewDistance; x++) {
            for (int z = viewDistance * -1; z < viewDistance; z++) {

            }
        }
    }

    void printChunkInfo(terrainChunk chunk) {
        std::cout << "Chunk ID: " << chunk.chunkID << std::endl;
        std::cout << "Chunk Origin: " << chunk.posX << ", " << chunk.posZ << std::endl;
        std::cout << "Verticies: " << chunk.vertices.size() / sizeof(float) << " Indices: " << chunk.indices.size() / sizeof(unsigned int) << std::endl;
        std::cout << "Generated: " << chunk.generated << " Visible: " << chunk.visible << std::endl;
        std::cout << "Number Strips: " << chunk.numStrips << " Number Vertices in Strip: " << chunk.numVertsPerStrip << std::endl;
    }

private:
    const unsigned int TEXTURE_SIZE = 10;

    float chunkGenerated(terrainChunk chunk) {
        return chunk.generated;
    }

    glm::vec3 calculateTriangleNormal(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
        glm::vec3 edge1 = v2 - v1;
        glm::vec3 edge2 = v3 - v1;

        glm::vec3 normal = glm::cross(edge1, edge2);

        return glm::normalize(normal);
    }

    void generateChunk(terrainChunk *chunk, float mapHeight, int chunkResolution, float lacunarity, float persistance, int octaves) {
        int vertexIndex = 0;
        float scale = 50.0f;
        chunk->vertices.clear();
        chunk->indices.clear();
        SimplexNoise simplex(0.1f / scale, 0.5f, lacunarity, persistance);
        std::cout << "pos x z " << chunk->posX << " / " << chunk->posZ << std::endl;
        std::cout << "Y from pos x and z:  " << simplex.fractal(octaves, chunk->posX, chunk->posZ) * mapHeight << std::endl;
        for (float x = chunk->posX; x < chunk->size + chunk->posX - 1; x += chunkResolution) { // subtrack one from comparison because we +1 to chunk size to fix z axis
            for (float z = chunk->posZ; z < chunk->size + chunk->posZ; z += chunkResolution) {
                // will end up with each vertice having 8 floats
                // 3 position floats, 3 normal floats, 2 texture floats
                float x1, x2, x3, y1, y2, y3, z1, z2, z3;
                glm::vec3 a, b, c;
                glm::vec3 normal;
                float texCoordX1 = x / TEXTURE_SIZE;
                float texCoordZ1 = z / TEXTURE_SIZE;
                float texCoordX2 = (x + chunkResolution) / TEXTURE_SIZE;
                float texCoordZ2 = (z + chunkResolution) / TEXTURE_SIZE;

                // getting vertices from triangle one
                x1 = x;
                z1 = z;
                y1 = simplex.fractal(octaves, x1, z1) * mapHeight;
                a = glm::vec3(x1, y1, z1);

                x2 = x;
                z2 = z + chunkResolution;
                y2 = simplex.fractal(octaves, x2, z2) * mapHeight;
                b = glm::vec3(x2, y2, z2);

                x3 = x + chunkResolution;
                z3 = z + chunkResolution;
                y3 = simplex.fractal(octaves, x3, z3) * mapHeight;
                c = glm::vec3(x3, y3, z3);
                normal = calculateTriangleNormal(a, b, c);

                // pushing vertices from triangle one along with texture coords and normal vector
                chunk->vertices.push_back(x1);
                chunk->vertices.push_back(y1);
                chunk->vertices.push_back(z1);
                chunk->vertices.push_back(normal.x); // normalized coordinates
                chunk->vertices.push_back(normal.y);
                chunk->vertices.push_back(normal.z);
                chunk->vertices.push_back(texCoordX1); // texture coordinates
                chunk->vertices.push_back(texCoordZ1);

                chunk->vertices.push_back(x2);
                chunk->vertices.push_back(y2);
                chunk->vertices.push_back(z2);
                chunk->vertices.push_back(normal.x); // normalized coordinates
                chunk->vertices.push_back(normal.y);
                chunk->vertices.push_back(normal.z);
                chunk->vertices.push_back(texCoordX1); // texture coordinates
                chunk->vertices.push_back(texCoordZ2);

                chunk->vertices.push_back(x3);
                chunk->vertices.push_back(y3);
                chunk->vertices.push_back(z3);
                chunk->vertices.push_back(normal.x); // normalized coordinates
                chunk->vertices.push_back(normal.y);
                chunk->vertices.push_back(normal.z);
                chunk->vertices.push_back(texCoordX2); // texture coordinates
                chunk->vertices.push_back(texCoordZ2);

                // getting vertices from triangle 2
                x1 = x;
                z1 = z;
                y1 = simplex.fractal(octaves, x1, z1) * mapHeight;
                a = glm::vec3(x1, y1, z1);

                x2 = x + chunkResolution;
                z2 = z;
                y2 = simplex.fractal(octaves, x2, z2) * mapHeight;
                b = glm::vec3(x2, y2, z2);

                x3 = x + chunkResolution;
                z3 = z + chunkResolution;
                y3 = simplex.fractal(octaves, x3, z3) * mapHeight;
                c = glm::vec3(x3, y3, z3);
                // set it to negative because the normal vector gets the vector from the opposite side of the traingle
                // fromt the first calculation... need to fix this 
                normal = -calculateTriangleNormal(a, b, c);

                // pushing vertices from triangle 2 along with coord and normal info
                chunk->vertices.push_back(x1);
                chunk->vertices.push_back(y1);
                chunk->vertices.push_back(z1);
                chunk->vertices.push_back(normal.x); // normalized coordinates
                chunk->vertices.push_back(normal.y);
                chunk->vertices.push_back(normal.z);
                chunk->vertices.push_back(texCoordX1); // texture coordinates
                chunk->vertices.push_back(texCoordZ1);

                chunk->vertices.push_back(x2);
                chunk->vertices.push_back(y2);
                chunk->vertices.push_back(z2);
                chunk->vertices.push_back(normal.x); // normalized coordinates
                chunk->vertices.push_back(normal.y);
                chunk->vertices.push_back(normal.z);
                chunk->vertices.push_back(texCoordX1); // texture coordinates
                chunk->vertices.push_back(texCoordZ2);

                chunk->vertices.push_back(x3);
                chunk->vertices.push_back(y3);
                chunk->vertices.push_back(z3);
                chunk->vertices.push_back(normal.x); // normalized coordinates
                chunk->vertices.push_back(normal.y);
                chunk->vertices.push_back(normal.z);
                chunk->vertices.push_back(texCoordX2); // texture coordinates
                chunk->vertices.push_back(texCoordZ2);

                // fill vertices matrix
                chunk->indices.push_back(vertexIndex + 1);
                chunk->indices.push_back(vertexIndex + 5);
                chunk->indices.push_back(vertexIndex + 2);

                // fill vertices matrix
                chunk->indices.push_back(vertexIndex + 2);
                chunk->indices.push_back(vertexIndex + 1);
                chunk->indices.push_back(vertexIndex + 5);

                vertexIndex += 6;

            }
        }
    }
};