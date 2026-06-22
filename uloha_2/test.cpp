#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <functional>
#include <compare>
#include <stdexcept>
#endif /* __PROGTEST__ */
using namespace std;

std::string removeZeroPadding(std::string& str)
{
    size_t pos = str.find_first_not_of('0');

    if (pos == std::string::npos) {
        return "0";
    }

    if(str.empty())
        str.push_back('0');
    return str.substr(pos);
}

class CBigInt
{
public:
    // default constructor
    CBigInt() = default;

    // probably not gonna use it
    // copying/assignment/destruction
//    CBigInt(CBigInt const &input) : m_number(input.m_number){}

    // int constructor
    CBigInt(int input)
    {
        std::stringstream ss;
        ss << input;
        m_number = ss.str();
        m_number = removeMark(m_number);
    }

    bool checkValidString(std::string num) const
    {
        if(num.empty())
            return false;

        uint i = 0;
        if(num[i] == '-')
            i++;

        bool hasDigit = false;
        for(; i < num.size(); i++)
        {
            if(!isdigit(num[i]))
                throw std::invalid_argument("BAD INPUT");

            hasDigit = true;
        }
        return hasDigit;
    }

    // string constructor
    CBigInt(std::string input) : m_number(input)
    {
        checkValidString(input);
        m_number = removeMark(m_number);
        m_number = removeZeroPadding(m_number);
    }

    std::string removeMark(std::string input)
    {
        if(input.empty())
            input.push_back('0');
        m_positive = true;
        if (input[0] == '-')
        {
            m_positive = false;
            input.erase(0, 1);
        }
        return input;
    }

    CBigInt& operator=(int input)
    {
        std::stringstream ss;
        ss << input;
        m_number = ss.str();
        m_number = removeMark(m_number);
        return *this;
    }

    CBigInt& operator=(std::string s)
    {
        checkValidString(s);
        s = removeMark(s);
        m_number = s;
        m_number = removeZeroPadding(m_number);
        return *this;
    }

    // operator +, any combination {CBigInt/int/string} + {CBigInt/int/string}
    //under the class

    CBigInt& addTwoNumbers(CBigInt other)
    {
        int maxLength = std::max(m_number.size(), other.m_number.size());

        // Pad with zeros to make lengths equal
        std::string val1 = std::string(maxLength - m_number.size(), '0') + m_number;
        std::string val2 = std::string(maxLength - other.m_number.size(), '0') + other.m_number;

        std::string result;
        int carry = 0;
        for (int i = maxLength - 1; i >= 0; i--)
        {
            int digitSum = (val1[i] - '0') + (val2[i] - '0') + carry;
            result.push_back(char((digitSum % 10) + '0'));
            carry = digitSum / 10;
        }

        if (carry)
            result.push_back('1');

        // Reverse the result
        reverse(result.begin(), result.end());

        m_number = result;

        return *this;
    }

    // TODO TEST
    CBigInt substractFromBigger(CBigInt bigger, CBigInt smaller)
    {
        bool mark = bigger.m_positive;
        std::string result;
        int borrow = 0;

        // Start from the least significant digit and work towards the most significant digit
        for (int i = int(bigger.m_number.length()) - 1, j = int(smaller.m_number.length()) - 1; i >= 0; --i, --j) {
            int digitBigger = (i >= 0) ? (bigger.m_number[i] - '0') : 0;
            int digitSmaller = (j >= 0) ? (smaller.m_number[j] - '0') : 0;

            int diff = digitBigger - digitSmaller - borrow;

            if (diff < 0) {
                diff += 10;
                borrow = 1;
            } else {
                borrow = 0;
            }

            result.push_back(char(diff + '0'));
        }

        // Reverse the result
        reverse(result.begin(), result.end());

        // Remove leading zeros
        result = removeZeroPadding(result);

        auto res = CBigInt(result);
        res.m_positive = mark;
        return res;
    }

