#include "FurnitureLibrary.h"
#include "rendering/Mesh.h"

// ── Paleta kolorow ─────────────────────────────────────────────────────
namespace C {
    static const glm::vec3
        WD(0.35f,0.20f,0.08f),  // drewno ciemne
        WL(0.62f,0.42f,0.22f),  // drewno jasne
        WM(0.50f,0.32f,0.14f),  // drewno srednie
        FG(0.45f,0.45f,0.50f),  // tkanina szara
        FK(0.32f,0.32f,0.37f),  // tkanina ciemna
        FB(0.78f,0.72f,0.62f),  // tkanina bezowa
        WH(0.95f,0.95f,0.95f),  // bialy
        OW(0.85f,0.85f,0.82f),  // off-white
        CR(0.90f,0.85f,0.75f),  // kremowy
        ST(0.58f,0.58f,0.62f),  // stal
        DG(0.25f,0.25f,0.28f),  // ciemnoszary
        GL(0.72f,0.82f,0.88f),  // szklo / blekitny
        BG(0.68f,0.78f,0.84f),  // niebieskoszary (wnetrze wan)
        CT(0.28f,0.28f,0.30f);  // blat kuchenny
}

// ── Pomocnik: dodaje prostopadloscian do meshu ─────────────────────────
static void addBox(Mesh& m, glm::vec3 mn, glm::vec3 mx) {
    auto& v = m.vertices;
    float x0=mn.x,y0=mn.y,z0=mn.z,x1=mx.x,y1=mx.y,z1=mx.z;
    auto quad=[&](glm::vec3 p0,glm::vec3 p1,glm::vec3 p2,glm::vec3 p3,glm::vec3 n){
        v.push_back({p0,n,glm::vec2(0,0)}); v.push_back({p1,n,glm::vec2(1,0)}); v.push_back({p2,n,glm::vec2(1,1)});
        v.push_back({p0,n,glm::vec2(0,0)}); v.push_back({p2,n,glm::vec2(1,1)}); v.push_back({p3,n,glm::vec2(0,1)});
    };
    quad({x0,y0,z0},{x1,y0,z0},{x1,y1,z0},{x0,y1,z0},glm::vec3( 0, 0,-1));
    quad({x1,y0,z1},{x0,y0,z1},{x0,y1,z1},{x1,y1,z1},glm::vec3( 0, 0, 1));
    quad({x0,y0,z1},{x0,y0,z0},{x0,y1,z0},{x0,y1,z1},glm::vec3(-1, 0, 0));
    quad({x1,y0,z0},{x1,y0,z1},{x1,y1,z1},{x1,y1,z0},glm::vec3( 1, 0, 0));
    quad({x0,y0,z1},{x1,y0,z1},{x1,y0,z0},{x0,y0,z0},glm::vec3( 0,-1, 0));
    quad({x0,y1,z0},{x1,y1,z0},{x1,y1,z1},{x0,y1,z1},glm::vec3( 0, 1, 0));
}

// Tworzy jedna czesc mebla (jeden box, jeden kolor)
static FurniturePart P(glm::vec3 mn, glm::vec3 mx, glm::vec3 col) {
    auto m = std::make_shared<Mesh>();
    addBox(*m, mn, mx);
    m->upload();
    return {m, col};
}

// ── Nazwy i kategorie ──────────────────────────────────────────────────
const char* furnitureName(FurnitureType t) {
    static const char* N[] = {
        "Krzeslo","Stol","Szafa","Szafka wisz.","Polka wisz.",
        "Komoda","Komoda kuch.","Blat kuchenny",
        "Lodowka","Kuchenka","Kanapa","Fotel","Lozko",
        "Toaleta","Wanna","Prysznic","Umywalka",
        "Pralka","Zmywarka"
    };
    int i=(int)t;
    return (i>=0&&i<(int)FurnitureType::COUNT)?N[i]:"?";
}

const char* furnitureCategory(FurnitureType t) {
    switch(t){
        case FurnitureType::CHAIR:
        case FurnitureType::TABLE:
        case FurnitureType::SOFA:
        case FurnitureType::ARMCHAIR:          return "Salon / Jadalnia";
        case FurnitureType::BED:
        case FurnitureType::WARDROBE:
        case FurnitureType::SMALL_DRESSER:     return "Sypialnia";
        case FurnitureType::KITCHEN_COUNTER:
        case FurnitureType::KITCHEN_DRESSER:
        case FurnitureType::HANGING_CABINET:
        case FurnitureType::HANGING_SHELF:
        case FurnitureType::STOVE:
        case FurnitureType::FRIDGE:
        case FurnitureType::DISHWASHER:        return "Kuchnia";
        default:                                return "Lazienka";
    }
}

