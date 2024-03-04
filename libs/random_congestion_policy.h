#ifndef RANDOM_CONGESTION_POLICY_H
#define RANDOM_CONGESTION_POLICY_H

#include <random>


template <int PERCENTAGE>
class RandomCongestionPolicy
{
  static_assert(PERCENTAGE >= 0 && PERCENTAGE <= 100, "Percentage must be between 0 and 100");

  bool m_off; 

  public:

    RandomCongestionPolicy() = default;

    template <typename CONTAINER>
    bool isCongested(const CONTAINER& container) const
    {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<> dis(0, 100);
      return !m_off && dis(gen) < PERCENTAGE;
    }
  
    void turnOff()
    {
      m_off = true;
    }
};



#endif
