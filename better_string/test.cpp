#ifndef __PROGTEST__

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <memory>
#include <stdexcept>

#endif /* __PROGTEST__ */

using namespace std;

class CPointer{
public:
    shared_ptr<char> first_letter;
    uint startPos = 0;
    uint textLength = 0;
    uint chars_before = 0;

    CPointer() = default;

    explicit CPointer(const char* input)
    {

        textLength  = strlen(input);

        char* str = new char[textLength + 1];
        strcpy(str, input);

        // weird, found online
        first_letter =  shared_ptr<char>(str, [](char* ptr) {
            delete[] ptr;
        });
    }

};


class CArray
{
public:
    size_t arr_count = 0;   //number of actuall elements
    size_t arr_capacity = 0;    //space allocated
    CPointer *arr_data = nullptr;  //first element

    CArray() = default;

    ~CArray() {
        delete[] arr_data;
    }

    CArray(const CArray& other) {
        arr_count = other.arr_count;
        arr_capacity = other.arr_capacity;
        arr_data = new CPointer[arr_capacity];
        for (size_t i = 0; i < arr_count; ++i) {
            arr_data[i] = other.arr_data[i];
        }
    }

    CArray& operator=(const CArray& other) {
        if (this != &other) {
            delete[] arr_data;

            arr_count = other.arr_count;
            arr_capacity = other.arr_capacity;
            arr_data = new CPointer[arr_capacity];
            for (size_t i = 0; i < arr_count; ++i) {
                arr_data[i] = other.arr_data[i];
            }
        }
        return *this;
    }

    size_t size() const
    {
        return arr_count;
    }

    CPointer& at(const size_t idx) const
    {
        return arr_data[idx];
    }

    void erase(const size_t idx) {
        if (idx >= arr_count) {
            return;
        }

        for (size_t i = idx; i < arr_count - 1; ++i) {
            arr_data[i] = arr_data[i + 1];
        }

        arr_count--;
    }

    void insert(size_t idx, const CPointer &value) {
        if (idx > arr_count) {
            throw std::out_of_range("Index out of range");
        }

        if (arr_count == arr_capacity) {
            size_t new_capacity = arr_capacity == 0 ? 1 : arr_capacity * 2;
            reserve(new_capacity);
        }

        for (size_t i = arr_count; i > idx; --i) {
            arr_data[i] = arr_data[i - 1];
        }

        arr_data[idx] = value;
        arr_count++;
    }

    void push_back(CPointer &value)
    {
        if(arr_count == arr_capacity)
        {
            reserve(arr_capacity == 0 ? 1 : arr_capacity * 2);
        }
        arr_data[arr_count] = value;
        arr_count++;
    }

    void reserve(const size_t new_capacity) {
        CPointer *new_data = new CPointer[new_capacity];

        for (size_t i = 0; i < arr_count; ++i)
        {
            new_data[i] = arr_data[i];
        }
        delete[] arr_data;
        arr_data = new_data;
        arr_capacity = new_capacity;
    }

    size_t lower_bound(uint chars_before) const
    {
        size_t left = 0;
        size_t right = arr_count;

        while (left < right)
        {
            size_t mid = left + (right - left) / 2;
            if (arr_data[mid].chars_before < chars_before)
                left = mid + 1;
            else
                right = mid;

        }

        if(left < size() && arr_data[left].chars_before < chars_before)
            left++;

        return left;
    }

};

class CPatchStr {
public:
    CPatchStr() = default;

    CPatchStr(const char *str)
    {
        auto tmp = CPointer(str);
        m_allBlocks.push_back(tmp);
        length = strlen(str);
    }

    void outOfBounds(uint from, uint len) const
    {
        if((from + len) > length)
            throw out_of_range("index out of range");
    }

    // seems that they aren't useful
    // TODO copy constructor
    // TODO destructor
    // TODO operator =

    CPointer start(size_t from, uint& startPosition, bool &last) const
    {
        startPosition = m_allBlocks.lower_bound(from);
        CPointer res;

        if(from == length)
        {
            startPosition--;
            last = true;
        }
        else
        {
            if(m_allBlocks.size() == startPosition || m_allBlocks.at(startPosition).chars_before != from)
                startPosition--;
            last = false;
            res = m_allBlocks.at(startPosition);
            // difference from - all letters behind
            res.startPos = m_allBlocks.at(startPosition).startPos + (from - m_allBlocks.at(startPosition).chars_before);
            // text length of previous one minus difference between starting positions
            res.textLength = res.textLength - (res.startPos - m_allBlocks.at(startPosition).startPos);

        }

        return res;
    }

