#ifndef __PROGTEST__

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <climits>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <functional>
#include <iterator>
#include <compare>

class CDate {
public:
    CDate(int y,
          int m,
          int d)
            : m_Y(y),
              m_M(m),
              m_D(d) {
    }


    bool operator==(const CDate &other) const {
        return m_Y == other.m_Y && m_M == other.m_M && m_D == other.m_D;
    }

    bool operator!=(const CDate &other) const {
        return !(*this == other);
    }

    bool operator<(const CDate &other) const {
        if (m_Y != other.m_Y) return m_Y < other.m_Y;
        if (m_M != other.m_M) return m_M < other.m_M;
        return m_D < other.m_D;
    }

    bool operator>(const CDate &other) const {
        return other < *this;
    }

    bool operator<=(const CDate &other) const {
        return !(other < *this);
    }

    bool operator>=(const CDate &other) const {
        return !(*this < other);
    }

//    std::strong_ordering operator<=>(const CDate &other) const = default;

    friend std::ostream &operator<<(std::ostream &os,
                                    const CDate &d) {
        return os << d.m_Y << '-' << d.m_M << '-' << d.m_D;
    }

private:
    int m_Y;
    int m_M;
    int m_D;
};

enum class ESortKey {
    NAME,
    BIRTH_DATE,
    ENROLL_YEAR
};
#endif /* __PROGTEST__ */
using namespace std;

bool contains(vector<string> real_name, vector<string> other) {
    if (real_name.size() < other.size())
        return false;

    // real_name is sorted and all characters are lowercase
    for(auto &name: other)
    {
        if(!binary_search(real_name.begin(), real_name.end(), name))
            return false;
    }
    return true;
}

vector<string> splitIntoLowerNamesSorted(string name)
{
//    std::vector<std::string> names;
//    std::istringstream iss(name);
//    std::string tmp;
//
//    while (iss >> tmp) {
//        names.push_back(tmp);
//    }

    vector<string> names;
    string tmp;
    for(char i : name)
    {
        if(i == ' ' && !tmp.empty())
        {
            names.push_back(tmp);
            tmp = "";
        }
        else if(i == ' ')
        {
            continue;
        }
        else
            tmp += char(tolower(i));
    }

    if(!tmp.empty())
        names.push_back(tmp);


    sort(names.begin(), names.end());
    return names;
}

class CStudent {
public:
    std::string m_Name;
    CDate m_Born = {0, 0, 0};
    int m_Enrolled;
    uint m_order = 0;
    vector<string> m_NameParts;

    CStudent() = default;

    CStudent(const string &name, const CDate &born, int enrolled) : m_Name(name), m_Born(born), m_Enrolled(enrolled)
    {
        m_NameParts = splitIntoLowerNamesSorted(name);
    }

    bool operator==(const CStudent &other) const
    {
        return m_Name == other.m_Name && m_Born == other.m_Born && m_Enrolled == other.m_Enrolled;
    }


    bool operator<(const CStudent &other) const
    {
        if (m_Name != other.m_Name) return m_Name < other.m_Name;
        if (m_Born != other.m_Born) return m_Born < other.m_Born;
        return m_Enrolled < other.m_Enrolled;
    }

    bool operator>(const CStudent &other) const
    {
        return other < *this;
    }

    bool operator<=(const CStudent &other) const
    {
        return !(other < *this);
    }

    bool operator>=(const CStudent &other) const
    {
        return !(*this < other);
    }

