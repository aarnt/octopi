/*
Code extracted from project: https://github.com/bricke/Qt-AES

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

/*
 * AES encryption algorithm class
 * Code extracted from project: https://github.com/bricke/Qt-AES
 */

#include "qaesencryption.h"

/*
 * Static Functions
 * */
QByteArray QAESEncryption::Crypt(QAESEncryption::Aes level, QAESEncryption::Mode mode, const QByteArray &rawText,
                                 const QByteArray &key, const QByteArray &iv, QAESEncryption::Padding padding)
{
    return QAESEncryption(level, mode, padding).encode(rawText, key, iv);
}

QByteArray QAESEncryption::Decrypt(QAESEncryption::Aes level, QAESEncryption::Mode mode, const QByteArray &rawText,
                                   const QByteArray &key, const QByteArray &iv, QAESEncryption::Padding padding)
{
     return QAESEncryption(level, mode, padding).decode(rawText, key, iv);
}

QByteArray QAESEncryption::ExpandKey(QAESEncryption::Aes level, QAESEncryption::Mode mode, const QByteArray &key)
{
     return QAESEncryption(level, mode).expandKey(key);
}

QByteArray QAESEncryption::RemovePadding(const QByteArray &rawText, QAESEncryption::Padding padding)
{
    if (rawText.isEmpty())
        return rawText;

    QByteArray ret(rawText);
    switch (padding)
    {
    case Padding::ZERO:
        //Works only if the last byte of the decoded array is not zero
        while (ret.at(ret.length()-1) == 0x00)
            ret.remove(ret.length()-1, 1);
        break;
    case Padding::PKCS7:
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        ret.remove(ret.length() - ret.back(), ret.back());
#else
        ret.remove(ret.length() - ret.at(ret.length() - 1), ret.at(ret.length() - 1));
#endif
        break;
    case Padding::ISO:
    {
        // Find the last byte which is not zero
        int marker_index = ret.length() - 1;
        for (; marker_index >= 0; --marker_index)
        {
            if (ret.at(marker_index) != 0x00)
            {
                break;
            }
        }

        // And check if it's the byte for marking padding
        if (ret.at(marker_index) == static_cast<char>(0x80))
        {
            ret.truncate(marker_index);
        }
        break;
    }
    default:
        //do nothing
        break;
    }
    return ret;
}
/*
 * End Static function declarations
 * */

/*
 * Inline Functions
 * */

inline quint8 xTime(quint8 x){
  return ((x<<1) ^ (((x>>7) & 1) * 0x1b));
}

inline quint8 multiply(quint8 x, quint8 y){
  return (((y & 1) * x) ^ ((y>>1 & 1) * xTime(x)) ^ ((y>>2 & 1) * xTime(xTime(x))) ^ ((y>>3 & 1)
            * xTime(xTime(xTime(x)))) ^ ((y>>4 & 1) * xTime(xTime(xTime(xTime(x))))));
}

/*
 * End Inline functions
 * */


QAESEncryption::QAESEncryption(Aes level, Mode mode,
                               Padding padding)
    : m_nb(4), m_blocklen(16), m_level(level), m_mode(mode), m_padding(padding)
{
    m_state = nullptr;

    switch (level)
    {
    case AES_128: {
        AES128 aes;
        m_nk = aes.nk;
        m_keyLen = aes.keylen;
        m_nr = aes.nr;
        m_expandedKey = aes.expandedKey;
        }
        break;
    case AES_192: {
        AES192 aes;
        m_nk = aes.nk;
        m_keyLen = aes.keylen;
        m_nr = aes.nr;
        m_expandedKey = aes.expandedKey;
        }
        break;
    case AES_256: {
        AES256 aes;
        m_nk = aes.nk;
        m_keyLen = aes.keylen;
        m_nr = aes.nr;
        m_expandedKey = aes.expandedKey;
        }
        break;
    default: {
        AES128 aes;
        m_nk = aes.nk;
        m_keyLen = aes.keylen;
        m_nr = aes.nr;
        m_expandedKey = aes.expandedKey;
        }
        break;
    }

}
QByteArray QAESEncryption::getPadding(int currSize, int alignment)
{
    int size = (alignment - currSize % alignment) % alignment;
    switch(m_padding)
    {
    case Padding::ZERO:
        return QByteArray(size, 0x00);
        break;
    case Padding::PKCS7:
        if (size == 0)
            size = alignment;
        return QByteArray(size, size);
        break;
    case Padding::ISO:
        if (size > 0)
            return QByteArray (size - 1, 0x00).prepend(0x80);
        break;
    default:
        return QByteArray(size, 0x00);
        break;
    }
    return QByteArray();
}