    // this will either be removed or substringed or sth
    CPointer startOld(size_t from, uint& startPosition, bool &last) const
    {
        uint where_at = 0;
        CPointer res;

        last = true;
        for(; startPosition < m_allBlocks.arr_count; startPosition++)
        {

            if(where_at + m_allBlocks.at(startPosition).textLength > from)
            {
                res = m_allBlocks.at(startPosition);
                // difference from - all letters behind
                res.startPos = m_allBlocks.at(startPosition).startPos + (from - where_at);
                // text length of previous one minus difference between starting positions
                res.textLength = res.textLength - (res.startPos - m_allBlocks.at(startPosition).startPos);
                last = false;
                break;
            }
            else
            {
                where_at += m_allBlocks.at(startPosition).textLength;
            }
        }

        // weird debugging
        if(last)
            startPosition--;

        return res;
    }

    void updateCharsBefore() const
    {
        m_allBlocks.at(0).chars_before = 0;
        for(uint i = 1; i < m_allBlocks.size(); i++)
        {
            m_allBlocks.at(i).chars_before = m_allBlocks.at(i - 1).chars_before + m_allBlocks.at(i - 1).textLength;
        }
    }

    void restOfInterval(size_t len, uint& startPosition, CPatchStr& newOne, uint currentTextLength) const
    {

        // only one block
        if(len < newOne.m_allBlocks.at(0).textLength)
        {
            newOne.m_allBlocks.at(0).textLength = len;
            return;
        }

        startPosition++;
        currentTextLength += newOne.m_allBlocks.at(0).textLength;

        // longer than only one block
        for(; startPosition < m_allBlocks.arr_count; startPosition++)
        {
            if(currentTextLength + m_allBlocks.at(startPosition).textLength >= len)
            {

                newOne.m_allBlocks.push_back(m_allBlocks.at(startPosition));
                newOne.m_allBlocks.at(newOne.m_allBlocks.size() - 1).textLength = len - currentTextLength;
                break;
            }
            else
            {
                newOne.m_allBlocks.push_back(m_allBlocks.at(startPosition));
                currentTextLength += m_allBlocks.at(startPosition).textLength;
            }
        }
    }

    CPatchStr subStr(size_t from, size_t len) const
    {
        outOfBounds(from, len);

        CPatchStr newOne;
        uint startPosition = 0;

        bool last = false;
        // make start
        CPointer tmp = start(from, startPosition, last);
        newOne.m_allBlocks.push_back(tmp);

        // add all in between
        // need to address problem when substr is from one block  startPosition++;
        restOfInterval(len, startPosition, newOne, 0);
        newOne.length = len;

        return newOne;
    }

    uint countLen() const
    {
        uint len = 0;
        for(uint i = 0; i < m_allBlocks.size(); i++)
            len += m_allBlocks.at(i).textLength;

        return len;
    }

    CPatchStr &append(const CPatchStr &srcOld)
    {
        if(srcOld.length == 0)
            return *this;

        CPatchStr src = srcOld;

        for(uint i = 0; i < src.m_allBlocks.size(); i++)
        {
            m_allBlocks.push_back(src.m_allBlocks.at(i));
        }
        length += src.countLen();
        updateCharsBefore();
        return *this;
    }

    CPatchStr &insert(size_t pos, const CPatchStr &srcOld)
    {
        outOfBounds(pos, 0);
        if(srcOld.length == 0)
            return *this;

        CPatchStr src = srcOld;
        uint startPos = 0;
        bool last = false;

        auto secondHalf = start(pos, startPos, last);

//        uint startPosOld = 0;
//        bool lastOld = false;
//        auto secondHalf_old = startOld(pos, startPosOld, lastOld);

        if(!last)
        {
            m_allBlocks.at(startPos).textLength = secondHalf.startPos - m_allBlocks.at(startPos).startPos;
            if(m_allBlocks.at(startPos).textLength == 0) {
                m_allBlocks.erase(startPos);
                startPos--;
            }
            m_allBlocks.insert(startPos + 1, secondHalf);
        }

        for(uint i = 0; i < src.m_allBlocks.size(); i++)
        {
            m_allBlocks.insert(startPos + 1 + i, src.m_allBlocks.at(i));
        }
        length += src.countLen();
        updateCharsBefore();
        return *this;
    }