    // operator +=, any of {CBigInt/int/string}
    template<typename T>
    CBigInt& operator+=(const T &tmp)
    {
        CBigInt other = CBigInt(tmp);

        if(other.m_positive == m_positive)
            return addTwoNumbers(other);
        else
        {
            // TODO TEST
            if(thisAbsBigger(other))
                *this =  substractFromBigger(*this, other);

            else
            {
                *this = substractFromBigger(other, *this);
                m_positive = other.m_positive;
            }

            m_number = removeZeroPadding(m_number);
            return *this;
        }
    }

    // operator *=, any of {CBigInt/int/string}
    template<typename T>
    CBigInt& operator*=(const T &right)
    {
        CBigInt other = CBigInt(right);
        std::string result(m_number.size() + other.m_number.size(), '0');

        for (int i = int(m_number.size()) - 1; i >= 0; --i) {
            int carry = 0;
            for (int j = int(other.m_number.size()) - 1; j >= 0; --j) {
                int product = (m_number[i] - '0') * (other.m_number[j] - '0') + (result[i + j + 1] - '0') + carry;
                result[i + j + 1] = char((product % 10) + '0');
                carry = product / 10;
            }
            result[i] = static_cast<char>((result[i] - '0' + carry) + '0');
        }

        // Remove leading zeros
        result = removeZeroPadding(result);

        m_number = result;
        m_positive = (m_positive == other.m_positive);
        return *this;
    }

    // operator *, any combination {CBigInt/int/string} * {CBigInt/int/string}
    // under the class

    // comparison operators, any combination {CBigInt/int/string} {<,<=,>,>=,==,!=} {CBigInt/int/string}
    template<class T>
    bool operator==(const T &other) const {
        CBigInt tmp = CBigInt(other);
        if(m_number == "0" && tmp.m_number == "0")
            return true;
        return (m_number == tmp.m_number) && (m_positive == tmp.m_positive);
    }

    template<class T>
    bool operator!=(const T &other) const {
        return !(*this == other);
    }

    template<class T>
    bool thisAbsBigger(const T &other) const {
        CBigInt tmp = CBigInt(other);
        if(tmp.m_number.size() < m_number.size())
            return true;
        else if(tmp.m_number.size() > m_number.size())
            return false;
        for(uint i = 0; i < m_number.size(); i++)
        {
            if(tmp.m_number[i] < m_number[i])
                return true;
            if(tmp.m_number[i] > m_number[i])
                return false;
        }
        return false;
    }

    // 0 same, 1 first bigger, 2 second bigger
    int accordingToMarks(const CBigInt &left, const CBigInt &right) const
    {
        if(left.m_number == "0" && right.m_number == "0")
            return 0;
        if(left.m_positive == right.m_positive)
            return 0;
        if(left.m_positive)
            return 1;
        return 2;
    }

    template<class T>
    bool operator>(const T &other) const {
        CBigInt tmp = CBigInt(other);
        if(*this == other)
            return false;

        auto res = accordingToMarks(*this, tmp);
        if(res == 1)
            return true;
        if(res == 2)
            return false;

        if(tmp.m_number.size() < m_number.size())
            return m_positive;
        else if(tmp.m_number.size() > m_number.size())
            return !m_positive;
        for(uint i = 0; i < m_number.size(); i++)
        {
            if(tmp.m_number[i] < m_number[i])
                return m_positive;
            if(tmp.m_number[i] > m_number[i])
                return !m_positive;
        }
        return !m_positive;
    }

    template<class T>
    bool operator <(const T &other) const
    {
        if(*this == other)
            return false;
        return !(*this > other);
    }

    template<class T>
    bool operator <=(const T &other) const
    {
        return ((*this < other) || (*this == other));
    }

    template<class T>
    bool operator >=(const T &other) const
    {
        return ((*this > other) || (*this == other));
    }

    // output operator <<
    friend std::ostream& operator<<(std::ostream& os, const CBigInt& bigint)
    {
        if(!bigint.m_positive && bigint.m_number != "0")
            os << '-';

        os << bigint.m_number;
        return os;
    }

