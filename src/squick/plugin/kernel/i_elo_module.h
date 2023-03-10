


#ifndef SQUICK_INTF_ELO_MODULE_H
#define SQUICK_INTF_ELO_MODULE_H

#include <iostream>
#include <squick/core/i_module.h>


class IELOModule
    : public IModule
{
public:
	// To calculate the Probability that B win A
	virtual float Probability(int ratingA, int ratingB) = 0;

	// To calculate Elo rating
	// aWin determines whether Player A wins or loses.
	//rA is the result for A, if A won B, rA > 0, else rA < 0
	//rB is the result for B, if B won A, rB > 0, else rB < 0
	//for example:
	/*
	 	int ratingA = 100;
		int ratingB = 160;

		float probability = Probability(ratingA, ratingB);

		int resultA = 0;
		int resultB = 0;
		EloRating(ratingA, ratingB, true, resultA, resultB);

	 	result:
	 	probability = 0.585;
	 	resultA = 29;
	 	resultB = -29;

		EloRating(ratingA, ratingB, false, resultA, resultB);
	 	resultA = -20;
	 	resultB = 20;
	 */
	virtual void EloRating(int ratingA, int ratingB, bool aWin, int& resultA, int& resultB) = 0;
};

#endif