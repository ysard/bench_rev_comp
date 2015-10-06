#ifndef _HG_REV_COMP_BIN_HPP_HG_
#define _HG_REV_COMP_BIN_HPP_HG_

#include <iostream>
#include <bitset>
#include <array>
#include <cstdint>
#include <cmath>
#include <ctime>
// http://www.boost.org/doc/libs/1_36_0/libs/dynamic_bitset/dynamic_bitset.html
#include <boost/dynamic_bitset.hpp>

// Binary values for each base (thanks to C++11)
#define A_BIN      0b001
#define T_BIN      0b100
#define G_BIN      0b011
#define C_BIN      0b110
// Decimal values for each bases (ASCII code)
#define A_ASCII    65
#define T_ASCII    84
#define G_ASCII    71
#define C_ASCII    67
// Base's encoding length in bits
#define BIN_LENGTH 3

#include "arev_comp.hpp"

///////////////////////////////////////////////////////////////////////////////
/// \brief The rev_comp_bin class
///
class rev_comp_bin : public arev_comp
{
public:
    rev_comp_bin();
    virtual std::string run(std::string sequence);
private:
    // Initialises the mapping between ASCII & Binary data
    std::array<uint8_t, 85> m_mappingDataADN;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief The Convert class
///
class Convert
{
    /* functor used to convert the sequence of chars to a sequence of bits */
public:
    Convert(uint64_t const& length);

    //void operator()(char const& base)
    void operator()(uint8_t const& base);
    boost::dynamic_bitset<>* getBitset();

private:
    boost::dynamic_bitset<> m_bitset;
    uint64_t                m_current_pos;
    std::array<uint8_t, 85> m_mappingADNData;
};

#endif /* _HG_REV_COMP_BIN_HPP_HG_ */