    bool operator!=(const CStudent &other) const
    {
        return !(*this == other);
    }

private:

};
/*
class CFilterSingle {
public:
    virtual vector<CStudent> filter(vector<CStudent> &students) = 0;

    virtual vector<string> getName(){return {};};
};

class CFilterName : public CFilterSingle {
public:
    string oldName;
    vector<string> m_Name;

    vector<string> getName() override
    {
        return m_Name;
    }


    CFilterName(string name)
    {
        m_Name = splitIntoLowerNamesSorted(name);
    };

    bool isSameName(string other) const
    {
        vector<string> otherNames = splitIntoLowerNamesSorted(other);
        return otherNames == m_Name;
    }

    vector<CStudent> filter(vector<CStudent> &students) override
    {
        vector<CStudent> result;
        for (const auto &student : students)
        {
            if (isSameName(student.m_Name))
            {
                result.push_back(student);
            }
        }
        return result;
    }
};

class CFilterBornBefore : public CFilterSingle {
public:
    CDate m_Date;

    CFilterBornBefore(CDate tmp): m_Date(tmp) {};

    vector<CStudent> filter(vector<CStudent> &students) override
    {
        vector<CStudent> result;
        for (const auto &student : students)
        {
            if (student.m_Born < m_Date)
            {
                result.push_back(student);
            }
        }
        return result;
    }

};

class CFilterBornAfter : public CFilterSingle {
public:
    CDate m_Date;

    CFilterBornAfter(CDate tmp): m_Date(tmp) {};

    vector<CStudent> filter(vector<CStudent> &students) override
    {
        vector<CStudent> result;
        for (const auto &student : students)
        {
            if (student.m_Born > m_Date)
            {
                result.push_back(student);
            }
        }
        return result;
    }


};

class CFilterEnrolledBefore : public CFilterSingle {
public:
    int m_Year;

    CFilterEnrolledBefore(int tmp): m_Year(tmp) {};

    vector<CStudent> filter(vector<CStudent> &students) override
    {
        vector<CStudent> result;
        for (const auto &student : students)
        {
            if (student.m_Enrolled < m_Year)
            {
                result.push_back(student);
            }
        }
        return result;
    }

};

class CFilterEnrolledAfter : public CFilterSingle {
public:
    int m_Year;

    CFilterEnrolledAfter(int tmp): m_Year(tmp) {};

    vector<CStudent> filter(vector<CStudent> &students) override
    {
        vector<CStudent> result;
        for (const auto &student : students)
        {
            if (student.m_Enrolled > m_Year)
            {
                result.push_back(student);
            }
        }
        return result;
    }
};
class CFilter {
public:
    CFilter() = default;

    bool isDuplicate(const string &name) const
    {
        auto tmp = splitIntoLowerNamesSorted(name);
        std::sort(tmp.begin(), tmp.end());

        for(uint i = 0; i < m_nameCount; i++)
        {
            if(m_Filters[i]->getName() == tmp)
                return true;
        }
        return false;
    }

    CFilter &name(const std::string &name)
    {
        if(isDuplicate(name))
            return *this;

        auto tmp = make_shared<CFilterName>(name);
        if(tmp->m_Name.empty())
            return *this;

        m_Filters.insert(m_Filters.cbegin() + m_nameCount, make_shared<CFilterName>(name));
        m_nameCount++;
        return *this;
    }

    CFilter &bornBefore(const CDate &date)
    {
        m_Filters.push_back(make_shared<CFilterBornBefore>(date));
        return *this;
    }

    CFilter &bornAfter(const CDate &date)
    {
        m_Filters.push_back(make_shared<CFilterBornAfter>(date));
        return *this;
    }

    CFilter &enrolledBefore(int year)
    {
        m_Filters.push_back(make_shared<CFilterEnrolledBefore>(year));
        return *this;
    }

    CFilter &enrolledAfter(int year)
    {
        m_Filters.push_back(make_shared<CFilterEnrolledAfter>(year));
        return *this;
    }

    vector<CStudent> applyFilters(vector<CStudent> &students) const
    {
        if(m_Filters.empty())
            return students;

        vector<CStudent> result;
        uint i = 0;

        for(;i < m_nameCount; i++)
        {
            auto tmp = m_Filters[i]->filter(students);
            result.insert(result.end(), tmp.begin(), tmp.end());
        }

        if(m_nameCount == 0)
        {
            result = m_Filters[0]->filter(students);
            i++;
        }

        for (;i < m_Filters.size(); i++)
        {
            result = m_Filters[i]->filter(result);
        }
        return result;
    }

private:
    uint m_nameCount = 0;
    vector<shared_ptr<CFilterSingle>> m_Filters;
};
*/

class CFilter {
public:
    CFilter() = default;

    bool belongsInFilter(const CStudent &student) const
    {
        if (!(student.m_Born < bornBeforeDate && student.m_Born > bornAfterDate))
            return false;

        if (!(student.m_Enrolled < enrolledBeforeYear && student.m_Enrolled > enrolledAfterYear))
            return false;

        if(all_names.empty())
            return true;

        auto name = splitIntoLowerNamesSorted(student.m_Name);

        return !(all_names.find(name) == all_names.end());
    }