    CPatchStr &remove(size_t from, size_t len)
    {
        outOfBounds(from, len);

        uint startPos = 0;
        uint currentlyRemoved = 0;

        bool last = false;
        CPointer secondHalf = start(from, startPos, last);

//        uint startPosOld = 0;
//        bool lastOld = false;
//        auto secondHalf_old = startOld(from, startPosOld, lastOld);

        m_allBlocks.at(startPos).textLength = secondHalf.startPos - m_allBlocks.at(startPos).startPos;
        // erasing withing the same block
        if(len <= secondHalf.textLength)
        {
            secondHalf.startPos += len;
            secondHalf.textLength -= len;
            if(secondHalf.textLength != 0)
                m_allBlocks.insert(startPos + 1, secondHalf);
            length -= len;
            if(m_allBlocks.at(startPos).textLength == 0)
            {
                m_allBlocks.erase(startPos);
                startPos--;
            }

            updateCharsBefore();
            return *this;
        }

        // remove useless block
        if(m_allBlocks.at(startPos).textLength == 0)
            m_allBlocks.erase(startPos);
        else
            startPos++;

        currentlyRemoved += secondHalf.textLength;

        for(; startPos < m_allBlocks.size(); startPos++)
        {
            if(currentlyRemoved + m_allBlocks.at(startPos).textLength >= len)
            {
//                auto tmp = m_allBlocks.at(startPos).startPos;

                // increase start position by the rest which is filling this interval
//                m_allBlocks.at(startPos).startPos += (currentlyRemoved + m_allBlocks.at(startPos).textLength - len);
                m_allBlocks.at(startPos).startPos += len - currentlyRemoved;

                // decrease text length by the difference of starting positions
                //m_allBlocks.at(startPos).textLength -= (m_allBlocks.at(startPos).startPos - tmp);
                m_allBlocks.at(startPos).textLength -= (len - currentlyRemoved);
//
//                if(tmp == m_allBlocks.at(startPos).startPos)
//                    m_allBlocks.at(startPos).textLength = 0;

                if(m_allBlocks.at(startPos).textLength == 0)
                    m_allBlocks.erase(startPos);
                length -= len;
                updateCharsBefore();
                return *this;
            }
            currentlyRemoved += m_allBlocks.at(startPos).textLength;
            m_allBlocks.erase(startPos);
            startPos--;
        }

        length -= len;
        updateCharsBefore();
        return *this;
    }

    char *toStr() const
    {
        char * res = new char [length + 1];
        char* currentPos = res;

        for (uint i = 0; i < m_allBlocks.size(); i++) {
            strncpy(currentPos, m_allBlocks.at(i).first_letter.get() + m_allBlocks.at(i).startPos, m_allBlocks.at(i).textLength);
            currentPos += m_allBlocks.at(i).textLength; // Move currentPos to the end of the copied substring
        }

        res[length] = '\0';

        return res;
    }


    CArray m_allBlocks;
private:
    uint length = 0;
};

#ifndef __PROGTEST__

bool stringMatch(char *str,
                 const char *expected) {
    bool res = std::strcmp(str, expected) == 0;
    delete[] str;
    return res;
}

