#include "rev_comp_bin.hpp"

// Yes i fuck namespaces in cpp !
using namespace std;

///////////////////////////////////////////////////////////////////////////////

Convert::Convert(uint64_t const& length):
    m_bitset(boost::dynamic_bitset<>(length*BIN_LENGTH)),
    m_current_pos(length*BIN_LENGTH -1)
{
    /* Why uint64_t here ?
     * Because we encode each character on 3 bits.
     * If the user exceeds ~ 700 million characters,
     * the number of bits exceeds 2 147 483 647;
     * which is the maximum value that can be stored in an int of 32 bits.
     * Beyond, the sign bit crashed and unpredictable behavior will occur.
     * When using the tables we will have a segfault "bad Alloc".
     * With a signed 32-bit int we allow the user to enter a sequence of
     * ~1.43 billion characters.
     * Which is not enough :p
     * With a 64-bit int we can store 1.84e19 bits and
     * 6.13e18 characters.
     * Which seems almost enough...
     */

    /*
     * I no longer use the map object because each searched item must be hashed
     * before being looked for in the container.
     * The maps are convenient but expensive in computing time.
     * I prefer to waste 81 bytes of memory with a vector containing only 4 helpful items.
     * The gain for 600 million bases is:
     * - encoding step:  from 9 to 6 seconds => 33%
     * - reversing step: from 18 to 11 => 39%
     *
     * PS: Using vector or array is the same deal here !
     */
    m_mappingADNData[A_ASCII] = A_BIN;
    m_mappingADNData[T_ASCII] = T_BIN;
    m_mappingADNData[G_ASCII] = G_BIN;
    m_mappingADNData[C_ASCII] = C_BIN;
}

void Convert::operator ()(uint8_t const& base)
{
    /* Bitwise operations are very fast.
     * Not because they are low level,
     * but because at a hardware level it is optimized for bitwise operations.
     * Other operations have to be ultimately converted into them.
     *
     * ATGC: 001 100 011 110
     * The numbers are stored in little endian but are read in big endian.
     * Therefore, on indices 0,1,2 we have the letter C
     * in 3,4,5 we will have the G etc.
     *
     * If we write the list in one direction: 0 => end,
     * we must read in the opposite direction to do the reverse: end => 0.
     * Or the inverse (write: end => 0; read: 0 => end);
     * Which is what i'm doing here.
     *
     * So, here an example for the first letter of ATGC:
     * on the bit 3x4-1=11 we have to put 0
     * on the bit 10       we have to put 0
     * on the bit 9        we have to put 1
     * Then we move to the letter T, etc.*
     * on the bit 2: 1
     * on the bit 1: 1
     * on the bit 0: 0
     *
     * If the program displays the result, it will give us big endian with the indices:
     * indices   11  10  9   8   7   6   5   4   3   2   1   0
     * values    0   0   1                           1   1   0
     *
     * In fact in memory (little endian):
     * Ob110......001
     *
     * 
     * To find the complementary chain and do the reverse at the same time,
     * we just read the string in the opposite direction of writing.
     *
     * If we traverse the table in memory, box by box, starting from the beginning:
     * 011 110 001 100
     * that to say:
     * GCAT
     *
     *
     * Write a letter in memory:
     * We must recover the bits 1 by 1; The more shifted left to the far right.
     * So for A: 0b001
     * 
     * least significant bit: ((0b001 >> 2) & 1) = 0 to put in box 11
     * middle bit :           ((0b001 >> 1) & 1) = 0               10
     * most significant bit:  ((0b001 >> 0) & 1) = 1               9
     *
     * Read a letter in memory in reverse way:
     * Letter A:
     * 000 + box 9
     * 001 << 1
     * 010 + box 10
     * 010 << 1
     * 100 + box 11
     * 100
     * Which is the code corresponding to the letter T.
     */

    uint8_t value = m_mappingADNData[base];

    // Debug:
    /*
    cout << base << "; " << bitset<3>(value) << endl;
    cout << "bits 0,1,2: "
         << ((value >> 0) & 1)
         << ((value >> 1) & 1)
         << ((value >> 2) & 1)
         << endl;
    */

    m_bitset[m_current_pos]   = ((value >> 2) & 1);
    m_bitset[m_current_pos-1] = ((value >> 1) & 1);
    m_bitset[m_current_pos-2] = ((value >> 0) & 1);

    m_current_pos -= BIN_LENGTH;
}

boost::dynamic_bitset<>* Convert::getBitset()
{
    return &m_bitset;
}

///////////////////////////////////////////////////////////////////////////////

rev_comp_bin::rev_comp_bin()
{
    // Initialises the mapping between ASCII & Binary data
    m_mappingDataADN[A_BIN] = A_ASCII;
    m_mappingDataADN[T_BIN] = T_ASCII;
    m_mappingDataADN[G_BIN] = G_ASCII;
    m_mappingDataADN[C_BIN] = C_ASCII;
}

string rev_comp_bin::run(string sequence)
{
    // Time measurement: beginning
    //int seconds1 = time(NULL);

    // Convert string to bitset with encoding given by m_mappingADNData
    Convert f_convert(sequence.length());
    f_convert = for_each(sequence.begin(), sequence.end(), f_convert);
    // Pointer is more convenient and don't use more memory
    boost::dynamic_bitset<>* encodedSequence = f_convert.getBitset();
    // Displays infos about the encoded sequence:
    //cout << "Number of bits:    " << encodedSequence->size() << endl;
    //cout << "Encoded sequence:  " << *encodedSequence        << endl;

    // Time measurement: encoding done
    //int seconds3 = time(NULL);

    for (uint64_t i = 0; i < encodedSequence->size(); i += BIN_LENGTH) {

        // Priority of the [] operator on * so we put () around the pointer
        uint8_t  value = (*encodedSequence)[i];
        value =  value << 1;
        value += (*encodedSequence)[i+1];
        value =  value << 1;
        value += (*encodedSequence)[i+2];

        // Displays the letter currently processed
        /*
        cout << "bitset: "   << bitset<3>(value)
             << "; val: "    << (int)value
             << endl;
        */

        sequence[i / BIN_LENGTH] = m_mappingDataADN[value];
    }

    // Time measurement: reversing done
    //int seconds4 = time(NULL);

    // Displays the new reversed & complementary sequence
    //cout << sequence << endl;

    /*cout << "Encoding done in:   " << seconds3 - seconds1 << endl
         << "Reversing done in:  " << seconds4 - seconds3 << endl;
     */

    // Returns reversed sequence
    return sequence;
}



