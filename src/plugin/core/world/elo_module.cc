#include "elo_module.h"

bool ELOModule::Start() { return true; }

bool ELOModule::Destroy() { return true; }

bool ELOModule::Update() { return true; }

bool ELOModule::AfterStart() {
    int ratingA = 100;
    int ratingB = 160;

    float probability = Probability(ratingA, ratingB);

    int resultA = 0;
    int resultB = 0;

    EloRating(ratingA, ratingB, true, resultA, resultB);

    EloRating(ratingA, ratingB, false, resultA, resultB);

    return true;
}

// Function to calculate the Probability
float ELOModule::Probability(int ratingA, int ratingB) { return (float)(1.0 * 1.0 / (1 + 1.0 * pow(10, 1.0 * (ratingA - ratingB) / 400))); }

// Function to calculate Elo rating
// K is a constant.
// d determines whether Player A wins or Player B.
void ELOModule::EloRating(int ratingA, int ratingB, bool aWin, int &resultA, int &resultB) {
    resultA = 0;
    resultB = 0;

    // To calculate the Winning
    // Probability of Player B
    float Pb = Probability(ratingA, ratingB);

    // To calculate the Winning
    // Probability of Player A
    float Pa = Probability(ratingB, ratingA);

    // Case -1 When Player A wins
    // Updating the Elo Ratings
    if (aWin == 1) {
        resultA = (int)(EloK() * (1 - Pa));
        resultB = (int)(EloK() * (0 - Pb));
    } else {
        // Case -2 When Player B wins
        // Updating the Elo Ratings

        resultA = (int)(EloK() * (0 - Pa));
        resultB = (int)(EloK() * (1 - Pb));
    }
}

int ELOModule::EloK() { return K; }