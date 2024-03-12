#ifndef RANDOM_CONGESTION_POLICY_H
#define RANDOM_CONGESTION_POLICY_H

#include <random>

template <int PERCENTAGE>
class RandomCongestionPolicy
{
    static_assert(PERCENTAGE >= 0 && PERCENTAGE <= 100,
                  "Percentage must be between 0 and 100");

    bool m_off = true;
    bool m_enabled = false;

  public:
    RandomCongestionPolicy() = default;

    bool isCongested(ns3::Queue<ns3::Packet>* queue) const
    {
       	if (!m_enabled) return false; 
	std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 100);
        return !m_off && dis(gen) < PERCENTAGE;
    }

    void turnOff()
    {
        m_off = false;
    }
   
    void enable()
    {
	m_enabled = true;	
    }
};

#endif
