#ifndef MODULO_CONGESTION_POLICY_H
#define MODULO_CONGESTION_POLICY_H

template <int MODULO>
class ModuloCongestionPolicy
{

  private:
    int counter;

  public:

    bool isCongested(ns3::Queue<ns3::Packet>* queue)
    {
        increment();
        return counter;
    }

    void increment()
    {
        counter = (counter + 1) % MODULO;
    }
};

#endif
