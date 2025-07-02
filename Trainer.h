#ifndef TRAINER_H
#define TRAINER_H

#include "util.h"
#include "OLC_NET/net_common.h"
using json = nlohmann::json;

class Trainer {
public:
    Trainer();
    void load_data(const std::vector<double>& data);
    std::vector<double> train() const;

private:
    size_t MAX_DATA_LENGTH;
    std::string json_body;
};



#endif //TRAINER_H
