#pragma once

#include <vector>
#include <cmath>
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
    std::pair<int, int> chunkMapCoords;
    bool generated = false;
    bool visible = true; // should be set to false later after testing
    bool buffered = false;
    unsigned int numStrips = 0;
    unsigned int numVertsPerStrip = 0;
};

class Terrain {
public:
    int chunksGenerated = 0;
    int chunkSize; // Multiple of 5 seems to work not sure about other multiples
    int chunkMapSize;
    std::map<std::pair<int, int>, terrainChunk> chunkMap;
    float chunkHeight;
    float lacunarity; 
    float persistance; 
    int octaves;
    int chunkResolution;
    std::pair<int, int> currentChunk = { 0,0 };

    // constructor, generates the initial area around the player
    Terrain(float chunkHeight, int chunkResolution, float lacunarity, float persistance, int octaves, int chunkMapSize, int chunkSize) {
        this->chunkHeight = chunkHeight;
        this->lacunarity = lacunarity;
        this->persistance = persistance;
        this->octaves = octaves;
        this->chunkResolution = chunkResolution;
        this->chunkSize = chunkSize;
        this->chunkMapSize = chunkMapSize;
    }

    std::vector<std::pair<int, int>> checkForVisibleChunks(int chunkMapSize, float playerPosX, float playerPosZ, glm::vec3 front) {
        checkCurrentChunk(&currentChunk, playerPosX, playerPosZ);

        std::vector<std::pair<int,int>> visibleChunks;        

        for (int x = currentChunk.first - (chunkMapSize / 2); x <= currentChunk.first + (chunkMapSize / 2); x++) {
            for (int z = currentChunk.second - (chunkMapSize / 2); z <= currentChunk.second + (chunkMapSize / 2); z++) {
                // Check if there is a value in the chunk map at the x, z position
                if (!chunkMap.count({x,z})) {
                    terrainChunk newChunk;
                    newChunk.posX = x * chunkSize;
                    newChunk.posZ = z * chunkSize;
                    newChunk.size = chunkSize + 1; // I don't know why but if you don't add a 1 here the z axis will be one column short
                    newChunk.numStrips = newChunk.size * 3;
                    newChunk.numVertsPerStrip = newChunk.size * 3;
                    generateChunk(&newChunk, chunkHeight, chunkResolution, lacunarity, persistance, octaves);
                    newChunk.generated = true;
                    chunkMap[{x, z}].visible = false;
                    newChunk.chunkID = chunksGenerated;
                    chunksGenerated += 1;
                    newChunk.chunkMapCoords.first = x;
                    newChunk.chunkMapCoords.second = z;
                    chunkMap[{x, z}] = newChunk;
                }
                // Calculate the direction from the camera to the chunk
                glm::vec3 chunkDir = glm::vec3(x * chunkSize, 0, z * chunkSize) - glm::vec3(playerPosX, 0, playerPosZ);

                // Check if the chunk is in front of the camera
                if (glm::dot(front, chunkDir) > -0.1f) {
                    // Chunk is visible, add it to the list
                    chunkMap[{x, z}].visible = true;
                    visibleChunks.push_back({ x, z });
                }
                else {
                    // Chunk is not visible, set it to not be visible
                    chunkMap[{x, z}].visible = false;
                }
            }
        }
        return visibleChunks;

    }

    void printChunkInfo(terrainChunk chunk) {
        std::cout << "Chunk ID: " << chunk.chunkID << std::endl;
        std::cout << "Chunk Coordinates: X " << chunk.posX << " Z " << chunk.posZ << std::endl;
        std::cout << "Generated: " << chunk.generated << " Visible: " << chunk.visible << std::endl;
    }

private:
    const unsigned int TEXTURE_SIZE = 10;

    void checkCurrentChunk(std::pair<int, int>* currentChunk, float playerPosX, float playerPosZ) {
        int adjustedPositionX = std::abs(playerPosX) / chunkSize; // truncates float, gives x and z values for chunkMap map
        int adjustedPositionZ = std::abs(playerPosZ) / chunkSize;

        if (playerPosX < 0) {
            adjustedPositionX *= -1;
        }
        if (playerPosZ < 0) {
            adjustedPositionZ *= -1;
        }

        if (currentChunk->first != adjustedPositionX || currentChunk->second != adjustedPositionZ) {
            currentChunk->first = adjustedPositionX;
            currentChunk->second = adjustedPositionZ;
        }
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

        chunk->generated = true;
    }
};