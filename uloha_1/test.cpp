#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <memory>
#endif /* __PROGTEST__ */

using namespace std;
class COwner;
class CIterator;

class CProperty
{
public:
    string city;
    string ownerName;
    shared_ptr<COwner> owner;
    string address;
    string region;
    uint ID;

    CProperty(const std::string city, const std::string addr, const std::string region, unsigned int id)
    : city(city), address(addr), region(region), ID(id){}

    bool lessThanViaRegId(const shared_ptr<CProperty>& other) const
    {
        if(other->region > region)
            return true;
        else if(other->region == region)
        {
            if(other->ID > ID)
                return true;
            else
                return false;
        }
        else
            return false;
    }

    bool lessThanViaCityAdd(const shared_ptr<CProperty>& other) const
    {
        if(other->city > city)
            return true;
        else if(other->city == city)
        {
            if(other->address > address)
                return true;
            else
                return false;
        }
        else
            return false;
    }

    bool operator==(const CProperty& other) const {
        return ((this->city == other.city && this->address == other.address) || (this->region == other.region && this->ID == other.ID));
    }
};

class COwner
{
public:
    string owner;
    string lowerOwner;
    vector<shared_ptr<CProperty>> possession;

    explicit COwner(const std::string &name) {
        string lowerCaseName = name;

        for (char &c : lowerCaseName) {
            c = char(tolower(c));
        }
        lowerOwner = lowerCaseName;
        owner = name;
    }

    void add(shared_ptr<CProperty> other)
    {
        possession.push_back(other);
    }

    bool lowerThan(const shared_ptr<COwner>& other) const
    {
        return other->lowerOwner < lowerOwner;
    }

    // todo: huge bottleneck
    void deleteLand(const shared_ptr<CProperty>& land)
    {
        for(uint i = 0; i < possession.size(); i++)
        {
            if(*possession[i] == *land)
            {
                possession.erase(possession.cbegin() + i);
                return;
            }
        }
    }
};


class CIterator
{
public:
    CIterator(vector<shared_ptr<CProperty>> &tmp): property(tmp)
    {
        size = tmp.size();
    }

    CIterator(uint l, vector<shared_ptr<CProperty>> &tmp): property(tmp), size(l){}

    bool atEnd() const
    {
        return position >= size;
    }

    void next()
    {position++;}

    std::string city() const
    {
        if(atEnd())
            return "";
        return property[position]->city;
    }

    std::string addr() const
    {
        if(atEnd())
            return "";
        return property[position]->address;
    }

    std::string region() const
    {
        if(atEnd())
            return "";
        return property[position]->region;
    }

    unsigned id() const
    {
        if(atEnd())
            return 0;
        return property[position]->ID;
    }

    std::string owner() const
    {
        if(atEnd())
            return "";
        return property[position]->ownerName;
    }

private:
    vector<shared_ptr<CProperty>> &property;
    uint size;
    uint position = 0;

};


class CLandRegister {
public:
    CLandRegister()
    {
        state = make_shared<COwner>("");
        landLords.push_back(state);
    }

    ~CLandRegister()
    {
        for(auto &ptr: byCity)
            ptr->owner = nullptr;
    }

    // todo(optional): check might be redundant
    bool add(const std::string &city, const std::string &addr, const std::string &region, unsigned int id)
    {
        auto land = make_shared<CProperty>(city, addr, region, id);

        auto itCity = findByCity(land);
        if(duplicateCity(land, itCity))
            return false;

        auto itRegion = findByRegion(land);
        if(duplicateRegion(land, itRegion))
            return false;

        byCity.insert(itCity, land);
        byRegion.insert(itRegion, land);

        auto stateIt = findOwner(state);
        shared_ptr<COwner> statePtr = *stateIt;
        land->owner = statePtr;
        statePtr->possession.push_back(land);
        return true;
    }