float furnitureMountHeight(FurnitureType t) {
    switch(t){
        case FurnitureType::HANGING_CABINET: return 1.4f;
        case FurnitureType::HANGING_SHELF:   return 1.5f;
        default: return 0.0f;
    }
}

// ── Dispatch ───────────────────────────────────────────────────────────
std::vector<FurniturePart> FurnitureLibrary::buildParts(FurnitureType t) {
    switch(t){
        case FurnitureType::CHAIR:           return buildChair();
        case FurnitureType::TABLE:           return buildTable();
        case FurnitureType::WARDROBE:        return buildWardrobe();
        case FurnitureType::HANGING_CABINET: return buildHangingCabinet();
        case FurnitureType::HANGING_SHELF:   return buildHangingShelf();
        case FurnitureType::SMALL_DRESSER:   return buildSmallDresser();
        case FurnitureType::KITCHEN_DRESSER: return buildKitchenDresser();
        case FurnitureType::KITCHEN_COUNTER: return buildKitchenCounter();
        case FurnitureType::FRIDGE:          return buildFridge();
        case FurnitureType::STOVE:           return buildStove();
        case FurnitureType::SOFA:            return buildSofa();
        case FurnitureType::ARMCHAIR:        return buildArmchair();
        case FurnitureType::BED:             return buildBed();
        case FurnitureType::TOILET:          return buildToilet();
        case FurnitureType::BATHTUB:         return buildBathtub();
        case FurnitureType::SHOWER:          return buildShower();
        case FurnitureType::SINK:            return buildSink();
        case FurnitureType::WASHING_MACHINE: return buildWashingMachine();
        case FurnitureType::DISHWASHER:      return buildDishwasher();
        default: return {};
    }
}

// ══════════════════════════════════════════════════════════════════════
//  MEBLE — origin = centrum podstawy, X=szerokosc, Y=gora, Z=glebokosc
// ══════════════════════════════════════════════════════════════════════

