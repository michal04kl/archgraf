#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>

class Mesh;

enum class FurnitureType : int {
    CHAIR=0, TABLE, WARDROBE, HANGING_CABINET, HANGING_SHELF,
    SMALL_DRESSER, KITCHEN_DRESSER, KITCHEN_COUNTER,
    FRIDGE, STOVE, SOFA, ARMCHAIR, BED,
    TOILET, BATHTUB, SHOWER, SINK,
    WASHING_MACHINE, DISHWASHER,
    COUNT
};

const char* furnitureName(FurnitureType t);
const char* furnitureCategory(FurnitureType t);
float       furnitureMountHeight(FurnitureType t);

struct FurniturePart {
    std::shared_ptr<Mesh> mesh;
    glm::vec3 color;
};

class FurnitureLibrary {
public:
    static std::vector<FurniturePart> buildParts(FurnitureType t);
private:
    static std::vector<FurniturePart> buildChair();
    static std::vector<FurniturePart> buildTable();
    static std::vector<FurniturePart> buildWardrobe();
    static std::vector<FurniturePart> buildHangingCabinet();
    static std::vector<FurniturePart> buildHangingShelf();
    static std::vector<FurniturePart> buildSmallDresser();
    static std::vector<FurniturePart> buildKitchenDresser();
    static std::vector<FurniturePart> buildKitchenCounter();
    static std::vector<FurniturePart> buildFridge();
    static std::vector<FurniturePart> buildStove();
    static std::vector<FurniturePart> buildSofa();
    static std::vector<FurniturePart> buildArmchair();
    static std::vector<FurniturePart> buildBed();
    static std::vector<FurniturePart> buildToilet();
    static std::vector<FurniturePart> buildBathtub();
    static std::vector<FurniturePart> buildShower();
    static std::vector<FurniturePart> buildSink();
    static std::vector<FurniturePart> buildWashingMachine();
    static std::vector<FurniturePart> buildDishwasher();
};