    // todo(optional): deletes are similar
    bool del(const std::string &city, const std::string &addr)
    {
        auto land = make_shared<CProperty>(city, addr, "", 0);
        auto itCity = findByCity(land);
        if(!duplicateCity(land, itCity))
            return false;

        auto ptr = *itCity;
        land->region = ptr->region;
        land->ID = ptr->ID;
        auto itRegion = findByRegion(land);

        byRegion.erase(itRegion);
        byCity.erase(itCity);

        ptr->owner->deleteLand(land);
        return true;
    }


    bool del(const std::string &region, unsigned int id)
    {
        auto land = make_shared<CProperty>("", "", region, id);
        auto itRegion = findByRegion(land);
        if(!duplicateRegion(land, itRegion))
            return false;

        auto ptr = *itRegion;
        land->city = ptr->city;
        land->address = ptr->address;
        auto itCity = findByCity(land);

        byRegion.erase(itRegion);
        byCity.erase(itCity);

        ptr->owner->deleteLand(land);
        return true;

    }

    bool getOwner(const std::string &city, const std::string &addr, std::string &owner) const
    {
        auto land = make_shared<CProperty>(city, addr, "", 0);
        auto cityIt = findByCity(land);
        if(!duplicateCity(land, cityIt))
            return false;

        const auto& tmp = *cityIt;
        owner = tmp->ownerName;
        return true;
    }

    bool getOwner(const std::string &region, unsigned int id, std::string &owner) const
    {
        auto land = make_shared<CProperty>("", "", region, id);
        auto regionIt = findByRegion(land);
        if(!duplicateRegion(land, regionIt))
            return false;

        const auto& tmp = *regionIt;
        owner = tmp->ownerName;
        return true;
    }

    bool newOwner(const std::string &city, const std::string &addr, const std::string &owner)
    {
        shared_ptr<COwner> tmpOwner = make_shared<COwner>(owner);
        shared_ptr<CProperty> land = make_shared<CProperty>(city, addr, "", 0);
        auto itOwner = findOwner(tmpOwner);
        if(!duplicateOwner(tmpOwner, itOwner))
            landLords.insert(itOwner, tmpOwner);

        itOwner = findOwner(tmpOwner);

        auto itLand = findByCity(land);
        if(!duplicateCity(land, itLand))
            return false;

        shared_ptr<CProperty> landPtr = *itLand;
        shared_ptr<COwner> ownerPtr = *itOwner;

        shared_ptr<COwner> oldOwner = landPtr->owner;
        if(oldOwner == ownerPtr)
            return false;

        landPtr->owner = ownerPtr;
        ownerPtr->add(landPtr);
        oldOwner->deleteLand(landPtr);
        landPtr->ownerName = owner;
        return true;
    }

    bool newOwner(const std::string &region, unsigned int id, const std::string &owner)
    {
        shared_ptr<CProperty> land = make_shared<CProperty>("", "", region, id);
        vector<shared_ptr<CProperty>>::iterator landIt = findByRegion(land);

        if(!duplicateRegion(land, landIt))
            return false;
        shared_ptr<CProperty> landPtr = *landIt;

        return newOwner(landPtr->city, landPtr->address, owner);
    }

    size_t count(const std::string &owner) const
    {
        auto lord = make_shared<COwner>(owner);
        auto lordIt = findOwner(lord);
        if(!duplicateOwner(lord, lordIt))
            return 0;

        const auto& lordPtr = *lordIt;
        return lordPtr->possession.size();
    }

    CIterator listByAddr() const
    {
        return CIterator(byCity);
    }

    CIterator listByOwner(const std::string &owner) const
    {
        auto lord = make_shared<COwner>(owner);
        auto ownerIt = findOwner(lord);

        //todo(optional): might be better to say that the vector is empty
        if(!duplicateOwner(lord, ownerIt))
            return CIterator(0, landLords[0]->possession);

        const auto& ownerPtr = *ownerIt;
        return CIterator(ownerPtr->possession);
    }

private:
    vector<shared_ptr<CProperty>>::iterator findByCity(const shared_ptr<CProperty>& land)
    {
        return lower_bound(byCity.begin(), byCity.end(), land, [](const shared_ptr<CProperty>& a, const shared_ptr<CProperty>& b) {
            return a->lessThanViaCityAdd(b);
        });
    }