    CFilter &name(const std::string &name)
    {
        auto tmp = splitIntoLowerNamesSorted(name);
        if(tmp.empty())
            return *this;

        all_names.insert(tmp);
        return *this;
    }

    CFilter &bornBefore(const CDate &date)
    {
        if(date < bornBeforeDate)
            bornBeforeDate = date;

        return *this;
    }

    CFilter &bornAfter(const CDate &date)
    {
        if(date > bornAfterDate)
            bornAfterDate = date;

        return *this;
    }

    CFilter &enrolledBefore(int year)
    {
        if(year < enrolledBeforeYear)
            enrolledBeforeYear = year;

        return *this;
    }

    CFilter &enrolledAfter(int year)
    {
        if(year > enrolledAfterYear)
            enrolledAfterYear = year;

        return *this;
    }

private:
    set<vector<string>> all_names;
    CDate bornBeforeDate = {INT_MAX, INT_MAX, INT_MAX};
    CDate bornAfterDate = {-1, -1, -1};
    int enrolledBeforeYear = INT_MAX;
    int enrolledAfterYear = INT_MIN;

};

class CSort {
public:
    CSort() = default;

    CSort &addKey(ESortKey key, bool ascending)
    {
        m_SortKeys.emplace_back(key, ascending);
        return *this;
    }

    bool operator()(CStudent &a, CStudent &b)
    {
        if(m_SortKeys.empty())
            return a.m_order < b.m_order;

        for (const auto &key : m_SortKeys)
        {
            switch (key.first)
            {
                case ESortKey::NAME:
                    if (a.m_Name < b.m_Name)
                    {
                        return key.second;
                    }
                    else if (a.m_Name > b.m_Name)
                    {
                        return !key.second;
                    }
                    break;
                case ESortKey::BIRTH_DATE:
                    if (a.m_Born < b.m_Born)
                    {
                        return key.second;
                    }
                    else if (a.m_Born > b.m_Born)
                    {
                        return !key.second;
                    }
                    break;
                case ESortKey::ENROLL_YEAR:
                    if (a.m_Enrolled < b.m_Enrolled)
                    {
                        return key.second;
                    }
                    else if (a.m_Enrolled > b.m_Enrolled)
                    {
                        return !key.second;
                    }
                    break;
            }
        }
        return a.m_order < b.m_order;
    }

private:
    vector<pair<ESortKey, bool>> m_SortKeys;
};

class CStudyDept {
public:
    CStudyDept() = default;

    bool addStudent(const CStudent &x)
    {
        auto tmp = x;
        if (m_students.find(tmp) != m_students.end())
        {
            return false;
        }
        tmp.m_order = m_StudentCount;
        m_students.insert(tmp);
        m_StudentCount++;

        auto it = std::lower_bound(m_namesForSuggest[tmp.m_NameParts].begin(), m_namesForSuggest[tmp.m_NameParts].end(), tmp.m_Name);

        if(!(it != m_namesForSuggest[tmp.m_NameParts].end() && *it == tmp.m_Name))
            m_namesForSuggest[tmp.m_NameParts].insert(it, tmp.m_Name);



        return true;
    }

    bool delStudent(const CStudent &x)
    {
        if (m_students.find(x) == m_students.end())
        {
            return false;
        }
        m_students.erase(x);

        m_namesForSuggest[x.m_NameParts].erase(std::lower_bound(m_namesForSuggest[x.m_NameParts].begin(), m_namesForSuggest[x.m_NameParts].end(), x.m_Name));
        if(m_namesForSuggest[x.m_NameParts].empty())
            m_namesForSuggest.erase(x.m_NameParts);

        //m_StudentCount--;
        // can be here, to assure that the order is correct

        return true;
    }

//    list<CStudent> searchOld(const CFilter &flt, const CSort &sortOpt) const
//    {
//        vector<CStudent> students;
//        students.reserve(m_students.size());
//        for (const auto &student : m_students)
//        {
//            students.push_back(student);
//        }
//        students = flt.applyFilters(students);
//        sort(students.begin(), students.end(), sortOpt);
//        list<CStudent> result;
//        for (const auto &student : students)
//        {
//            result.push_back(student);
//        }
//        list<CStudent> tmp (result.begin(), result.end());
//        return tmp;
//    }

