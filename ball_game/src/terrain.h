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
    bool visible = false;
    bool buffered = false;
    bool hasWater = false;
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

    std::vector<std::pair<int, int>> checkForVisibleChunks(int chunkMapSize, float playerPosX, float playerPosZ, const glm::vec3& front) {
        checkCurrentChunk(&currentChunk, playerPosX, playerPosZ);

        std::vector<std::pair<int, int>> visibleChunks;
        visibleChunks.reserve(chunkMapSize * chunkMapSize); // Reserve space to avoid multiple allocations

        int halfMapSize = chunkMapSize / 2;
        int startX = currentChunk.first - halfMapSize;
        int endX = currentChunk.first + halfMapSize;
        int startZ = currentChunk.second - halfMapSize;
        int endZ = currentChunk.second + halfMapSize;

        for (int x = startX; x <= endX; ++x) {
            for (int z = startZ; z <= endZ; ++z) {
                auto chunkCoords = std::make_pair(x, z);
                auto it = chunkMap.find(chunkCoords);

                if (it == chunkMap.end()) {
                    terrainChunk newChunk;
                    newChunk.posX = x * chunkSize;
                    newChunk.posZ = z * chunkSize;
                    newChunk.size = chunkSize + 1;
                    newChunk.numStrips = newChunk.size * 3;
                    newChunk.numVertsPerStrip = newChunk.size * 3;
                    generateChunk(&newChunk, chunkHeight, chunkResolution, lacunarity, persistance, octaves);
                    newChunk.generated = true;
                    newChunk.chunkID = chunksGenerated++;
                    newChunk.chunkMapCoords = chunkCoords;
                    chunkMap[chunkCoords] = newChunk;
                    it = chunkMap.find(chunkCoords);
                }

                terrainChunk& chunk = it->second;
                glm::vec3 chunkDir = glm::vec3(chunk.posX, 0, chunk.posZ) - glm::vec3(playerPosX, 0, playerPosZ);

                if (glm::dot(front, chunkDir) > 0.0f) {
                    chunk.visible = true;
                    visibleChunks.push_back(chunkCoords);
                }
                else {
                    chunk.visible = false;
                    // Problems with not rendering unseen chunks so having them rendered for now
                    visibleChunks.push_back(chunkCoords);
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
    const float waterLevel = (chunkHeight * 0.4f) - chunkHeight; // If chunkmap.frag's water level is changed from 0.2f adjust this value

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

    void generateChunk(terrainChunk* chunk, float mapHeight, int chunkResolution, float lacunarity, float persistance, int octaves) {
        int vertexIndex = 0;
        float scale = 50.0f;
        chunk->vertices.clear();
        chunk->indices.clear();

        SimplexNoise simplex(0.1f / scale, 0.5f, lacunarity, persistance);
        chunk->vertices.reserve((chunk->size - 1) * (chunk->size - 1) * 6 * 8); // Reserve space for vertices
        chunk->indices.reserve((chunk->size - 1) * (chunk->size - 1) * 6); // Reserve space for indices

        for (float x = chunk->posX; x < chunk->size + chunk->posX - 1; x += chunkResolution) {
            for (float z = chunk->posZ; z < chunk->size + chunk->posZ; z += chunkResolution) {
                float x1 = x, z1 = z, y1 = simplex.fractal(octaves, x1, z1) * mapHeight;
                float x2 = x, z2 = z + chunkResolution, y2 = simplex.fractal(octaves, x2, z2) * mapHeight;
                float x3 = x + chunkResolution, z3 = z + chunkResolution, y3 = simplex.fractal(octaves, x3, z3) * mapHeight;
                float x4 = x + chunkResolution, z4 = z, y4 = simplex.fractal(octaves, x4, z4) * mapHeight;

                glm::vec3 a(x1, y1, z1), b(x2, y2, z2), c(x3, y3, z3), d(x4, y4, z4);
                glm::vec3 normal1 = calculateTriangleNormal(a, b, c);
                glm::vec3 normal2 = calculateTriangleNormal(a, c, d);

                float texCoordX1 = x / TEXTURE_SIZE, texCoordZ1 = z / TEXTURE_SIZE;
                float texCoordX2 = (x + chunkResolution) / TEXTURE_SIZE, texCoordZ2 = (z + chunkResolution) / TEXTURE_SIZE;

                auto pushVertex = [&](float x, float y, float z, const glm::vec3& normal, float texX, float texZ) {
                    chunk->vertices.push_back(x);
                    chunk->vertices.push_back(y);
                    chunk->vertices.push_back(z);
                    chunk->vertices.push_back(normal.x);
                    chunk->vertices.push_back(normal.y);
                    chunk->vertices.push_back(normal.z);
                    chunk->vertices.push_back(texX);
                    chunk->vertices.push_back(texZ);
                    };

                // Triangle 1
                pushVertex(x1, y1, z1, normal1, texCoordX1, texCoordZ1);
                pushVertex(x2, y2, z2, normal1, texCoordX1, texCoordZ2);
                pushVertex(x3, y3, z3, normal1, texCoordX2, texCoordZ2);

                // Triangle 2
                pushVertex(x1, y1, z1, normal2, texCoordX1, texCoordZ1);
                pushVertex(x3, y3, z3, normal2, texCoordX2, texCoordZ2);
                pushVertex(x4, y4, z4, normal2, texCoordX2, texCoordZ1);

                // Indices
                chunk->indices.push_back(vertexIndex);
                chunk->indices.push_back(vertexIndex + 1);
                chunk->indices.push_back(vertexIndex + 2);
                chunk->indices.push_back(vertexIndex + 3);
                chunk->indices.push_back(vertexIndex + 4);
                chunk->indices.push_back(vertexIndex + 5);

                vertexIndex += 6;

                if (y1 < waterLevel || y2 < waterLevel || y3 < waterLevel || y4 < waterLevel) {
					chunk->hasWater = true;
				}
            }
        }
        chunk->generated = true;
    }

    void generateWaterPlane(terrainChunk* chunk, float mapHeight) {
        std::vector<float> waterVertices;
        
        // If I want the water to be textured remember to add texture coordinatess
        //float texCoordX1 = x / TEXTURE_SIZE, texCoordZ1 = z / TEXTURE_SIZE;
        //float texCoordX2 = (x + chunkResolution) / TEXTURE_SIZE, texCoordZ2 = (z + chunkResolution) / TEXTURE_SIZE;

        float x1 = chunk->posX, z1 = chunk->posZ; // Bottom left
        float x2 = chunk->posX, z2 = chunk->posZ + chunk->size; // Top left
        float x3 = chunk->posX + chunk->size, z3 = chunk->posZ + chunk->size; // Top right
        float x4 = chunk->posX + chunk->size, z4 = chunk->posZ; // Bottom right

        auto pushVertex = [&](float x, float y, float z) {
			waterVertices.push_back(x);
			waterVertices.push_back(y);
			waterVertices.push_back(z);
		};

        // Triangle 1
        pushVertex(x1, waterLevel, z1);
        pushVertex(x2, waterLevel, z2);
        pushVertex(x3, waterLevel, z3);

        // Triangle 2
        pushVertex(x1, waterLevel, z1);
        pushVertex(x3, waterLevel, z3);
        pushVertex(x4, waterLevel, z4);

    }
};