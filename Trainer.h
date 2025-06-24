#ifndef TRAINER_H
#define TRAINER_H

#include <Python.h>
#include <variant>
#include "util.h"


class Trainer {
public:
    using VariantType = std::variant<unsigned int, double, size_t>;
    Trainer();
    std::vector<double> trainData(std::tuple<double, double, double, unsigned long, unsigned int, unsigned long> data);

private:
    unsigned int lastBlock;
    double totalSupply;
    double circSupply;
    double Balance;
    size_t periodVotes;
    unsigned int height;
    size_t txVolume;
};



#endif //TRAINER_H