    // input operator >>
    friend std::istream& operator>>(std::istream& is, CBigInt& bigint)
    {
        std::string input;

        while(is.peek() == ' ')
            is.get();

        if(is.peek() == '-')
            input.push_back(is.get());

        while(isdigit(is.peek()))
            input.push_back(is.get());

        if(!bigint.checkValidString(input)) {
            is.setstate(ios::failbit);
            return is;
        }

        bigint = CBigInt(input);
        return is;
    }

  private:
    std::string m_number;
    bool m_positive = true;
};

void testRelataionOperators();

template<typename T1, typename T2>
CBigInt operator+(const T1 &left, const T2 &right)
{
    CBigInt l1 = CBigInt(left);
    l1 += right;
    return l1;
}

template<typename T1, typename T2>
CBigInt operator*(const T1 &left, const T2 &right)
{
    CBigInt l1 = CBigInt(left);
    l1 *= right;
    return l1;
}

bool operator<(const int &left, const CBigInt &right)
{
    CBigInt l1 = CBigInt(left);
    return l1 < right;
}

bool operator<(const std::string &left , const CBigInt &right)
{
    CBigInt l1 = CBigInt(left);
    return l1 < right;
}

bool operator<=(const int &left, const CBigInt &right)
{
    CBigInt l1 = CBigInt(left);
    return l1 <= right;
}

bool operator<=(const std::string &left, const CBigInt &right)
{
    CBigInt l1 = CBigInt(left);
    return l1 <= right;
}

bool operator==(const int &left, const CBigInt &right)
{
    const CBigInt l1 = CBigInt(left);
    return l1 == right;
}

bool operator==(const std::string &left, const CBigInt &right)
{
    const CBigInt l1 = CBigInt(left);
    return l1 == right;
}

bool operator!=(const int &left, const CBigInt &right)
{
    CBigInt l1 = CBigInt(left);
    return l1 != right;
}

bool operator!=(const std::string &left, const CBigInt &right)
{
    CBigInt l1 = CBigInt(left);
    return l1 != right;
}

bool operator>(const int &left, const CBigInt &right)
{
    CBigInt l1 = CBigInt(left);
    return l1 > right;
}

bool operator>(const std::string &left, const CBigInt &right)
{
    CBigInt l1 = CBigInt(left);
    return l1 > right;
}

bool operator>=(const int &left, const CBigInt &right)
{
    const CBigInt l1 = CBigInt(left);
    return l1 >= right;
}

bool operator>=(const std::string &left, const CBigInt &right)
{
    const CBigInt l1 = CBigInt(left);
    return l1 >= right;
}

#ifndef __PROGTEST__
static bool equal ( const CBigInt & x, const char val [] )
{
    std::ostringstream oss;
    oss << x;
    return oss . str () == val;
}

static bool equalHex ( const CBigInt & x, const char val [] )
{
  return true; // hex output is needed for bonus tests only
  // ostringstream oss;
  // oss << hex << x;
  // return oss . str () == val;
}


