#ifndef RANDOM_CONGESTION_POLICY_H
#define RANDOM_CONGESTION_POLICY_H

#include <random>

template <int PERCENTAGE>
class RandomCongestionPolicy
{
    static_assert(PERCENTAGE >= 0 && PERCENTAGE <= 100,
                  "Percentage must be between 0 and 100");

    bool m_off = false;

  public:
    RandomCongestionPolicy() = default;

    bool isCongested(ns3::Queue<ns3::Packet>* queue) const;

    void turnOff()
    {
        m_off = true;
    }
};

template <int PERCENTAGE>
bool RandomCongestionPolicy<PERCENTAGE>::isCongested(
    ns3::Queue<ns3::Packet>* queue) const
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 100);
    return !m_off && dis(gen) < PERCENTAGE;
}

template <>
bool RandomCongestionPolicy<100>::isCongested(
    ns3::Queue<ns3::Packet>* queue) const
{
    return !m_off;
}

#endif