    vector<shared_ptr<CProperty>>::const_iterator findByCity(const shared_ptr<CProperty>& land) const
    {
        return lower_bound(byCity.begin(), byCity.end(), land, [](const shared_ptr<CProperty>& a, const shared_ptr<CProperty>& b) {
            return a->lessThanViaCityAdd(b);
        });
    }

    bool duplicateCity(const shared_ptr<CProperty>& land, vector<shared_ptr<CProperty>>::const_iterator it) const
    {
        if(it >= byCity.end())
            return false;
        const shared_ptr<CProperty>& tmp = *it;
        if((tmp->region == land->region && tmp->ID == land->ID) || (tmp->address == land->address && tmp->city == land->city))
            return true;
        return false;
    }

    vector<shared_ptr<CProperty>>::iterator findByRegion(const shared_ptr<CProperty>& land)
    {
        return lower_bound(byRegion.begin(), byRegion.end(), land, [](const shared_ptr<CProperty>& a, const shared_ptr<CProperty>& b) {
            return a->lessThanViaRegId(b);
        });
    }

    vector<shared_ptr<CProperty>>::const_iterator findByRegion(const shared_ptr<CProperty>& land) const
    {
        return lower_bound(byRegion.begin(), byRegion.end(), land, [](const shared_ptr<CProperty>& a, const shared_ptr<CProperty>& b) {
            return a->lessThanViaRegId(b);
        });
    }


    bool duplicateRegion(const shared_ptr<CProperty>& land, vector<shared_ptr<CProperty>>::const_iterator it) const
    {
        if(it >= byRegion.end())
            return false;
        const shared_ptr<CProperty>& tmp = *it;
        if((tmp->region == land->region && tmp->ID == land->ID) || (tmp->address == land->address && tmp->city == land->city))
            return true;
        return false;
    }

    vector<shared_ptr<COwner>>::iterator findOwner(const shared_ptr<COwner>& owner)
    {
        return lower_bound(landLords.begin(), landLords.end(), owner, [](const shared_ptr<COwner>& a, const shared_ptr<COwner>& b) {
            return a->lowerThan(b);
        });
    }

    vector<shared_ptr<COwner>>::const_iterator findOwner(const shared_ptr<COwner>& owner) const
    {
        return lower_bound(landLords.begin(), landLords.end(), owner, [](const shared_ptr<COwner>& a, const shared_ptr<COwner>& b) {
            return a->lowerThan(b);
        });
    }

    bool duplicateOwner(const shared_ptr<COwner>& owner, vector<shared_ptr<COwner>>::const_iterator it) const
    {
        if(it >= landLords.end())
            return false;
        const auto& tmp = *it;
        return tmp->lowerOwner == owner->lowerOwner;
    }

    mutable vector<shared_ptr<CProperty>> byCity;
    vector<shared_ptr<CProperty>> byRegion;
    mutable vector<shared_ptr<COwner>> landLords;
    shared_ptr<COwner> state;
};