    list<CStudent> search(const CFilter &flt, const CSort &sortOpt) const
    {
        list<CStudent> result;
        for (const auto &student : m_students)
        {
            if(flt.belongsInFilter(student))
            {
                result.push_back(student);
            }

        }

        result.sort(sortOpt);
        return result;
    }


    static void getRidOfRedundantNames(vector<string> &names)
    {
        // names is sorted and all characters are lowercase
        for(uint i = 0; i < names.size() - 1; i++)
        {
            if(names[i] == names[i + 1])
            {
                names.erase(names.begin() + i);
                i--;
            }
        }

    }

    set<string> suggest(const std::string &name) const
    {
        vector<string> names;
        names = splitIntoLowerNamesSorted(name);
        getRidOfRedundantNames(names);

        set<string> result;

        // find all

        for(auto &student : m_namesForSuggest)
        {
            if(student.first[0] > names[0])
                break;

            if(contains(student.first, names))
            {
                result.insert(student.second.begin(), student.second.end());
            }
        }
        
        return result;
    }

private:
    uint m_StudentCount = 0;
    set<CStudent> m_students;

    // name parts and all names associated with them
    map<vector<string>, vector<string>> m_namesForSuggest;
};

#ifndef __PROGTEST__

void test()
{
    CStudyDept x0;
    assert (CStudent("James Bond", CDate(1980, 4, 11), 2010) == CStudent("James Bond", CDate(1980, 4, 11), 2010));
    assert (!(CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("James Bond", CDate(1980, 4, 11), 2010)));
    assert (CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("Peter Peterson", CDate(1980, 4, 11), 2010));
    assert (!(CStudent("James Bond", CDate(1980, 4, 11), 2010) ==
              CStudent("Peter Peterson", CDate(1980, 4, 11), 2010)));
    assert (CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("James Bond", CDate(1997, 6, 17), 2010));
    assert (!(CStudent("James Bond", CDate(1980, 4, 11), 2010) == CStudent("James Bond", CDate(1997, 6, 17), 2010)));
    assert (CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("James Bond", CDate(1980, 4, 11), 2016));
    assert (!(CStudent("James Bond", CDate(1980, 4, 11), 2010) == CStudent("James Bond", CDate(1980, 4, 11), 2016)));
    assert (CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("Peter Peterson", CDate(1980, 4, 11), 2016));
    assert (!(CStudent("James Bond", CDate(1980, 4, 11), 2010) ==
              CStudent("Peter Peterson", CDate(1980, 4, 11), 2016)));
    assert (CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("Peter Peterson", CDate(1997, 6, 17), 2010));
    assert (!(CStudent("James Bond", CDate(1980, 4, 11), 2010) ==
              CStudent("Peter Peterson", CDate(1997, 6, 17), 2010)));
    assert (CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("James Bond", CDate(1997, 6, 17), 2016));
    assert (!(CStudent("James Bond", CDate(1980, 4, 11), 2010) == CStudent("James Bond", CDate(1997, 6, 17), 2016)));
    assert (CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("Peter Peterson", CDate(1997, 6, 17), 2016));
    assert (!(CStudent("James Bond", CDate(1980, 4, 11), 2010) ==
              CStudent("Peter Peterson", CDate(1997, 6, 17), 2016)));
    assert (x0.addStudent(CStudent("John Peter Taylor", CDate(1983, 7, 13), 2014)));
    assert (x0.addStudent(CStudent("John Taylor", CDate(1981, 6, 30), 2012)));
    assert (x0.addStudent(CStudent("Peter Taylor", CDate(1982, 2, 23), 2011)));
    assert (x0.addStudent(CStudent("Peter John Taylor", CDate(1984, 1, 17), 2017)));
    assert (x0.addStudent(CStudent("James Bond", CDate(1981, 7, 16), 2013)));
    assert (x0.addStudent(CStudent("James Bond", CDate(1982, 7, 16), 2013)));
    assert (x0.addStudent(CStudent("James Bond", CDate(1981, 8, 16), 2013)));
    assert (x0.addStudent(CStudent("James Bond", CDate(1981, 7, 17), 2013)));
    assert (x0.addStudent(CStudent("James Bond", CDate(1981, 7, 16), 2012)));
    assert (x0.addStudent(CStudent("Bond James", CDate(1981, 7, 16), 2013)));
    auto tmp = x0.search(CFilter(), CSort());
    assert (x0.search(CFilter(), CSort()) == (std::list<CStudent>
            {
                    CStudent("John Peter Taylor", CDate(1983, 7, 13), 2014),
                    CStudent("John Taylor", CDate(1981, 6, 30), 2012),
                    CStudent("Peter Taylor", CDate(1982, 2, 23), 2011),
                    CStudent("Peter John Taylor", CDate(1984, 1, 17), 2017),
                    CStudent("James Bond", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond", CDate(1982, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 8, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 17), 2013),
                    CStudent("James Bond", CDate(1981, 7, 16), 2012),
                    CStudent("Bond James", CDate(1981, 7, 16), 2013)
            }));

    assert (x0.search(CFilter(), CSort().addKey(ESortKey::NAME, true)) == (std::list<CStudent>
            {
                    CStudent("Bond James", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond", CDate(1982, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 8, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 17), 2013),
                    CStudent("James Bond", CDate(1981, 7, 16), 2012),
                    CStudent("John Peter Taylor", CDate(1983, 7, 13), 2014),
                    CStudent("John Taylor", CDate(1981, 6, 30), 2012),
                    CStudent("Peter John Taylor", CDate(1984, 1, 17), 2017),
                    CStudent("Peter Taylor", CDate(1982, 2, 23), 2011)
            }));
    assert (x0.search(CFilter(), CSort().addKey(ESortKey::NAME, false)) == (std::list<CStudent>
            {
                    CStudent("Peter Taylor", CDate(1982, 2, 23), 2011),
                    CStudent("Peter John Taylor", CDate(1984, 1, 17), 2017),
                    CStudent("John Taylor", CDate(1981, 6, 30), 2012),
                    CStudent("John Peter Taylor", CDate(1983, 7, 13), 2014),
                    CStudent("James Bond", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond", CDate(1982, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 8, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 17), 2013),
                    CStudent("James Bond", CDate(1981, 7, 16), 2012),
                    CStudent("Bond James", CDate(1981, 7, 16), 2013)
            }));
    assert (x0.search(CFilter(),
                      CSort().addKey(ESortKey::ENROLL_YEAR, false).addKey(ESortKey::BIRTH_DATE, false).addKey(
                              ESortKey::NAME, true)) == (std::list<CStudent>
            {
                    CStudent("Peter John Taylor", CDate(1984, 1, 17), 2017),
                    CStudent("John Peter Taylor", CDate(1983, 7, 13), 2014),
                    CStudent("James Bond", CDate(1982, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 8, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 17), 2013),
                    CStudent("Bond James", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 16), 2012),
                    CStudent("John Taylor", CDate(1981, 6, 30), 2012),
                    CStudent("Peter Taylor", CDate(1982, 2, 23), 2011)
            }));

    assert (x0.search(CFilter().name("james bond"),
                      CSort().addKey(ESortKey::ENROLL_YEAR, false).addKey(ESortKey::BIRTH_DATE, false).addKey(
                              ESortKey::NAME, true)) == (std::list<CStudent>
            {
                    CStudent("James Bond", CDate(1982, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 8, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 17), 2013),
                    CStudent("Bond James", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 16), 2012)
            }));
    assert (x0.search(CFilter().bornAfter(CDate(1980, 4, 11)).bornBefore(CDate(1983, 7, 13)).name("John Taylor").name(
            "james BOND"), CSort().addKey(ESortKey::ENROLL_YEAR, false).addKey(ESortKey::BIRTH_DATE, false).addKey(
            ESortKey::NAME, true)) == (std::list<CStudent>
            {
                    CStudent("James Bond", CDate(1982, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 8, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 17), 2013),
                    CStudent("Bond James", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 16), 2012),
                    CStudent("John Taylor", CDate(1981, 6, 30), 2012)
            }));
    assert (x0.search(CFilter().name("james"), CSort().addKey(ESortKey::NAME, true)) == (std::list<CStudent>
            {
            }));

    auto hjkl = x0.suggest("peter");
    assert (x0.suggest("peter") == (std::set<std::string>
            {
                    "John Peter Taylor",
                    "Peter John Taylor",
                    "Peter Taylor"
            }));
    assert (x0.suggest("bond") == (std::set<std::string>
            {
                    "Bond James",
                    "James Bond"
            }));
    assert (x0.suggest("peter joHn") == (std::set<std::string>
            {
                    "John Peter Taylor",
                    "Peter John Taylor"
            }));
    assert (x0.suggest("peter joHn bond") == (std::set<std::string>
            {
            }));
    assert (x0.suggest("pete") == (std::set<std::string>
            {
            }));
    assert (x0.suggest("peter joHn PETER") == (std::set<std::string>
            {
                    "John Peter Taylor",
                    "Peter John Taylor"
            }));
    assert (!x0.addStudent(CStudent("James Bond", CDate(1981, 7, 16), 2013)));
    assert (x0.delStudent(CStudent("James Bond", CDate(1981, 7, 16), 2013)));
    assert (x0.search(CFilter().bornAfter(CDate(1980, 4, 11)).bornBefore(CDate(1983, 7, 13)).name("John Taylor").name(
            "james BOND"), CSort().addKey(ESortKey::ENROLL_YEAR, false).addKey(ESortKey::BIRTH_DATE, false).addKey(
            ESortKey::NAME, true)) == (std::list<CStudent>
            {
                    CStudent("James Bond", CDate(1982, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 8, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 17), 2013),
                    CStudent("Bond James", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 16), 2012),
                    CStudent("John Taylor", CDate(1981, 6, 30), 2012)
            }));
    assert (!x0.delStudent(CStudent("James Bond", CDate(1981, 7, 16), 2013)));

    cout << "TEST PASSED" << endl;
}

