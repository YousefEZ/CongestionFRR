#ifndef MODULO_CONGESTION_POLICY_H
#define MODULO_CONGESTION_POLICY_H

template <int MODULO>
class ModuloCongestionPolicy
{

  private:
    int counter;

  public:
    template <typename CONTAINER>
    bool isCongested(const CONTAINER& container)
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
