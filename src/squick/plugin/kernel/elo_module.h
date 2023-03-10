

#ifndef SQUICK_ELO_MODULE_H
#define SQUICK_ELO_MODULE_H

#include "i_elo_module.h"

class ELOModule
    : public IELOModule
{
public:
	ELOModule(IPluginManager* p)
    {
        pPluginManager = p;
    }
    virtual ~ELOModule() {};

    virtual bool Start() override;
    virtual bool Destory() override;
    virtual bool Update() override;
    virtual bool AfterStart() override;


	virtual float Probability(int ratingA, int ratingB) override;

	virtual void EloRating(int ratingA, int ratingB, bool aWin, int& resultA, int& resultB) override;

private:
	int EloK();

private:
	int K = 50;
};


#endif
