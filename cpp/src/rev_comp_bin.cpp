#include "rev_comp_bin.hpp"

// Yes i fuck namespaces in cpp !
using namespace std;

///////////////////////////////////////////////////////////////////////////////

LifeGenerator::LifeGenerator():
    m_basesCharset("ATGC")
{
    // Initialises the seed
    srand(time(NULL));
}

char LifeGenerator::operator ()()
{
    // Returns pseudo random char taken in the charset
    return m_basesCharset[ rand() % (/*m_basesTab.length()*/4) ];
}

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
     * Les nombres sont stockés en little endian mais sont lus en big endian.
     * Par conséquent, en cases 0,1,2 on aura la lettre C
     * en 3,4,5 on aura la lettre G etc.
     *
     * Si on écrit la liste dans un sens 0 => fin,
     * il faudra la lire dans le sens opposé pour faire le reverse: fin => 0.
     * Ou l'inverse (écriture: fin => 0; lecture: 0 => fin);
     * ce que je fais ici.
     *
     * Donc ex pour la première lettre de ATGC:
     * sur le bit 3x4-1=11 on doit mettre 0
     * sur le bit 10       on doit mettre 0
     * sur le bit 9        on doit mettre 1
     * Puis on passe à la lettre T, etc.
     * sur le bit 2: 1
     * sur le bit 1: 1
     * sur le bit 0: 0
     *
     *
     * Si on affiche le résultat le programme nous donne en big endian avec les indices:
     * indices   11  10  9   8   7   6   5   4   3   2   1   0
     * valeurs   0   0   1                           1   1   0
     *
     * En réalité en mémoire (little endian):
     * Ob110......001
     *
     * Pour trouver la chaine complémentaire et faire le reverse en même temps,
     * il suffit de lire la chaine dans le sens inverse de l'écriture.
     *
     * Si on parcourt le tableau en mémoire, case par case en partant du début:
     * 011 110 001 100
     * soit:
     * GCAT
     *
     *
     * Ecrire une lettre en mémoire:
     * On doit récupérer les bits 1 à 1; Du plus décalé à gauche vers le plus à droite.
     * Donc pour A: 0b001
     * bit de poids faible: ((0b001 >> 2) & 1) = 0 à mettre en case 11
     * bit central:         ((0b001 >> 1) & 1) = 0                  10
     * bit de poids fort:   ((0b001 >> 0) & 1) = 1                  9
     *
     * Lire une lettre en mémoire à l'envers:
     * Lettre A:
     * 000 + case 9
     * 001 << 1
     * 010 + case 10
     * 010 << 1
     * 100 + case 11
     * 100
     * code correspondant à la lettre T.
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
    // Debug:
    cout << sequence << endl;

    // Time measurement: beginning
    int seconds1 = time(NULL);

    // Convert string to bitset with encoding given by m_mappingADNData
    Convert f_convert(sequence.length());
    f_convert = for_each(sequence.begin(), sequence.end(), f_convert);
    // Pointer is more convenient and don't use more memory
    boost::dynamic_bitset<>* encodedSequence = f_convert.getBitset();
    // Debug:
    //cout << "Number of bits:    " << encodedSequence->size() << endl;
    //cout << "Encoded sequence:  " << *encodedSequence        << endl;

    // Time measurement: encoding done
    int seconds3 = time(NULL);

    for (uint64_t i = 0; i < encodedSequence->size(); i += BIN_LENGTH) {

        // Priority of the [] operator on * so we put () around the pointer
        uint8_t  value = (*encodedSequence)[i];
        value =  value << 1;
        value += (*encodedSequence)[i+1];
        value =  value << 1;
        value += (*encodedSequence)[i+2];

        // Debug:
        /*
        cout << "bitset: "   << bitset<3>(value)
             << "; val: "    << (int)value
             << "; bool ?: " << (value == 3)
             << endl;
        */

        sequence[i / BIN_LENGTH] = m_mappingDataADN[value];
    }

    // Time measurement: reversing done
    int seconds4 = time(NULL);

    // Displays the new reversed & complementary sequence
    cout << sequence << endl;

    cout << "Encoding done:   " << seconds3 - seconds1 << endl
         << "Reversing done:  " << seconds4 - seconds3 << endl;

    // Returns reversed sequence
    return sequence;
}