QByteArray QAESEncryption::expandKey(const QByteArray &key)
{
  int i, k;
  quint8 tempa[4]; // Used for the column/row operations
  QByteArray roundKey(key);

  // The first round key is the key itself.
  // ...

  // All other round keys are found from the previous round keys.
  //i == Nk
  for(i = m_nk; i < m_nb * (m_nr + 1); i++)
  {
    tempa[0] = (quint8) roundKey.at((i-1) * 4 + 0);
    tempa[1] = (quint8) roundKey.at((i-1) * 4 + 1);
    tempa[2] = (quint8) roundKey.at((i-1) * 4 + 2);
    tempa[3] = (quint8) roundKey.at((i-1) * 4 + 3);

    if (i % m_nk == 0)
    {
        // This function shifts the 4 bytes in a word to the left once.
        // [a0,a1,a2,a3] becomes [a1,a2,a3,a0]

        // Function RotWord()
        k = tempa[0];
        tempa[0] = tempa[1];
        tempa[1] = tempa[2];
        tempa[2] = tempa[3];
        tempa[3] = k;

        // Function Subword()
        tempa[0] = getSBoxValue(tempa[0]);
        tempa[1] = getSBoxValue(tempa[1]);
        tempa[2] = getSBoxValue(tempa[2]);
        tempa[3] = getSBoxValue(tempa[3]);

        tempa[0] =  tempa[0] ^ Rcon[i/m_nk];
    }
    if (m_level == AES_256 && i % m_nk == 4)
    {
        // Function Subword()
        tempa[0] = getSBoxValue(tempa[0]);
        tempa[1] = getSBoxValue(tempa[1]);
        tempa[2] = getSBoxValue(tempa[2]);
        tempa[3] = getSBoxValue(tempa[3]);
    }
    roundKey.insert(i * 4 + 0, (quint8) roundKey.at((i - m_nk) * 4 + 0) ^ tempa[0]);
    roundKey.insert(i * 4 + 1, (quint8) roundKey.at((i - m_nk) * 4 + 1) ^ tempa[1]);
    roundKey.insert(i * 4 + 2, (quint8) roundKey.at((i - m_nk) * 4 + 2) ^ tempa[2]);
    roundKey.insert(i * 4 + 3, (quint8) roundKey.at((i - m_nk) * 4 + 3) ^ tempa[3]);
  }
  return roundKey;
}

// This function adds the round key to state.
// The round key is added to the state by an XOR function.
void QAESEncryption::addRoundKey(const quint8 round, const QByteArray expKey)
{
  QByteArray::iterator it = m_state->begin();
  for(int i=0; i < 16; ++i)
      it[i] = (quint8) it[i] ^ (quint8) expKey.at(round * m_nb * 4 + (i/4) * m_nb + (i%4));
}

// The SubBytes Function Substitutes the values in the
// state matrix with values in an S-box.
void QAESEncryption::subBytes()
{
  QByteArray::iterator it = m_state->begin();
  for(int i = 0; i < 16; i++)
    it[i] = getSBoxValue((quint8) it[i]);
}

// The ShiftRows() function shifts the rows in the state to the left.
// Each row is shifted with different offset.
// Offset = Row number. So the first row is not shifted.
void QAESEncryption::shiftRows()
{
    QByteArray::iterator it = m_state->begin();
    quint8 temp;
    //Keep in mind that QByteArray is column-driven!!

     //Shift 1 to left
    temp   = (quint8)it[1];
    it[1]  = (quint8)it[5];
    it[5]  = (quint8)it[9];
    it[9]  = (quint8)it[13];
    it[13] = (quint8)temp;

    //Shift 2 to left
    temp   = (quint8)it[2];
    it[2]  = (quint8)it[10];
    it[10] = (quint8)temp;
    temp   = (quint8)it[6];
    it[6]  = (quint8)it[14];
    it[14] = (quint8)temp;

    //Shift 3 to left
    temp   = (quint8)it[3];
    it[3]  = (quint8)it[15];
    it[15] = (quint8)it[11];
    it[11] = (quint8)it[7];
    it[7]  = (quint8)temp;
}