void mineTest()
{
    CStudyDept x0;
    assert (x0.addStudent(CStudent("James Bond", CDate(1981, 7, 16), 2013)));
    assert (x0.addStudent(CStudent("James Bond", CDate(1982, 7, 16), 2013)));
    assert (x0.addStudent(CStudent("James Bond", CDate(1981, 8, 16), 2013)));
    assert (x0.addStudent(CStudent("James Bond", CDate(1981, 7, 17), 2013)));
    assert (x0.addStudent(CStudent("James Bond", CDate(1981, 7, 16), 2012)));

    assert(x0.search(CFilter().enrolledBefore(2013), CSort()) == (std::list<CStudent>
            {
                    CStudent("James Bond", CDate(1981, 7, 16), 2012)
            }));

    assert(x0.search(CFilter().enrolledAfter(2012), CSort()) == (std::list<CStudent>
            {
                    CStudent("James Bond", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond", CDate(1982, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 8, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 17), 2013)
            }));

    assert(x0.search(CFilter().bornBefore(CDate(1982, 7, 16)), CSort()) == (std::list<CStudent>
            {
                    CStudent("James Bond", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 8, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 17), 2013),
                    CStudent("James Bond", CDate(1981, 7, 16), 2012)
            }));


    assert(x0.search(CFilter().bornAfter(CDate(1981, 7, 16)), CSort()) == (std::list<CStudent>
            {
                    CStudent("James Bond", CDate(1982, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 8, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 17), 2013)
            }));

    assert(x0.search(CFilter().bornBefore(CDate(1982, 7, 16)).bornAfter(CDate(1981, 7, 16)), CSort()) == (std::list<CStudent>
            {
                    CStudent("James Bond", CDate(1981, 8, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 17), 2013)
            }));

    assert(x0.search(CFilter().bornBefore(CDate(1982, 7, 16)).bornAfter(CDate(1981, 7, 16)).enrolledAfter(2013), CSort()) == (std::list<CStudent>
            {
            }));

    x0.addStudent(CStudent("uz na to nemam", CDate(1981, 7, 16), 2013));
    x0.addStudent(CStudent("asi se zabiju", CDate(1981, 7, 16), 2013));

    auto tmp = x0.search(CFilter().name("uz na to nemam").name("asi se zabiju"), CSort());

    assert(x0.search(CFilter().name("uz na to nemam").name("asi se zabiju"), CSort()) == (std::list<CStudent>
            {
                    CStudent("uz na to nemam", CDate(1981, 7, 16), 2013),
                    CStudent("asi se zabiju", CDate(1981, 7, 16), 2013)
            }));

    assert(x0.search(CFilter(), CSort()) == (std::list<CStudent>
            {
                    CStudent("James Bond", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond", CDate(1982, 7, 16), 2013),
                    CStudent("James Bond", CDate(1981, 8, 16), 2013),
                    CStudent("James Bond", CDate(1981, 7, 17), 2013),
                    CStudent("James Bond", CDate(1981, 7, 16), 2012),
                    CStudent("uz na to nemam", CDate(1981, 7, 16), 2013),
                    CStudent("asi se zabiju", CDate(1981, 7, 16), 2013)
            }));

    cout << "MINE TEST PASSED" << endl;
}


void test2()
{
    CStudyDept studyDept;

    assert(studyDept.addStudent(CStudent("John Doe", CDate(1990, 1, 1), 2010)));
    assert(studyDept.addStudent(CStudent("Jane Doe", CDate(1992, 2, 2), 2011)));
    assert(studyDept.addStudent(CStudent("Alice Smith", CDate(1989, 3, 3), 2009)));
    assert(studyDept.addStudent(CStudent("Bob Johnson", CDate(1991, 4, 4), 2010)));

    auto result = studyDept.search(CFilter(), CSort());
    assert(result.size() == 4);

    result = studyDept.search(CFilter().name("John Doe"), CSort());
    assert(result.size() == 1);

    result = studyDept.search(CFilter().bornBefore(CDate(1990, 1, 1)), CSort());
    assert(result.size() == 1);

    result = studyDept.search(CFilter().bornAfter(CDate(1990, 1, 1)), CSort());
    assert(result.size() == 2);

    result = studyDept.search(CFilter().enrolledBefore(2010), CSort());
    assert(result.size() == 1);

    result = studyDept.search(CFilter().enrolledAfter(2010), CSort());
    assert(result.size() == 1);

    result = studyDept.search(CFilter().name("John Doe").bornBefore(CDate(1991, 1, 1)), CSort());
    assert(result.size() == 1);

    result = studyDept.search(CFilter().name("Nonexistent Student"), CSort());
    assert(result.size() == 0);

    cout << "TEST 2 PASSED" << endl;
}

void testSuggest()
{
    CStudyDept studyDept;

    studyDept.addStudent(CStudent("John Doe", CDate(1990, 1, 1), 2010));
    studyDept.addStudent(CStudent("Jane Doe", CDate(1992, 2, 2), 2011));
    studyDept.addStudent(CStudent("Alice Smith", CDate(1989, 3, 3), 2009));
    studyDept.addStudent(CStudent("Bob Johnson", CDate(1991, 4, 4), 2010));

    assert(studyDept.suggest("John") == set<string>{"John Doe"});

    auto res = set<string>{"John Doe", "Jane Doe"};
    assert(studyDept.suggest("doe") == res);
    assert(studyDept.suggest("Alice") == set<string>{"Alice Smith"});
    assert(studyDept.suggest("Bob") == set<string>{"Bob Johnson"});
    assert(studyDept.suggest("Nonexistent Student") == set<string>{});

    studyDept.addStudent(CStudent("John Don Doue Doe", CDate(1990, 1, 1), 2010));
    studyDept.addStudent(CStudent("Jane Meh Doe", CDate(1992, 2, 2), 2011));
    studyDept.addStudent(CStudent("Alice A Smith", CDate(1989, 3, 3), 2009));
    studyDept.addStudent(CStudent("Bob Johnson", CDate(1991, 4, 4), 2010));

    assert(studyDept.suggest("A") == set<string>{"Alice A Smith"});
    assert(studyDept.suggest("John Don") == set<string>{"John Don Doue Doe"});
    res = set<string>{"John Doe", "John Don Doue Doe"};
    assert(studyDept.suggest("dOe jOhn") == res);

    cout << "TEST SUGGEST" << endl;
}

void lastChance()
{
    CStudyDept x0;
    assert (x0.addStudent(CStudent("James Bond1", CDate(1981, 7, 16), 2013)));
    assert (x0.addStudent(CStudent("James Bond2", CDate(1982, 7, 16), 2013)));
    assert (x0.addStudent(CStudent("James Bond3", CDate(1981, 8, 16), 2013)));
    assert (x0.addStudent(CStudent("James Bond4", CDate(1981, 7, 17), 2013)));
    assert (x0.addStudent(CStudent("James Bond5", CDate(1981, 7, 16), 2012)));

    assert(x0.search(CFilter().enrolledAfter(2012).enrolledBefore(2013), CSort()) == (std::list<CStudent>
            {}));

    assert(x0.search(CFilter().enrolledAfter(2012).enrolledBefore(2014), CSort()) == (std::list<CStudent>
            {
                    CStudent("James Bond1", CDate(1981, 7, 16), 2013),
                    CStudent("James Bond2", CDate(1982, 7, 16), 2013),
                    CStudent("James Bond3", CDate(1981, 8, 16), 2013),
                    CStudent("James Bond4", CDate(1981, 7, 17), 2013)
            }));

    assert (x0.addStudent(CStudent("Jan Jakub Ryba", CDate(1981, 7, 16), 2012)));
    assert (x0.addStudent(CStudent("Jan Ryba", CDate(1981, 7, 16), 2012)));
    assert (x0.addStudent(CStudent("Jan Novak", CDate(1981, 7, 16), 2012)));

    auto tmp = std::set<std::string> {"Jan Jakub Ryba", "Jan Ryba", "Jan Novak"};
    assert(x0.suggest("jan") == tmp);
    assert(x0.suggest("Ryba Ja") == std::set<std::string> {});
    tmp = {"Jan Jakub Ryba", "Jan Ryba"};
    assert(x0.suggest("Ryba Jan") == tmp);
    assert(x0.suggest("Ryba Jakub") == std::set<std::string> {"Jan Jakub Ryba"});
    tmp = {"Jan Jakub Ryba", "Jan Ryba"};
    assert(x0.suggest("Jan jan JAN Ryba RyBa") == tmp);


    assert (x0.addStudent(CStudent("Jakub Jan Ryba", CDate(1981, 7, 16), 2012)));
    assert (x0.addStudent(CStudent("RYBA jan JaKuB", CDate(1981, 7, 16), 2012)));

    CStudyDept x1;
    // filtr jmen case INsensitive, shodna s jinym poradim

    assert (x1.addStudent(CStudent("Jan Jakub Ryba", CDate(1981, 7, 16), 2013)));
    assert (x1.addStudent(CStudent("Jakub Jan Ryba", CDate(1982, 7, 16), 2013)));
    assert (x1.addStudent(CStudent("RYBA jan JaKuB", CDate(1981, 8, 16), 2013)));
    assert (x1.addStudent(CStudent("RYBAl jan JaKuB", CDate(1981, 8, 16), 2013)));




    assert(x1.search(CFilter().name("ryba jan jakub"), CSort()) == (std::list<CStudent>
            {
                    CStudent("Jan Jakub Ryba", CDate(1981, 7, 16), 2013),
                    CStudent("Jakub Jan Ryba", CDate(1982, 7, 16), 2013),
                    CStudent("RYBA jan JaKuB", CDate(1981, 8, 16), 2013)
            }));

    assert(x1.search(CFilter().name("Jan Jakub Jan Ryba"), CSort()) == (std::list<CStudent>
            {}));

    auto res = x1.search(CFilter().name("RYBA JaKuB").name("RYBA JaKuB"), CSort());

    assert(x1.search(CFilter().name("RYBA JaKuB").name("RYBA JaKuB"), CSort()) == (std::list<CStudent>
            {}));



    assert(x1.search(CFilter().name("RYBA jan JaKuB").name("RYBA jan JaKuB").name("RYBAl jan JaKuB"), CSort()) == (std::list<CStudent>
            {
                    CStudent("Jan Jakub Ryba", CDate(1981, 7, 16), 2013),
                    CStudent("Jakub Jan Ryba", CDate(1982, 7, 16), 2013),
                    CStudent("RYBA jan JaKuB", CDate(1981, 8, 16), 2013),
                    CStudent("RYBAl jan JaKuB", CDate(1981, 8, 16), 2013)
            }));

    assert(x1.search(CFilter().name("lkjwahfehfkjrslkjhergf").name("RYBA jan JaKuB").name("lkjwahfehfkjrslkjhergf"), CSort()) == (std::list<CStudent>
            {
                    CStudent("Jan Jakub Ryba", CDate(1981, 7, 16), 2013),
                    CStudent("Jakub Jan Ryba", CDate(1982, 7, 16), 2013),
                    CStudent("RYBA jan JaKuB", CDate(1981, 8, 16), 2013)
            }));

    cout << "LAST CHANCE" << endl;
}

int main() {
    test();
    test2();
    mineTest();
    testSuggest();
    lastChance();

    cout << "All tests passed" << endl;
    return EXIT_SUCCESS;
}

#endif /* __PROGTEST__ */