std::vector<FurniturePart> FurnitureLibrary::buildChair() {
    return {
        P({-0.25f,0.44f,-0.25f},{0.25f,0.50f,0.25f},  C::WL), // siedzisko
        P({-0.22f,0.50f, 0.19f},{0.22f,0.90f,0.25f},  C::WL), // oparcie
        P({-0.22f,0,-0.22f},{-0.16f,0.44f,-0.16f},    C::WD), // noga PP
        P({ 0.16f,0,-0.22f},{ 0.22f,0.44f,-0.16f},    C::WD), // noga LP
        P({-0.22f,0, 0.16f},{-0.16f,0.44f, 0.22f},    C::WD), // noga PT
        P({ 0.16f,0, 0.16f},{ 0.22f,0.44f, 0.22f},    C::WD), // noga LT
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildTable() {
    return {
        P({-0.60f,0.72f,-0.40f},{0.60f,0.75f,0.40f},  C::WL), // blat
        P({-0.57f,0,-0.37f},{-0.52f,0.72f,-0.32f},    C::WD),
        P({ 0.52f,0,-0.37f},{ 0.57f,0.72f,-0.32f},    C::WD),
        P({-0.57f,0, 0.32f},{-0.52f,0.72f, 0.37f},    C::WD),
        P({ 0.52f,0, 0.32f},{ 0.57f,0.72f, 0.37f},    C::WD),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildWardrobe() {
    return {
        P({-0.50f,0,-0.30f},{0.50f,2.00f,0.30f},          C::WL),
        P({-0.02f,0.02f,-0.30f},{0.02f,1.98f,-0.28f},     C::WD), // pion
        P({-0.50f,0.98f,-0.30f},{0.50f,1.02f,-0.28f},     C::WD), // poziom
        P({-0.11f,0.90f,-0.31f},{-0.05f,1.10f,-0.29f},    C::ST), // uchwyt L
        P({ 0.05f,0.90f,-0.31f},{ 0.11f,1.10f,-0.29f},    C::ST), // uchwyt R
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildHangingCabinet() {
    return {
        P({-0.30f,0,-0.175f},{0.30f,0.70f,0.175f},        C::CR),
        P({-0.02f,0.02f,-0.175f},{0.02f,0.68f,-0.17f},    C::DG),
        P({-0.06f,0.32f,-0.178f},{0.06f,0.38f,-0.175f},   C::ST),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildHangingShelf() {
    return {
        P({-0.40f,0,-0.125f},{0.40f,0.04f,0.125f},        C::WM),
        P({-0.36f,-0.12f,0.08f},{-0.32f,0,0.125f},        C::ST),
        P({ 0.32f,-0.12f,0.08f},{ 0.36f,0,0.125f},        C::ST),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildSmallDresser() {
    return {
        P({-0.50f,0,-0.225f},{0.50f,0.80f,0.225f},           C::WL),
        P({-0.50f,0.255f,-0.225f},{0.50f,0.265f,-0.22f},     C::WD),
        P({-0.50f,0.445f,-0.225f},{0.50f,0.455f,-0.22f},     C::WD),
        P({-0.50f,0.635f,-0.225f},{0.50f,0.645f,-0.22f},     C::WD),
        P({-0.08f,0.09f,-0.228f},{0.08f,0.14f,-0.225f},      C::ST),
        P({-0.08f,0.33f,-0.228f},{0.08f,0.37f,-0.225f},      C::ST),
        P({-0.08f,0.52f,-0.228f},{0.08f,0.56f,-0.225f},      C::ST),
        P({-0.08f,0.71f,-0.228f},{0.08f,0.75f,-0.225f},      C::ST),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildKitchenDresser() {
    return {
        P({-0.50f,0,-0.30f},{0.50f,0.85f,0.30f},             C::CR),
        P({-0.50f,0.85f,-0.315f},{0.50f,0.88f,0.315f},       C::CT),
        P({-0.49f,0.02f,-0.30f},{-0.02f,0.83f,-0.295f},      C::OW),
        P({ 0.02f,0.02f,-0.30f},{ 0.49f,0.83f,-0.295f},      C::OW),
        P({-0.10f,0.40f,-0.303f},{-0.02f,0.44f,-0.298f},     C::ST),
        P({ 0.02f,0.40f,-0.303f},{ 0.10f,0.44f,-0.298f},     C::ST),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildKitchenCounter() {
    return {
        P({-0.30f,0,-0.30f},{0.30f,0.85f,0.30f},             C::CR),
        P({-0.315f,0.85f,-0.315f},{0.315f,0.90f,0.315f},     C::CT),
        P({-0.29f,0.02f,-0.30f},{0.29f,0.83f,-0.295f},       C::OW),
        P({-0.04f,0.40f,-0.303f},{0.04f,0.44f,-0.298f},      C::ST),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildFridge() {
    return {
        P({-0.30f,0,-0.325f},{0.30f,1.85f,0.325f},           C::WH),
        P({-0.30f,0.60f,-0.325f},{0.30f,0.62f,-0.32f},       C::ST),
        P({ 0.24f,0.80f,-0.33f},{ 0.28f,1.10f,-0.325f},      C::ST),
        P({ 0.24f,0.15f,-0.33f},{ 0.28f,0.55f,-0.325f},      C::ST),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildStove() {
    return {
        P({-0.30f,0,-0.325f},{0.30f,0.85f,0.325f},           C::CR),
        P({-0.30f,0.85f,-0.325f},{0.30f,0.88f,0.325f},       C::DG),
        P({-0.22f,0.88f,-0.22f},{-0.07f,0.91f,-0.07f},       C::DG),
        P({ 0.07f,0.88f,-0.22f},{ 0.22f,0.91f,-0.07f},       C::DG),
        P({-0.22f,0.88f, 0.07f},{-0.07f,0.91f, 0.22f},       C::DG),
        P({ 0.07f,0.88f, 0.07f},{ 0.22f,0.91f, 0.22f},       C::DG),
        P({-0.28f,0.02f,-0.326f},{0.28f,0.48f,-0.322f},      C::DG),
        P({-0.09f,0.48f,-0.329f},{0.09f,0.52f,-0.325f},      C::ST),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildSofa() {
    return {
        P({-1.00f,0,-0.45f},{1.00f,0.45f,0.45f},             C::FG),
        P({-1.00f,0.45f,0.20f},{1.00f,0.85f,0.45f},          C::FG),
        P({-1.00f,0.45f,-0.45f},{-0.83f,0.72f,0.45f},        C::FK),
        P({ 0.83f,0.45f,-0.45f},{ 1.00f,0.72f,0.45f},        C::FK),
        P({-0.83f,0.45f,-0.40f},{0.00f,0.57f,0.18f},         C::FB),
        P({ 0.00f,0.45f,-0.40f},{0.83f,0.57f,0.18f},         C::FB),
        P({-0.96f,0,-0.42f},{-0.90f,0.08f,-0.36f},           C::DG),
        P({ 0.90f,0,-0.42f},{ 0.96f,0.08f,-0.36f},           C::DG),
        P({-0.96f,0, 0.36f},{-0.90f,0.08f, 0.42f},           C::DG),
        P({ 0.90f,0, 0.36f},{ 0.96f,0.08f, 0.42f},           C::DG),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildArmchair() {
    return {
        P({-0.45f,0,-0.45f},{0.45f,0.45f,0.45f},             C::FG),
        P({-0.45f,0.45f,0.20f},{0.45f,0.85f,0.45f},          C::FG),
        P({-0.45f,0.45f,-0.45f},{-0.32f,0.68f,0.45f},        C::FK),
        P({ 0.32f,0.45f,-0.45f},{ 0.45f,0.68f,0.45f},        C::FK),
        P({-0.32f,0.45f,-0.40f},{0.32f,0.56f,0.18f},         C::FB),
        P({-0.42f,0,-0.42f},{-0.36f,0.08f,-0.36f},           C::DG),
        P({ 0.36f,0,-0.42f},{ 0.42f,0.08f,-0.36f},           C::DG),
        P({-0.42f,0, 0.36f},{-0.36f,0.08f, 0.42f},           C::DG),
        P({ 0.36f,0, 0.36f},{ 0.42f,0.08f, 0.42f},           C::DG),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildBed() {
    return {
        P({-0.80f,0,-1.00f},{0.80f,0.25f,1.00f},             C::WD),
        P({-0.75f,0.25f,-0.88f},{0.75f,0.44f,0.92f},         C::FB),
        P({-0.65f,0.44f,0.55f},{-0.10f,0.54f,0.82f},         C::WH),
        P({ 0.10f,0.44f,0.55f},{ 0.65f,0.54f,0.82f},         C::WH),
        P({-0.80f,0.25f,0.88f},{0.80f,0.72f,1.00f},          C::WD),
        P({-0.78f,0,-0.98f},{-0.72f,0.25f,-0.92f},           C::WD),
        P({ 0.72f,0,-0.98f},{ 0.78f,0.25f,-0.92f},           C::WD),
        P({-0.78f,0, 0.92f},{-0.72f,0.25f, 0.98f},           C::WD),
        P({ 0.72f,0, 0.92f},{ 0.78f,0.25f, 0.98f},           C::WD),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildToilet() {
    return {
        P({-0.19f,0,-0.05f},{0.19f,0.38f,0.325f},            C::WH),
        P({-0.17f,0.38f,0.12f},{0.17f,0.72f,0.325f},         C::WH),
        P({-0.17f,0.38f,-0.05f},{0.17f,0.42f,0.12f},         C::OW),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildBathtub() {
    return {
        P({-0.40f,0,-0.85f},{0.40f,0.55f,0.85f},             C::WH),
        P({-0.32f,0.10f,-0.78f},{0.32f,0.55f,0.78f},         C::BG),
        P({-0.06f,0.55f,-0.70f},{0.06f,0.65f,-0.64f},        C::ST),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildShower() {
    return {
        P({-0.45f,0,-0.45f},{0.45f,0.10f,0.45f},             C::WH),
        P({-0.45f,0.10f,-0.45f},{-0.42f,2.20f,0.45f},        C::GL),
        P({ 0.42f,0.10f,-0.45f},{ 0.45f,2.20f,0.45f},        C::GL),
        P({-0.45f,0.10f, 0.42f},{ 0.45f,2.20f,0.45f},        C::GL),
        P({-0.04f,1.80f,-0.46f},{ 0.04f,1.88f,-0.42f},       C::ST),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildSink() {
    return {
        P({-0.10f,0,-0.10f},{0.10f,0.65f,0.12f},             C::WH),
        P({-0.275f,0.65f,-0.225f},{0.275f,0.78f,0.225f},     C::WH),
        P({-0.22f,0.68f,-0.18f},{0.22f,0.78f,0.18f},         C::BG),
        P({-0.03f,0.78f,-0.06f},{0.03f,0.90f,0.02f},         C::ST),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildWashingMachine() {
    return {
        P({-0.30f,0,-0.30f},{0.30f,0.85f,0.30f},             C::WH),
        P({-0.22f,0.25f,-0.31f},{0.22f,0.72f,-0.27f},        C::ST),
        P({-0.16f,0.31f,-0.312f},{0.16f,0.66f,-0.27f},       C::DG),
        P({-0.30f,0.78f,-0.30f},{0.30f,0.85f,-0.27f},        C::DG),
        P({ 0.08f,0.80f,-0.31f},{ 0.20f,0.83f,-0.30f},       C::ST),
    };
}

std::vector<FurniturePart> FurnitureLibrary::buildDishwasher() {
    return {
        P({-0.30f,0,-0.30f},{0.30f,0.85f,0.30f},             C::CR),
        P({-0.29f,0.02f,-0.31f},{0.29f,0.72f,-0.30f},        C::OW),
        P({-0.30f,0.72f,-0.31f},{0.30f,0.85f,-0.30f},        C::DG),
        P({-0.06f,0.77f,-0.312f},{0.06f,0.81f,-0.31f},       C::ST),
    };
}