// MixColumns function mixes the columns of the state matrix
//optimized!!
void QAESEncryption::mixColumns()
{
  QByteArray::iterator it = m_state->begin();
  quint8 tmp, tm, t;

  for(int i = 0; i < 16; i += 4){
    t       = (quint8)it[i];
    tmp     =  (quint8)it[i] ^ (quint8)it[i+1] ^ (quint8)it[i+2] ^ (quint8)it[i+3] ;

    tm      = xTime( (quint8)it[i] ^ (quint8)it[i+1] );
    it[i]   = (quint8)it[i] ^ (quint8)tm ^ (quint8)tmp;

    tm      = xTime( (quint8)it[i+1] ^ (quint8)it[i+2]);
    it[i+1] = (quint8)it[i+1] ^ (quint8)tm ^ (quint8)tmp;

    tm      = xTime( (quint8)it[i+2] ^ (quint8)it[i+3]);
    it[i+2] =(quint8)it[i+2] ^ (quint8)tm ^ (quint8)tmp;

    tm      = xTime((quint8)it[i+3] ^ (quint8)t);
    it[i+3] =(quint8)it[i+3] ^ (quint8)tm ^ (quint8)tmp;
  }
}

// MixColumns function mixes the columns of the state matrix.
// The method used to multiply may be difficult to understand for the inexperienced.
// Please use the references to gain more information.
void QAESEncryption::invMixColumns()
{
  QByteArray::iterator it = m_state->begin();
  quint8 a,b,c,d;
  for(int i = 0; i < 16; i+=4){
    a = (quint8) it[i];
    b = (quint8) it[i+1];
    c = (quint8) it[i+2];
    d = (quint8) it[i+3];

    it[i]   = (quint8) (multiply(a, 0x0e) ^ multiply(b, 0x0b) ^ multiply(c, 0x0d) ^ multiply(d, 0x09));
    it[i+1] = (quint8) (multiply(a, 0x09) ^ multiply(b, 0x0e) ^ multiply(c, 0x0b) ^ multiply(d, 0x0d));
    it[i+2] = (quint8) (multiply(a, 0x0d) ^ multiply(b, 0x09) ^ multiply(c, 0x0e) ^ multiply(d, 0x0b));
    it[i+3] = (quint8) (multiply(a, 0x0b) ^ multiply(b, 0x0d) ^ multiply(c, 0x09) ^ multiply(d, 0x0e));
  }
}

// The SubBytes Function Substitutes the values in the
// state matrix with values in an S-box.
void QAESEncryption::invSubBytes()
{
    QByteArray::iterator it = m_state->begin();
    for(int i = 0; i < 16; ++i)
        it[i] = getSBoxInvert((quint8) it[i]);
}

void QAESEncryption::invShiftRows()
{
    QByteArray::iterator it = m_state->begin();
    uint8_t temp;

    //Keep in mind that QByteArray is column-driven!!

    //Shift 1 to right
    temp   = (quint8)it[13];
    it[13] = (quint8)it[9];
    it[9]  = (quint8)it[5];
    it[5]  = (quint8)it[1];
    it[1]  = (quint8)temp;

    //Shift 2
    temp   = (quint8)it[10];
    it[10] = (quint8)it[2];
    it[2]  = (quint8)temp;
    temp   = (quint8)it[14];
    it[14] = (quint8)it[6];
    it[6]  = (quint8)temp;

    //Shift 3
    temp   = (quint8)it[15];
    it[15] = (quint8)it[3];
    it[3]  = (quint8)it[7];
    it[7]  = (quint8)it[11];
    it[11] = (quint8)temp;
}

QByteArray QAESEncryption::byteXor(const QByteArray &a, const QByteArray &b)
{
  QByteArray::const_iterator it_a = a.begin();
  QByteArray::const_iterator it_b = b.begin();
  QByteArray ret;

  //for(int i = 0; i < m_blocklen; i++)
  for(int i = 0; i < std::min(a.size(), b.size()); i++)
      ret.insert(i,it_a[i] ^ it_b[i]);

  return ret;
}

// Cipher is the main function that encrypts the PlainText.
QByteArray QAESEncryption::cipher(const QByteArray &expKey, const QByteArray &in)
{

  //m_state is the input buffer...
  QByteArray output(in);
  m_state = &output;

  // Add the First round key to the state before starting the rounds.
  addRoundKey(0, expKey);

  // There will be Nr rounds.
  // The first Nr-1 rounds are identical.
  // These Nr-1 rounds are executed in the loop below.
  for(quint8 round = 1; round < m_nr; ++round){
    subBytes();
    shiftRows();
    mixColumns();
    addRoundKey(round, expKey);
  }

  // The last round is given below.
  // The MixColumns function is not here in the last round.
  subBytes();
  shiftRows();
  addRoundKey(m_nr, expKey);

  return output;
}