void test()
{
    std::cout << "base tests" << std::endl;
    CBigInt a, b;
    std::istringstream is;
    a = 10;
    a += 20;
    assert ( equal ( a, "30" ) );
    a *= 5;
    assert ( equal ( a, "150" ) );
    b = a + 3;
    assert ( equal ( b, "153" ) );
    b = a * 7;
    assert ( equal ( b, "1050" ) );
    assert ( equal ( a, "150" ) );
    assert ( equalHex ( a, "96" ) );

    a = 10;
    a += -20;
    assert ( equal ( a, "-10" ) );
    a *= 5;
    assert ( equal ( a, "-50" ) );
    b = a + 73;
    assert ( equal ( b, "23" ) );
    b = a * -7;
    assert ( equal ( b, "350" ) );
    assert ( equal ( a, "-50" ) );
    assert ( equalHex ( a, "-32" ) );

    std::cout << "big number tests" << std::endl;
    a = "12345678901234567890";
    a += "-99999999999999999999";
    assert ( equal ( a, "-87654321098765432109" ) );
    a *= "54321987654321987654";
    assert ( equal ( a, "-4761556948575111126880627366067073182286" ) );
    a *= 0;
    assert ( equal ( a, "0" ) );
    a = 10;
    b = a + "400";
    assert ( equal ( b, "410" ) );
    b = a * "15";
    assert ( equal ( b, "150" ) );
    assert ( equal ( a, "10" ) );
    assert ( equalHex ( a, "a" ) );

    std::cout << "input tests" << std::endl;
    is . clear ();
    is . str ( " 1234" );
    assert ( is >> b );
    assert ( equal ( b, "1234" ) );
    is . clear ();
    is . str ( " 12 34" );
    assert ( is >> b );
    assert ( equal ( b, "12" ) );
    is . clear ();
    is . str ( "999z" );
    assert ( is >> b );
    assert ( equal ( b, "999" ) );
    is . clear ();
    is . str ( "abcd" );
    assert ( ! ( is >> b ) );
    is . clear ();
    is . str ( "- 758" );
    assert ( ! ( is >> b ) );
    a = 42;
    try
    {
        a = "-xyz";
        assert ( "missing an exception" == nullptr );
    }
    catch ( const std::invalid_argument & e )
    {
        assert ( equal ( a, "42" ) );
    }

    std::cout << "comparer tests" << std::endl;
    a = "73786976294838206464";
    assert (equal(a, "73786976294838206464"));
    assert (equalHex(a, "40000000000000000"));
    assert (a < "1361129467683753853853498429727072845824");
    assert (a <= "1361129467683753853853498429727072845824");
    assert (!(a > "1361129467683753853853498429727072845824"));
    assert (!(a >= "1361129467683753853853498429727072845824"));
    assert (!(a == "1361129467683753853853498429727072845824"));
    assert (a != "1361129467683753853853498429727072845824");
    assert (!(a < "73786976294838206464"));
    assert (a <= "73786976294838206464");
    assert (!(a > "73786976294838206464"));
    assert (a >= "73786976294838206464");
    assert (a == "73786976294838206464");
    assert (!(a != "73786976294838206464"));
    assert (a < "73786976294838206465");
    assert (a <= "73786976294838206465");
    assert (!(a > "73786976294838206465"));
    assert (!(a >= "73786976294838206465"));
    assert (!(a == "73786976294838206465"));
    assert (a != "73786976294838206465");
    a = "2147483648";
    assert (!(a < -2147483648));
    assert (!(a <= -2147483648));
    assert (a > -2147483648);
    assert (a >= -2147483648);
    assert (!(a == -2147483648));
    assert (a != -2147483648);
    a = "-12345678";
    assert (!(a < -87654321));
    assert (!(a <= -87654321));
    assert (a > -87654321);
    assert (a >= -87654321);
    assert (!(a == -87654321));
    assert (a != -87654321);

    std::cout << "base test good " << std::endl;
}

void mineTest()
{
    CBigInt a = CBigInt(1000);
    CBigInt b = CBigInt(999);
    CBigInt c = a.substractFromBigger(a, b);
    assert (a == 1000);

    a = "10";
    b = "-11";
    std::cout << a << " : " << b << std::endl;
    assert(a.thisAbsBigger(b) == false);

    a = 45;
    a += -99;
    assert(a == -54);

    CBigInt d ("00400");
    std::cout << d << std::endl;

}

