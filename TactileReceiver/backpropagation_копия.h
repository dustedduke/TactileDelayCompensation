#ifndef BACKPROPAGATION_H
#define BACKPROPAGATION_H


#include <stdio.h>
#include <stdexcept>
#include "memoryblock.h"
#include "neurallayer.h"
#include "feedforwardnetwork.h"

using std::logic_error;

class Backpropagation {
private:
    FeedforwardNetwork* m_network;

public:
    Backpropagation(FeedforwardNetwork* network);
    void Train(const MemoryBlock& target);

    float error;
};

#endif // BACKPROPAGATION_H