QByteArray QAESEncryption::invCipher(const QByteArray &expKey, const QByteArray &in)
{
    //m_state is the input buffer.... handle it!
    QByteArray output(in);
    m_state = &output;

    // Add the First round key to the state before starting the rounds.
    addRoundKey(m_nr, expKey);

    // There will be Nr rounds.
    // The first Nr-1 rounds are identical.
    // These Nr-1 rounds are executed in the loop below.
    for(quint8 round=m_nr-1; round>0 ; round--){
        invShiftRows();
        invSubBytes();
        addRoundKey(round, expKey);
        invMixColumns();
    }

    // The last round is given below.
    // The MixColumns function is not here in the last round.
    invShiftRows();
    invSubBytes();
    addRoundKey(0, expKey);

    return output;
}

QByteArray QAESEncryption::encode(const QByteArray &rawText, const QByteArray &key, const QByteArray &iv)
{
    if (m_mode >= CBC && (iv.isNull() || iv.size() != m_blocklen))
       return QByteArray();

    QByteArray ret;
    QByteArray expandedKey = expandKey(key);
    QByteArray alignedText(rawText);

    //Fill array with padding
    alignedText.append(getPadding(rawText.size(), m_blocklen));

    switch(m_mode)
    {
    case ECB:
        for(int i=0; i < alignedText.size(); i+= m_blocklen)
            ret.append(cipher(expandedKey, alignedText.mid(i, m_blocklen)));
        break;
    case CBC: {
            QByteArray ivTemp(iv);
            for(int i=0; i < alignedText.size(); i+= m_blocklen) {
                alignedText.replace(i, m_blocklen, byteXor(alignedText.mid(i, m_blocklen),ivTemp));
                ret.append(cipher(expandedKey, alignedText.mid(i, m_blocklen)));
                ivTemp = ret.mid(i, m_blocklen);
            }
        }
        break;
    case CFB: {
            ret.append(byteXor(alignedText.left(m_blocklen), cipher(expandedKey, iv)));
            for(int i=0; i < alignedText.size(); i+= m_blocklen) {
                if (i+m_blocklen < alignedText.size())
                    ret.append(byteXor(alignedText.mid(i+m_blocklen, m_blocklen),
                                       cipher(expandedKey, ret.mid(i, m_blocklen))));
            }
        }
        break;
    case OFB: {
            QByteArray ofbTemp;
            ofbTemp.append(cipher(expandedKey, iv));
            for (int i=m_blocklen; i < alignedText.size(); i += m_blocklen){
                ofbTemp.append(cipher(expandedKey, ofbTemp.right(m_blocklen)));
            }
            ret.append(byteXor(alignedText, ofbTemp));
        }
        break;
    default: break;
    }
    return ret;
}

QByteArray QAESEncryption::decode(const QByteArray &rawText, const QByteArray &key, const QByteArray &iv)
{
    if (m_mode >= CBC && (iv.isNull() || iv.size() != m_blocklen))
       return QByteArray();

    QByteArray ret;
    QByteArray expandedKey = expandKey(key);

    switch(m_mode)
    {
    case ECB:
        for(int i=0; i < rawText.size(); i+= m_blocklen)
            ret.append(invCipher(expandedKey, rawText.mid(i, m_blocklen)));
        break;
    case CBC: {
            QByteArray ivTemp(iv);
            for(int i=0; i < rawText.size(); i+= m_blocklen){
                ret.append(invCipher(expandedKey, rawText.mid(i, m_blocklen)));
                ret.replace(i, m_blocklen, byteXor(ret.mid(i, m_blocklen),ivTemp));
                ivTemp = rawText.mid(i, m_blocklen);
            }
        }
        break;
    case CFB: {
            ret.append(byteXor(rawText.mid(0, m_blocklen), cipher(expandedKey, iv)));
            for(int i=0; i < rawText.size(); i+= m_blocklen){
                if (i+m_blocklen < rawText.size()) {
                    ret.append(byteXor(rawText.mid(i+m_blocklen, m_blocklen),
                                       cipher(expandedKey, rawText.mid(i, m_blocklen))));
                }
            }
        }
        break;
    case OFB: {
        QByteArray ofbTemp;
        ofbTemp.append(cipher(expandedKey, iv));
        for (int i=m_blocklen; i < rawText.size(); i += m_blocklen){
            ofbTemp.append(cipher(expandedKey, ofbTemp.right(m_blocklen)));
        }
        ret.append(byteXor(rawText, ofbTemp));
    }
        break;
    default:
        //do nothing
        break;
    }
    return ret;
}

QByteArray QAESEncryption::removePadding(const QByteArray &rawText)
{
    return RemovePadding(rawText, (Padding) m_padding);
}