#ifndef __PROGTEST__
static void test0 ()
{
  CLandRegister x;
  std::string owner;

  assert ( x . add ( "Prague", "Thakurova", "Dejvice", 12345 ) );
  assert ( x . add ( "Prague", "Evropska", "Vokovice", 12345 ) );
  assert ( x . add ( "Prague", "Technicka", "Dejvice", 9873 ) );
  assert ( x . add ( "Plzen", "Evropska", "Plzen mesto", 78901 ) );
  assert ( x . add ( "Liberec", "Evropska", "Librec", 4552 ) );
  CIterator i0 = x . listByAddr ();
  assert ( ! i0 . atEnd ()
           && i0 . city () == "Liberec"
           && i0 . addr () == "Evropska"
           && i0 . region () == "Librec"
           && i0 . id () == 4552
           && i0 . owner () == "" );
  i0 . next ();
  assert ( ! i0 . atEnd ()
           && i0 . city () == "Plzen"
           && i0 . addr () == "Evropska"
           && i0 . region () == "Plzen mesto"
           && i0 . id () == 78901
           && i0 . owner () == "" );
  i0 . next ();
  assert ( ! i0 . atEnd ()
           && i0 . city () == "Prague"
           && i0 . addr () == "Evropska"
           && i0 . region () == "Vokovice"
           && i0 . id () == 12345
           && i0 . owner () == "" );
  i0 . next ();
  assert ( ! i0 . atEnd ()
           && i0 . city () == "Prague"
           && i0 . addr () == "Technicka"
           && i0 . region () == "Dejvice"
           && i0 . id () == 9873
           && i0 . owner () == "" );
  i0 . next ();
  assert ( ! i0 . atEnd ()
           && i0 . city () == "Prague"
           && i0 . addr () == "Thakurova"
           && i0 . region () == "Dejvice"
           && i0 . id () == 12345
           && i0 . owner () == "" );
  i0 . next ();
  assert ( i0 . atEnd () );

  assert ( x . count ( "" ) == 5 );
  CIterator i1 = x . listByOwner ( "" );
  assert ( ! i1 . atEnd ()
           && i1 . city () == "Prague"
           && i1 . addr () == "Thakurova"
           && i1 . region () == "Dejvice"
           && i1 . id () == 12345
           && i1 . owner () == "" );
  i1 . next ();
  assert ( ! i1 . atEnd ()
           && i1 . city () == "Prague"
           && i1 . addr () == "Evropska"
           && i1 . region () == "Vokovice"
           && i1 . id () == 12345
           && i1 . owner () == "" );
  i1 . next ();
  assert ( ! i1 . atEnd ()
           && i1 . city () == "Prague"
           && i1 . addr () == "Technicka"
           && i1 . region () == "Dejvice"
           && i1 . id () == 9873
           && i1 . owner () == "" );
  i1 . next ();
  assert ( ! i1 . atEnd ()
           && i1 . city () == "Plzen"
           && i1 . addr () == "Evropska"
           && i1 . region () == "Plzen mesto"
           && i1 . id () == 78901
           && i1 . owner () == "" );
  i1 . next ();
  assert ( ! i1 . atEnd ()
           && i1 . city () == "Liberec"
           && i1 . addr () == "Evropska"
           && i1 . region () == "Librec"
           && i1 . id () == 4552
           && i1 . owner () == "" );
  i1 . next ();
  assert ( i1 . atEnd () );

  assert ( x . count ( "CVUT" ) == 0 );
  CIterator i2 = x . listByOwner ( "CVUT" );
  assert ( i2 . atEnd () );

  assert ( x . newOwner ( "Prague", "Thakurova", "CVUT" ) );
  assert ( x . newOwner ( "Dejvice", 9873, "CVUT" ) );
  assert ( x . newOwner ( "Plzen", "Evropska", "Anton Hrabis" ) );
  assert ( x . newOwner ( "Librec", 4552, "Cvut" ) );
  assert ( x . getOwner ( "Prague", "Thakurova", owner ) && owner == "CVUT" );
  assert ( x . getOwner ( "Dejvice", 12345, owner ) && owner == "CVUT" );
  assert ( x . getOwner ( "Prague", "Evropska", owner ) && owner == "" );
  assert ( x . getOwner ( "Vokovice", 12345, owner ) && owner == "" );
  assert ( x . getOwner ( "Prague", "Technicka", owner ) && owner == "CVUT" );
  assert ( x . getOwner ( "Dejvice", 9873, owner ) && owner == "CVUT" );
  assert ( x . getOwner ( "Plzen", "Evropska", owner ) && owner == "Anton Hrabis" );
  assert ( x . getOwner ( "Plzen mesto", 78901, owner ) && owner == "Anton Hrabis" );
  assert ( x . getOwner ( "Liberec", "Evropska", owner ) && owner == "Cvut" );
  assert ( x . getOwner ( "Librec", 4552, owner ) && owner == "Cvut" );
  CIterator i3 = x . listByAddr ();
  assert ( ! i3 . atEnd ()
           && i3 . city () == "Liberec"
           && i3 . addr () == "Evropska"
           && i3 . region () == "Librec"
           && i3 . id () == 4552
           && i3 . owner () == "Cvut" );
  i3 . next ();
  assert ( ! i3 . atEnd ()
           && i3 . city () == "Plzen"
           && i3 . addr () == "Evropska"
           && i3 . region () == "Plzen mesto"
           && i3 . id () == 78901
           && i3 . owner () == "Anton Hrabis" );
  i3 . next ();
  assert ( ! i3 . atEnd ()
           && i3 . city () == "Prague"
           && i3 . addr () == "Evropska"
           && i3 . region () == "Vokovice"
           && i3 . id () == 12345
           && i3 . owner () == "" );
  i3 . next ();
  assert ( ! i3 . atEnd ()
           && i3 . city () == "Prague"
           && i3 . addr () == "Technicka"
           && i3 . region () == "Dejvice"
           && i3 . id () == 9873
           && i3 . owner () == "CVUT" );
  i3 . next ();
  assert ( ! i3 . atEnd ()
           && i3 . city () == "Prague"
           && i3 . addr () == "Thakurova"
           && i3 . region () == "Dejvice"
           && i3 . id () == 12345
           && i3 . owner () == "CVUT" );
  i3 . next ();
  assert ( i3 . atEnd () );

  assert ( x . count ( "cvut" ) == 3 );
  CIterator i4 = x . listByOwner ( "cVuT" );
  assert ( ! i4 . atEnd ()
           && i4 . city () == "Prague"
           && i4 . addr () == "Thakurova"
           && i4 . region () == "Dejvice"
           && i4 . id () == 12345
           && i4 . owner () == "CVUT" );
  i4 . next ();
  assert ( ! i4 . atEnd ()
           && i4 . city () == "Prague"
           && i4 . addr () == "Technicka"
           && i4 . region () == "Dejvice"
           && i4 . id () == 9873
           && i4 . owner () == "CVUT" );
  i4 . next ();
  assert ( ! i4 . atEnd ()
           && i4 . city () == "Liberec"
           && i4 . addr () == "Evropska"
           && i4 . region () == "Librec"
           && i4 . id () == 4552
           && i4 . owner () == "Cvut" );
  i4 . next ();
  assert ( i4 . atEnd () );

  assert ( x . newOwner ( "Plzen mesto", 78901, "CVut" ) );
  assert ( x . count ( "CVUT" ) == 4 );
  CIterator i5 = x . listByOwner ( "CVUT" );
  assert ( ! i5 . atEnd ()
           && i5 . city () == "Prague"
           && i5 . addr () == "Thakurova"
           && i5 . region () == "Dejvice"
           && i5 . id () == 12345
           && i5 . owner () == "CVUT" );
  i5 . next ();
  assert ( ! i5 . atEnd ()
           && i5 . city () == "Prague"
           && i5 . addr () == "Technicka"
           && i5 . region () == "Dejvice"
           && i5 . id () == 9873
           && i5 . owner () == "CVUT" );
  i5 . next ();
  assert ( ! i5 . atEnd ()
           && i5 . city () == "Liberec"
           && i5 . addr () == "Evropska"
           && i5 . region () == "Librec"
           && i5 . id () == 4552
           && i5 . owner () == "Cvut" );
  i5 . next ();
  assert ( ! i5 . atEnd ()
           && i5 . city () == "Plzen"
           && i5 . addr () == "Evropska"
           && i5 . region () == "Plzen mesto"
           && i5 . id () == 78901
           && i5 . owner () == "CVut" );
  i5 . next ();
  assert ( i5 . atEnd () );

  assert ( x . del ( "Liberec", "Evropska" ) );
  assert ( x . del ( "Plzen mesto", 78901 ) );
  assert ( x . count ( "cvut" ) == 2 );
  CIterator i6 = x . listByOwner ( "cVuT" );
  assert ( ! i6 . atEnd ()
           && i6 . city () == "Prague"
           && i6 . addr () == "Thakurova"
           && i6 . region () == "Dejvice"
           && i6 . id () == 12345
           && i6 . owner () == "CVUT" );
  i6 . next ();
  assert ( ! i6 . atEnd ()
           && i6 . city () == "Prague"
           && i6 . addr () == "Technicka"
           && i6 . region () == "Dejvice"
           && i6 . id () == 9873
           && i6 . owner () == "CVUT" );
  i6 . next ();
  assert ( i6 . atEnd () );

  assert ( x . add ( "Liberec", "Evropska", "Librec", 4552 ) );
    cout << "TEST 0 DONE" << endl;
}