int typeTest()
{
    CBigInt a = CBigInt("10");
    CBigInt b = 10 + a;
    CBigInt c = "6" + a;
    CBigInt d = b + c;
    CBigInt e = a + 10;
    CBigInt f = a + "42";

//    std::cout << a << endl << b << endl << c << endl << d << endl << e << endl << f << endl;
    
    a += b;
    a += "45";
    a += 45;

     a = CBigInt("10");
     b = 10 * a;
     c = "6" * a;
     d = b * c;
     e = a * 10;
     f = a * "42";

//    std::cout << a << endl << b << endl << c << endl << d << endl << e << endl << f << endl;

    a *= b;
    a *= "45";
    a *= 45;

    if(a > b ||
        a > "45" ||
        a > 45||
        a >= b||
        a >= "45"||
        a >= 45||
        a == b||
        a == "45"||
        a == 45 ||
        b > a||
        "45" > a||
        45 < a||
        b <= a||
        "45" >= a||
        45 >= a||
        b == a||
        "45" == a||
        45 == a)
        return 1;
    return 0;
}

void edgeTest()
{
    CBigInt a = CBigInt(-1);
    a *= 0;
    assert(equal(a, "0"));
    CBigInt b;
    a = -1;
    b = CBigInt("-1");
    b = b * -1;
    CBigInt c = b * a;
    assert(equal(c, "-1"));

    c = c + "-1001";
    c += 999;
    assert(equal(c, "-3"));
}

void anotherTest()
{
    std::cout << "base tests" << std::endl;
    CBigInt a, b;
    std::istringstream is;
    a = 10;
    a += 20;
    assert(equal(a, "30"));
    a *= 5;
    assert(equal(a, "150"));
    b = a + 3;
    assert(equal(b, "153"));
    b = a * 7;
    assert(equal(b, "1050"));
    assert(equal(a, "150"));
    assert(equalHex(a, "96"));

    a = 10;
    a += -20;
    assert(equal(a, "-10"));
    a *= 5;
    assert(equal(a, "-50"));
    b = a + 73;
    assert(equal(b, "23"));
    b = a * -7;
    assert(equal(b, "350"));
    assert(equal(a, "-50"));
    assert(equalHex(a, "-32"));

    std::cout << "big number tests" << std::endl;
    a = "12345678901234567890";
    a += "-99999999999999999999";
    assert(equal(a, "-87654321098765432109"));
    a *= "54321987654321987654";
    assert(equal(a, "-4761556948575111126880627366067073182286"));
    a *= 0;
    assert(equal(a, "0"));
    a = 10;
    b = a + "400";
    assert(equal(b, "410"));
    b = a * "15";
    assert(equal(b, "150"));
    assert(equal(a, "10"));
    assert(equalHex(a, "a"));

    std::cout << "input tests" << std::endl;
    is.clear();
    is.str(" 1234");
    assert(is >> b);
    assert(equal(b, "1234"));
    is.clear();
    is.str(" 12 34");
    assert(is >> b);
    assert(equal(b, "12"));
    is.clear();
    is.str("999z");
    assert(is >> b);
    assert(equal(b, "999"));
    is.clear();
    is.str("abcd");
    assert(!(is >> b));
    is.clear();
    is.str("- 758");
    assert(!(is >> b));
    a = 42;
    try {
        a = "-xyz";
        assert("missing an exception" == nullptr);
    } catch (const std::invalid_argument& e) {
        assert(equal(a, "42"));
    }

    std::cout << "comparer tests" << std::endl;
    a = "73786976294838206464";
    assert(equal(a, "73786976294838206464"));
    assert(equalHex(a, "40000000000000000"));
    assert(a < "1361129467683753853853498429727072845824");
    assert(a <= "1361129467683753853853498429727072845824");
    assert(!(a > "1361129467683753853853498429727072845824"));
    assert(!(a >= "1361129467683753853853498429727072845824"));
    assert(!(a == "1361129467683753853853498429727072845824"));
    assert(a != "1361129467683753853853498429727072845824");
    assert(!(a < "73786976294838206464"));
    assert(a <= "73786976294838206464");
    assert(!(a > "73786976294838206464"));
    assert(a >= "73786976294838206464");
    assert(a == "73786976294838206464");
    assert(!(a != "73786976294838206464"));
    assert(a < "73786976294838206465");
    assert(a <= "73786976294838206465");
    assert(!(a > "73786976294838206465"));
    assert(!(a >= "73786976294838206465"));
    assert(!(a == "73786976294838206465"));
    assert(a != "73786976294838206465");
    a = "2147483648";
    assert(!(a < -2147483648));
    assert(!(a <= -2147483648));
    assert(a > -2147483648);
    assert(a >= -2147483648);
    assert(!(a == -2147483648));
    assert(a != -2147483648);
    std::cout << "anotherTest" << std::endl;
}