void test()
{
    char tmpStr[100];

    CPatchStr a("test");
    assert (stringMatch(a.toStr(), "test"));
    std::strncpy(tmpStr, " da", sizeof(tmpStr) - 1);
    a.append(tmpStr);
    assert (stringMatch(a.toStr(), "test da"));
    std::strncpy(tmpStr, "ta", sizeof(tmpStr) - 1);
    a.append(tmpStr);

    assert (stringMatch(a.toStr(), "test data"));
    std::strncpy(tmpStr, "foo text", sizeof(tmpStr) - 1);
    CPatchStr b(tmpStr);
    assert (stringMatch(b.toStr(), "foo text"));
    CPatchStr c(a);
    assert (stringMatch(c.toStr(), "test data"));
    CPatchStr d(a.subStr(3, 5));
    assert (stringMatch(d.toStr(), "t dat"));
    d.append(b);
    assert (stringMatch(d.toStr(), "t datfoo text"));
    d.append(b.subStr(3, 4));
    assert (stringMatch(d.toStr(), "t datfoo text tex"));
    c.append(d);
    assert (stringMatch(c.toStr(), "test datat datfoo text tex"));
    c.append(c);
    assert (stringMatch(c.toStr(), "test datat datfoo text textest datat datfoo text tex"));
    d.insert(2, c.subStr(6, 9));
    assert (stringMatch(d.toStr(), "t atat datfdatfoo text tex"));

    b = "abcdefgh";
    assert (stringMatch(b.toStr(), "abcdefgh"));
    assert (stringMatch(d.toStr(), "t atat datfdatfoo text tex"));
    assert (stringMatch(d.subStr(4, 8).toStr(), "at datfd"));
    assert (stringMatch(b.subStr(2, 6).toStr(), "cdefgh"));
    try {
        b.subStr(2, 7).toStr();
        assert ("Exception not thrown" == nullptr);
    }
    catch (const std::out_of_range &e) {
    }
    catch (...) {
        assert ("Invalid exception thrown" == nullptr);
    }

    uint geh = 0;
    bool trws = false;
    for(uint i = 0; i < b.countLen(); i++)
    {
        for(uint j = 0; j + i < b.countLen(); i++)
        {
            b.start(i, geh, trws);
        }
    }

    /*
    cout << a.toStr() << endl;
    a.remove(3, 5);
    cout << a.toStr() << endl;
    assert (stringMatch(a.toStr(), "tesa"));
    cout << "default tests pass" << endl;
*/
}

void mineTest()
{
    CPatchStr a("test");

    CPatchStr b("taky");
    CPatchStr c("nah");
    assert(stringMatch(a.toStr(),"test"));
    assert(stringMatch(b.toStr(),"taky"));
    a.append(b);
    assert(stringMatch(a.toStr(),"testtaky"));

    cout << "testing insert" << endl;
    a.insert(4, c);
    assert(stringMatch(a.toStr(),"testnahtaky"));
    a.insert(1, c);
    assert(stringMatch(a.toStr(),"tnahestnahtaky"));
    a.insert(0, c);
    assert(stringMatch(a.toStr(),"nahtnahestnahtaky"));
    b.insert(4, c);
    assert(stringMatch(b.toStr(),"takynah"));

    cout << "testing remove" << endl;
    CPatchStr d("abcdefgh");
    d.remove(0, 2);
    assert(stringMatch(d.toStr(),"cdefgh"));
    d.remove(1, 2);
    assert(stringMatch(d.toStr(),"cfgh"));
    cout << "here" << endl;
    d.remove(3, 1);
    cout << "here" << endl;
    assert(stringMatch(d.toStr(),"cfg"));

    CPatchStr e("tesk");
    d.append(e);
    assert(stringMatch(d.toStr(), "cfgtesk"));
    d.remove(3, 3);
    assert(stringMatch(d.toStr(),"cfgk"));
    d.remove(1, 2);
    assert(stringMatch(d.toStr(),"ck"));
    d.remove(0,2);
    assert(stringMatch(d.toStr(),""));

    cout << "testing substr" << endl;
    CPatchStr f("substr");
    CPatchStr g("another");
    CPatchStr h("bites");
    f.append(g);
    CPatchStr i = f.subStr(4, 4);

//    cout << f.toStr() << endl;
//    cout << i.toStr() << endl;
    assert(stringMatch(i.toStr(),"tran"));


    cout << "weird edge-cases" << endl;
    CPatchStr j("first");
    CPatchStr k("second");
    j.append(j);

    bool trws = true;
    uint geh = 0;


    for(uint i = 0; i < a.countLen(); i++)
    {
        for(uint j = 0; j + i < a.countLen(); i++)
        {
            a.start(i, geh, trws);
        }
    }

    j.append("");

   // auto tmp = j.subStr(j.countLen()-1, 0);
    //j.remove(j.countLen()+1,0);



    cout << "mine tests pass" << endl;
}

void testingLowerBound()
{

}

int main() {
    testingLowerBound();
    mineTest();
    test();
    cout << "WE DID IT BOYS" << endl;
    return EXIT_SUCCESS;
}

#endif /* __PROGTEST__ */