static void test1 ()
{
  CLandRegister x;
  std::string owner;

  assert ( x . add ( "Prague", "Thakurova", "Dejvice", 12345 ) );
  assert ( x . add ( "Prague", "Evropska", "Vokovice", 12345 ) );
  assert ( x . add ( "Prague", "Technicka", "Dejvice", 9873 ) );
  assert ( ! x . add ( "Prague", "Technicka", "Hradcany", 7344 ) );
  assert ( ! x . add ( "Brno", "Bozetechova", "Dejvice", 9873 ) );
  assert ( !x . getOwner ( "Prague", "THAKUROVA", owner ) );
  assert ( !x . getOwner ( "Hradcany", 7343, owner ) );
  CIterator i0 = x . listByAddr ();
  assert ( ! i0 . atEnd ()
           && i0 . city () == "Prague"
           && i0 . addr () == "Evropska"
           && i0 . region () == "Vokovice"
           && i0 . id () == 12345
           && i0 . owner () == "" );
  i0 . next ();
  assert ( ! i0 . atEnd ()
           && i0 . city () == "Prague"
           && i0 . addr () == "Technicka"
           && i0 . region () == "Dejvice"
           && i0 . id () == 9873
           && i0 . owner () == "" );
  i0 . next ();
  assert ( ! i0 . atEnd ()
           && i0 . city () == "Prague"
           && i0 . addr () == "Thakurova"
           && i0 . region () == "Dejvice"
           && i0 . id () == 12345
           && i0 . owner () == "" );
  i0 . next ();
  assert ( i0 . atEnd () );

    assert ( x . newOwner ( "Prague", "Thakurova", "CVUT" ) );
    assert ( ! x . newOwner ( "Prague", "technicka", "CVUT" ) );
    assert ( ! x . newOwner ( "prague", "Technicka", "CVUT" ) );

    assert ( ! x . newOwner ( "dejvice", 9873, "CVUT" ) );

    assert ( ! x . newOwner ( "Dejvice", 9973, "CVUT" ) );
    assert ( ! x . newOwner ( "Dejvice", 12345, "CVUT" ) );
    assert ( x . count ( "CVUT" ) == 1 );
    CIterator i1 = x . listByOwner ( "CVUT" );
  assert ( ! i1 . atEnd ()
           && i1 . city () == "Prague"
           && i1 . addr () == "Thakurova"
           && i1 . region () == "Dejvice"
           && i1 . id () == 12345
           && i1 . owner () == "CVUT" );
  i1 . next ();
  assert ( i1 . atEnd () );

  assert ( ! x . del ( "Brno", "Technicka" ) );
  assert ( ! x . del ( "Karlin", 9873 ) );
  assert ( x . del ( "Prague", "Technicka" ) );
  assert ( ! x . del ( "Prague", "Technicka" ) );
  assert ( ! x . del ( "Dejvice", 9873 ) );
    cout << "TEST 1 DONE" << endl;
}

void myUnitTest()
{
    vector<shared_ptr<COwner>> dudes;
    dudes.push_back(make_shared<COwner>("Marek Bulant"));
    dudes.push_back(make_shared<COwner>("MarEk BuLaNR"));


}

int main ( void )
{
//    myUnitTest();
    test0 ();
    test1 ();

    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