void edgeValueTests()
{
    std::cout << "edge value tests" << std::endl;
    CBigInt a, b;
    std::istringstream is;

    // Largest Positive and Negative Values
    a = std::numeric_limits<int>::max();
    assert(equal(a, "2147483647"));
    a += 1;
    assert(equal(a, "2147483648"));
    b = std::numeric_limits<int>::min();
    assert(equal(b, "-2147483648"));
    b += -1;
    assert(equal(b, "-2147483649"));

    // Smallest Positive and Negative Values
    a = std::numeric_limits<int>::min();
    assert(equal(a, "-2147483648"));
    a += -1;
    assert(equal(a, "-2147483649"));
    b = std::numeric_limits<int>::max();
    assert(equal(b, "2147483647"));
    b += 1;
    assert(equal(b, "2147483648"));

    CBigInt e = CBigInt(-100);
    CBigInt f = 10;

    assert(equal(e+f, "-90"));
    assert(equal(f+e, "-90"));


    std::cout << "All edge value tests passed!" << std::endl;
}

void inputCheck()
{

    std::stringstream ss("123x");
    CBigInt x;
    ss >> x;

    // After extraction, check remaining characters
    std::string remaining;
    std::getline(ss, remaining);

    std::cout << "Remaining characters in stringstream: " << remaining << std::endl;
    assert(remaining == "x");

    std::stringstream ss1("-12x");
    std::stringstream ss2("  12x");
    std::stringstream ss3("12  x");
    std::stringstream ss4("12");
    std::stringstream ss5("  12  ");
    std::stringstream ss6("x12x");
    std::stringstream ss7("-x12x");
    std::stringstream ss8("- x12x");
    std::stringstream ss9(" -x12x");
    std::stringstream ss10("- 12x");
    std::stringstream ss11(" -12x");

    assert (( ss1 >> x) && equal(x, "-12"));
    assert (( ss2 >> x) && equal(x, "12"));
    assert (( ss3 >> x) && equal(x, "12"));
    assert (( ss4 >> x) && equal(x, "12"));
    assert (( ss5 >> x) && equal(x, "12"));
    assert (!( ss6 >> x));
    assert (!( ss7 >> x));
    assert (!( ss8 >> x));
    assert (!( ss9 >> x));
    assert (!( ss10 >> x));
    assert (( ss11 >> x) && equal(x, "-12"));

    std::cout << "Input check done" << std::endl;

}

void testRelataionOperators() {
    CBigInt a = CBigInt("-0");
    CBigInt b = CBigInt("0");
    CBigInt c = CBigInt("-100");
    CBigInt d = CBigInt("100");
    CBigInt e = CBigInt("-99");
    CBigInt f = CBigInt("-101");
    CBigInt g = CBigInt("101");
    CBigInt h = CBigInt("99");

    assert(a==b);
    assert(!(a!=b));
    assert(a<=b);
    assert(a>=b);

    assert(!(a>a));

    assert(c<d);
    assert(!(c>d));
    assert(e>c);
    assert(!(e<c));



    assert(!(a>b));
    assert(!(a<b));

}

int main ()
{
    inputCheck();
    anotherTest();
    typeTest();
    mineTest();
    test();
    edgeValueTests();
    edgeTest();
    testRelataionOperators();

    std::cout << "KONEC NAZDAR" << std::endl;
    return EXIT_SUCCESS;
}

#endif /* __PROGTEST__ */
